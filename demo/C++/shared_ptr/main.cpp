#include "factory.h"


int main()
{
	std::shared_ptr<StockFactory> fac(new StockFactory());

	shared_ptr<stock> p = fac->get("test");
	p->show();

	//fac.reset();

	p.reset();
	
	return 0;
}
