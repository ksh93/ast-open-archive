/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1996-2003 AT&T Corp.                *
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
*                 Phong Vo <kpv@research.att.com>                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
#pragma prototyped

#include "rskeyhdr.h"

#include <dlldefs.h>
#include <option.h>

typedef Rsdisc_t* (*Rslib_f)(Rskey_t*, const char*);

int
#if __STD_C
rslib(Rskey_t* kp, const char* lib)
#else
rslib(kp, lib)
Rskey_t*	kp;
const char*	lib;
#endif
{
	register char*		s;
	void*			dll;
	char*			p;
	Rsdisc_t*		disc;
	Rslib_f			fun;
	char			path[PATH_MAX];
	Opt_t			opt;

	static const char	prefix[] = "sort";
	static const char	symbol[] = "rs_disc";

	for (s = (char*)lib; *s && *s != ',' && *s != '\t' && *s != '\r' && *s != '\n'; s++);
	sfsprintf(path, sizeof(path), "%s%-.*s", prefix, s - (char*)lib, lib);
	p = strchr(path, '/') ? (path + sizeof(prefix) - 1) : path;
	if (!(dll = dllfind(p, NiL, RTLD_LAZY, path, sizeof(path))))
	{
		if (kp->keydisc->errorf)
			(*kp->keydisc->errorf)(kp, kp->keydisc, 2, "%s: library not found", p);
		return -1;
	}
	if (!(fun = (Rslib_f)dlllook(dll, symbol)))
	{
		if (kp->keydisc->errorf)
			(*kp->keydisc->errorf)(kp, kp->keydisc, 2, "%s: %s: initialization function not found in library", path, symbol);
		return -1;
	}
	if (*s)
		s++;
	opt = opt_info;
	disc = (*fun)(kp, s);
	opt_info = opt;
	if (!disc)
		return -1;
	if (!kp->tail)
		kp->tail = kp->disc;
	kp->tail = kp->tail->disc = disc;
	return 0;
}
