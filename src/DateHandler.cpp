#include "../includes/DateHandler.h"

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
