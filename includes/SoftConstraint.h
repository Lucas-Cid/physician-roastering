#ifndef SOFTCONSTRAINT_H_
#define SOFTCONSTRAINT_H_
#include <string>

using namespace std;

class SoftConstraint {
public:
	string name;
	int value;
	SoftConstraint(string name, int value);
};

#endif /* SOFTCONSTRAINT_H_ */
