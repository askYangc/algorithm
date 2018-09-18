#ifndef _STATE_H_
#define _STATE_H_

#include <iostream>
#include <list>

class state;

class obj {
public:
	obj();
	virtual ~obj();
	void request();
public:
	state *s;
};

class state {
public:
	virtual ~state(){}
	virtual void setState(obj *j) {}
	virtual void show(){
		std::cout << "now state" << std::endl;
	}
};
	


class state_idle : public state {
public:
	virtual void setState(obj *j);	
	void show(){
		std::cout << "state_idle" << std::endl;
	}
};

class state_estab : public state {
public:
	virtual void setState(obj *j);	
	void show(){
		std::cout << "state_estab" << std::endl;
	}
};




#endif
