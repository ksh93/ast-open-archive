/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1985-2005 AT&T Corp.                  *
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
*                  David Korn <dgk@research.att.com>                   *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * basename(3) implementation
 */

#include <ast_map.h>
#include <libgen.h>

char *basename(register char *pathname)
{
	register char *first, *last;
	for(first=last=pathname; *last; last++);
	/* back over trailing '/' */
	if(last>first)
		while(*--last=='/' && last > first);
	if(last==first && *last=='/')
	{
		/* all '/' or "" */
		if(*first=='/')
			if(*++last=='/')	/* keep leading // */
				last++;
	}
	else
	{
		for(first=last++;first>pathname && *first!='/';first--);
		if(*first=='/')
			first++;
	}
	*last = 0;
	return(first);
}
