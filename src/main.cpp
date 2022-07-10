#include <ilcp/cp.h>
#include <string>
#include <vector>
#include <fstream>
#include "matplotlibcpp.h"
#include "../includes/roastering.h"
#include "../includes/Solution.h"
#include "../includes/Date.h"
#include "../includes/CsvHandler.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <queue>
#include <bits/stdc++.h>
#include "../includes/DateHandler.h"

namespace plt = matplotlibcpp;

using namespace std;
class Physician;

RosteringInput readData(string configFile, string physicianFile, string shiftFile, string areaFile){
	RosteringInput input;
	//Lê o arquivo com as características dos médicos
	readPhysiciansData(&input.physicians, physicianFile);

	//Lê o arquivo de configuração

	readConfigData(&input.maxHoursMargin, &input.minHoursMargin, &input.maxNightShifts, &input.days, &input.normalization, &input.idealAndNadirPointVerification, &input.layers, &input.timePerSolution ,configFile);

	//Lê o arquivo com as características dos turnos
	readShiftsData(&input.shifts, shiftFile);

	//Lê o arquivo com as características das áreas do hospital
	readAreasData(&input.areas, areaFile);

	return input;
}

void printSolution(Solution solution, RosteringInput input){
	string weekdaysName[7] = {"Dom", "Seg", "Ter", "Qua", "Qui",
									"Sex", "Sab"};
	int weeks = ceil(input.days.size() / 7.0f);

	for(int i = 0; i < (int) solution.realSoftConstraints.size(); i++){
		cout << solution.realSoftConstraints[i].name << " constraint value: " <<  solution.realSoftConstraints[i].value << endl;
	}

	for (int w = 0; w < weeks; w++) {
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

		for (int d = w * 7; d < (w + 1) * 7 && d < (int) input.days.size(); d++) {
			cout << ((input.days[d].day < 10) ? "0" : "") << input.days[d].day << "/" <<  ((input.days[d].month < 10) ? "0" : "") << input.days[d].month << " ("<< weekdaysName[input.days[d].weekDay] << ")" << "\t";
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

void printSolutionFromUserInput(vector<vector<Solution>> solutions,  vector<RosteringInput> inputs){
	int solutionIndex = 0;
	while(solutionIndex != -1){
		cout << "Digite um conjunto de soluções para visualizá-la (digite 0 para sair):" << endl;
		cin >> solutionIndex;

		solutionIndex--;

		int solutionSet = 0;
		int subtraction = 0;
		bool found = false;
		for(int i = 0; i < (int) solutions.size(); i++){
			if(!found){
				subtraction += solutions[i].size();
			}
			if(solutionIndex - subtraction >= 0){
				solutionSet++;
			}else{
				found = true;
			}
		}
		int realIndex = solutionIndex - (subtraction - solutions[solutionSet].size());

		if(solutionIndex == -1)
			return;

		if(realIndex >= 0 && realIndex < (int) solutions[solutionSet].size()){
			printSolution(solutions[solutionSet][realIndex], inputs[solutionSet]);
		}
		else
			cout << "Solução não existe" << endl;

	}
}

void printWeights(vector<vector<double>> weights){
	cout << "{ ";
	for(int j = 0; j < (int) weights.size(); j++){
		cout << "{ ";
		for(int k = 0; k < (int) weights[j].size(); k++){
			cout << weights[j][k] << " ";
		}
		cout << "} ";
	}
	cout << " }" << endl;
}

void enumerateSolutions(vector<vector<Solution>> solutions){
	int counter = 1;
	for(int i = 0; i < (int) solutions.size(); i++){
		for(int j = 0; j < (int) solutions[i].size(); j++){
			plt::annotate(to_string(counter), solutions[i][j].softConstraints[0].value, solutions[i][j].softConstraints[1].value);
			counter++;
		}
	}
}

void plotSolutions(vector<Solution> solutions){
	vector<double> xAxis;
	vector<double> yAxis;
	for(int i = 0; i < (int) solutions.size(); i++){
		for(int j = 0; j < (int) solutions[i].softConstraints.size(); j++){
			yAxis.push_back(solutions[i].softConstraints[j].value);
			xAxis.push_back(i * (solutions[i].softConstraints.size() + 1) + j);
		}
	}
	plt::bar(xAxis, yAxis);
}

void plotAll(vector<vector<Solution>> solutionsArray){
	plt::xlabel("Solutions");
	plt::ylabel("Value");

	for(int i = 0; i < (int) solutionsArray.size(); i++){
		plotSolutions(solutionsArray[i]);
	}

	plt::show();
}

void showScatterPlot(vector<vector<Solution>> solutions, vector<RosteringInput> inputs){
	enumerateSolutions(solutions);

	pid_t c_pid = fork();

	if (c_pid == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	} else if (c_pid > 0) {
		plotAll(solutions);
		wait(nullptr);
	} else {
		printSolutionFromUserInput(solutions, inputs);
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
		//removeDominatedBySolution(solutions, newSolution);
		// insere a solução em um vetor ordenado de acordo com o valor da primeira função objetivo
		insertSolutionOrderedVector(solutions, newSolution);
		input->solutions = solutions;
	}
}

vector<double> sumVectorsAndDivide(vector<vector<double>> v, int divisor){
	for(int i = 1; i < (int) v.size(); i++){
		if(v[i].size() != v[i-1].size()){
			return {};
		}
	}
	vector<double> result;
	for(int j = 0; j < (int) v[0].size(); j++){
		double iValue = 0;

		for(int i = 0; i < (int) v.size(); i++){
			iValue += v[i][j];
		}

		result.push_back(iValue / divisor);
	}
	return result;
}

vector<vector<double>> weightGenerator(RosteringInput input, int dimension){
	vector<vector<double>> generatedWeights;

	// inicialmente, ativa somente um peso em cada conjunto
	// [1, 0, 0], [0, 1, 0], [0, 0, 1]
	for(int i = 0; i < dimension; i++){
		vector<double> currentWeights;
		for(int j = 0; j < dimension; j++){
			currentWeights.push_back(j == i ? 1 : 0);
		}
		generatedWeights.push_back(currentWeights);
	}

	// cria uma fila de conjuntos de pesos
	queue<vector<vector<double>>> weightQueue;
	vector<vector<double>> weightsCombination;

	// Adiciona os pesos iniciais como um item nesta fila
	for(int i = 0; i < (int) generatedWeights.size(); i++){
		weightsCombination.push_back(generatedWeights[i]);
	}
	weightQueue.push(weightsCombination);

	int i = 0;
	while(i < (pow(dimension, input.layers) - 1) / (dimension - 1)){
		// pega o primeiro par de pesos da fila
		vector<vector<double>> weights = weightQueue.front();

		// gera novos pesos pk usando o conjunto de pesos retirado 'weights
		vector<double> pk = sumVectorsAndDivide(weights, dimension);

		// cria novos pares de pesos a partir da combinação com o novo peso gerado
		for(int j = 0; j < (int) weights.size(); j++){
			vector<vector<double>> newWeightsCombination;
			newWeightsCombination.push_back(pk);
			for(int k = 0; k < (int) weights.size(); k++){
				if(k != j){
					newWeightsCombination.push_back(weights[k]);
				}
			}
			weightQueue.push(newWeightsCombination);
		}

		generatedWeights.push_back(pk);
		weightQueue.pop();

		i++;
	}

	return generatedWeights;
}

vector<Solution> searchAll(RosteringInput input, vector<vector<double>> weights, int objectiveFunctions){
	vector<Solution> solutions;
	input.solutions = solutions;

	// Inicializa os valores utilizados na normalização para que eles não interfiram
	// nas duas primeiras soluções
	for(int i = 0; i < objectiveFunctions; i++){
		input.idealConstraintsValues.push_back(0);
		input.nadirConstraintsValues.push_back(1);
	}


	// inicialmente, faz buscas utilizando os pesos que ativem somente uma função objetivo
	for(int i = 0; i < objectiveFunctions; i++){
		search(weights[i], &input, solutions);
	}

	if(input.idealAndNadirPointVerification){
		input.verificationOn = true;
		// Verifica se os pontos ideal e nadir estão realmente corretos.
		// Fixa o valor em um eixo e otimiza os outros
		for(int i = 0; i < objectiveFunctions; i++){
			vector<double> currentWeights;
			for(int j = 0; j < objectiveFunctions; j++){
				currentWeights.push_back(j == i ? 0 : 1);
			}
			input.solutions = {solutions[i]};
			search(currentWeights, &input, solutions);
		}
		input.verificationOn = false;
	}


	// As primeiras soluções com pesos apresentam, obrigatoriamente
	// os melhores e piores valores para as funções objetivo de cada restrição. Por isso
	// já podemos definir os valores ideais e valores nadir para cada uma delas
	for(int i = 0; i < objectiveFunctions; i++){
		double worst = solutions[0].softConstraints[i].value;
		double best = solutions[0].softConstraints[i].value;

		for(int j = 1; j < (int) solutions.size(); j++){
			if(solutions[j].softConstraints[i].value > worst)
				worst = solutions[j].softConstraints[i].value;
			if(solutions[j].softConstraints[i].value < best)
				best = solutions[j].softConstraints[i].value;
		}

		input.idealConstraintsValues[i] = best;
		input.nadirConstraintsValues[i] = worst;
	}

	for(int i = objectiveFunctions; i < (int) weights.size(); i++){
		search(weights[i], &input, solutions);
	}

	return solutions;
}

// TODO generalizar gerador de pesos corretamente

// TODO implementar varíavel para cada médico que indique quantas horas ele está em haver e
// considerar isso no escalonamento

int main() {
	// lê as configurações de input
	string searchNumber;

	cout << "Número de buscas: ";
	getline(cin, searchNumber);


	vector<RosteringInput> inputs;
	for(int i = 0; i < stoi(searchNumber); i++){
		string configFile, physicianFile, shiftFile, areaFile;
		cout << "Arquivos de configuração" << endl;
		cout << "[Config] [Physicians] [Shifts] [Areas]" << endl;

		string line;
		getline(cin, line);
		vector<string> configurationStrings = split(line, " ");
		if(configurationStrings.size() == 4){
			configFile = configurationStrings[0];
			physicianFile = configurationStrings[1];
			shiftFile = configurationStrings[2];
			areaFile = configurationStrings[3];
		}else {
			cout << "Arquivo de configuração faltando, arquivos padrão serão utilizados" << endl;
		}
		RosteringInput input = readData(configFile, physicianFile, shiftFile, areaFile);
		if(input.days.size() == 0 || input.physicians.size() == 0 || input.shifts.size() == 0 || input.areas.size() == 0 ){
			cout << "Um dos arquivos não foi encontrado" << endl;
			i--;
		} else{
			inputs.push_back(input);
		}
	}

	vector<vector<Solution>> allSolutions;
	int dimension = 4;
	for(int i = 0; i < (int) inputs.size(); i++){
		// utiliza o gerador de pesos para obter um conjunto de soluções
		vector<vector<double>> weights = weightGenerator(inputs[i], dimension);

		vector<Solution> solutions = searchAll(inputs[i], weights, dimension);
		allSolutions.push_back(solutions);

		if(solutions.size() < 1){
			cout << "Nenhuma solução encontrada" << endl;
			return 0;
		}
	}
	showScatterPlot(allSolutions, inputs);

	return 0;
}
