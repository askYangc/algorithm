#ifndef _LIB_H_
#define _LIB_H_

void hello();

void bt_show();
static inline void test_inline()
{
	bt_show();
}

#endif
