/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1989-2002 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*                 Phong Vo <kpv@research.att.com>                  *
*******************************************************************/
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
