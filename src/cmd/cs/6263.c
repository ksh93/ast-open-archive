/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1990-2000 AT&T Corp.              *
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
 * simple name service on port 6263 (use the phone keypad luke)
 * client sends cs connect stream path
 * server sends back /dev/tcp/.../... path
 */

static const char id[] = "@(#)cs.6263 (AT&T Research) 2000-05-11\0\n";

#include <css.h>
#include <error.h>

static int
actionf(register Css_t* css, register Cssfd_t* fp, Cssdisc_t* disc)
{
	register ssize_t	n;
	register char*		s;
	register char*		t;
	int			ud;

	static char		path[PATH_MAX];

	if (fp->status == CS_POLL_READ)
	{
		if ((n = csread(css->state, fp->fd, path, sizeof(path) - 1, CS_LINE)) <= 1)
			return -1;
		path[n - 1] = 0;
		if ((ud = csopen(css->state, path, CS_OPEN_READ)) >= 0)
		{
			s = cspath(css->state, ud, 0);
			if (t = strrchr(s, '.'))
				*t = 0;
			close(ud);
		}
		else
			s = 0;
		if (s)
			n = strlen(s);
		else
		{
			s = path;
			n = 0;
		}
		s[n++] = '\n';
		cswrite(css->state, fp->fd, s, n);
	}
	return -1;
}

static int
exceptf(Css_t* css, unsigned long op, unsigned long arg, Cssdisc_t* disc)
{
	switch (op)
	{
	case CSS_INTERRUPT:
		error(ERROR_SYSTEM|3, "%s: interrupt exit", fmtsignal(arg));
		return 0;
	case CSS_DORMANT:
		error(2, "dormant exit");
		exit(0);
	}
	error(ERROR_SYSTEM|3, "poll error op=0x%08x arg=0x%08x", op, arg);
	return -1;
}

int
main(int argc, char** argv)
{
	Css_t*		css;
	Cssdisc_t	disc;

	NoP(argc);
	memset(&disc, 0, sizeof(disc));
	disc.version = CSS_VERSION;
	disc.flags = CSS_DAEMON|CSS_ERROR|CSS_DORMANT|CSS_INTERRUPT;
	disc.timeout = 20 * 60 * 1000L;
	disc.actionf = actionf;
	disc.errorf = (Csserror_f)errorf;
	disc.exceptf = exceptf;
	if (!(css = cssopen(argv[1], &disc)))
		exit(1);
	error_info.id = css->id;
	csspoll(CS_NEVER, 0);
	exit(1);
}
