#ifndef _MEMO_H_
#define _MEMO_H_

#include <iostream>

class memo {
public:
	memo(int age, int health):age(age),health(health) {}
public:
	int age;
	int health;	
};

class role {
public:
	role(int age, int health):age(age),health(health) {}
	memo *getmemo(){
		return new memo(age, health);
	}
	void setmemo(memo *memo){
		age = memo->age;
		health = memo->health;
	}
	void show(){
		std::cout << age << " " << health << std::endl;
	}
	
public:
	int age;
	int health;
};

#endif
