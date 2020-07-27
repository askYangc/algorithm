#ifndef __SERVER_TYPES_HEADER__
#define __SERVER_TYPES_HEADER__

#ifdef WIN32
typedef unsigned char           u_int8_t;
typedef unsigned short          u_int16_t;
typedef unsigned int            u_int32_t;
typedef unsigned long long      u_int64_t;
#else
#include <sys/types.h>
#ifndef U64_DEF
#define U64_DEF
typedef unsigned long long u64;
#endif
typedef char          s_int8_t;
typedef short         s_int16_t;
#endif

#define TRUE 1
#define FALSE 0
#define BITS(n) (1<<(n))
#endif
