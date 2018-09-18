#ifndef _CLONE_H_
#define _CLONE_H_

#include <iostream>

class cloneable {
public:
	virtual cloneable *clone(){
		return NULL;
	}
};

class student : public cloneable {
public:
	virtual ~student(){}
	student(char *n, int a) {
		name = n;
		age = a;
	}
	student *clone() {
		return new student(name, age);
	}
	void setname(char *name){
		this->name = name;
	}
	void setage(int age){
		this->age = age;
	}	
	void show() {
		std::cout << "name: " << name << ",age: " << age << std::endl;
	}
private:
	char *name;
	int age;
};

#endif
