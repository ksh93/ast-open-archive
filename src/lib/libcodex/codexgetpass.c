/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2008 AT&T Intellectual Property          *
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
*                  David Korn <dgk@research.att.com>                   *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * prompt for password and return it in buf
 * return <  0 : error
 * return >= n : n-1 in buf, nul terminated
 */

#include <codex.h>
#include <sig.h>
#include <ast_tty.h>

ssize_t
codexgetpass(const char* prompt, void* buf, size_t n)
{
	char*		s;
	long		flags;
	ssize_t		r;
	Sfio_t*		rp;
	Sfio_t*		wp;
	Sig_handler_t	sigint;
	struct termios	tty;

	if (rp = sfopen(NiL, "/dev/tty", "r+"))
		wp = rp;
	else
	{
		rp = sfstdin;
		wp = sfstderr;
	}
	sigint = signal(SIGINT, SIG_IGN);
	tcgetattr(sffileno(rp), &tty);
	flags = tty.c_lflag;
	tty.c_lflag &= ~(ECHO|ECHONL);
	tcsetattr(sffileno(rp), TCSANOW, &tty);
	tty.c_lflag = flags;
	if (prompt)
	{
		sfprintf(wp, "%s", prompt);
		sfsync(wp);
	}
	if (s = sfgetr(rp, '\n', 1))
		r = strncopy((char*)buf, s, n) - (char*)buf;
	else
		r = -1;
	sfprintf(wp, "\n");
	sfsync(wp);
	tcsetattr(sffileno(rp), TCSANOW, &tty);
	signal(SIGINT, sigint);
	if (rp == wp)
		sfclose(rp);
	return r;
}
