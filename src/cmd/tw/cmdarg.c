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
*      If you have copied this software without agreeing       *
*      to the terms of the license you are infringing on       *
*         the license and copyright and are violating          *
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
 * AT&T Research
 *
 * xargs/tw command arg list support
 */

#include <ast.h>
#include <ctype.h>
#include <error.h>
#include <proc.h>

#include "cmdarg.h"

#ifndef EXIT_QUIT
#define EXIT_QUIT	255
#endif

static const char*	echo[] = { "echo", 0 };

/*
 * open a cmdarg stream
 * initialize the command for execution
 * argv[-1] is reserved for procopen(PROC_ARGMOD)
 */

Cmdarg_t*
cmdopen(char** argv, int argmax, int size, const char* argpat, int flags)
{
	register Cmdarg_t*	cmd;
	register int		n;
	register char**		p;
	register char*		s;
	char*			sh;
	int			c;
	int			m;
	int			argc;
	long			x;

	char**			post = 0;

	n = sizeof(char**);
	if (*argv)
	{
		for (p = argv + 1; *p; p++)
		{
			if ((flags & CMD_POST) && argpat && streq(*p, argpat))
			{
				*p = 0;
				post = p + 1;
				argpat = 0;
			}
			else
				n += strlen(*p) + 1;
		}
		argc = p - argv;
	}
	else
		argc = 0;
	for (p = environ; *p; p++)
		n += sizeof(char**) + strlen(*p) + 1;
	if ((x = strtol(astconf("ARG_MAX", NiL, NiL), NiL, 0)) <= 0)
		x = ARG_MAX;
	if (size <= 0 || size > x)
		size = x;
	sh = pathshell();
	m = n + (argc + 2) * sizeof(char**) + strlen(sh) + 2;
	if (size < m)
		error(3, "size must be at least %d", m);
	if ((m = x / 10) > 2048)
		m = 2048;
	if (size > (x - m))
		size = x - m;
	n = size - n;
	m = ((flags & CMD_INSERT) && argpat) ? (strlen(argpat) + 1) : 0;
	if (!(cmd = newof(0, Cmdarg_t, 1, n + m)))
		error(ERROR_SYSTEM|3, "out of space [arg handle]");
	c = n / sizeof(char**);
	if (argmax <= 0 || argmax > c)
		argmax = c;
	s = cmd->buf;
	if (!argv[0])
	{
		argv = (char**)echo;
		cmd->echo = 1;
	}
	else if (streq(argv[0], echo[0]))
	{
		cmd->echo = 1;
		flags &= ~CMD_NEWLINE;
	}
	else
	{
		if (!pathpath(s, argv[0], NiL, PATH_REGULAR|PATH_EXECUTE))
		{
			error(ERROR_SYSTEM|2, "%s: command not found", argv[0]);
			exit(EXIT_NOTFOUND);
		}
		argv[0] = s;
	}
	s += strlen(s) + 1;
	if (m)
	{
		cmd->insert = strcpy(s, argpat);
		cmd->insertlen = m - 1;
		s += m;
	}
	s += sizeof(char**) - (s - cmd->buf) % sizeof(char**);
	p = (char**)s;
	n -= strlen(*p++ = sh) + 1;
	cmd->argv = p;
	while (*p = *argv++)
		p++;
	if (m)
	{
		argmax = 1;
		*p++ = 0;
		cmd->insertarg = p;
		argv = cmd->argv;
		c = *cmd->insert;
		while (s = *argv)
		{
			while ((s = strchr(s, c)) && strncmp(cmd->insert, s, cmd->insertlen))
				s++;
			*p++ = s ? *argv : (char*)0;
			argv++;
		}
		*p++ = 0;
	}
	cmd->firstarg = cmd->nextarg = p;
	cmd->laststr = cmd->nextstr = cmd->buf + n;
	cmd->argmax = argmax;
	cmd->flags = flags;
	cmd->offset = ((cmd->postarg = post) ? (argc - (post - argv)) : 0) + 3;
	return cmd;
}

/*
 * flush outstanding command file args
 */

