/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1986-2000 AT&T Corp.                *
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
*                 This software was created by the                 *
*                 Network Services Research Center                 *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * common preprocessor pragma handler
 */

#include "pplib.h"

void
pppragma(char* directive, char* pass, char* name, char* value, int newline)
{
	register int	sep = 0;

	if (directive)
	{
		ppprintf("#%s", directive);
		sep = 1;
	}
	if (pass)
	{
		if (sep)
		{
			sep = 0;
			ppprintf(" ");
		}
		ppprintf("%s:", pass);
	}
	if (name)
	{
		if (sep) ppprintf(" ");
		ppprintf("%s", name);
		sep = 1;
	}
	if (value)
	{
		if (sep || pass) ppprintf(" ");
		ppprintf("%s", value);
	}
	if (newline) ppprintf("\n");
}
