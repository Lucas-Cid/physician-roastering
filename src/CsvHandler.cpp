#include "../includes/CsvHandler.h"

vector<vector<string>> readCSV(string folder, string fileName){
	string base = "src/" + folder + "/";
	string path = fileName + ".csv";
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

void readPhysiciansData(vector<Physician> *physicians, string fileName){
	fileName = fileName.size() > 0 ? fileName : "Physicians";

	vector<vector<string>> csv = readCSV("Physicians", fileName);

	if(csv.size() == 0) return;

	for(int i = 0; i < (int)csv.size(); i++){
		physicians->push_back(Physician(csv[i][0], stoi(csv[i][1]), stoi(csv[i][2]), csv[i][3], stoi(csv[i][4])));
	}
}

void readConfigData(int *maxHoursMargin, int *minHoursMargin, int *maxNightShifts, int *weeks, int *days, bool *normalization, bool *idealAndNadirPointVerification, int *layers, int *timePerSolution, string fileName){
	fileName = fileName.size() > 0 ? fileName : "Config";

	vector<vector<string>> csv = readCSV("Configs", fileName);

	if(csv.size() == 0) return;

	for(int i = 0; i < (int)csv.size(); i++){
		*minHoursMargin = stoi(csv[i][0]);
		*maxHoursMargin = stoi(csv[i][1]);
		*maxNightShifts = stoi(csv[i][2]);
		*weeks = stoi(csv[i][3]);
		*days = *weeks * 7;
		*normalization = csv[i][4] == "sim" ? true : false;
		*idealAndNadirPointVerification = csv[i][5] == "sim" ? true : false;
		*layers = stoi(csv[i][6]);
		*timePerSolution = stoi(csv[i][7]);
	}

}

void readShiftsData(vector<Shift> *shifts, string fileName){
	fileName = fileName.size() > 0 ? fileName : "Shifts";

	vector<vector<string>> csv = readCSV("Shifts", fileName);

	if(csv.size() == 0) return;

	for(int i = 0; i < (int)csv.size(); i++){
		shifts->push_back(Shift(csv[i][0], stoi(csv[i][1])));
	}
}

void readAreasData(vector<Area> *areas, string fileName){
	fileName = fileName.size() > 0 ? fileName : "Areas";

	vector<vector<string>> csv = readCSV("Areas", fileName);

	if(csv.size() == 0) return;

	for(int i = 0; i < (int)csv.size(); i++){
		areas->push_back(Area(csv[i][0], stoi(csv[i][1])));
	}

}