int
cmdflush(register Cmdarg_t* cmd)
{
	register char*	s;
	register char**	p;
	register int	n;

	if (cmd->nextarg <= cmd->firstarg)
	{
		if (!(cmd->flags & CMD_EMPTY))
			return 0;
		cmd->flags &= ~CMD_EMPTY;
	}
	if ((cmd->flags & CMD_MINIMUM) && cmd->argcount < cmd->argmax)
		error(3, "%d arg command would be too long", cmd->argcount);
	cmd->total.args += cmd->argcount;
	cmd->total.commands++;
	cmd->argcount = 0;
	if (p = cmd->postarg)
		while (*cmd->nextarg++ = *p++);
	else
		*cmd->nextarg = 0;
	if (s = cmd->insert)
	{
		char*	a;
		char*	b;
		char*	e;
		char*	t;
		char*	u;
		int	c;
		int	m;

		a = cmd->firstarg[0];
		b = (char*)&cmd->nextarg[1];
		e = cmd->nextstr;
		c = *s;
		m = cmd->insertlen;
		for (n = 1; cmd->argv[n]; n++)
			if (t = cmd->insertarg[n])
			{
				cmd->argv[n] = b;
				for (;;)
				{
					if (!(u = strchr(t, c)))
					{
						b += sfsprintf(b, e - b, "%s", t);
						break;
					}
					if (!strncmp(s, u, m))
					{
						b += sfsprintf(b, e - b, "%-.*s%s", u - t, t, a);
						t = u + m;
					}
					else t = u + 1;
				}
				if (b < e)
					*b++ = 0;
			}
		if (b >= e)
			error(3, "%s: command too large after insert", a);
	}
	cmd->nextarg = cmd->firstarg;
	cmd->nextstr = cmd->laststr;
	if (cmd->flags & (CMD_QUERY|CMD_TRACE))
	{
		p = cmd->argv;
		sfprintf(sfstderr, "+ %s", *p);
		while (s = *++p)
			sfprintf(sfstderr, " %s", s);
		if (!(cmd->flags & CMD_QUERY))
			sfprintf(sfstderr, "\n");
		else if (astquery(1, "? "))
			return 0;
	}
	if (cmd->echo)
	{
		n = (cmd->flags & CMD_NEWLINE) ? '\n' : ' ';
		for (p = cmd->argv + 1; s = *p++;)
			sfputr(sfstdout, s, *p ? n : '\n');
		n = 0;
	}
	else if ((n = procclose(procopen(*cmd->argv, cmd->argv, NiL, NiL, PROC_ARGMOD))) == -1)
	{
		error(ERROR_SYSTEM|2, "%s: command exec error", *cmd->argv);
		exit(EXIT_NOTFOUND - 1);
	}
	else if (n >= EXIT_NOTFOUND - 1)
		exit(n);
	else if (!(cmd->flags & CMD_IGNORE))
	{
		if (n == EXIT_QUIT)
			exit(2);
		if (n)
			error_info.errors++;
	}
	return n;
}

/*
 * add file to the command arg list
 */

int
cmdarg(register Cmdarg_t* cmd, const char* file, register int len)
{
	int	r;

	if (len)
	{
		while ((cmd->nextstr -= len + 1) < (char*)(cmd->nextarg + cmd->offset))
		{
			if (cmd->nextarg == cmd->firstarg)
				error(3, "%s: path too long for exec args", file);
			if (r = cmdflush(cmd))
				return r;
		}
		*cmd->nextarg++ = cmd->nextstr;
		memcpy(cmd->nextstr, file, len);
		cmd->nextstr[len] = 0;
		cmd->argcount++;
		if (cmd->argcount >= cmd->argmax)
			return cmdflush(cmd);
	}
	return 0;
}

/*
 * close a cmdarg stream
 */

int
cmdclose(Cmdarg_t* cmd)
{
	int	n;

	if ((cmd->flags & CMD_MINIMUM) && cmd->argcount < cmd->argmax)
		error(3, "%d: not enough arguments for last command", cmd->argcount);
	n = cmdflush(cmd);
	free(cmd);
	return n;
}
