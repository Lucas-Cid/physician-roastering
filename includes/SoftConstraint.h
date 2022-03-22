#ifndef SOFTCONSTRAINT_H_
#define SOFTCONSTRAINT_H_
#include <string>

using namespace std;

class SoftConstraint {
public:
	string name;
	double value;
	SoftConstraint(string name, double value);
};

#endif /* SOFTCONSTRAINT_H_ */
