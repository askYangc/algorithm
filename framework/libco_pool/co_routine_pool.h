#ifndef __CO_ROUTINE_POOL_H__
#define __CO_ROUTINE_POOL_H__

#include "co_routine_inner.h"


void stCoRoutine_pool_free();
stCoRoutine_t *stCoRoutine_alloc();
void stCoRoutine_release(stCoRoutine_t *co);



#endif
