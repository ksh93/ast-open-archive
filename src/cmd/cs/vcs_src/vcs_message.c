/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1990-2004 AT&T Corp.                  *
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
