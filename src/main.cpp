#include <ilcp/cp.h>
#include <string>
#include <vector>
#include <fstream>
#include "matplotlibcpp.h"
#include "../includes/roastering.h"
#include "../includes/Solution.h"
#include "../includes/SolutionSet.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace plt = matplotlibcpp;

using namespace std;
class Physician;


void printSolution(SolutionSet solutionSet, int index){
	Solution solution = solutionSet.solutions[index];
	string weekdaysName[7] = {"Mon", "Tue", "Wed", "Thur",
									"Fri", "Sat", "Sun"};
	for(int i = 0; i < (int) solution.softConstraints.size(); i++){
		cout << solution.softConstraints[i].name << " constraint value: " <<  solution.softConstraints[i].value << endl;
	}

	for (int w = 0; w < solutionSet.weeks; w++) {
		cout << std::endl << std::endl;
		cout << "\t\t";
		for (int i = 0; i < (int) solutionSet.shifts.size(); i++) {
			cout << solutionSet.shifts[i].name;
			for (int j = 0; j < (int) solutionSet.areas.size(); j++)
				for (int k = 0; k < solutionSet.areas[j].spots; k++)
					cout << "\t\t";
		}

		cout << std::endl;
		cout << "\t\t";
		for (int i = 0; i < (int) solutionSet.shifts.size(); i++) {
			for (int i = 0; i < (int) solutionSet.areas.size(); i++) {
				cout << solutionSet.areas[i].name;
				for (int k = 0; k < solutionSet.areas[i].spots; k++)
					cout << "\t\t";
			}
		}
		cout << std::endl << std::endl;
		cout << "\t\t";
		for (int i = 0; i < (int) solutionSet.shifts.size(); i++)
			for (int j = 0; j < (int) solutionSet.areas.size(); j++)
				for (int k = 0; k < solutionSet.areas[j].spots; k++)
					cout << k + 1 << "\t\t";
		cout << std::endl;

		for (int d = w * 7, dw = 0; d < (w + 1) * 7, dw < 7; d++, dw++) {
			cout << weekdaysName[dw] << "\t\t";
			for (int s = 0; s < (int) solutionSet.shifts.size(); s++) {
				for (int a = 0; a < (int) solutionSet.areas.size(); a++) {
					for (int v = 0; v < solutionSet.areas[a].spots; v++) {
						cout << solutionSet.physicians[solution.schedule[d][s][a][v]].name << "\t\t";
					}
				}
			}
			cout << endl << endl;
		}
	}
}

void printSolutionFromUserInput(SolutionSet solutionSet){
	int solutionIndex = -1;
	while(solutionIndex != 0){
		cout << "Digite um índice de solução para visualizá-la (digite 0 para sair):" << endl;
		cin >> solutionIndex;

		if(solutionIndex == 0)
			return;

		if(solutionIndex >= 1 && solutionIndex <= (int) solutionSet.solutions.size())
			printSolution(solutionSet, solutionIndex - 1);
		else
			cout << "Solução não existe" << endl;

	}
}

void enumerateSolutions(vector<Solution> solutions){
	for(int i = 0; i < (int) solutions.size(); i++){
		plt::annotate(to_string(i + 1), solutions[i].softConstraints[0].value, solutions[i].softConstraints[1].value);
	}
}

void plotSolutions(vector<Solution> solutions){
	vector<int> xAxis;
	vector<int> yAxis;
	for(int i = 0; i < (int) solutions.size(); i++){
		xAxis.push_back(solutions[i].softConstraints[0].value);
		yAxis.push_back(solutions[i].softConstraints[1].value);
	}
	plt::scatter(xAxis, yAxis, 25);
}

void plotAll(vector<vector<Solution>> solutionsArray){
	plt::xlabel(solutionsArray[1][0].softConstraints[0].name);
	plt::ylabel(solutionsArray[1][0].softConstraints[1].name);

	for(int i = 0; i < (int) solutionsArray.size(); i++){
		plotSolutions(solutionsArray[i]);
	}

	plt::show();
}

// TODO Pensar em uma forma utilizando ordenação por uma das funções objetivo
// TODO Realizar testes variando as configurações
// TODO Utilizar algoritmo de geração de pesos junto com restrições que limitam as funções objetivo
// TODO Lembrar Mário de cancelar a bolsa pibic
// TODO Solicitar mudança de data de início da fapesp

vector<Solution> removeDominated(SolutionSet *solutionSet){
	vector<Solution> dominatedSolutions;
	for(int i = 0; i < (int)solutionSet->solutions.size(); i++){
		for(int j = i; j < (int) solutionSet->solutions.size(); j++){
			bool hasConstraintGreater = false;
			bool hasConstraintLower = false;
			// Para cada restrição flexível, iremos comparar os valores de duas soluções.
			// Caso a solução i apresente valor maior que a solução j para a restrição k,
			// o valor de hasConstraintGreater será true. O oposto vale paraHasConstraintLower
			for(int k = 0; k < (int) solutionSet->solutions[i].softConstraints.size(); k++){
				if(solutionSet->solutions[i].softConstraints[k].value > solutionSet->solutions[j].softConstraints[k].value)
					hasConstraintGreater = true;
				if(solutionSet->solutions[i].softConstraints[k].value < solutionSet->solutions[j].softConstraints[k].value)
					hasConstraintLower = true;
			}
			// Caso hasConstraintLower seja true e hasConstraintGreater seja false,
			// podemos afirmar que a solução i domina j, uma vez que j nunca tem uma restrição
			// menor que i, mas i tem ao menos uma restrição menor que j
			if(hasConstraintLower && !hasConstraintGreater){
				dominatedSolutions.push_back(solutionSet->solutions[j]);
				solutionSet->solutions.erase(solutionSet->solutions.begin() + j);
				j--;
			}
			// O mesmo vale para o oposto. Neste caso j domina i
			if(!hasConstraintLower && hasConstraintGreater){
				dominatedSolutions.push_back(solutionSet->solutions[i]);
				solutionSet->solutions.erase(solutionSet->solutions.begin() + i);
				j = i;
			}
		}
	}

	return dominatedSolutions;
}

int main(int, char *argv[]) {
	SolutionSet solutionSet = rostering(argv);

	if(solutionSet.solutions.size() < 1){
		cout << "Nenhuma solução encontrada" << endl;
		return 0;
	}

	vector<Solution> dominatedSolutions = removeDominated(&solutionSet);
	enumerateSolutions(solutionSet.solutions);

	pid_t c_pid = fork();
	if (c_pid == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	} else if (c_pid > 0) {
		plotAll({dominatedSolutions, solutionSet.solutions});
		wait(nullptr);
	} else {
		printSolutionFromUserInput(solutionSet);
		kill (getppid(), 9);
	}

	return 0;
}
