/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1991-2004 AT&T Corp.                  *
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
 * David Korn
 * Glenn Fowler
 *
 * AT&T Research
 *
 * hash index file interface definitions
 */

#ifndef _HIX_H
#define _HIX_H

#include <ast.h>

#define HIX_VERSION	20010214L	/* implementation version	*/

#define HIX_OPEN	0		/* open event			*/
#define HIX_CLOSE	1		/* close event			*/

#define HIX_ERROR	(1<<0)		/* previous op caused error	*/
#define HIX_LOCAL	(1<<1)		/* index data in . ok		*/
#define HIX_READONLY	(1<<2)		/* use but don't generate index	*/
#define HIX_REGENERATE	(1<<3)		/* regenerate used indexes	*/
#define HIX_TEST1	(1<<4)		/* internal test #1		*/
#define HIX_TEST2	(1<<5)		/* internal test #2		*/
#define HIX_USER	(1<<9)		/* first user defined flag	*/

#define HIX_and		(-1)
#define HIX_or		(-2)

struct Hix_s;
struct Hixdisc_s;

typedef char* (*Hixfind_f)(struct Hix_s*, char*, size_t, const char*, struct Hixdisc_s*);

typedef struct Hixdisc_s
{
	unsigned long	version;	/* interface version		*/
	unsigned long	flags;		/* HIX_* flags			*/
	char*		stamp;		/* application stamp		*/
	char*		lib;		/* pathfind() lib		*/
	Error_f		errorf;		/* error function		*/
	int		(*eventf)(struct Hix_s*, int, void*, struct Hixdisc_s*);
					/* event function		*/
	void*		(*splitf)(struct Hix_s*, Sfio_t*, char*, char*, struct Hixdisc_s*);
					/* read and split record	*/
} Hixdisc_t;

typedef struct Hix_s
{
	char*		name;		/* first part name		*/
	Sflong_t	records;	/* number of hashed records	*/
	Sflong_t	offset;		/* offset for last hixget()	*/
	int		size;		/* size for last hixget()	*/
	int		maxsize;	/* max hixget() size		*/
	int		flags;		/* HIX_* flags			*/
	int		delimiter;	/* virtual file delimiter	*/
	int		partition;	/* partition for last hixget()	*/
	int		partitions;	/* number of input partitions	*/
	int		restricted;	/* restricted to this partition	*/

#ifdef _HIX_PRIVATE_
	_HIX_PRIVATE_
#endif

} Hix_t;

#define hixend		(-1)
#define hixref(i,r)	((1<<14)|((i)<<7)|(r))

#define hixand(p)	hixop(p,HIX_and,0)
#define hixeq(p,i,h)	hixop(p,i,h)
#define hixor(p)	hixop(p,HIX_or,0)
#define hixtell(p)	((p)->offset)

extern int		hixclose(Hix_t*);
extern char*		hixerror(Hix_t*);
extern void*		hixget(Hix_t*, int);
extern Hix_t*		hixopen(const char*, const char*, const char*, int*, Hixdisc_t*);
extern int		hixop(Hix_t*, int, long);
extern int		hixpos(Hix_t*);
extern int		hixput(Hix_t*, long*);
extern int		hixseek(Hix_t*, off_t);
extern int		hixset(Hix_t*, int);

#endif
