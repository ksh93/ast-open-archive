/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1996-2010 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
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
	setsid();
	argv++;
	execvp(argv[0], argv);
	error(3, "%s: exec failed", argv[2]);
	return 1;
}
