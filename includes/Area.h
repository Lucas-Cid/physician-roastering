#ifndef AREA_H_
#define AREA_H_
#include <string>
#include <vector>
using namespace std;

class Area {
public:
	string name;
	vector<int> spots;

	Area(string name, vector<int> spots);
};

#endif /* AREA_H_ */
