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
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * common include reference handler
 * the type arg is inclusive or of PP_SYNC_*
 */

#include "pplib.h"

void
ppincref(char* parent, char* file, int line, int type)
{
	register struct ppinstk*	sp;
	int				level;

	NoP(parent);
	NoP(line);
	if (type & PP_SYNC_PUSH)
	{
		level = 0;
		for (sp = pp.in; sp; sp = sp->prev)
			if (sp->type == IN_FILE)
				level++;
		if (level > 0)
			level--;
		error(0, "%-*s%s", level * 4, "", file);
	}
}
