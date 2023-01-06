#include <ilcp/cp.h>
#include <string>
#include <vector>
#include <fstream>
#include <numeric>
#include <algorithm>

#include "../includes/Physician.h"
#include "../includes/Shift.h"
#include "../includes/Area.h"
#include "../includes/Solution.h"
#include "../includes/RosteringInput.h"

typedef IloArray <IloIntVarArray> IloIntVarArray2;
typedef IloArray <IloIntVarArray2> IloIntVarArray3;
typedef IloArray <IloIntVarArray3> IloIntVarArray4;

typedef IloArray <IloIntExprArray> IloIntExprArray2;


class Physician;

using namespace std;

Solution rostering(RosteringInput input){
  IloEnv env;
  try {
	IloModel model(env);

	vector<Physician> physicians = input.physicians;
	vector<Shift> shifts = input.shifts;
	vector<Area> areas = input.areas;

	//minHours é a margem inferior para a quantidade de horas trabalhadas de um médico
	//se um médico tem um contrato de 40 horas semanais e o minHours é 10, este médico será alocado para, no mínimo, 30 horas (40 - 10)
	//o mesmo vale para o maxHours, mas para o limite superior
	int maxHoursMargin = input.maxHoursMargin, minHoursMargin = input.minHoursMargin,
		maxNightShifts = input.maxNightShifts, weeks = ceil(input.days.size() / 7.0f), days = input.days.size();

	IloIntVarArray4 assignment(env);
	//Assignment[d][s][a][t] representa o médico que preencherá a vaga t da área a no
	//turno s do dia d
	for(int d = 0; d < days; d++){
		assignment.add(IloIntVarArray3(env));
		for(int s = 0; s < (int)shifts.size(); s++){
			assignment[d].add(IloIntVarArray2(env));
			for(int a = 0; a < (int)areas.size(); a++){
				assignment[d][s].add(IloIntVarArray(env, areas[a].spots[s], 0, physicians.size() - 1));
			}
		}
	}

	//Garante que um médico só seja alocado para um turno em cada período
	for(int d = 0; d < days; d++){
		for(int s = 0; s < (int)shifts.size(); s++){
			IloIntVarArray working(env);
			for(int a = 0; a < (int)areas.size(); a++){
				for(int v = 0; v < (int)areas[a].spots[s]; v++){
					working.add(assignment[d][s][a][v]);
				}
			}
			model.add(IloAllDiff(env, working));
		}
	}

	// Impede que um médico seja escalado para turnos e dias que ele não está disponível
	for(int p = 0; p < (int)physicians.size(); p++){
		for(int d = 0; d < days; d++){
			for(int s = 0; s < (int)shifts.size(); s++){
				bool restricted = !physicians[p].possiblePeriod[d].shifts[s];
				if(restricted){
					for(int a = 0; a < (int)areas.size(); a++){
						for(int v = 0; v < areas[a].spots[s]; v++){
							model.add(assignment[d][s][a][v] != p);
						}
					}
				}
			}
		}
	}

	// TODO Ao invés de considerar semanas, considerar todos os conjuntos de 7 dias
	// Tentar com função objetivo
	// Garante que o médico irá trabalhar um número mínimo e um máximo de horas em
	// qualquer conjunto de 7 dias consecutivos
	for(int p = 0; p < (int)physicians.size(); p++){
		for(int d = 0; d <= days - 7; d++){
			IloIntExpr periodWorkedHours(env);
			for(int w = d; w < d + 7; w++){
				for(int s = 0; s < (int)shifts.size(); s++){
					for(int a = 0; a < (int)areas.size(); a++){
						for(int v = 0; v < areas[a].spots[s]; v++){
							periodWorkedHours += (assignment[w][s][a][v] == p) * shifts[s].hours;
						}
					}
				}
			}
			model.add(periodWorkedHours >= physicians[p].hours - minHoursMargin && periodWorkedHours <= physicians[p].hours + maxHoursMargin);
		}
	}


	//Garante que caso um médico trabalhe no turno noturno, ele não irá trabalhar no
	//turno da manha e da tarde no dia seguinte
	for(int p = 0; p < (int)physicians.size(); p++){
		for(int w = 0; w < weeks; w++){
			for(int d = w*7; d < (w+1)*7 && d < days; d++){
				IloIntExpr workedNightShift(env);
				for(int a = 0; a < (int)areas.size(); a++){
					for(int v = 0; v < areas[a].spots[shifts.size()-1]; v++){
						workedNightShift += assignment[d][shifts.size() - 1][a][v] == p;
					}
				}
				if(d != (weeks * 7)-1 && d < (days - 1)){
					for(int shiftsWontWork = 0; shiftsWontWork < 2; shiftsWontWork++)
						for(int areasWontWork = 0; areasWontWork < (int)areas.size(); areasWontWork++)
							for(int spotWontWork = 0; spotWontWork < areas[areasWontWork].spots[shiftsWontWork]; spotWontWork++)
								model.add(!(workedNightShift >= 1) || (assignment[d+1][shiftsWontWork][areasWontWork][spotWontWork] != p));
				}
			}
		}
	}

	//Garante que um médico trabalhe no máximo um certo numero de turnos noturnos por semana
	for(int p = 0; p < (int)physicians.size(); p++){
		for(int w = 0; w < weeks; w++){
			IloIntExpr workedNightShifts(env);
			for(int d = w*7; d < (w+1)*7 && d < days; d++){
				for(int s = 0; s < (int)shifts.size(); s++){
					for(int a = 0; a < (int)areas.size(); a++){
						for(int v = 0; v < areas[a].spots[s]; v++){
							if(s == (int)shifts.size() - 1){
								workedNightShifts += assignment[d][s][a][v] == p;
							}
						}
					}
				}
			}
			model.add(workedNightShifts <= maxNightShifts);
		}
	}

	// Restrição para quebra de simetria
	for(int d = 0; d < days; d++){
		for(int s = 0; s < (int)shifts.size(); s++){
			for(int a = 0; a < (int)areas.size(); a++){
				for(int v = 0; v < (int)areas[a].spots[s] - 1; v++){
					// Ao impormos esta restrição, eliminamos soluções iguais que a
					// penas permutariam os médicos entre as vagas
					model.add(assignment[d][s][a][v] < assignment[d][s][a][v+1]);
				}
			}
		}
	}

	// Restrição que penaliza a inconsistência no escalonamento.
	// Isto é, se um médico é escalonado para um determinado dia
	// e turno em uma semana, mas não trabalha nesse mesmo período
	// ao longo das outras semanas, a função objetivo é penalizada
	IloNumExpr inconsistency(env);
	IloNumExpr normalizedInconsistency(env);
	IloNumExpr realInconsistency(env);
	IloNumExprArray physicianInconsistencyArray(env);
	for(int p = 0; p < (int)physicians.size(); p++){
		IloNumExpr physicianInconsistency(env);

		// fixa o dia e o turno e varia a semana
		for(int weekDay = 0; weekDay < 7; weekDay++){
			for(int s = 0; s < (int)shifts.size(); s++){

				// Cria um array com os turnos trabalhados no turno s do dia weekDay, ao longo das semanas
				IloIntExprArray shiftsByDayAlongWeeks(env);
				for(int w = 0; w < weeks && (weekDay + w*7) < days; w++){
					for(int a = 0; a < (int)areas.size(); a++){
						for(int v = 0; v < areas[a].spots[s]; v++){
							shiftsByDayAlongWeeks.add(assignment[weekDay + w*7][s][a][v] == p);
						}
					}
				}
				IloIntExpr sumShiftsByDayAlongWeeks = IloSum(shiftsByDayAlongWeeks);
				physicianInconsistency += (sumShiftsByDayAlongWeeks > 0) * (weeks - sumShiftsByDayAlongWeeks);

			}
		}
		inconsistency += physicianInconsistency * physicianInconsistency;
		realInconsistency +=  physicianInconsistency;
		physicianInconsistencyArray.add(physicianInconsistency);
	}
	normalizedInconsistency = (inconsistency - input.idealConstraintsValues[0]) /
			(input.nadirConstraintsValues[0] - input.idealConstraintsValues[0]);

	//Restrição que penaliza as cargas de trabalho diárias que ultrapassem 12 horas
	IloNumExpr overtime(env);
	IloNumExpr normalizedOvertime;
	IloNumExpr realOvertime(env);
	IloNumExprArray physicianOvertimeArray(env);
	for(int p = 0; p < (int)physicians.size(); p++){
		IloNumExpr physicianOvertime(env);
		for(int w = 0; w < weeks; w++){
			for(int d = w*7; d < (w+1)*7 && d < days; d++){
				// Variável quer armazena as horas trabalhadas por um médico no dia d
				IloIntExpr workedHours(env);
				for(int s = 0; s < (int)shifts.size(); s++){
					for(int a = 0; a < (int)areas.size(); a++){
						for(int v = 0; v < areas[a].spots[s]; v++){
							workedHours += (assignment[d][s][a][v] == p) * shifts[s].hours;
						}
					}
				}
				/*
				 * Caso as horas trabalhadas não excedam 12 horas, a
				 * expressão workedHours > 12 será 0, fazendo com que
				 * toda a espressão dê 0.
				 *
				 * Caso as horas trabalhadas excedam 12 horas,
				 * workedHours > 12 será 1, e workedHours - 12 (horas
				 * excedidas) será acidicionado ao overtime do médico
				*/
				physicianOvertime += (workedHours > 12) * (workedHours - 12);
			}
		}
		overtime += physicianOvertime * physicianOvertime;
		realOvertime += physicianOvertime;
		physicianOvertimeArray.add(physicianOvertime);
	}
	normalizedOvertime = input.normalization ? (overtime - input.idealConstraintsValues[1]) /
			(input.nadirConstraintsValues[1] - input.idealConstraintsValues[1]) : overtime;

	// Restrição que penaliza as horas de trabalho mensais trabalhadas a menos ou a mais
	IloNumExpr workDeviation(env);
	IloNumExpr normalizedWorkDeviation;
	IloNumExpr realWorkDeviation(env);
	IloNumExprArray physicianDeviationArray(env);
	for(int p = 0; p < (int)physicians.size(); p++){
		IloNumExpr physicianWorkDeviation(env);
		IloIntExpr workedHours(env);
		for(int w = 0; w < weeks; w++){
			for(int d = w*7; d < (w+1)*7 && d < days; d++){
				// Variável quer armazena as horas trabalhadas por um médico
				for(int s = 0; s < (int)shifts.size(); s++){
					for(int a = 0; a < (int)areas.size(); a++){
						for(int v = 0; v < areas[a].spots[s]; v++){
							workedHours += (assignment[d][s][a][v] == p) * shifts[s].hours;
						}
					}
				}
			}
		}
		IloNumExpr deviation(env);
		deviation = (
						(weeks * physicians[p].hours) -
						 physicians[p].hoursBank +
						 physicians[p].dueHours
					)
					- workedHours;

		physicianWorkDeviation += IloAbs(deviation);

		workDeviation += physicianWorkDeviation * physicianWorkDeviation;
		realWorkDeviation += physicianWorkDeviation;
		physicianDeviationArray.add(physicianWorkDeviation);
	}
	normalizedWorkDeviation = input.normalization ? (workDeviation - input.idealConstraintsValues[2]) /
				(input.nadirConstraintsValues[2] - input.idealConstraintsValues[2]) : workDeviation;

	// Restrição que penaliza desvios na quantidade de horas em conjuntos de 7 dias
	IloNumExpr workDistribution(env);
	IloNumExpr normalizedWorkDistribution(env);
	IloNumExpr realWorkDistribution(env);
	IloNumExprArray physicianDistributionArray(env);
	for(int p = 0; p < (int)physicians.size(); p++){
		IloNumExpr physicianWorkDistribution(env);
		for(int d = 0; d <= days - 7; d++){
			IloIntExpr periodWorkedHours(env);
			for(int w = d; w < d + 7; w++){
				for(int s = 0; s < (int)shifts.size(); s++){
					for(int a = 0; a < (int)areas.size(); a++){
						for(int v = 0; v < areas[a].spots[s]; v++){
							periodWorkedHours += (assignment[w][s][a][v] == p) * shifts[s].hours;
						}
					}
				}
			}
			physicianWorkDistribution += IloAbs(periodWorkedHours - physicians[p].hours);
		}
		workDistribution += physicianWorkDistribution * physicianWorkDistribution;
		realWorkDistribution += physicianWorkDistribution;
		physicianDistributionArray.add(physicianWorkDistribution);
	}
	normalizedWorkDistribution = input.normalization ? (workDistribution - input.idealConstraintsValues[3]) /
					(input.nadirConstraintsValues[3] - input.idealConstraintsValues[3]) : workDistribution;

	if(input.verificationOn){
		IloAnd softConstraintLimitExpression(env);
		softConstraintLimitExpression.add((inconsistency <= input.solutions[0].softConstraints[0].value) &&
										  (overtime <= input.solutions[0].softConstraints[1].value) &&
										  (workDeviation <= input.solutions[0].softConstraints[2].value) &&
										  (workDistribution <= input.solutions[0].softConstraints[3].value));
		model.add(softConstraintLimitExpression);
	}

	if(input.solutions.size() > 0){
		IloAnd softConstraintLimitExpression(env);
		for(int i = 0; i < (int) input.solutions.size(); i++){
			softConstraintLimitExpression.add((inconsistency < input.solutions[i].softConstraints[0].value) ||
											  (overtime < input.solutions[i].softConstraints[1].value) ||
											  (workDeviation < input.solutions[i].softConstraints[2].value) ||
											  (workDistribution < input.solutions[i].softConstraints[3].value));
		}
		model.add(softConstraintLimitExpression);
    }

	if(input.optimize){
		IloObjective obj(env);
		if(input.normalization){
			obj = IloMinimize(env,  (input.weights[0] * normalizedInconsistency) +
									(input.weights[1] * normalizedOvertime) +
									(input.weights[2] * normalizedWorkDeviation) +
									(input.weights[3] * normalizedWorkDistribution));
		} else{
			obj = IloMinimize(env,  (input.weights[0] * inconsistency) +
									(input.weights[1] * overtime) +
									(input.weights[2] * workDeviation) +
									(input.weights[3] * workDistribution));
		}
		model.add(obj);
	}


    IloCP cp(model);
    cp.setParameter(IloCP::TimeLimit, input.timePerSolution);
//    cp.setParameter(IloCP::LogVerbosity, IloCP::Quiet);

    if (cp.solve()){
    	Solution newSolution;

    	vector<double> auxPhysicianInconsistencyArray;
    	vector<double> auxPhysicianOvertimeArray;
		vector<double> auxPhysicianDeviationArray;
		vector<double> auxPhysicianDistributionArray;
    	for(int i = 0; i < physicians.size(); i++){
    		auxPhysicianInconsistencyArray.push_back(cp.getValue(physicianInconsistencyArray[i]));
    		auxPhysicianOvertimeArray.push_back(cp.getValue(physicianOvertimeArray[i]));
    		auxPhysicianDeviationArray.push_back(cp.getValue(physicianDeviationArray[i]));
    		auxPhysicianDistributionArray.push_back(cp.getValue(physicianDistributionArray[i]));

    	}

    	newSolution.softConstraints.push_back(SoftConstraint(
    			"Inconsistent assignment over weeks",
    			cp.getValue(inconsistency),
				cp.getValue(realInconsistency),
				*min_element(auxPhysicianInconsistencyArray.begin(), auxPhysicianInconsistencyArray.end()),
				*max_element(auxPhysicianInconsistencyArray.begin(), auxPhysicianInconsistencyArray.end()),
				std::accumulate(auxPhysicianInconsistencyArray.begin(), auxPhysicianInconsistencyArray.end(), 0.0)/auxPhysicianInconsistencyArray.size()
		));

		newSolution.softConstraints.push_back(SoftConstraint(
				"Daily workload over recomendation",
				cp.getValue(overtime),
				cp.getValue(realOvertime),
				*min_element(auxPhysicianOvertimeArray.begin(), auxPhysicianOvertimeArray.end()),
				*max_element(auxPhysicianOvertimeArray.begin(), auxPhysicianOvertimeArray.end()),
				std::accumulate(auxPhysicianOvertimeArray.begin(), auxPhysicianOvertimeArray.end(), 0.0)/auxPhysicianOvertimeArray.size()
		));

		newSolution.softConstraints.push_back(SoftConstraint(
				"Monthly workload deviation",
				cp.getValue(workDeviation),
				cp.getValue(realWorkDeviation),
				*min_element(auxPhysicianDeviationArray.begin(), auxPhysicianDeviationArray.end()),
				*max_element(auxPhysicianDeviationArray.begin(), auxPhysicianDeviationArray.end()),
				std::accumulate(auxPhysicianDeviationArray.begin(), auxPhysicianDeviationArray.end(), 0.0)/auxPhysicianDeviationArray.size()
		));

		newSolution.softConstraints.push_back(SoftConstraint(
				"Weekly workload deviation",
				cp.getValue(workDistribution),
				cp.getValue(realWorkDistribution),
				*min_element(auxPhysicianDistributionArray.begin(), auxPhysicianDistributionArray.end()),
				*max_element(auxPhysicianDistributionArray.begin(), auxPhysicianDistributionArray.end()),
				std::accumulate(auxPhysicianDistributionArray.begin(), auxPhysicianDistributionArray.end(), 0.0)/auxPhysicianDistributionArray.size()
		));


    	vector<vector<vector<vector<int>>>> schedule;
    	for(int w = 0; w < weeks; w++){
			for(int d = w*7; d < (w+1)*7 && d < days; d++){
				vector<vector<vector<int>>> dVector;
				schedule.push_back(dVector);
				for(int s = 0; s < (int)shifts.size(); s++){
				  vector<vector<int>> sVector;
				  schedule[d].push_back(sVector);
				  for(int a = 0; a < (int)areas.size(); a++){
					  vector<int> aVector;
					  schedule[d][s].push_back(aVector);
					  for(int v = 0; v < areas[a].spots[s]; v++){
						  int vVector = cp.getValue(assignment[d][s][a][v]);
						  schedule[d][s][a].push_back(vVector);
					  }
				  }
			  }
			}
    	}
    	newSolution.schedule = schedule;
    	return newSolution;
    }
  }
  catch (IloException& ex) {
    env.out() << "Error: " << ex << std::endl;
  }
  env.end();
  Solution notFound;
  return notFound;
}
