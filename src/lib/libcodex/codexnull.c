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
 * return null write stream for pure codexdata()
 */

#include <sfio_t.h>
#include <codex.h>

static ssize_t
nullread(Sfio_t* f, void* buf, size_t n, Sfdisc_t* disc)
{
	return 0;
}

static ssize_t
nullwrite(Sfio_t* f, const void* buf, size_t n, Sfdisc_t* disc)
{
	return n;
}

static Sfdisc_t		nulldisc = { nullread, nullwrite };

static const char	nullbuf[1];

static Sfio_t		null = SFNEW(nullbuf, 0, 0, SF_WRITE, &nulldisc, 0);

Sfio_t*
codexnull(void)
{
	return &null;
}
