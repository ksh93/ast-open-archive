/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1987-2004 AT&T Corp.                *
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
*                                                                  *
*******************************************************************/
#pragma prototyped

/*
 * <tv_sec,tv_usec> interface
 */

#ifndef _TV_H
#define _TV_H		1

#include <ast.h>
#include <sys/stat.h>

typedef struct Tv_s
{
	time_t		tv_sec;
	time_t		tv_nsec;
} Tv_t;

#if _BLD_ast && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern int		tvgettime(Tv_t*);
extern int		tvcmp(const Tv_t*, const Tv_t*);
extern int		tvgetstat(const struct stat*, Tv_t*, Tv_t*, Tv_t*);
extern int		tvsetstat(struct stat*, const Tv_t*, const Tv_t*, const Tv_t*);
extern int		tvtouch(const char*, const Tv_t*, const Tv_t*, const Tv_t*, int);

#undef	extern

#endif
