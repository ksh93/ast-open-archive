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
 * Glenn Fowler
 * AT&T Bell Laboratories
 *
 * touch -- touch file times
 */

static const char usage[] =
"[-?\n@(#)$Id: touch (AT&T Labs Research) 2002-09-11 $\n]"
USAGE_LICENSE
"[+NAME?touch - change file access and modification times]"
"[+DESCRIPTION?\btouch\b changes the modification time, access time or both"
"	of each \afile\a. The modification time is the \ast_mtime\a member of"
"	the \bstat\b(2) information and the access time is the \ast_atime\a"
"	member.]"
"[+?If neither the \b--reference\b nor the \b--time\b options are specified"
"	then the time used will be the \adate\a operand or the current time"
"	if \adate\a is omitted.]"
"[+?If the \adate\a operand consists of 4, 6, 8, 10 or 12 digits followed"
"	by an optional \b.\b and two digits then it is interpreted as:"
"	\aHHMM.SS\a, \addHHMM.SS\a, \ammddHHMM.SS\a, \ammddHHMMyy.SS\a or"
"	\ayymmddHHMM.SS\a, or \ammddHHMMccyy.SS\a or \accyymmddHHMM.SS\a."
"	Conflicting standards and practice allow a leading or trailing"
"	2 or 4 digit year for the 10 and 12 digit forms; the X/Open leading"
"	form is used to disambiguate (\bdate\b(1) uses the trailing form.)"
"	Avoid the 10 digit form to avoid confusion. The digit fields are:]{"
"		[+cc?Century - 1, 19-20.]"
"		[+yy?Year in century, 00-99.]"
"		[+mm?Month, 01-12.]"
"		[+dd?Day of month, 01-31.]"
"		[+HH?Hour, 00-23.]"
"		[+MM?Minute, 00-59.]"
"		[+SS?Seconds, 00-60.]"
"}"

"[a:access|atime|use?Change the access time. Do not change the modification"
"	time unless \b--modify\b is also specified.]"
"[n:change|ctime|neither?Change only the file status change time \ast_ctime\a."
"	Most systems only allow \ast_ctime\a to be set to the current time.]"
"[c!:create?Create the \afile\a if it does not exist, but write no diagnostic.]"
"[f:force?Ignored by this implementation.]"
"[m:modify|mtime|modification?Change the modify time. Do not change the"
"	access time unless \b--access\b is also specified.]"
"[r:reference?Use the corresponding time of \afile\a instead of the current"
"	time.]:[file]"
"[t|d:time|date?Use the specified \adate-time\a instead of the current"
"	date-time. Most common formats are accepted. If \adate-time\a consists"
"	of 4, 6, 8, 10 or 12 digits followed by an optional \b.\b and 2 digits"
"	then it is interpreted as the \adate\a operand above, except that the"
"	leading 2 or 4 digit year form is used to disambiguate. Avoid the 10"
"	digit form to avoid confusion. If \b--reference\b is specified or if"
"	\afile\a already exists then \atime\a may also be one of:]:[date-time]{"
"		[+access|atime|use?The access time of the reference file.]"
"		[+change|ctime?The change time of the reference file.]"
"		[+modify|mtime|modification?The modify time of the reference"
"			file.]"
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

#define ATIME		01
#define CTIME		02
#define MTIME		04

#define SAME		((time_t)(-1))

int
main(int argc, register char** argv)
{
	register char*	reference = 0;
	int		create = 1;
	int		set = 0;
	int		verbatim = 0;
	int		verbose = 0;
	time_t		date = 0;
	time_t		atime = SAME;
	time_t		mtime = SAME;
	time_t*		use = 0;

	register char*	file;
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
			set |= ATIME;
			continue;
		case 'c':
			create = 0;
			continue;
		case 'd':
		case 't':
			verbatim = 1;
			if (streq(opt_info.arg, "access") || streq(opt_info.arg, "atime") || streq(opt_info.arg, "use"))
				use = &st.st_atime;
			else if (streq(opt_info.arg, "change") || streq(opt_info.arg, "ctime"))
				use = &st.st_ctime;
			else if (streq(opt_info.arg, "modify") || streq(opt_info.arg, "mtime") || streq(opt_info.arg, "modification"))
				use = &st.st_mtime;
			else
			{
				reference = 0;
				date = tmdate(opt_info.arg, &s, NiL);
				if (*s)
					error(3, "%s: invalid date specification", s);
			}
			continue;
		case 'f':
			continue;
		case 'm':
			set |= MTIME;
			continue;
		case 'n':
			set |= CTIME;
			continue;
		case 'r':
			verbatim = 1;
			reference = opt_info.arg;
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
	if (!reference)
	{
		if (!verbatim)
		{
			for (file = *argv; *file >= '0' && *file <= '9'; file++);
			if (((n = (file - *argv)) == 4 || n == 6 || n == 8 || n == 10 || n == 12) && (!*file || *file == '.' && *(file + 1) >= '0' && *(file + 1) <= '9' && *(file + 2) >= '0' && *(file + 2) <= '9' && !*(file + 3)))
			{
				date = tmdate(file = *argv++, &e, NiL);
				if (*e)
					error(3, "%s: invalid date specification", file);
				verbatim = 1;
			}
		}
		st.st_atime = st.st_mtime = date;
	}
	else if (stat(reference, &st))
		error(ERROR_SYSTEM|3, "%s: not found", reference);
	if (set & CTIME)
		set = 0;
	else if (!set)
		set = MTIME;
	if (set & ATIME)
		atime = use ? *use : st.st_atime;
	if (set & MTIME)
		mtime = use ? *use : st.st_mtime;
	if (use && reference)
		use = 0;
	while (file = *argv++)
	{
		if (use)
		{
			if (stat(file, &st))
			{
				error(2, "%s: not found", file);
				continue;
			}
			if (set & ATIME)
				atime = *use;
			if (set & MTIME)
				mtime = *use;
		}
		if (touch(file, atime, mtime, verbatim && ((set & ATIME) && (atime == (time_t)0 || atime == (time_t)-1) || (set & MTIME) && (mtime == (time_t)0 || mtime == (time_t)-1)) ? -create : create))
			if (errno != ENOENT)
				error(ERROR_SYSTEM|2, "%s: cannot touch", file);
			else if (verbose)
				error(1, "%s: not found", file);
	}
	return error_info.errors != 0;
}
