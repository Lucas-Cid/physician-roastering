#ifndef NOTATION_H_
#define NOTATION_H_
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

class Notation {
public:
	int recurrency;
	int day;
	int weekDay;
	vector<int> shifts;

	Notation(string notation);
};

#endif /* NOTATION_H_ */
