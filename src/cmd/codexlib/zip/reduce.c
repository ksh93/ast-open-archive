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
