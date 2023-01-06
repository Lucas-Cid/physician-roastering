#ifndef PERIODRESTRICTION_H_
#define PERIODRESTRICTION_H_
#include <vector>
#include "../includes/Date.h"
using namespace std;

class PeriodRestriction {
public:
	Date date;
	vector<bool> shifts;

	PeriodRestriction(int day, int month, int year, int weekDay, bool value);
};

#endif /* PERIODRESTRICTION_H_ */
