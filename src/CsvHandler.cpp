#include "../includes/CsvHandler.h"
#include <algorithm>

vector<vector<string>> readCSV(string fileName){
	string path = fileName + ".csv";
    ifstream file(path);
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
			cell.erase(std::remove(cell.begin(), cell.end(), '\r'), cell.end());
            cell.erase(std::remove(cell.begin(), cell.end(), ' '), cell.end());
			cell.erase(std::remove(cell.begin(), cell.end(), '\n'), cell.end());
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

void readPhysiciansData(vector<Physician> *physicians, string fileName, vector<Date> days){
	fileName = fileName.size() > 0 ? fileName : "src/Physicians/Physicians";

	vector<vector<string>> csv = readCSV(fileName);

	if(csv.size() == 0) return;

	for(int i = 0; i < (int)csv.size(); i++){
		Physician newPhysician = Physician(csv[i][0], stoi(csv[i][1].size() != 0 ? csv[i][1] : "0"), stoi(csv[i][2].size() != 0 ? csv[i][2] : "0"), csv[i][3], stoi(csv[i][4]), stoi(csv[i][5]), stoi(csv[i][6]));

		string possiblePeriod = "";
		string restrictedPeriod = "";

		if(csv[i].size() >= 8)
			restrictedPeriod = csv[i][7];
		if(csv[i].size() >= 9)
			possiblePeriod = csv[i][8];

		newPhysician.possiblePeriod = handlePossiblePeriod(possiblePeriod, restrictedPeriod, days);

		physicians->push_back(newPhysician);
	}
}

void readConfigData(int *maxHoursMargin, int *minHoursMargin, int *maxNightShifts, vector<Date> *days, bool *normalization, bool *idealAndNadirPointVerification, int *layers, int *timePerSolution, string fileName){
	fileName = fileName.size() > 0 ? fileName : "src/Configs/Config";

	vector<vector<string>> csv = readCSV(fileName);
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
	fileName = fileName.size() > 0 ? fileName : "src/Shifts/Shifts";

	vector<vector<string>> csv = readCSV(fileName);

	if(csv.size() == 0) return;

	for(int i = 0; i < (int)csv.size(); i++){
		shifts->push_back(Shift(csv[i][0], stoi(csv[i][1])));
	}
}

void readAreasData(vector<Area> *areas, string fileName){
	fileName = fileName.size() > 0 ? fileName : "src/Areas/Areas";

	vector<vector<string>> csv = readCSV(fileName);

	if(csv.size() == 0) return;

	for(int i = 0; i < (int)csv.size(); i++){
		areas->push_back(Area(csv[i][0], {stoi(csv[i][1]), stoi(csv[i][2]), stoi(csv[i][3])}));
	}

}

void writeSolutionFile(Solution solution, RosteringInput input, string fileName){
	string path = fileName + ".csv";
	ofstream file(path);
	if(!file.good()){
		cout << "Erro na abertura do arquivo de saida" << endl;
		return;
	}

	string weekdaysName[7] = {"Dom", "Seg", "Ter", "Qua", "Qui",
									"Sex", "Sab"};
	int weeks = ceil(input.days.size() / 7.0f);

	file << "," << "Values" << "," << "Real values" << "," << "Min" << "," << "Max" << "," << "Mean" << endl;
	for(int i = 0; i < (int) solution.softConstraints.size(); i++){
		file << solution.softConstraints[i].name <<  "," << solution.softConstraints[i].value << ",";
		file << solution.softConstraints[i].realValue << ",";
		file << solution.softConstraints[i].min << ",";
		file << solution.softConstraints[i].max << ",";
		file << round(solution.softConstraints[i].mean) << endl;

	}

	for (int w = 0; w < weeks; w++) {
		file << std::endl << std::endl;
		file << ",";
		for (int i = 0; i < (int) input.shifts.size(); i++) {
			file << input.shifts[i].name;
			for (int j = 0; j < (int) input.areas.size(); j++)
				for (int k = 0; k < input.areas[j].spots[i]; k++)
					file << ",";
		}

		file << std::endl;
		file << ",";
		for (int i = 0; i < (int) input.shifts.size(); i++) {
			for (int j = 0; j < (int) input.areas.size(); j++) {
				file << input.areas[j].name;
				for (int k = 0; k < input.areas[j].spots[i]; k++)
					file << ",";
			}
		}
		file << std::endl;
		file << ",";
		for (int i = 0; i < (int) input.shifts.size(); i++)
			for (int j = 0; j < (int) input.areas.size(); j++)
				for (int k = 0; k < input.areas[j].spots[i]; k++)
					file << k + 1 << ",";
		file << std::endl;

		for (int d = w * 7; d < (w + 1) * 7 && d < (int) input.days.size(); d++) {
			file << ((input.days[d].day < 10) ? "0" : "") << input.days[d].day << "/" <<  ((input.days[d].month < 10) ? "0" : "") << input.days[d].month << " ("<< weekdaysName[input.days[d].weekDay] << ")" << ",";
			for (int s = 0; s < (int) input.shifts.size(); s++) {
				for (int a = 0; a < (int) input.areas.size(); a++) {
					for (int v = 0; v < input.areas[a].spots[s]; v++) {
						file << input.physicians[solution.schedule[d][s][a][v]].name << ",";
					}
				}
			}
			file << endl;
		}
	}
	file.close();
}

