/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1986-2004 AT&T Corp.                *
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
