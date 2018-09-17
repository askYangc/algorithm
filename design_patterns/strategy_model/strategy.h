#ifndef _STRATEGY_H_
#define _STRATEGY_H_

#include <iostream>


class strategy {
public:
	virtual ~strategy(){}
	virtual void get_result(){};
	virtual void setNum(int a, int b) {
		NumA = a;
		NumB = b;
	}
	

protected:
	int NumA;
	int NumB;
};

class strategy1:public strategy {
public:
	strategy1() {
		std::cout << "strategy1" << std::endl;
	}
	void get_result() {
		std::cout << NumA << " + " << NumB << " = " << NumA+NumB << std::endl;	
	}
};
	
class strategy2:public strategy {
public:
	strategy2() {
		std::cout << "strategy2" << std::endl;
	}
	void get_result() {
		std::cout << NumA << " - " << NumB << " = " << NumA-NumB << std::endl;	
	}
};

class context {
public:

	void set_strategy(char c) {
		switch(c) {
			case '+':
				ste = new strategy1();
				break;
			case '-':
				ste = new strategy2();
				break;	
			default:
				break;
		};
	}
	void get_result(int a, int b) {
		ste->setNum(a, b);
		return ste->get_result();
	}
	~context(){
		if(ste) delete ste;
	}

private:
	strategy *ste;
};

#endif
