#include <ilcp/cp.h>
#include <string>
#include <vector>
#include <fstream>

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
	//se um médico tem um contrato de 40 horas e o minHours é 10, este médico será alocado para, no mínimo, 30 horas (40 - 10)
	//o mesmo vale para o maxHours, mas para o limite superior
	int maxHoursMargin = input.maxHoursMargin, minHoursMargin = input.minHoursMargin,
		maxNightShifts = input.maxNightShifts, weeks = input.weeks, days = input.days;

	string weekdaysName[7] = {"Mon", "Tue", "Wed", "Thur",
								"Fri", "Sat", "Sun"};

	IloIntVarArray4 assignment(env);
	//Assignment[d][s][a][t] representa o médico que preencherá a vaga t da área a no
	//turno s do dia d
	for(int d = 0; d < days; d++){
		assignment.add(IloIntVarArray3(env));
		for(int s = 0; s < (int)shifts.size(); s++){
			assignment[d].add(IloIntVarArray2(env));
			for(int a = 0; a < (int)areas.size(); a++){
				assignment[d][s].add(IloIntVarArray(env, areas[a].spots, 0, physicians.size() - 1));
			}
		}
	}

	//Garante que um médico só seja alocado para um turno em cada período
	for(int d = 0; d < days; d++){
		for(int s = 0; s < (int)shifts.size(); s++){
			IloIntVarArray working(env);
			for(int a = 0; a < (int)areas.size(); a++){
				for(int v = 0; v < (int)areas[a].spots; v++){
					working.add(assignment[d][s][a][v]);
				}
			}
			model.add(IloAllDiff(env, working));
		}
	}

	//Garante que o médico irá trabalhar um número mínimo e um máximo de horas por semana
	for(int p = 0; p < (int)physicians.size(); p++){
		for(int w = 0; w < weeks; w++){
			IloIntExpr workedHours(env);
			for(int d = w*7; d < (w+1)*7; d++){
				for(int s = 0; s < (int)shifts.size(); s++){
					for(int a = 0; a < (int)areas.size(); a++){
						for(int v = 0; v < areas[a].spots; v++){
							//Caso um médico trabalhe em determinado turno, a quantidade de horas desse turno é somada
							workedHours += (assignment[d][s][a][v] == p) * shifts[s].hours;
						}
					}
				}
			}
			model.add(workedHours >= physicians[p].hours - minHoursMargin && workedHours <= physicians[p].hours + maxHoursMargin);
		}
	}


	//Garante que caso um médico trabalhe no turno noturno, ele não irá trabalhar no
	//turno da manha e da tarde no dia seguinte
	for(int p = 0; p < (int)physicians.size(); p++){
		for(int w = 0; w < weeks; w++){
			for(int d = w*7; d < (w+1)*7; d++){
				IloIntExpr workedNightShift(env);
				for(int a = 0; a < (int)areas.size(); a++){
					for(int v = 0; v < areas[a].spots; v++){
						workedNightShift += assignment[d][shifts.size() - 1][a][v] == p;
					}
				}
				if(d != (weeks * 7)-1){
					for(int shiftsWontWork = 0; shiftsWontWork < 2; shiftsWontWork++)
						for(int areasWontWork = 0; areasWontWork < (int)areas.size(); areasWontWork++)
							for(int spotWontWork = 0; spotWontWork < areas[areasWontWork].spots; spotWontWork++)
								model.add(!(workedNightShift >= 1) || (assignment[d+1][shiftsWontWork][areasWontWork][spotWontWork] != p));
				}
			}
		}
	}

	//Garante que um médico trabalhe no máximo um certo numero de turnos noturnos por semana
	for(int p = 0; p < (int)physicians.size(); p++){
		for(int w = 0; w < weeks; w++){
			IloIntExpr workedNightShifts(env);
			for(int d = w*7; d < (w+1)*7; d++){
				for(int s = 0; s < (int)shifts.size(); s++){
					for(int a = 0; a < (int)areas.size(); a++){
						for(int v = 0; v < areas[a].spots; v++){
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

	//Garante que, caso um médico trabalhe um fim de semana completo (sábado e domingo),
	//ele não trabalhará no próximo fim de semana
	for(int p = 0; p < (int)physicians.size(); p++){
		for(int w = 0; w < weeks; w++){
			IloIntExpr workedSat(env);
			IloIntExpr workedSun(env);
			for(int d = w*7; d < (w+1)*7; d++){
				for(int s = 0; s < (int)shifts.size(); s++){
					for(int a = 0; a < (int)areas.size(); a++){
						for(int v = 0; v < areas[a].spots; v++){
							if(d%7 == 5)
								workedSat += assignment[d][s][a][v] == p;
							if(d%7 == 6)
								workedSun += assignment[d][s][a][v] == p;
						}
					}
				}
			}

			IloIntExpr weekendShifts(env);
			if(w < weeks - 1){
				for(int d = (w+2)*7 - 2; d < (w+2)*7; d++){
					for(int s = 0; s < (int)shifts.size(); s++){
						for(int a = 0; a < (int)areas.size(); a++){
							for(int v = 0; v < areas[a].spots; v++){
								weekendShifts += assignment[d][s][a][v] == p;
							}
						}
					}
				}
			}

			model.add(!((workedSat >= 1) && (workedSun >= 1)) || (weekendShifts == 0));
		}
	}


	// Restrição para quebra de simetria
	for(int d = 0; d < days; d++){
		for(int s = 0; s < (int)shifts.size(); s++){
			for(int a = 0; a < (int)areas.size(); a++){
				for(int v = 0; v < (int)areas[a].spots - 1; v++){
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
	IloNumExpr realInconsistency(env);
	for(int p = 0; p < (int)physicians.size(); p++){
		IloNumExpr physicianInconsistency(env);

		// fixa o dia e o turno e varia a semana
		for(int weekDay = 0; weekDay < 7; weekDay++){
			for(int s = 0; s < (int)shifts.size(); s++){

				// Cria um array com os turnos trabalhados no turno s do dia weekDay, ao longo das semanas
				IloIntExprArray shiftsByDayAlongWeeks(env);
				for(int w = 0; w < weeks; w++){
					for(int a = 0; a < (int)areas.size(); a++){
						for(int v = 0; v < areas[a].spots; v++){
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
	}
	// Normalização
	IloNumExpr normalizedInconsistency = (inconsistency - input.idealConstraintsValues[0]) /
			(input.nadirConstraintsValues[0] - input.idealConstraintsValues[0]);

	//Restrição que penaliza o cargas de trabalho diárias que ultrapassem 12 horas
	IloNumExpr overtime(env);
	IloNumExpr realOvertime(env);
	for(int p = 0; p < (int)physicians.size(); p++){
		IloNumExpr physicianOvertime(env);
		for(int w = 0; w < weeks; w++){
			for(int d = w*7; d < (w+1)*7; d++){
				// Variável quer armazena as horas trabalhadas por um médico no dia d
				IloIntExpr workedHours(env);
				for(int s = 0; s < (int)shifts.size(); s++){
					for(int a = 0; a < (int)areas.size(); a++){
						for(int v = 0; v < areas[a].spots; v++){
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
	}
	// Normalização
	IloNumExpr normalizedOvertime = (overtime - input.idealConstraintsValues[1]) /
			(input.nadirConstraintsValues[1] - input.idealConstraintsValues[1]);

    IloObjective obj = IloMinimize(env,  (input.weights[0] * (input.normalization ? normalizedInconsistency : inconsistency)) + (input.weights[1] * (input.normalization ? normalizedOvertime : overtime)));
	model.add(obj);

	if(input.verificationOn){
		IloAnd softConstraintLimitExpression(env);
		softConstraintLimitExpression.add((inconsistency <= input.solutions[0].softConstraints[0].value) &&
										  (overtime <= input.solutions[0].softConstraints[1].value));
		model.add(softConstraintLimitExpression);
	}

	if(input.solutions.size() > 0){
		IloAnd softConstraintLimitExpression(env);
		for(int i = 0; i < (int) input.solutions.size(); i++){
			softConstraintLimitExpression.add((inconsistency < input.solutions[i].softConstraints[0].value) ||
											  (overtime < input.solutions[i].softConstraints[1].value));
		}
		model.add(softConstraintLimitExpression);
    }

    IloCP cp(model);
    cp.setParameter(IloCP::TimeLimit, input.timePerSolution);
//    cp.setParameter(IloCP::LogVerbosity, IloCP::Quiet);

    if (cp.solve()){
    	Solution newSolution;
    	newSolution.softConstraints.push_back(SoftConstraint("Inconsistency", cp.getValue(inconsistency)));
    	newSolution.softConstraints.push_back(SoftConstraint("Overtime", cp.getValue(overtime)));

    	newSolution.realSoftConstraints.push_back(SoftConstraint("Inconsistency", cp.getValue(realInconsistency)));
    	newSolution.realSoftConstraints.push_back(SoftConstraint("Overtime", cp.getValue(realOvertime)));

    	vector<vector<vector<vector<int>>>> schedule;
    	for(int w = 0; w < weeks; w++){
			for(int d = w*7; d < (w+1)*7; d++){
				vector<vector<vector<int>>> dVector;
				schedule.push_back(dVector);
				for(int s = 0; s < (int)shifts.size(); s++){
				  vector<vector<int>> sVector;
				  schedule[d].push_back(sVector);
				  for(int a = 0; a < (int)areas.size(); a++){
					  vector<int> aVector;
					  schedule[d][s].push_back(aVector);
					  for(int v = 0; v < areas[a].spots; v++){
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
