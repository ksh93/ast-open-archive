/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*           Copyright (c) 2000-2006 AT&T Knowledge Ventures            *
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
 * prefix table interface definitions
 *
 * Glenn Fowler
 * AT&T Research
 */

#ifndef _PT_H
#define _PT_H

#include <ast.h>

#define PT_VERSION		20020329L

#define PTBITS			32
#define PTSHIFT			1

#define PTMIN(a,b)		((a)&~((b)?((((Ptaddr_t)1)<<(PTBITS-(b)))-1):~((Ptaddr_t)0)))
#define PTMAX(a,b)		((a)|((b)?((((Ptaddr_t)1)<<(PTBITS-(b)))-1):~((Ptaddr_t)0)))

struct Pt_s; typedef struct Pt_s Pt_t;
struct Ptdisc_s; typedef struct Ptdisc_s Ptdisc_t;
struct Ptprefix_s; typedef struct Ptprefix_s Ptprefix_t;

typedef unsigned _ast_int4_t Ptaddr_t;
typedef unsigned _ast_intmax_t Ptcount_t;

struct Ptprefix_s
{
	Ptaddr_t		min;
	Ptaddr_t		max;
#ifdef _PTPREFIX_PRIVATE_
	_PTPREFIX_PRIVATE_
#endif
};

struct Ptdisc_s				/* user discipline		*/
{
	unsigned long	version;	/* interface version		*/
	Error_f		errorf;		/* error function		*/
};

struct Pt_s
{
	Ptcount_t	entries;
	Dt_t*		dict;
	Ptdisc_t*	disc;
#ifdef _PT_PRIVATE_
	_PT_PRIVATE_
#endif
};

#if _BLD_pt && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

#define ptinit(d)	(memset(d,0,sizeof(Ptdisc_t)),(d)->version=PT_VERSION)

extern Pt_t*		ptopen(Ptdisc_t*);
extern int		ptclose(Pt_t*);
extern int		ptinsert(Pt_t*, Ptaddr_t, Ptaddr_t);
extern int		ptdelete(Pt_t*, Ptaddr_t, Ptaddr_t);

extern Ptprefix_t*	ptmatch(Pt_t*, Ptaddr_t);

extern int		ptprint(Pt_t*, Sfio_t*);
extern int		ptstats(Pt_t*, Sfio_t*);
extern int		ptdump(Pt_t*, Sfio_t*);

extern Ptcount_t	ptaddresses(Pt_t*);
extern Ptcount_t	ptranges(Pt_t*);
extern Ptcount_t	ptsize(Pt_t*);

extern Pt_t*		ptcopy(Pt_t*);
extern Pt_t*		ptinvert(Pt_t*);
extern Pt_t*		ptintersect(Pt_t*, Pt_t*);
extern Pt_t*		ptunion(Pt_t*, Pt_t*);
extern Pt_t*		ptdifference(Pt_t*, Pt_t*);
extern Pt_t*		ptcover(Pt_t*, Pt_t*);

extern int		ptequal(Pt_t*, Pt_t*);
extern int		ptsubset(Pt_t*, Pt_t*);

#undef	extern

#endif
