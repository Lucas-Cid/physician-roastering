#include "../includes/Date.h"

Date::Date(int day, int month, int year, int weekDay){
	this->day = day;
	this->month = month;
	this->year = year;
	this->weekDay = weekDay;
}

Date::Date(){
	this->day = 0;
	this->month = 0;
	this->year = 0;
	this->weekDay = 0;
}

