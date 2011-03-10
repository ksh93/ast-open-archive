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
 * zip shrink decoder
 */

#include "zip.h"

static int
shrink_open(Codex_t* p, char* const args[], Codexnum_t flags)
{
	return -1;
}

static int
shrink_init(Codex_t* p)
{
	return -1;
}

static ssize_t
shrink_read(Sfio_t* sp, void* buf, size_t n, Sfdisc_t* disc)
{
	return -1;
}

Codexmeth_t	codex_zip_shrink =
{
	"shrink",
	"zip shrink compression (PKZIP method 1).",
	0,
	CODEX_DECODE|CODEX_COMPRESS,
	0,
	0,
	shrink_open,
	0,
	shrink_init,
	0,
	shrink_read,
	0,
	0,
	0,
	0,
	0,
	0,
	0
};
