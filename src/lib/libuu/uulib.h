/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1998-2002 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*    If you have copied or used this software without agreeing     *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
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
