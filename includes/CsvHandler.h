#ifndef CSVHANDLER_H_
#define CSVHANDLER_H_

#include <ilcp/cp.h>
#include <string>
#include <vector>
#include <fstream>

#include "../includes/Physician.h"
#include "../includes/Shift.h"
#include "../includes/Area.h"
#include "../includes/Solution.h"
#include "../includes/Date.h"
#include "../includes/DateHandler.h"

class Physician;
using namespace std;

vector<vector<string>> readCSV(string folder,string fileName);

void readPhysiciansData(vector<Physician> *physicians, string fileName);

void readConfigData(int *maxHoursMargin, int *minHoursMargin, int *maxNightShifts, vector<Date> *days, bool *normalization, bool *idealAndNadirPointVerification, int *layers, int *timePerSolution, string fileName);

void readShiftsData(vector<Shift> *shifts, string fileName);

void readAreasData(vector<Area> *areas, string fileName);

#endif /* CSVHANDLER_H_ */
