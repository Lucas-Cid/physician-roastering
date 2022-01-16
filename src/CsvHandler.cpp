#include "../includes/CsvHandler.h"

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

