#ifndef _CALCULATE_H_
#define _CALCULATE_H_

#include <iostream>


class cal {
public:
	virtual ~cal(){}
	virtual void get_result(int a, int b){};
};

class cal_add:public cal {
public:
	cal_add() {
		std::cout << "cal_add" << std::endl;
	}
	void get_result(int a, int b) {
		std::cout << a << " + " << b << " = " << a+b << std::endl;	
	}
};
	
class cal_del:public cal {
public:
	cal_del() {
		std::cout << "cal_del" << std::endl;
	}
	void get_result(int a, int b) {
		std::cout << a << " - " << b << " = " << a-b << std::endl;	
	}
};




#endif
