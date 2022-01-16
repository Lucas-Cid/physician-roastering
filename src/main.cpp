#include <ilcp/cp.h>
#include <string>
#include <vector>
#include <fstream>
#include "matplotlibcpp.h"
#include "../includes/roastering.h"
#include "../includes/Solution.h"
#include "../includes/CsvHandler.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <queue>
#include <bits/stdc++.h>

namespace plt = matplotlibcpp;

using namespace std;
class Physician;

RosteringInput readData(char *argv[]){
	RosteringInput input;
	//Lê o arquivo com as características dos médicos
	readPhysiciansData(&input.physicians, argv[1]);

	//Lê o arquivo de configuração
	readConfigData(&input.maxHoursMargin, &input.minHoursMargin, &input.maxNightShifts, &input.weeks, &input.days, argv[2]);

	//Lê o arquivo com as características dos turnos
	readShiftsData(&input.shifts, argv[3]);

	//Lê o arquivo com as características das áreas do hospital
	readAreasData(&input.areas, argv[4]);

	return input;
}

void printSolution(Solution solution, RosteringInput input){
	string weekdaysName[7] = {"Mon", "Tue", "Wed", "Thur",
									"Fri", "Sat", "Sun"};
	for(int i = 0; i < (int) solution.softConstraints.size(); i++){
		cout << solution.softConstraints[i].name << " constraint value: " <<  solution.softConstraints[i].value << endl;
	}

	for (int w = 0; w < input.weeks; w++) {
		cout << std::endl << std::endl;
		cout << "\t\t";
		for (int i = 0; i < (int) input.shifts.size(); i++) {
			cout << input.shifts[i].name;
			for (int j = 0; j < (int) input.areas.size(); j++)
				for (int k = 0; k < input.areas[j].spots; k++)
					cout << "\t\t";
		}

		cout << std::endl;
		cout << "\t\t";
		for (int i = 0; i < (int) input.shifts.size(); i++) {
			for (int i = 0; i < (int) input.areas.size(); i++) {
				cout << input.areas[i].name;
				for (int k = 0; k < input.areas[i].spots; k++)
					cout << "\t\t";
			}
		}
		cout << std::endl << std::endl;
		cout << "\t\t";
		for (int i = 0; i < (int) input.shifts.size(); i++)
			for (int j = 0; j < (int) input.areas.size(); j++)
				for (int k = 0; k < input.areas[j].spots; k++)
					cout << k + 1 << "\t\t";
		cout << std::endl;

		for (int d = w * 7, dw = 0; d < (w + 1) * 7 || dw < 7; d++, dw++) {
			cout << weekdaysName[dw] << "\t\t";
			for (int s = 0; s < (int) input.shifts.size(); s++) {
				for (int a = 0; a < (int) input.areas.size(); a++) {
					for (int v = 0; v < input.areas[a].spots; v++) {
						cout << input.physicians[solution.schedule[d][s][a][v]].name << "\t\t";
					}
				}
			}
			cout << endl << endl;
		}
	}
}

