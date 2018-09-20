#include "factory.h"


std::shared_ptr<stock> StockFactory::get(const string &key)
{
	boost::mutex::scoped_lock lock(mutex_);

	std::weak_ptr<stock> &wt = stock_[key];

	std::shared_ptr<stock> st(wt.lock());
	if( NULL == st) {
        /*
        将share_ptr<StockFactory>转换为weak_ptr<StockFactory>，同时bind会复制一份实参存储在bind内部，这里deleteStockcallback的第一个参数是引用，实际上也可以修改普通的factory形参，这里设置为引用是难得在调用deleteStockcallback时再复制一次std::weak_ptr<StockFactory>
        */
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