/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 2003-2004 AT&T Corp.                *
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
*                David Korn <dgk@research.att.com>                 *
*                 Phong Vo <kpv@research.att.com>                  *
*                                                                  *
*******************************************************************/
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
