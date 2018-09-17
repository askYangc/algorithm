#ifndef _FACTORY_H_
#define _FACTORY_H_

#include <iostream>
#include "calculate.h"

class fac {
public:
	fac(){}
	cal *getcal(char c) {
		switch(c) {
			case '+':
				return new cal_add();
			break;
			case '-':
				return new cal_del();
			break;
			default:
				return NULL;
				break;
		}
	}
};



#endif
