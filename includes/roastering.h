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
#include "../includes/RosteringInput.h"


class Physician;

using namespace std;

Solution rostering(RosteringInput input);

#endif /* ROASTERING_H_ */
