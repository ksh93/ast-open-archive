/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1996-2002 AT&T Corp.                *
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
