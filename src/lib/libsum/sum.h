/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*           Copyright (c) 1996-2007 AT&T Knowledge Ventures            *
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
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * Glenn Fowler
 * AT&T Research
 *
 * checksum library interface
 */

#ifndef _SUM_H
#define _SUM_H

#include <ast.h>

#define SUM_SIZE	(1<<0)		/* print size too		*/
#define SUM_SCALE	(1<<1)		/* traditional size scale	*/
#define SUM_TOTAL	(1<<2)		/* print totals since sumopen	*/

#define _SUM_PUBLIC_	const char*	name;

typedef struct Sumdata_s
{
	uint32_t	size;
	uint32_t	num;
	void*		buf;
} Sumdata_t;

typedef struct Sum_s
{
	_SUM_PUBLIC_
#ifdef	_SUM_PRIVATE_
	_SUM_PRIVATE_
#endif
} Sum_t;

extern Sum_t*	sumopen(const char*);
extern int	suminit(Sum_t*);
extern int	sumblock(Sum_t*, const void*, size_t);
extern int	sumdone(Sum_t*);
extern int	sumdata(Sum_t*, Sumdata_t*);
extern int	sumprint(Sum_t*, Sfio_t*, int, size_t);
extern int	sumusage(Sfio_t*);
extern int	sumclose(Sum_t*);

#endif
