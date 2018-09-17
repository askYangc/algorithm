#ifndef _FACTORY_H_
#define _FACTORY_H_

#include <iostream>
#include "calculate.h"

class fac {
public:
	virtual ~fac(){}
	virtual cal *get_cal(){ return NULL;}
};

class addfac:public fac {
public:
	cal *get_cal() {
		return new cal_add();
	}
};

class delfac:public fac {
public:
	cal *get_cal() {
		return new cal_del();
	}
};



#endif
