/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1985-2004 AT&T Corp.                  *
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
*                  David Korn <dgk@research.att.com>                   *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * David Korn
 * AT&T Research
 *
 * Interface definitions for a stack-like storage library
 *
 */

#ifndef _STAK_H
#define _STAK_H

#include	<stk.h>

#define Stak_t		Sfio_t
#define	staksp		stkstd
#define STAK_SMALL	STK_SMALL

#define	stakptr(n)		stkptr(stkstd,n)
#define	staktell()		stktell(stkstd)
#define stakputc(c)		sfputc(stkstd,(c))
#define stakwrite(b,n)		sfwrite(stkstd,(b),(n))
#define stakputs(s)		(sfputr(stkstd,(s),0),--stkstd->_next)
#define stakseek(n)		stkseek(stkstd,n)
#define stakcreate(n)		stkopen(n)
#define stakinstall(s,f)	stkinstall(s,f)
#define stakdelete(s)		stkclose(s)
#define staklink(s)		stklink(s)
#define stakalloc(n)		stkalloc(stkstd,n)
#define stakcopy(s)		stkcopy(stkstd,s)
#define stakset(c,n)		stkset(stkstd,c,n)
#define stakfreeze(n)		stkfreeze(stkstd,n)

#endif
