#ifndef _DECORATOR_H_
#define _DECORATOR_H_

#include <iostream>

class person {
public:
	person(){}
	person(char *name):name(name){}
	virtual void do_operation(){
		std::cout << "人" << std::endl;
	}
private:
	char *name;
};

class decorator_clothes : public person {
public:
	virtual ~decorator_clothes(){}
	virtual void do_operation(){
		p->do_operation();
	};
	void set_decorator_clothes(person *pp){
		p = pp;
	}

protected:
	person *p;
};


class yifu:public decorator_clothes {
public:
	yifu() {
		std::cout << "yifu" << std::endl;
	}
	void do_operation() {		
		decorator_clothes::do_operation();
		std::cout << "好衣服" << std::endl;	
	}
};
	
class kuzi:public decorator_clothes {
public:
	kuzi() {
		std::cout << "kuzi" << std::endl;
	}
	void do_operation() {
		decorator_clothes::do_operation();		
		std::cout << "好裤子" << std::endl;	
	}
};



#endif
