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
