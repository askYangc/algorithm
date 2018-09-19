#ifndef _COMMAND_H_
#define _COMMAND_H_

#include <iostream>
#include <list>

using namespace std;

class receiver {
public:
	virtual ~receiver(){}
	virtual void do_action(){
		std::cout << "receiver do action" << std::endl;
	}
};

class sqlreceiver: public receiver {
public:
	sqlreceiver(){}
	void do_action(){
		std::cout << "sqlreceiver do action" << std::endl;
	}
};

class namereceiver: public receiver {
public:
	namereceiver(){}
	void do_action(){
		std::cout << "namereceiver do action" << std::endl;
	}
};



class command {
public:
	command(int id, receiver *recver):id(id),recver(recver){}
	virtual ~command(){}
	bool isequal(int id) { return id == this->id;}
	virtual void execute(){}
protected:
	int id;
	receiver *recver;
};

class sqlcomand : public command {
public:
	sqlcomand(int id, receiver *recver):command(id, recver){}
	void execute(){
		std::cout << "sqlcomand execute" << std::endl;
		recver->do_action();
	}
};

class namecomand : public command {
public:
	namecomand(int id, receiver *recver):command(id, recver){}
	void execute(){
		std::cout << "namecomand execute" << std::endl;
		recver->do_action();
	}
};


class invoker {
public:
	~invoker() {
		list<command*>::iterator it = commands.begin();
		for(;it!=commands.end();) {
			delete *it;
			it = commands.erase(it);
		}		
	}
	void addCommand(command *c) {
		commands.push_back(c);
	}
	void delCommand(int id) {
		list<command*>::iterator it = commands.begin();
		for(;it!=commands.end();it++) {
			if((*it)->isequal(id)) {
				std::cout << "删除id " << id << std::endl;
				commands.erase(it);
				return;
			}
		}
		
	}
	void doCommands() {
		list<command*>::iterator it = commands.begin();
		for(;it!=commands.end();it++) {
			(*it)->execute();
		}
	}
private:
	list<command*> commands;
};


#endif
