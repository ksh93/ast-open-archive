/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 2003-2006 AT&T Corp.                  *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                            by AT&T Corp.                             *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#ifndef _VCSFIO_H
#define _VCSFIO_H	1

#include <vcodex.h>

#define VCSF_TRAINSIZE		(1024*1024)

typedef struct Vcsfalias_s Vcsfalias_t;
struct Vcsfalias_s
{
	char*		name;	/* alias name		*/
	char*		meth;	/* alias expansion	*/
#ifdef _VCSFALIAS_PRIVATE_
	_VCSFALIAS_PRIVATE_
#endif
};

/* vcsfio definitions */

_BEGIN_EXTERNS_

#if _BLD_vcodex && defined(__EXPORT__)
#define extern		extern __EXPORT__
#endif

typedef int (*Vcsferror_f) _ARG_((void*, void*, int, ...));

extern Vcsfmeth_t*	vcsfcomp _ARG_((const char*, ssize_t*, char**, Vcsferror_f));
extern Vcsfmeth_t*	vcsfbest _ARG_((const char*, ssize_t*, int, ssize_t, char**, Vcsferror_f));
extern Vcsfmeth_t*	vcsfdup _ARG_((Vcsfmeth_t*, ssize_t));
extern Vcsfalias_t*	vcsfgetalias _ARG_((const char*, char**));
extern Vcsfalias_t*	vcsfnextalias _ARG_((Vcsfalias_t*));
extern ssize_t		vcsfgetwindow _ARG_((const char*, Vcwmethod_t**, char**, Vcsferror_f));
extern int		sfdcvcodex _ARG_((Sfio_t*, Vcsfmeth_t*, ssize_t, unsigned int));

#undef	extern

_END_EXTERNS_

#endif /*_VCSFIO_H*/
