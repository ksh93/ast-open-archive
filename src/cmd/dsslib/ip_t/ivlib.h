/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2000-2009 AT&T Intellectual Property          *
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
#ifndef _IVHDR_H
#define _IVHDR_H	1

#if _PACKAGE_ast || _PACKAGE_astsa
#include		<ast.h>
#else

extern void*		malloc _ARG_((size_t));
extern void*		calloc _ARG_((size_t, size_t));
extern void		free _ARG_((void*));

extern void		abort _ARG_((void));

#endif

#include		<iv.h>
#include		<ivmethods.h>
#include		<cdt.h>

#ifndef NIL
#define NIL(t)		((t)0)
#endif

#ifndef OFFSET
#define OFFSET(t,e)	((size_t)(&(((t*)0)->e)) )
#endif

#ifndef ASSERT
#ifdef DEBUG
#define	ASSERT(p)	((p) ? 0 : (abort(),0))
#else
#define ASSERT(p)
#endif
#endif

#endif /* _IVHDR_H */
