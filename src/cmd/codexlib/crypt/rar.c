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
 * crypt-rar decoder/encoder wrapper
 */

#include "rar.h"

static int
rar_open(Codex_t* p, char* const args[], Codexnum_t flags)
{
	int		version;
	char*		e;

	if (!args[2] || !args[3] || strcmp(args[2], "rar") || (version = strtol(args[3], &e, 10)) <= 0 || *e)
		return -1;
	if (version <= 13)
		p->meth = &crypt_rar_13;
	else if (version <= 15)
		p->meth = &crypt_rar_15;
	else
		p->meth = &crypt_rar_20;
	return (*p->meth->openf)(p, args, flags);
}

Codexmeth_t	crypt_rar =
{
	"crypt-rar",
	"\brar\b encryption. The first option is the \brar\b implementation"
	" version. Versions { 13 15 20 } are supported.",
	"[+(version)?crypt-rar-13 2003-12-29]"
	"[+(author)?Eugene Roshal]"
	"[+(copyright)?Copyright (c) 1999-2003 Eugene Roshal]"
	"[+(license)?GPL]",
	CODEX_DECODE|CODEX_ENCODE|CODEX_CRYPT,
	0,
	0,
	rar_open,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	crypt_rar_next
};
