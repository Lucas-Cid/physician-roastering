#ifndef PHYSICIAN_H_
#define PHYSICIAN_H_
#include <string>
#include <vector>
#include "./RestrictedShift.h"

using namespace std;

class Physician{
public:
	string name;
	string specialty;
	int age;
	int experienceTime;
	int hours;
	int dueHours;
	int hoursBank;
	vector<RestrictedShift> restrictedShifts;

	Physician(string name, int age, int experienceTime, string specialty, int hours, int dueHours, int hoursBank);
};


#endif /* PHYSICIAN_H_ */
