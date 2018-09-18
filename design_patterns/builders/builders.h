#ifndef _BUILDERS_H_
#define _BUILDERS_H_

#include <iostream>

class person {
public:
	virtual ~person(){}
	virtual void build_body() {};
	virtual void build_face() {};
};

class boy:public person {
public:
	void build_body() {
		std::cout << "创建男孩身体" << std::endl;
	}
	void build_face() {
		std::cout << "创建男孩的脸" << std::endl;
	}	
};

class girl:public person {
public:
	void build_body() {
		std::cout << "创建女孩身体" << std::endl;
	}
	void build_face() {
		std::cout << "创建女孩的脸" << std::endl;
	}	
};


class personDirector {
public:
	~personDirector(){}
	virtual void do_build(person &p){
		p.build_body();
		p.build_face();
	}
};


#endif
