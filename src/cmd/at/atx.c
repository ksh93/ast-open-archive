/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1996-2004 AT&T Corp.                  *
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
 * atx -- the at impersonator
 *
 *	cd AT_JOB_DIR
 *	$PWD/AT_EXEC_FILE $SHELL <job>
 */

static const char id[] = "\n@(#)$Id: atx (AT&T Research) 2000-06-16 $\0\n";

#include "at.h"

int
main(int argc, char** argv)
{
	register int	n = 0;
	unsigned long	uid;
	unsigned long	gid;
	unsigned long	tid;
	struct stat	ds;
	struct stat	js;
	struct stat	xs;

	error_info.id = "atx";
	if (argc != 3 ||
	    ++n && lstat(".", &ds) ||
	    ++n && !AT_DIR_OK(&ds) ||
	    ++n && lstat(argv[2], &js) ||
	    ++n && !AT_JOB_OK(&ds, &js) ||
	    ++n && !S_ISREG(js.st_mode) ||
	    ++n && lstat(AT_EXEC_FILE, &xs) ||
	    ++n && !AT_EXEC_OK(&ds, &xs) ||

	    ++n && sfsscanf(argv[2], "%..36lu.%..36lu.%..36lu", &uid, &gid, &tid) != 3)
		error(3, "%s: command garbled [%d]", argc >= 3 ? argv[2] : (char*)0, n);
	if (setuid(uid))
		error(3, "%s: user denied", argv[2]);
	if (setgid(gid))
		error(3, "%s: group denied", argv[2]);
	argv++;
	execvp(argv[0], argv);
	error(3, "%s: exec failed", argv[2]);
	return 1;
}
