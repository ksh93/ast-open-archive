/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1990-2005 AT&T Corp.                  *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                            by AT&T Corp.                             *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#pragma prototyped

#include "vcs_rscs.h"

extern int	debug;

static void tracev(int level, va_list ap)
{
	char*	format;
	char	buf[1024];

	if (level && ((!debug || (level > debug))))
		return;

	format = va_arg(ap, char *);
	vsprintf(buf, format, ap);
	if (!level)
		printf("%s\n", buf);
	else
		printf("[%d] %s\n", level, buf);
}

void trace(int level, ...)
{
	va_list		ap;
	
	va_start(ap, level);
	tracev(level, ap);
	va_end(ap);
}
