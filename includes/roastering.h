/*
 * roastering.h
 *
 *  Created on: 12 de dez. de 2021
 *      Author: lucas
 */

#ifndef ROASTERING_H_
#define ROASTERING_H_

#include <ilcp/cp.h>
#include <string>
#include <vector>
#include <fstream>

#include "../includes/Physician.h"
#include "../includes/Shift.h"
#include "../includes/Area.h"
#include "../includes/Solution.h"
#include "../includes/SolutionSet.h"


class Physician;

using namespace std;

vector<vector<string>> readCSV(char fileName[]);

void readPhysiciansData(vector<Physician> *physicians, char fileName[]);

void readConfigData(int *maxHoursMargin, int *minHoursMargin, int *maxNightShifts, int *weeks, int *days, char fileName[]);

void readShiftsData(vector<Shift> *shifts, char fileName[]);

void readAreasData(vector<Area> *areas, char fileName[]);

SolutionSet rostering(char *argv[]);

#endif /* ROASTERING_H_ */
