#ifndef DATEHANDLER_H_
#define DATEHANDLER_H_
#include "../includes/Date.h"
#include "../includes/PeriodRestriction.h"
#include <vector>
#include <string>
#include <ctime>
using namespace std;
class Date;

time_t getTime(int day, int month, int year);

vector<string> split(string s, string delimiter);

vector<Date> getDaysFromRange(string begginingDateString, string endingDateString);

vector<PeriodRestriction> initializePeriodRestriction(vector<Date> days, bool initialize);

vector<PeriodRestriction> handlePossiblePeriod(string possiblePeriodsString, string restrictedPeriodsString, vector<Date> days);

vector<PeriodRestriction> handleAmbulatoryPeriod(string ambulatoryPeriodsString, vector<Date> days);

void interpretNotation(vector<string> notationVector, vector<PeriodRestriction> &periods, bool value);

#endif /* DATEHANDLER_H_ */
