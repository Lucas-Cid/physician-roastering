#include <ilcp/cp.h>
#include <string>
#include <vector>
#include <fstream>

#include "../includes/Physician.h"
#include "../includes/Shift.h"
#include "../includes/Area.h"
typedef IloArray <IloIntVarArray> IloIntVarArray2;
typedef IloArray <IloIntVarArray2> IloIntVarArray3;
typedef IloArray <IloIntVarArray3> IloIntVarArray4;

typedef IloArray <IloIntExprArray> IloIntExprArray2;


class Physician;

using namespace std;

vector<vector<string>> readCSV(char fileName[]){
	string base = "src/";
	string path = fileName;
    ifstream file(base+path);
	string line;
	vector<vector<string>> csv;

	if(!file.good()){
		cout << "Arquivo " << path << " não encontrado" << endl;
		return csv;
	}

	//Ignora a primeira linha, a qual contém os rótulos das colunas do csv
	getline(file, line);

	//Lê uma linha por vez
	while(getline(file, line)){

		istringstream s(line);
		string cell;
		vector<string> fields;

		//Lê cada célula de uma linha e coloca em um vetor
		while (getline(s, cell,',')){
			fields.push_back(cell);
		}
		csv.push_back(fields);
	}
	return csv;
}

void readPhysiciansData(vector<Physician> *physicians, char fileName[]){
	vector<vector<string>> csv = readCSV(fileName);

	for(int i = 0; i < (int)csv.size(); i++){
		physicians->push_back(Physician(csv[i][0], stoi(csv[i][1]), stoi(csv[i][2]), csv[i][3], stoi(csv[i][4])));
	}
}

void readConfigData(int *maxHoursMargin, int *minHoursMargin, int *maxNightShifts, int *weeks, int *days, char fileName[]){
	vector<vector<string>> csv = readCSV(fileName);

	for(int i = 0; i < (int)csv.size(); i++){
		*minHoursMargin = stoi(csv[i][0]);
		*maxHoursMargin = stoi(csv[i][1]);
		*maxNightShifts = stoi(csv[i][2]);
		*weeks = stoi(csv[i][3]);
		*days = *weeks * 7;
	}
}

void readShiftsData(vector<Shift> *shifts, char fileName[]){
	vector<vector<string>> csv = readCSV(fileName);

	for(int i = 0; i < (int)csv.size(); i++){
		shifts->push_back(Shift(csv[i][0], stoi(csv[i][1])));
	}
}

void readAreasData(vector<Area> *areas, char fileName[]){
	vector<vector<string>> csv = readCSV(fileName);

	for(int i = 0; i < (int)csv.size(); i++){
		areas->push_back(Area(csv[i][0], stoi(csv[i][1])));
	}

}


