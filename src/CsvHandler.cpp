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

vector<RestrictedShift> transformRestrictedShifts(string restrictedShiftsString){
	vector<RestrictedShift> restrictedShifts;

	vector<string> restrictedDays = split(restrictedShiftsString, ";");


	for(int i = 0; i < (int) restrictedDays.size(); i++){
		vector<string> auxDateShifts = split(restrictedDays[i], ":");
		string date = auxDateShifts[0];
		int day = stoi(split(date, "/")[0]);
		int month = stoi(split(date, "/")[1]);

		if(auxDateShifts.size() > 1){
			vector<string> shifts = split(auxDateShifts[1], "/");
			for (int j = 0; j < (int) shifts.size(); j++){
				int shiftNumber;
				if(shifts[j] == "manhã"){
					shiftNumber = 0;
				} else if(shifts[j] == "tarde"){
					shiftNumber = 1;
				} else if(shifts[j] == "noite"){
					shiftNumber = 2;
				} else{
					shiftNumber = -1;
				}

				if(shiftNumber != -1){
					RestrictedShift newRestrictedShift = RestrictedShift(day, month, shiftNumber);
					restrictedShifts.push_back(newRestrictedShift);
				}
			}
		}else{
			for (int j = 0; j < 3; j++){
				RestrictedShift newRestrictedShift = RestrictedShift(day, month, j);
				restrictedShifts.push_back(newRestrictedShift);
			}
		}
	}
	return restrictedShifts;
}

void readPhysiciansData(vector<Physician> *physicians, string fileName){
	fileName = fileName.size() > 0 ? fileName : "Physicians";

	vector<vector<string>> csv = readCSV("Physicians", fileName);

	if(csv.size() == 0) return;

	for(int i = 0; i < (int)csv.size(); i++){
		Physician newPhysician = Physician(csv[i][0], stoi(csv[i][1]), stoi(csv[i][2]), csv[i][3], stoi(csv[i][4]), stoi(csv[i][5]), stoi(csv[i][6]));

		if(csv[i].size() == 8){
			string restrictedShiftsString = csv[i][7];
			vector<RestrictedShift> restrictedShifts = transformRestrictedShifts(restrictedShiftsString);
			newPhysician.restrictedShifts = restrictedShifts;
		}

		physicians->push_back(newPhysician);
	}
}

void readConfigData(int *maxHoursMargin, int *minHoursMargin, int *maxNightShifts, vector<Date> *days, bool *normalization, bool *idealAndNadirPointVerification, int *layers, int *timePerSolution, string fileName){
	fileName = fileName.size() > 0 ? fileName : "Config";

	vector<vector<string>> csv = readCSV("Configs", fileName);
	if(csv.size() == 0) return;

	for(int i = 0; i < (int)csv.size(); i++){
		*minHoursMargin = stoi(csv[i][0]);
		*maxHoursMargin = stoi(csv[i][1]);
		*maxNightShifts = stoi(csv[i][2]);
		*days = getDaysFromRange(csv[i][3], csv[i][4]);
		*normalization = csv[i][5] == "sim" ? true : false;
		*idealAndNadirPointVerification = csv[i][6] == "sim" ? true : false;
		*layers = stoi(csv[i][7]);
		*timePerSolution = stoi(csv[i][8]);
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

