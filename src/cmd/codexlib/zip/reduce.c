/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1986-2011 AT&T Intellectual Property          *
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
 * zip reduce decoder
 */

#include "zip.h"

static int
reduce_open(Codex_t* p, char* const args[], Codexnum_t flags)
{
	return -1;
}

static int
reduce_init(Codex_t* p)
{
	return -1;
}

static ssize_t
reduce_read(Sfio_t* sp, void* buf, size_t n, Sfdisc_t* disc)
{
	return -1;
}

Codexmeth_t	codex_zip_reduce =
{
	"reduce",
	"zip reduce compression (PKZIP methods 2-5).",
	0,
	CODEX_DECODE|CODEX_COMPRESS,
	0,
	0,
	reduce_open,
	0,
	reduce_init,
	0,
	reduce_read,
	0,
	0,
	0,
	0,
	0,
	0,
	0
};
