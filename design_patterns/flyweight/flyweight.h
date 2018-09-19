#ifndef _FLYWEIGHT_H_
#define _FLYWEIGHT_H_

#include <iostream>
#include <string>
#include <map>

using namespace std;


class user {
public:
	user(char *name):name(name) {}
	char *getname() { return name; }
protected:
	char *name;
};


class website {
public:
	website(){}
	virtual ~website(){}
	virtual void webshow(user &u) {}
};

class boke : public website {
public:
	boke(){}
	void webshow(user &u) {
		std::cout << "博客展示 用户: " << u.getname() << std::endl;
	}
};

class shopping:public website {
public:
	shopping(){}
	void webshow(user &u) {
		std::cout << "购物网站展示 用户: " << u.getname() << std::endl;
	}	
};

//需要一个工厂管理已经实现好的实例，实际上就是一个类似内存池的东西
class fac {
public:
	virtual ~fac(){}
	website *getWebSite(string &key) {
		map<string, website*>::iterator w = webs.find(key);
		if(w == webs.end()) {
			if(!key.compare("boke")) {
				website *b = new boke();
				webs.insert(make_pair(key, b));
				return b;
			}else if(!key.compare("shopping")) {
				website *b = new shopping();
				webs.insert(make_pair(key, b));
				return b;
			}else {
				cout << "没有该类型的网站" << endl;
				return NULL;
			}
		}
		return w->second;
 	}
	
	void getwebscount(){ cout << "网站个数: " << webs.size() << endl;}
protected:
	map<string, website*> webs;
};

#endif
