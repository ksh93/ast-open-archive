/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                  Copyright (c) 2000 AT&T Corp.                   *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*                 This software was created by the                 *
*                 Network Services Research Center                 *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Labs Research
 */

static const char usage[] =
"[-?\n@(#)msgget (AT&T Labs Research) 2000-05-17\n]"
USAGE_LICENSE
"[+NAME?msgget - get a message from a message catalog]"
"[+DESCRIPTION?\bmsgget\b gets the message corresponding to the parameters."
"	If \alocale\a is \b-\b then the current locale is used. \acommand\a"
"	may be specified for command specific messages. \acatalog\a specifies"
"	the message catalog name. [\aset\a.]]\anumber\a identifies the message"
"	by message \anumber\a and an optional message \aset\a; if specified as"
"	\b-\b then the message set and number are determined by looking up"
"	\atext\a in the corresponding \bC\b locale message catalog.]"

"\n"
"\nlocale [command:]catalog [set.]number [ text ]\n"
"\n"

"[+SEE ALSO?\biconv\b(1), \bmsgcc\b(1), \bmsggen\b(1)]"
;

#include <ast.h>
#include <error.h>
#include <mc.h>

int
main(int argc, char** argv)
{
	register Mc_t*	mc;
	register char*	s;
	char*		t;
	char*		e;
	char*		loc;
	char*		cmd;
	char*		cat;
	char*		msg;
	int		set;
	int		num;
	Sfio_t*		sp;
	char		path[PATH_MAX];

	NoP(argc);
	error_info.id = "msgget";
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			continue;
		case ':':
			error(2, "%s", opt_info.arg);
			continue;
		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors || !(loc = *argv++) || !(cmd = *argv++) || !(s = *argv++))
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	set = num = 0;
	if (!streq(s, "-"))
	{
		if (t = strchr(s, '.'))
		{
			*t++ = 0;
			set = (int)strtol(s, &e, 0);
			if (*e)
				set = MC_MESSAGE_SET(s);
			s = t;
		}
		else
			set = 1;
		num = (int)strtol(s, &t, 0);
	}
	if (!(msg = *argv++))
		msg = "";
	else if (*argv)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	if (streq(loc, "-"))
		loc = 0;
	if (cat = strchr(cmd, ':'))
		*cat++ = 0;
	if (!mcfind(path, loc, cmd, LC_MESSAGES, 0) && (!cat || !mcfind(path, loc, cat, LC_MESSAGES, 0)))
	{
		if (cat)
			*--cat = ':';
		error(3, "%s: cannot locate message catalog", cmd);
	}
	if (!(sp = sfopen(NiL, path, "r")))
		error(ERROR_SYSTEM|3, "%s: cannot read message catalog", path);
	if (!(mc = mcopen(sp)))
		error(3, "%s: invalid message catalog", path);
	if (set)
		s = mcget(mc, set, num, msg);
	else
		s = errorx(loc, cmd, cat, msg);
	sfputr(sfstdout, s, '\n');
	return error_info.errors != 0;
}
