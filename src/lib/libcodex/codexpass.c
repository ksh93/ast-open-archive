/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2011 AT&T Intellectual Property          *
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
