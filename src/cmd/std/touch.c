/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1989-2000 AT&T Corp.              *
*      and it may only be used by you under license from       *
*                     AT&T Corp. ("AT&T")                      *
*       A copy of the Source Code Agreement is available       *
*              at the AT&T Internet web site URL               *
*                                                              *
*     http://www.research.att.com/sw/license/ast-open.html     *
*                                                              *
*     If you received this software without first entering     *
*       into a license with AT&T, you have an infringing       *
*           copy and cannot use it without violating           *
*             AT&T's intellectual property rights.             *
*                                                              *
*               This software was created by the               *
*               Network Services Research Center               *
*                      AT&T Labs Research                      *
*                       Florham Park NJ                        *
*                                                              *
*             Glenn Fowler <gsf@research.att.com>              *
*                                                              *
***************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Bell Laboratories
 *
 * touch -- touch file times
 */

static const char usage[] =
"[-?\n@(#)touch (AT&T Labs Research) 2000-01-25\n]"
USAGE_LICENSE
"[+NAME?touch - change file access and modification times]"
"[+DESCRIPTION?\btouch\b changes the modification time, access time or both"
"	of each \afile\a. The modification time is the \ast_mtime\a member of"
"	the \bstat\b(2) information and the access time is the \ast_atime\a"
"	member.]"
"[+?If neither the \b--reference\b nor the \b--time\b options are specified"
"	then the time used will be the \adate\a operand or the current time"
"	if \adate\a is omitted.]"

"[a:access|atime|use?Change the access time. Do not change the modification"
"	time unless \b--modify\b is also specified.]"
"[n:change|ctime|neither?Change only the file status change time \ast_ctime\a.]"
"[c!:create?Create the \afile\a if it does not exist, but write no diagnostic.]"
"[f:force?Ignored by this implementation.]"
"[m:modify|mtime|modification?Change the modify time. Do not change the"
"	access time unless \b--access\b is also specified.]"
"[r:reference?Use the corresponding time of \afile\a instead of the current"
"	time.]:[file]"
"[t|d:time|date?Use the specified \atime\a instead of the current time. Most"
"	common date formats are accepted. See \bdate\b(1) for details. For"
"	compatibility, \atime\a may also be one of:]:[time]{"
"		[+access|atime|use?Equivalent to \b--access\b.]"
"		[+modify|mtime|modification?Equivalent to \b--modify\b.]"
"		[+neither|none?Equivalent to \b--neither\b.]"
"}"
"[v:verbose?Write a diagnostic for each nonexistent \afile\a.]"

"\n"
"\n [ date ] file ...\n"
"\n"

"[+CAVEATS?Some systems or file system types may limit the range of times that"
"	can be set. These limitations may not show up until a subsequent"
"	\bstat\b(2) call (yes, the time can be set but not checked!) Upper"
"	limits of <0xffffffff and <0x7fffffff have been observed.]"
"[+SEE ALSO?\bdate\b(1), \bnmake\b(1), \butime\b(2), \btm\b(3)]"
;

#include <ast.h>
#include <ls.h>
#include <tm.h>
#include <error.h>

#define SAME	((time_t)(-1))

int
main(int argc, register char** argv)
{
	register char*	file = 0;
	int		create = 1;
	int		modify = 0;
	int		verbose = 0;
	time_t		date = 0;
	time_t		atime = SAME;
	time_t		mtime = 0;

	char*		s;
	char*		e;
	int		n;
	struct stat	st;

	NoP(argc);
	error_info.id = "touch";
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'a':
			atime = 0;
			mtime = SAME;
			continue;
		case 'c':
			create = 0;
			continue;
		case 'd':
		case 't':
			if (streq(opt_info.arg, "access") || streq(opt_info.arg, "atime") || streq(opt_info.arg, "use"))
				atime = 0;
			else if (streq(opt_info.arg, "modify") || streq(opt_info.arg, "mtime") || streq(opt_info.arg, "modification"))
				modify = 1;
			else if (streq(opt_info.arg, "change") || streq(opt_info.arg, "ctime") || streq(opt_info.arg, "neither"))
				atime = mtime = SAME;
			else
			{
				file = 0;
				date = tmdate(opt_info.arg, &s, NiL);
				if (*s)
					error(3, "%s: invalid date specification", s);
			}
			continue;
		case 'f':
			continue;
		case 'm':
			modify = 1;
			continue;
		case 'n':
			atime = mtime = SAME;
			continue;
		case 'r':
			file = opt_info.arg;
			date = 0;
			continue;
		case 'v':
			verbose = 1;
			continue;
		case ':':
			error(2, "%s", opt_info.arg);
			break;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			break;
		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors || !*argv)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	if (!file)
	{
		if (!date)
		{
			for (file = *argv; *file >= '0' && *file <= '9'; file++);
			if (!*file && ((n = (file - *argv)) == 6 || n == 8))
			{
				date = tmscan(file = *argv++, &s, n == 6 ? "%m%d%H" : "%m%d%H%y", &e, NiL, 0);
				if (*s || *e)
					error(3, "%s: invalid date specification", file);
			}
		}
		st.st_atime = st.st_mtime = date;
	}
	else if (stat(file, &st))
		error(3, "%s: not found", file);
	if (!atime)
		atime = st.st_atime;
	if (!mtime || modify)
		mtime = st.st_mtime;
	while (file = *argv++)
		if (touch(file, atime, mtime, create))
			if (errno != ENOENT)
				error(ERROR_SYSTEM|2, "%s: cannot touch", file);
			else if (verbose)
				error(1, "%s: not found", file);
	return error_info.errors != 0;
}
