#ifndef _FACADE_H_
#define _FACADE_H_

#include <iostream>

class person {
public:
	void do_eat() {
		std::cout <<  "吃饭" << std::endl;
	}
	void do_sleep() {
		std::cout <<  "睡觉" << std::endl;
	}	
};

class team {
public:
	~team(){}
	virtual void do_operationA(){
		a.do_eat();
		b.do_sleep();
	}

	virtual void do_operationB(){
		a.do_eat();
	}
		
	
private:
	person a;
	person b;
};


#endif
