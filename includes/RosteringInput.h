#ifndef ROSTERINGINPUT_H_
#define ROSTERINGINPUT_H_

#include <vector>
#include "../includes/Physician.h"
#include "../includes/Shift.h"
#include "../includes/Area.h"
#include "../includes/Solution.h"
#include "../includes/SoftConstraint.h"
#include "../includes/Date.h"


using namespace std;
class RosteringInput {
public:
	vector<Physician> physicians;
	vector<Shift> shifts;
	vector<Area> areas;
	int maxHoursMargin;
	int minHoursMargin;
	int maxNightShifts;
	vector<Date> days;
	bool normalization;
	bool idealAndNadirPointVerification;
	bool verificationOn;
	bool optimize;
	int layers;
	int timePerSolution;
	vector<double> idealConstraintsValues;
	vector<double> nadirConstraintsValues;
	vector<double> weights;
	vector<Solution> solutions;
	RosteringInput();
};

#endif /* ROSTERINGINPUT_H_ */
