/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1987-2004 AT&T Corp.                  *
*                      and is licensed under the                       *
*          Common Public License, Version 1.0 (the "License")          *
*                        by AT&T Corp. ("AT&T")                        *
*      Any use, downloading, reproduction or distribution of this      *
*      software constitutes acceptance of the License.  A copy of      *
*                     the License is available at                      *
*                                                                      *
*         http://www.research.att.com/sw/license/cpl-1.0.html          *
*         (with md5 checksum 8a5e0081c856944e76c69a1cf29c2e8b)         *
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
