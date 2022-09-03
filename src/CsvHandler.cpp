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
	file.close();
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

void writeSolutionFile(Solution solution, RosteringInput input, int solutionNumber){
	string base = "src/solutions/";
	string path = "solution" + to_string(solutionNumber) + ".csv";
	ofstream file(base+path);

	if(!file.good()){
		cout << "Erro na abertura do arquivo" << endl;
		return;
	}

	string weekdaysName[7] = {"Dom", "Seg", "Ter", "Qua", "Qui",
									"Sex", "Sab"};
	int weeks = ceil(input.days.size() / 7.0f);

	file << "\t" << "Values" << "\t" << "Real values" << "\t" << "Min" << "\t" << "Max" << "\t" << "Mean" << endl;
	for(int i = 0; i < (int) solution.softConstraints.size(); i++){
		file << solution.softConstraints[i].name <<  "\t" << solution.softConstraints[i].value << "\t";
		file << solution.softConstraints[i].realValue << "\t";
		file << solution.softConstraints[i].min << "\t";
		file << solution.softConstraints[i].max << "\t";
		file << round(solution.softConstraints[i].mean) << endl;

	}

	for (int w = 0; w < weeks; w++) {
		file << std::endl << std::endl;
		file << "\t";
		for (int i = 0; i < (int) input.shifts.size(); i++) {
			file << input.shifts[i].name;
			for (int j = 0; j < (int) input.areas.size(); j++)
				for (int k = 0; k < input.areas[j].spots; k++)
					file << "\t";
		}

		file << std::endl;
		file << "\t";
		for (int i = 0; i < (int) input.shifts.size(); i++) {
			for (int i = 0; i < (int) input.areas.size(); i++) {
				file << input.areas[i].name;
				for (int k = 0; k < input.areas[i].spots; k++)
					file << "\t";
			}
		}
		file << std::endl;
		file << "\t";
		for (int i = 0; i < (int) input.shifts.size(); i++)
			for (int j = 0; j < (int) input.areas.size(); j++)
				for (int k = 0; k < input.areas[j].spots; k++)
					file << k + 1 << "\t";
		file << std::endl;

		for (int d = w * 7; d < (w + 1) * 7 && d < (int) input.days.size(); d++) {
			file << ((input.days[d].day < 10) ? "0" : "") << input.days[d].day << "/" <<  ((input.days[d].month < 10) ? "0" : "") << input.days[d].month << " ("<< weekdaysName[input.days[d].weekDay] << ")" << "\t";
			for (int s = 0; s < (int) input.shifts.size(); s++) {
				for (int a = 0; a < (int) input.areas.size(); a++) {
					for (int v = 0; v < input.areas[a].spots; v++) {
						file << input.physicians[solution.schedule[d][s][a][v]].name << "\t";
					}
				}
			}
			file << endl;
		}
	}
	file.close();
}

