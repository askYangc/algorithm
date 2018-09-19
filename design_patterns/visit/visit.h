#ifndef _VISIT_H_
#define _VISIT_H_

#include <iostream>
#include <string>

using namespace std;

class human;

class feel {
public:
	virtual void show(human *h) {}
};

class succ : public feel {
public:
	virtual void show(human *h);
};

class fail : public feel {
public:
	virtual void show(human *h);
};



class human {
public:
	human(string name):name(name){}
	string getname() { return name; }
	virtual void getstate(feel &f){
		f.show(this);
	}
protected:
	string name;
};

class man :public human {
public:
	man():human("男人"){}
};

class woman :public human {
public:
	woman():human("女人"){}
};


#endif
