/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 2003-2004 AT&T Corp.                  *
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
