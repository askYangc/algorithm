#ifndef _FACTORY_H_
#define _FACTORY_H_

#include <iostream>
#include <string>
#include <map>
#include <memory>
#include "boost/noncopyable.hpp"
#include "boost/thread.hpp"
#include "boost/thread/mutex.hpp"
#include "boost/bind.hpp"
//#include "boost/enable_shared_from_this.hpp"

using namespace std;

class stock {
public:
	stock(const string &key):k(key) {}		
	~stock(){}
	string get_key() {return k;}
	void show() { 
		cout << "key is " << k << endl;
	}
private:
	string k;
};

class StockFactory : public std::enable_shared_from_this<StockFactory>,public boost::noncopyable {
public:
	std::shared_ptr<stock> get(const string &key);
	void deleteStock(stock *sk);
	static void deleteStockcallback(const std::weak_ptr<StockFactory> &factory, stock *sk);
private:
	boost::mutex mutex_;
	map<string, std::weak_ptr<stock> > stock_;
};

#endif
