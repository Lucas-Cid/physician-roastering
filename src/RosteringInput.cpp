#include "../includes/RosteringInput.h"

RosteringInput::RosteringInput() {
	this->maxHoursMargin = 0;
	this->minHoursMargin = 0;
	this->maxNightShifts = 0;
	this->weeks = 0;
	this->days = 0;
	this->normalization = false;
	this->idealAndNadirPointVerification = false;
	this->verificationOn = false;
	this->layers = 1;
	this->timePerSolution = 1;
}

