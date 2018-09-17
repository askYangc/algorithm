#ifndef _DECORATOR_H_
#define _DECORATOR_H_

#include <iostream>

class person {
public:
	person(){}
	person(char *name):name(name){}

private:
	char *name;
};

class decorator_clothes {
public:
	decorator_clothes() {p = NULL; ps = NULL;}
	virtual ~decorator_clothes(){}
	virtual void do_operation(){
		if(p) p->do_operation();
		if(ps) std::cout << "人" << std::endl;
	};
	void set_decorator_clothes(decorator_clothes *pp){
		p = pp;
	}
	void set_person(person *p) { ps = p;}

protected:
	decorator_clothes *p;
	person *ps;
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
