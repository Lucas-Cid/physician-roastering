#ifndef SOFTCONSTRAINT_H_
#define SOFTCONSTRAINT_H_
#include <string>

using namespace std;

class SoftConstraint {
public:
	string name;
	double value;
	double realValue;
	double min;
	double max;
	double mean;
	SoftConstraint(string name, double value, double realValue, double min, double max, double mean);
};

#endif /* SOFTCONSTRAINT_H_ */
