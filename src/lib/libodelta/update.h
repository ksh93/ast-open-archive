/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2011 AT&T Intellectual Property          *
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
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#pragma prototyped
#ifndef _UPDATE_H
#define _UPDATE_H

#include <ast.h>

/* values for the instruction field */
#define DELTA_TYPE	0300
#define DELTA_MOVE	0200
#define DELTA_ADD	0100

/* number of bytes required to code a value */
#define BASE		256
#define ONE		(BASE)
#define TWO		(BASE*BASE)
#define THREE		(BASE*BASE*BASE)
#define NBYTE(v)	((v) < ONE ? 1 : ((v) < TWO ? 2 : ((v) < THREE ? 3 : 4)))

#define BUFSIZE	2048

#ifndef NULL
#define NULL	(0L)
#endif

extern int		delta(char*, long, char*, long, int);
extern long		mtchstring(char*, long, char*, long, char**);
extern int		update(int, long, int, int);

#endif
