#ifndef _PROXY_H_
#define _PROXY_H_

#include <iostream>

class proxy {
public:
	~proxy(){}
	virtual void do_operation(){}
};

class realoper : public proxy {
public:
	void do_operation(){
		std::cout << "realoper do_operation" << std::endl;
	}
};

class proxyA : public proxy {
public:
	proxyA(){
		p = new realoper();
	}
	void do_operation(){
		//There can do something
		//...
		p->do_operation();
	}
private:
	proxy *p;
};


#endif
