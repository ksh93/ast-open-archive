/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1989-2004 AT&T Corp.                  *
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
 * AT&T Bell Laboratories
 * mamexec state support
 *
 * mamstate reference [ file ... | <files ]
 *
 * stdout is list of <file,delta> pairs where delta
 * is diff between reference and file times
 * non-existent files are not listed
 */

static const char id[] = "\n@(#)$Id: mamstate (AT&T Bell Laboratories) 1989-06-26 $\0\n";

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

main(int argc, register char** argv)
{
	register char*	s;
	char*		id;
	unsigned long	ref;
	struct stat	st;
	char		buf[1024];

	id = "mamstate";
	if (!(s = *++argv))
	{
		fprintf(stderr, "%s: reference file argument expected\n", id);
		return 1;
	}
	if (stat(s, &st))
	{
		fprintf(stderr, "%s: %s: cannot stat\n", id, s);
		return 1;
	}
	ref = (unsigned long)st.st_mtime;
	if (s = *++argv)
		do
		{
			if (!stat(s, &st))
				printf("%s %ld\n", s, (unsigned long)st.st_mtime - ref);
		} while (s = *++argv);
	else
		while (s = fgets(buf, sizeof(buf), stdin))
			if (!stat(s, &st))
				printf("%s %ld\n", s, (unsigned long)st.st_mtime - ref);
	return 0;
}
