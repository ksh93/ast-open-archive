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
 * integer list regular expression interface
 *
 * Glenn Fowler
 * AT&T research
 */

#ifndef _IRE_H
#define _IRE_H

#include <ast_common.h>

#define IRE_VERSION	20030115L		/* interface version	*/

#define IRE_SET		(~0)			/* as path set marker	*/

#define ireinit(p)	(memset(p,0,sizeof(*(p))),(p)->version=IRE_VERSION)

typedef unsigned _ast_int4_t Ireint_t;

struct Ire_s; typedef struct Ire_s Ire_t;
struct Iredisc_s; typedef struct Iredisc_s Iredisc_t;

typedef void* (*Ireresize_f)(void*, void*, size_t);

struct Iredisc_s
{
	unsigned long	version;	/* discipline version		*/
	Error_f		errorf;		/* error function		*/
	Ireresize_f	resizef;	/* alloc/free function		*/
	void*		resizehandle;	/* resizef handle		*/
};

struct Ire_s				/* RE handle			*/
{
	const char*	id;		/* interface id			*/
	int		element;	/* element size			*/
	int		tuple;		/* tuple size			*/
	unsigned long	group;		/* embedded group mark		*/
#ifdef _IRE_PRIVATE_
	_IRE_PRIVATE_
#endif
};

#if _BLD_bgp && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern Ire_t*		irecomp(const char*, int, int, int, Iredisc_t*);
extern int		ireexec(Ire_t*, void*, size_t);
extern int		irefree(Ire_t*);

#undef	extern

#endif
