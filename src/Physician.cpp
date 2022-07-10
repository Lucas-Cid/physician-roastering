#include "../includes/Physician.h"

Physician::Physician(string name, int age, int experienceTime, string specialty, int hours, int dueHours, int hoursBank) {
	this->name = name;
	this->age = age;
	this->experienceTime = experienceTime;
	this->specialty = specialty;
	this->hours = hours;
	this->dueHours = dueHours;
	this->hoursBank = hoursBank;
}

