/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1985-2004 AT&T Corp.                  *
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
*                  David Korn <dgk@research.att.com>                   *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * integral representation conversion support definitions
 * supports sizeof(integral_type)<=sizeof(int_max)
 */

#ifndef _SWAP_H
#define _SWAP_H

#include <int.h>

#define SWAP_MAX	8

#define SWAPOP(n)	(((n)&int_swap)^(n))

#if _BLD_ast && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern void*		swapmem(int, const void*, void*, size_t);
extern int_max		swapget(int, const void*, int);
extern void*		swapput(int, void*, int, int_max);
extern int		swapop(const void*, const void*, int);

#undef	extern

#endif
