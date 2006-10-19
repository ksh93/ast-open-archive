/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*           Copyright (c) 1989-2006 AT&T Knowledge Ventures            *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                      by AT&T Knowledge Ventures                      *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                   Eduardo Krell <ekrell@adexus.cl>                   *
*                                                                      *
***********************************************************************/
#pragma prototyped

#define msgreserve(p)	reserve(p)

#define gethostname	______gethostname

#include "3d.h"

#undef	gethostname

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
