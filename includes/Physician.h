#ifndef PHYSICIAN_H_
#define PHYSICIAN_H_
#include <string>

using namespace std;

class Physician{
public:
	string name;
	string specialty;
	int age;
	int experienceTime;
	int hours;

	Physician(string name, int age, int experienceTime, string specialty, int hours);
};


#endif /* PHYSICIAN_H_ */
