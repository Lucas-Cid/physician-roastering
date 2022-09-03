#include "../includes/SoftConstraint.h"

SoftConstraint::SoftConstraint(string name, double value, double realValue, double min, double max, double mean){
	this->name = name;
	this->value = value;
	this->realValue = realValue;
	this->min = min;
	this->max = max;
	this->mean = mean;
}

