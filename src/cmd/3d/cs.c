/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1989-2003 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*    If you have copied or used this software without agreeing     *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                David Korn <dgk@research.att.com>                 *
*                 Eduardo Krell <ekrell@adexus.cl>                 *
*                                                                  *
*******************************************************************/
#pragma prototyped

#define msgreserve(p)	reserve(p)

#include "3d.h"

#include <ctype.h>

#define CS_LIB_LOCAL	1

#define _3d_fmttime(a,b)	"[NOW]"

#if FS

#undef	NoN
#define NoN(x)

#include "csdata.c"
#undef	csaddr
#define csaddr(p,x)	0
#include "csauth.c"
#include "csbind.c"
#include "cslocal.c"
#include "csname.c"
#include "csntoa.c"
#include "csread.c"
#include "csrecv.c"
#include "cssend.c"
#include "csvar.c"
#include "cswrite.c"

#include "msgbuf.c"
#include "msgread.c"
#include "msguser.c"

#endif

#include "cspeek.c"
#include "cspipe.c"

#if FS
#include "cspoll.c"	/* follows cs*.c because of #undef's */
#endif

#include "msggetmask.c"
#include "msgindex.c"
#include "msginfo.c"
#include "msgname.c"
#include "msgsetmask.c"
