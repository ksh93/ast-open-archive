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
 * return password buf
 * return <  0 : error
 * return >= n : n-1 in buf
 */

#include <codex.h>

ssize_t
codexpass(void* buf, size_t n, Codexdisc_t* disc, Codexmeth_t* meth)
{
	char		prompt[2 * CODEX_NAME];

	if (disc->passphrase)
		return strncopy((char*)buf, disc->passphrase, n) - (char*)buf;
	if (disc->passf)
		return (*disc->passf)(buf, n, disc, meth);
	sfsprintf(prompt, sizeof(prompt), "Enter %s passphrase: ", meth->name);
	return codexgetpass(prompt, buf, n);
}
