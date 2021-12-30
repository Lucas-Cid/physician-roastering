#ifndef SOLUTION_H_
#define SOLUTION_H_
#include <vector>
#include "../includes/SoftConstraint.h"

using namespace std;

class Solution {
public:
	vector<vector<vector<vector<int>>>> schedule;
	vector<SoftConstraint> softConstraints;
	Solution();
};

#endif /* SOLUTION_H_ */
