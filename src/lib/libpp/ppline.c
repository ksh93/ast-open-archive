/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1986-2002 AT&T Corp.                *
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
 * Glenn Fowler
 * AT&T Research
 *
 * common preprocessor line sync handler
 */

#include "pplib.h"

void
ppline(int line, char* file)
{
	char*		s;
	static char	type[5];

	if (pp.flags & PP_lineignore)
	{
		pp.flags &= ~PP_lineignore;
		if (!(pp.flags & PP_linetype) || *pp.lineid)
		{
			ppline(1, file);
			file = error_info.file;
		}
		else
			type[1] = PP_sync_ignore;
	}
	else if (file != pp.lastfile)
	{
		if (!pp.firstfile)
			pp.firstfile = file;
		type[1] = ((pp.flags & PP_linetype) && !*pp.lineid && pp.lastfile) ? (line <= 1 ? (file == pp.firstfile ? PP_sync : PP_sync_push) : PP_sync_pop) : PP_sync;
		pp.lastfile = file;
	}
	else
	{
		if (!(pp.flags & PP_linefile))
			file = 0;
		type[1] = PP_sync;
	}
	if (!(pp.flags & PP_linetype) || *pp.lineid || type[1] == PP_sync)
		type[0] = 0;
	else
	{
		type[0] = ' ';
		if ((pp.flags & (PP_hosted|PP_linehosted)) == (PP_hosted|PP_linehosted))
		{
			type[2] = ' ';
			type[3] = PP_sync_hosted;
		}
		else
			type[2] = 0;
	}

	/*
	 * some front ends can't handle two line syncs in a row
	 */

	if (pp.pending == pppendout())
		ppputchar('\n');
	if (file)
		ppprintf("#%s %d \"%s\"%s\n", pp.lineid, line, (pp.flags & PP_linebase) && (s = strrchr(file, '/')) ? s + 1 : file, type);
	else
		ppprintf("#%s %d\n", pp.lineid, line);
	if (!pp.macref)
		pp.pending = pppendout();
}