int main(int , char *argv[]){
  IloEnv env;
  try {
	IloModel model(env);

	vector<Physician> physicians;
	vector<Shift> shifts;
	vector<Area> areas;

	//minHours é a margem inferior para a quantidade de horas trabalhadas de um médico
	//se um médico tem um contrato de 40 horas e o minHours é 10, este médico será alocano para, no mínimo, 30 hoas (40 - 10)
	//o mesmo vale para o maxHours, mas para o limite superior
	int maxHoursMargin, minHoursMargin, maxNightShifts, weeks, days;

	//Lê o arquivo com as características dos médicos
	readPhysiciansData(&physicians, argv[1]);

	//Lê o arquivo de configuração
	readConfigData(&maxHoursMargin, &minHoursMargin, &maxNightShifts, &weeks, &days, argv[2]);

	//Lê o arquivo com as características dos turnos
	readShiftsData(&shifts, argv[3]);

	//Lê o arquivo com as características das áreas do hospital
	readAreasData(&areas, argv[4]);

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
	//TODO ler documentação sobre os resources para utilizar nessa restrição (considerando os médicos como resources)
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


//	// Restrição que penaliza a inconsistência no escalonamento.
//	// Isto é, se um médico é escalonado para um determinado dia
//	// e turno em uma semana, mas não é escalonado para este mesmo
//	// dia e turno na próxima semana, a função objetivo é penalizada
//	IloIntExpr inconsistency(env);
//	for(int p = 0; p < (int)physicians.size(); p++){
//		IloIntExpr physicianInconsistency(env);
//		for(int w = 0; w < weeks - 1; w++){
//			for(int d = w*7; d < (w+1)*7; d++){
//				for(int s = 0; s < (int)shifts.size(); s++){
//					IloIntExpr workedShiftOnDay(env);
//					IloIntExpr workedShiftOnDayNextWeek(env);
//					for(int a = 0; a < (int)areas.size(); a++){
//						for(int v = 0; v < areas[a].spots; v++){
//							workedShiftOnDay += assignment[d][s][a][v] == p;
//							workedShiftOnDayNextWeek = assignment[d + 7][s][a][v] == p;
//						}
//					}
//					physicianInconsistency += (workedShiftOnDay >= 1 && workedShiftOnDayNextWeek < 1) == 1;
//				}
//			}
//		}
//		inconsistency += physicianInconsistency * physicianInconsistency;
//	}


	// Restrição que penaliza a inconsistência no escalonamento.
	// Isto é, se um médico é escalonado para um determinado dia
	// e turno em uma semana, mas não trabalha nesse mesmo período
	// ao longo das outras semanas, a função objetivo é penalizada
	IloIntExpr inconsistency(env);
	for(int p = 0; p < (int)physicians.size(); p++){
		IloIntExpr physicianInconsistency(env);

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
	}

	//Restrição que penaliza o cargas de trabalho diárias que ultrapassem 12 horas
	IloIntExpr overtime(env);
	for(int p = 0; p < (int)physicians.size(); p++){
		IloIntExpr physicianOvertime(env);
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
	}

	//TODO pesquisar como adicionar restrição durante a execução
	//TODO utilizar algoritimo de geração de pesos para as restrições flexiveis
	//TODO retirar minimização e mandar o solver devolver todas as soluções. Descartar as dominadas
	//TODO plotar gráfico com as soluções
	//TODO tentar obter um conjunto de soluções sem retirar a função objetivo

//    IloObjective obj = IloMinimize(env,  (2 * inconsistency) + overtime);
//	  IloObjective obj = IloMinimize(env,  inconsistency + (2 * overtime));
    IloObjective obj = IloMinimize(env,  inconsistency + overtime);
//    IloObjective obj = IloMinimize(env,  inconsistency);
//    IloObjective obj = IloMinimize(env,  overtime);

	model.add(obj);
    IloCP cp(model);
    cp.setParameter(IloCP::TimeLimit, 20);

    // Para devolver todas as soluções
//    cp.setParameter(IloCP::LogVerbosity, IloCP::Quiet);
//
//    cp.startNewSearch();
//    while(cp.next()){

		if (cp.solve())
      {
      cp.out() << std::endl << cp.getStatus() << " Solution" << std::endl;
      cp.out() << "Inconsistency contraint value: " << cp.getValue(inconsistency) << std::endl;
      cp.out() << "Overtime contraint value: " << cp.getValue(overtime) << std::endl;
      for(int w = 0; w < weeks; w++){
    	  cp.out() << std::endl << std::endl;
		  cp.out() << "\t\t";
		  for(int i = 0; i < (int)shifts.size(); i++){
			  cp.out() << shifts[i].name;
			  for(int j = 0; j < (int)areas.size(); j++)
				  for(int k = 0; k < areas[j].spots; k++)
					  cp.out() << "\t\t";
		  }

		  cp.out() << std::endl;
		  cp.out() << "\t\t";
		  for(int i = 0; i < (int)shifts.size(); i++){
			  for(int i = 0; i < (int)areas.size(); i++){
				  cp.out() << areas[i].name;
				  for(int k = 0; k < areas[i].spots; k++)
					  cp.out() << "\t\t";
			  }
		  }
		  cp.out() << std::endl << std::endl;
		  cp.out() << "\t\t";
		  for(int i = 0; i < (int)shifts.size(); i++)
			  for(int j = 0; j < (int)areas.size(); j++)
				  for(int k = 0; k < areas[j].spots; k++)
					  cp.out() << k+1 << "\t\t";
		  cp.out() << std::endl;

		  for(int d = w*7, dw = 0; d < (w+1)*7, dw < 7; d++, dw++){
			  cp.out() << weekdaysName[dw] << "\t\t";
			  for(int s = 0; s < (int)shifts.size(); s++){
				  for(int a = 0; a < (int)areas.size(); a++){
					  for(int v = 0; v < areas[a].spots; v++){
						  cp.out() << physicians[cp.getValue(assignment[d][s][a][v])].name << "\t\t";
					  }
				  }
			  }
			  cp.out() << std::endl;
		  }
      }
    }
  }
  catch (IloException& ex) {
    env.out() << "Error: " << ex << std::endl;
  }
  env.end();
  return 0;
}
