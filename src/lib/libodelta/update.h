/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1989-2004 AT&T Corp.                  *
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
