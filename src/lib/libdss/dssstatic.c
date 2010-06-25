/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2002-2010 AT&T Intellectual Property          *
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
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * dss static dssmeth() implementation
 *
 * Glenn Fowler
 * AT&T Research
 */

#include "dsshdr.h"

/*
 * return method for name given statically linked lib
 */

Dssmeth_t*
dssstatic(const char* name, Dsslib_t* lib, Dssdisc_t* disc)
{
	Dssmeth_t*	meth;

	if (!(meth = lib->meth))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: no method in static library", name);
		return 0;
	}
	if (dssadd(lib, disc))
		return 0;
	return dssmethinit(name, NiL, NiL, disc, meth);
}
