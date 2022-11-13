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

Solution solve(RosteringInput *input, int dimension){
	Solution newSolution;

	// Inicializa os valores utilizados na normalização para que eles não interfiram
	// nas primeiras soluções
	for(int i = 0; i < dimension; i++){
		input->idealConstraintsValues.push_back(0);
		input->nadirConstraintsValues.push_back(1);
	}

	input->idealAndNadirPointVerification = false;
	input->normalization = false;


	newSolution = rostering(*input);
	return newSolution;
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
			currentWeights.push_back(j == i ? 0.97 : 0.03);
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
	// nas primeiras soluções
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


int main(int argc, char** argv) {
	// lê as configurações de input
	string searchNumber = "1";

	vector<RosteringInput> inputs;
	for(int i = 0; i < stoi(searchNumber); i++){
		string configFile, physicianFile, shiftFile, areaFile;
		if(argc > 2){
			if(argc >= 6){
				configFile = argv[2];
				physicianFile = argv[3];
				shiftFile = argv[4];
				areaFile = argv[5];
			} else {
				cout << "Invalid Arguments" << endl;
				exit(1);
			}

		} else{
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
		}
		RosteringInput input = readData(configFile, physicianFile, shiftFile, areaFile);
		if(input.days.size() == 0 || input.physicians.size() == 0 || input.shifts.size() == 0 || input.areas.size() == 0 ){
			cout << "Um dos arquivos não foi encontrado" << endl;
			if(argc > 2)
				exit(1);
			i--;
		} else{
			inputs.push_back(input);
		}
	}

	vector<vector<Solution>> allSolutions;
	int dimension = 4;
	for(int i = 0; i < (int) inputs.size(); i++){
		vector<Solution> solutions;
		if(!strcmp(argv[1], "solve")){

			inputs[i].optimize = false;
			Solution newSolution = solve(&inputs[i], dimension);
			if(newSolution.schedule.size() > 0)
				solutions.push_back(newSolution);

		} else if(!strcmp(argv[1], "simpleOptimization")){

			vector<double> weights;
			for(int j = 0; j < dimension; j++){
				weights.push_back(1);
			}
			inputs[i].optimize = true;
			inputs[i].weights = weights;

			Solution newSolution = solve(&inputs[i], dimension);
			if(newSolution.schedule.size() > 0)
				solutions.push_back(newSolution);

		} else if(!strcmp(argv[1], "weightsOptimization")){
			// utiliza o gerador de pesos para obter um conjunto de soluções
			vector<vector<double>> weights = weightGenerator(inputs[i], dimension);
			inputs[i].optimize = true;
			solutions = searchAll(inputs[i], weights, dimension);

		} else{
			cout << "Nenhum modo de busca foi passado" << endl;
		}

		allSolutions.push_back(solutions);

		if(solutions.size() < 1){
			cout << "Nenhuma solução encontrada" << endl;
			return 0;
		}
	}

	if(allSolutions[0].size() <= 1 && argc >= 7){
		writeSolutionFile(allSolutions[0][0], inputs[0], argv[6]);
		cout << "Solução encontrada" << endl;
	} else{
		for(int i = 0; i < (int) allSolutions[0].size(); i++){
			writeSolutionFile(allSolutions[0][i], inputs[0], "src/solutions/solution"+to_string(i+1));
		}
	}

	return 0;
}
