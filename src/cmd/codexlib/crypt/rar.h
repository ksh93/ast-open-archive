#pragma prototyped

/*
 * rar crypt common definitions
 */

#ifndef _RAR_H
#define _RAR_H		1

#include "cryptlib.h"

#define ROL(x,n)	(((x)<<(n))|((x)>>(8*sizeof(x)-(n))))
#define ROR(x,n)	(((x)>>(n))|((x)<<(8*sizeof(x)-(n))))

#endif
