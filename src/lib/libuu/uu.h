/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1998-2000 AT&T Corp.              *
*      and it may only be used by you under license from       *
*                     AT&T Corp. ("AT&T")                      *
*       A copy of the Source Code Agreement is available       *
*              at the AT&T Internet web site URL               *
*                                                              *
*     http://www.research.att.com/sw/license/ast-open.html     *
*                                                              *
*     If you received this software without first entering     *
*       into a license with AT&T, you have an infringing       *
*           copy and cannot use it without violating           *
*             AT&T's intellectual property rights.             *
*                                                              *
*               This software was created by the               *
*               Network Services Research Center               *
*                      AT&T Labs Research                      *
*                       Florham Park NJ                        *
*                                                              *
*             Glenn Fowler <gsf@research.att.com>              *
*                                                              *
***************************************************************/
#pragma prototyped
/*
 * uu encode/decode interface definitions
 *
 * AT&T Research
 */

#ifndef _UU_H
#define _UU_H

#include <ast.h>

#define UU_VERSION	19980611L

#define UU_HEADER	(1<<0)		/* header/trailer encoded too	*/
#define UU_TEXT		(1<<1)		/* process text file		*/

struct Uu_s; typedef struct Uu_s Uu_t;
struct Uudisc_s; typedef struct Uudisc_s Uudisc_t;
struct Uumeth_s; typedef struct Uumeth_s Uumeth_t;

typedef int (*Uu_f)(Uu_t*);
typedef int (*Uuerror_f)(Uu_t*, Uudisc_t*, int, const char*, ...);

struct Uumeth_s
{
	const char*	name;
	const char*	alias;
	const char*	id;
	Uu_f		headerf;
	Uu_f		encodef;
	Uu_f		decodef;
	void*		data;
};

struct Uudisc_s
{
	unsigned long	version;
	unsigned long	flags;
	Uuerror_f	errorf;
};

struct Uu_s
{
	const char*	id;
	Uumeth_t	meth;
	Uudisc_t*	disc;
	char*		path;

#ifdef _UU_PRIVATE_
	_UU_PRIVATE_
#endif

};

#if _BLD_uu && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern Uu_t*		uuopen(Uudisc_t*, Uumeth_t*);
extern int		uuclose(Uu_t*);

extern ssize_t		uuencode(Uu_t*, Sfio_t*, Sfio_t*, size_t, const char*);
extern ssize_t		uudecode(Uu_t*, Sfio_t*, Sfio_t*, size_t, const char*);

extern int		uumain(char**, int);

extern int		uulist(Sfio_t*);
extern Uumeth_t*	uumeth(const char*);

#undef	extern

#endif
