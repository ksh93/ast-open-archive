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
 * ff -- fast find test harness
 */

static const char id[] = "\n@(#)ff (AT&T Research) 1998-02-26\0\n";

#include <ast.h>
#include <error.h>
#include <find.h>
#include <ls.h>

static struct
{
	int		list;
	unsigned long	total;
	unsigned long	dirs;
	unsigned long	changed;
	unsigned long	error;
} state;

/*
 * count directories
 */

static int
count(Find_t* fp, const char* path, size_t len, Finddisc_t* disc)
{
	state.dirs++;
	if (state.list)
		sfprintf(sfstdout, "%s\n", path);
	return 0;
}

/*
 * verify directories
 */

static int
verify(Find_t* fp, const char* path, size_t len, Finddisc_t* disc)
{
	struct stat	st;

	if (stat(path, &st))
	{
		state.error++;
		if (state.list)
			sfprintf(sfstdout, "%s\n", path);
	}
	else if ((unsigned long)st.st_mtime > fp->stamp)
	{
		state.changed++;
		if (state.list)
			sfprintf(sfstdout, "%s\n", path);
	}
	else
		state.dirs++;
	return 0;
}

int
main(int argc, register char** argv)
{
	register char*	s;
	int		list = 1;
	char*		codes = 0;
	char*		pattern = "*";
	char*		type = 0;
	Find_t*		fp;
	Finddisc_t	disc;
	static char*	dot[] = { ".", 0 };

	setlocale(LC_ALL, "");
	error_info.id = "ff";
	state.list = 1;
	memset(&disc, 0, sizeof(disc));
	disc.version = FIND_VERSION;
	disc.errorf = (Finderror_f)errorf;
	for (;;)
	{
		switch (optget(argv, "cf:[pattern]mnt:[type]F:[codes]I dir ..."))
		{
		case 'c':
			disc.verifyf = count;
			list = 0;
			continue;
		case 'f':
			pattern = opt_info.arg;
			continue;
		case 'm':
			disc.verifyf = verify;
			list = 0;
			continue;
		case 'n':
			state.list = list = 0;
			continue;
		case 't':
			type = opt_info.arg;
			continue;
		case 'F':
			codes = opt_info.arg;
			continue;
		case 'I':
			disc.flags |= FIND_ICASE;
			continue;
		case '?':
			error(ERROR_USAGE|4, opt_info.arg);
			continue;
		case ':':
			error(2, opt_info.arg);
			continue;
		}
		break;
	}
	argv += opt_info.index;
	argc -= opt_info.index;
	if (error_info.errors)
		error(ERROR_USAGE|4, optusage(NiL));
	disc.dirs = *argv ? argv : dot;
	if (!(fp = findopen(codes, pattern, type, &disc)))
		exit(1);
	while (s = findread(fp))
	{
		state.total++;
		if (list)
			sfputr(sfstdout, s, '\n');
	}
	findclose(fp);
	sfprintf(sfstdout, "total %lu", state.total);
	if (state.dirs)
		sfprintf(sfstdout, " dirs %lu", state.dirs);
	if (state.changed)
		sfprintf(sfstdout, " changed %lu", state.changed);
	if (state.error)
		sfprintf(sfstdout, " error %lu", state.error);
	sfprintf(sfstdout, "\n");
	exit(error_info.errors != 0);
}
