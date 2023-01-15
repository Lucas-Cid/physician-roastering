#include "../includes/PeriodRestriction.h"

PeriodRestriction::PeriodRestriction(int day, int month, int year, int weekDay, bool value){
	this->date.day = day;
	this->date.month = month;
	this->date.year = year;
	this->date.weekDay = weekDay;
	this->shifts = {value, value, value};
}

