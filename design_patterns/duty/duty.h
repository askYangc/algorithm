#ifndef _DUTY_H_
#define _DUTY_H_

#include <iostream>

class oa;

class leader {
public:
	leader(char *name):name(name) {}
	virtual ~leader(){}
	char *getname(){ return name;}
	void setboss(leader *boss){ this->boss = boss;}
	virtual void exectue(oa *o){}
protected:
	char *name;
	leader *boss;
};

//组长
class groupLeader : public leader {
public:
	groupLeader(char *name):leader(name){}
	void exectue(oa *o);
};

//项目经理
class itemLeader : public leader {
public:
	itemLeader(char *name):leader(name){}
	void exectue(oa *o);
};

//总监
class directorLeader : public leader {
public:
	directorLeader(char *name):leader(name){}
	void exectue(oa *o);
};



class oa  {
public:
	oa(int level, char *name):level(level),name(name){}
	virtual ~oa(){}
	int getlevel(){ return level;}
	char *getname(){ return name;}
protected:
	int level;	
	char *name;
};

//财务
class financialoa : public oa {
public:
	financialoa():oa(3, (char*)"财务"){}
};

//请假
class vacateoa : public oa {
public:
	vacateoa():oa(1, (char*)"请假"){}
};

//奖励
class rewardoa : public oa {
public:
	rewardoa():oa(2, (char*)"奖励"){}
};




#endif
