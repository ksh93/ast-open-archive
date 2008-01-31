/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2000-2008 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
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
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * integer tuple list cx internal/external interface
 *
 * Glenn Fowler
 * AT&T Research
 */

#ifndef _ITL_H
#define _ITL_H

#include <cx.h>

#if _BLD_itl && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern ssize_t		itl1external(Cx_t*, Cxtype_t*, int, int, int, const char*, Cxformat_t**, Cxvalue_t*, char*, size_t, Cxdisc_t*);
extern ssize_t		itl2external(Cx_t*, Cxtype_t*, int, int, int, const char*, Cxformat_t**, Cxvalue_t*, char*, size_t, Cxdisc_t*);
extern ssize_t		itl4external(Cx_t*, Cxtype_t*, int, int, int, const char*, Cxformat_t**, Cxvalue_t*, char*, size_t, Cxdisc_t*);

extern ssize_t		itl1internal(Cx_t*, Cxvalue_t*, int, int, int, const char*, size_t, Vmalloc_t*, Cxdisc_t*);
extern ssize_t		itl2internal(Cx_t*, Cxvalue_t*, int, int, int, const char*, size_t, Vmalloc_t*, Cxdisc_t*);
extern ssize_t		itl4internal(Cx_t*, Cxvalue_t*, int, int, int, const char*, size_t, Vmalloc_t*, Cxdisc_t*);

#undef	extern

#endif
