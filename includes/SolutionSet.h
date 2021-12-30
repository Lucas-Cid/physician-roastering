#ifndef SOLUTIONSET_H_
#define SOLUTIONSET_H_
#include "../includes/Solution.h"
#include "../includes/Physician.h"
#include "../includes/Shift.h"
#include "../includes/Area.h"

class SolutionSet {
public:
	vector<Physician> physicians;
	vector<Shift> shifts;
	vector<Area> areas;
	int weeks;
	vector<Solution> solutions;
	SolutionSet();
};

#endif /* SOLUTIONSET_H_ */
