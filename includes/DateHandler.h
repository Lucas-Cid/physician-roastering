#ifndef DATEHANDLER_H_
#define DATEHANDLER_H_
#include "../includes/Date.h"
#include <vector>
#include <string>
#include <ctime>
using namespace std;
class Date;

time_t getTime(int day, int month, int year);

vector<string> split(string s, string delimiter);

vector<Date> getDaysFromRange(string begginingDateString, string endingDateString);

#endif /* DATEHANDLER_H_ */
