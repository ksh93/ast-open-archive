/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1998-2004 AT&T Corp.                  *
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
 * uuencode/uudecode private interface
 *
 * AT&T Research
 */

#ifndef _UULIB_H
#define _UULIB_H

#define _UU_PRIVATE_ \
	unsigned long	flags;	\
	int		mode;	\
	Sfio_t*		lp;	\
	Sfio_t*		ip;	\
	Sfio_t*		op;	\
	size_t		count;

#include "uu.h"

#define UU_CLOSEOUT	(1<<4)		/* close op when done		*/
#define UU_FREEPATH	(1<<5)		/* free path when done		*/
#define UU_HEADERMUST	(1<<6)		/* header must be called	*/
#define UU_LENGTH	(1<<7)		/* length encoded too		*/
#define UU_DEFAULT	(1<<8)		/* original method was default	*/
#define UU_METHOD	(1<<9)		/* first method specific bit	*/

typedef struct
{
	const char*	end;
	int		pad;
	int		fill;
	unsigned long	flags;
	int		size;
	const char	map[65];
} Uudata_t;

#endif
