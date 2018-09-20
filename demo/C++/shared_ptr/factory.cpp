#include "factory.h"


std::shared_ptr<stock> StockFactory::get(const string &key)
{
	boost::mutex::scoped_lock lock(mutex_);

	std::weak_ptr<stock> &wt = stock_[key];

	std::shared_ptr<stock> st(wt.lock());
	if( NULL == st) {
		st.reset(new stock(key), 
			boost::bind(&StockFactory::deleteStockcallback, std::weak_ptr<StockFactory>(shared_from_this()), _1));
		wt = st;
	}
	
	return st;
}

void StockFactory::deleteStock(stock *sk) 
{
	if(sk) {
		boost::mutex::scoped_lock lock(mutex_);
		stock_.erase(sk->get_key());
		cout << "key " << sk->get_key() << " is delete" << endl;
	}
}

void StockFactory::deleteStockcallback(const std::weak_ptr<StockFactory> &factory, stock *sk)
{
	std::shared_ptr<StockFactory> f = factory.lock();
	if(f) {
		f->deleteStock(sk);
	}

	delete sk;
}