void printSolutionFromUserInput(vector<Solution> solutions,  RosteringInput input){
	int solutionIndex = -1;
	while(solutionIndex != 0){
		cout << "Digite um índice de solução para visualizá-la (digite 0 para sair):" << endl;
		cin >> solutionIndex;

		if(solutionIndex == 0)
			return;

		if(solutionIndex >= 1 && solutionIndex <= (int) solutions.size())
			printSolution(solutions[solutionIndex - 1], input);
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
	plt::xlabel(solutionsArray[0][0].softConstraints[0].name);
	plt::ylabel(solutionsArray[0][0].softConstraints[1].name);

	for(int i = 0; i < (int) solutionsArray.size(); i++){
		plotSolutions(solutionsArray[i]);
	}

	plt::show();
}

void showScatterPlot(vector<Solution> solutions, RosteringInput input){
	enumerateSolutions(solutions);
	pid_t c_pid = fork();
	if (c_pid == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	} else if (c_pid > 0) {
		plotAll({solutions});
		wait(nullptr);
	} else {
		printSolutionFromUserInput(solutions, input);
		kill (getppid(), 9);
	}
}

bool compareSolutions(Solution s1, Solution s2){
	return s1.softConstraints[0].value < s2.softConstraints[0].value;
}

vector<Solution> removeDominated(vector<Solution> &solutions){
	vector<Solution> dominatedSolutions;

	sort(solutions.begin(), solutions.end(), compareSolutions);

	for(int i = 1; i < (int) solutions.size(); i++){
		if(solutions[i-1].softConstraints[1].value <= solutions[i].softConstraints[1].value){
			dominatedSolutions.push_back(solutions[i]);
			solutions.erase(solutions.begin() + i);
			i--;
		}
	}

	return dominatedSolutions;
}

void removeDominatedBySolution(vector<Solution> &solutions, Solution newSolution){
	int i = solutions.size() - 1;

	// Itera, a partir do final do vetor, por todas as soluções já encontradas.
	// O vetor, pelo modo como é feita a inserção de novas soluções, está ordenado
	// pela primeira função objetivo.
	while(i >= 0 && solutions[i].softConstraints[0].value >= newSolution.softConstraints[0].value){
		bool iHasBetterConstraint = false;
		for(int j = 1; j < (int) newSolution.softConstraints.size(); j++){
			if(solutions[i].softConstraints[j].value < newSolution.softConstraints[j].value){
				iHasBetterConstraint = true;
			}
		}
		// Se a i-ésima solução não tiver nenhuma função objetivo de valor melhor
		// que as funções objetivo da nova solução, então esta é dominada e deve
		// ser removida
		if(!iHasBetterConstraint){
			solutions.erase(solutions.begin() + i);

			// Caso a última solução do vetor seja removida, i terá um valor maior
			// que o tamanho do vetor, podendo gerar erros
			i = i > ((int) solutions.size()) - 1 ? solutions.size() - 1 : i;
		} else{
			i--;
		}
	}
}

void insertSolutionOrderedVector(vector<Solution> &solutions, Solution newSolution){
	// Acha um iterator para o primeiro elemento maior que newSolution no vetor solutions,
	// seguindo o critério da função compareSolutions
	auto it = std::upper_bound(solutions.cbegin(), solutions.cend(), newSolution, compareSolutions);

	// Insere newSolution antes do iterator encontrado
	solutions.insert(it, newSolution);
}

void search(vector<double> weights, RosteringInput *input, vector<Solution> &solutions){
	Solution newSolution;
	input->weights = weights;
	newSolution = rostering(*input);
	if(newSolution.schedule.size() > 0){
		// remove possíveis soluções que são dominadas pela nova solução gerada
		removeDominatedBySolution(solutions, newSolution);
		// insere a solução em um vetor ordenado de acordo com o valor da primeira função objetivo
		insertSolutionOrderedVector(solutions, newSolution);
		input->solutions = solutions;
	}
}

vector<double> sumVectorsAndDivide(vector<double> vector1, vector<double> vector2, int divisor){
	if(vector1.size() != vector2.size()){
		return {};
	}
	vector<double> result;
	for(int i = 0; i < (int) vector1.size(); i++){
		result.push_back((vector1[i] + vector2[i]) / divisor);
	}
	return result;
}

vector<Solution> weightGenerator(RosteringInput input){
	vector<Solution> solutions;
	input.solutions = solutions;

	// inicialmente, adiciona os pesos {1, 0} e {0, 1}
	search({1, 0}, &input, solutions);
	search({0, 1}, &input, solutions);

	// cria uma lista de pares de pesos
	queue<vector<vector<double>>> weightQueue;
	weightQueue.push({{1, 0}, {0, 1}});

	// busca por i soluções
	int i = 2;
	while(i < 10){
		// pega o primeiro par de pesos da fila
		vector<vector<double>> weights = weightQueue.front();

		// gera novos pesos pk usando o par retirado pi e pj
		vector<double> pi = weights[0];
		vector<double> pj = weights[1];
		vector<double> pk = sumVectorsAndDivide(pi, pj, 2);

		// busca uma solução utilizando o novo peso gerado
		search(pk, &input, solutions);

		// cria dois novos pares de pesos
		weightQueue.push({pi, pk});
		weightQueue.push({pj, pk});
		weightQueue.pop();

		i++;
	}

	return solutions;
}

// TODO Pensar em uma condição de parada utilizando o número de camadas
// TODO Tentar utilizar normalização
// TODO Verificar se houve melhoria nas mudanças realizadas
// TODO Variar as instâncias
int main(int, char *argv[]) {
	// lê as configurações de input
	RosteringInput input = readData(argv);

	// utiliza o gerador de pesos para obter um conjunto de soluções
	vector<Solution> solutions = weightGenerator(input);

	if(solutions.size() < 1){
		cout << "Nenhuma solução encontrada" << endl;
		return 0;
	}

	showScatterPlot(solutions, input);

	return 0;
}
