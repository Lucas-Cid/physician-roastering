#include "../includes/DateHandler.h"
#include "../includes/Notation.h"
#include <iostream>

vector<string> split(string s, string delimiter){
	s.substr(0, s.find(delimiter));
	size_t pos = 0;
	vector<string> sVector;
	while ((pos = s.find(delimiter)) != std::string::npos) {
	    string aux = s.substr(0, pos);
	    sVector.push_back(aux);
	    s.erase(0, pos + delimiter.length());
	}
	sVector.push_back(s);
	return sVector;
}

time_t getTime(int day, int month, int year){
	  tm time_in = { 0, 0, 0, day, month - 1, year - 1900};

	  time_t time_temp = mktime(&time_in);

	  return time_temp;
}

vector<Date> getDaysFromRange(string begginingDateString, string endingDateString){
	vector<Date> days;
	string delimiter = "/";
	vector<string> begginingDateSplit = split(begginingDateString, delimiter);
	vector<string> endingDateSplit = split(endingDateString, delimiter);

	time_t currentDate = getTime(stoi(begginingDateSplit[0]),
								 stoi(begginingDateSplit[1]),
								 stoi(begginingDateSplit[2]));

	time_t endingDate =  getTime(stoi(endingDateSplit[0]),
								 stoi(endingDateSplit[1]),
								 stoi(endingDateSplit[2]));

	double timeDiff = difftime(currentDate, endingDate);

	while(timeDiff <= 0){
		const tm * newDate = localtime(&currentDate);
		days.push_back(Date(newDate->tm_mday, newDate->tm_mon + 1, newDate->tm_year + 1900, newDate->tm_wday));
		currentDate = getTime(newDate->tm_mday + 1, newDate->tm_mon + 1, newDate->tm_year + 1900);
		timeDiff = difftime(currentDate, endingDate);
	}

	return days;
}

vector<PeriodRestriction> initializePeriodRestriction(vector<Date> days, bool value){
	vector<PeriodRestriction> periodRestrictionVector;
	for(int i = 0; i < (int) days.size(); i++){
		PeriodRestriction periodRestriction = PeriodRestriction(days[i].day, days[i].month, days[i].year, days[i].weekDay, value);
		periodRestrictionVector.push_back(periodRestriction);
	}

	return periodRestrictionVector;
}

vector<PeriodRestriction> handlePossiblePeriod(string possiblePeriodsString, string restrictedPeriodsString, vector<Date> days){
	vector<string> possiblePeriods = split(possiblePeriodsString, ";");
	vector<string> restrictedPeriods = split(restrictedPeriodsString, ";");

	bool initializeValue = true;
	if(possiblePeriodsString.size() > 0)
		initializeValue = false;

	vector<PeriodRestriction> periods = initializePeriodRestriction(days, initializeValue);


	//inicializa finais de semana como períodos possíveis
	for(int d = 0; d < (int) periods.size(); d++){
		// Começa em 0
		int weekDay = periods[d].date.weekDay + 1;
		if(weekDay == 1 || weekDay == 7){
			for(int s = 0; s < (int) periods[d].shifts.size(); s++){
				periods[d].shifts[s] = true;
			}
		}
	}

	if(possiblePeriodsString.size() > 0)
		interpretNotation(possiblePeriods, periods, true);
	if(restrictedPeriodsString.size() > 0)
		interpretNotation(restrictedPeriods, periods, false);

	return periods;
}

vector<PeriodRestriction> handleAmbulatoryPeriod(string ambulatoryPeriodsString, vector<Date> days){
	vector<string> ambulatoryPeriods = split(ambulatoryPeriodsString, ";");

	vector<PeriodRestriction> periods = initializePeriodRestriction(days, false);

	if(ambulatoryPeriodsString.size() > 0)
		interpretNotation(ambulatoryPeriods, periods, true);

	return periods;
}


void interpretNotation(vector<string> notationVector, vector<PeriodRestriction> &periods, bool value){
	// TODO Adicionar regex para verificar se a notação é válida
	// Para cada período indicado
	for(int i = 0; i < (int) notationVector.size(); i++){
		// Traduz a notação em uma classe
		Notation notation = Notation(notationVector[i]);
		int counter = 0;
		// Varre todos os dias do cronograma e verifica se ele corresponde ao dia da notação
		for(int d = 0; d < (int) periods.size(); d++){
			// A notação pode indicar um dia específico ou um dia da semana
			bool sameDay = periods[d].date.day == notation.day;
			bool sameWeekDay = periods[d].date.weekDay == notation.weekDay - 1;

			// Verifica se o dia atual corresponde ao dia desejado, caso este tenha sido indicado
			bool dayRestricted = sameDay || sameWeekDay || (!notation.day && !notation.weekDay);

			if(dayRestricted){
				counter++;
				if(!notation.recurrency || counter == notation.recurrency){
					for(int s = 0; s < (int) notation.shifts.size(); s++){
						periods[d].shifts[notation.shifts[s]-1] = value;
					}
				}
			}
		}
	}
}











