/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1986-2004 AT&T Corp.                  *
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
