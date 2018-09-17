#ifndef _CALCULATE_H_
#define _CALCULATE_H_

#include <iostream>


class cal {
public:
	virtual ~cal(){}
	virtual void get_result(){};
	virtual void setNum(int a, int b) {
		NumA = a;
		NumB = b;
	}
	

protected:
	int NumA;
	int NumB;
};

class cal_add:public cal {
public:
	cal_add() {
		std::cout << "cal_add" << std::endl;
	}
	void get_result() {
		std::cout << NumA << " + " << NumB << " = " << NumA+NumB << std::endl;	
	}
};
	
class cal_del:public cal {
public:
	cal_del() {
		std::cout << "cal_del" << std::endl;
	}
	void get_result() {
		std::cout << NumA << " - " << NumB << " = " << NumA-NumB << std::endl;	
	}
};




#endif
