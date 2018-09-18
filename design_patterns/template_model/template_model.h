#ifndef _TEMPLATE_MODEL_H_
#define _TEMPLATE_MODEL_H_

#include <iostream>

class person {
public:
	virtual ~person(){}
	virtual void do_cook() = 0;
	virtual void do_eat() = 0;
	virtual void do_wash() = 0;
	void do_lunch(){
		do_cook();
		do_eat();
		do_wash();
	}

};

class boy : public person {
public:
	virtual void do_cook() {std::cout << "男孩在做饭" << std::endl;}
	virtual void do_eat() {std::cout << "男孩在吃饭，还在玩手机" << std::endl;}
	virtual void do_wash() {std::cout << "男孩在洗碗" << std::endl;}
};

class girl : public person {
public:
	virtual void do_cook() {std::cout << "女孩在做饭" << std::endl;}
	virtual void do_eat() {std::cout << "女孩在吃饭" << std::endl;}
	virtual void do_wash() {std::cout << "女孩在洗碗" << std::endl;}
};




#endif
