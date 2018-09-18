#ifndef _OBSERVER_H_
#define _OBSERVER_H_

#include <iostream>
#include <list>

class observer {
public:
	~observer(){}
	virtual void update(){}
};

class nbaobserver : public observer {
public:
	void update(){
		std::cout << "NBA observer get update" << std::endl;
	}
};

class newsobserver : public observer {
public:
	void update(){
		std::cout << "news observer get update" << std::endl;
	}
};

class obj {
public:
	void update() {
		std::list<observer*>::iterator o = obs.begin();
		for(;o!=obs.end();o++) {
			(*o)->update();
		}
	}
	void insert(observer *o) {
		obs.push_back(o);
	}
private:
	std::list<observer*> obs;
};


#endif
