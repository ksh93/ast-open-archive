/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 2000-2006 AT&T Corp.                  *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                            by AT&T Corp.                             *
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

#include "ptlib.h"

/*
 * print table a in normal form on sp
 */

int
ptprint(Pt_t* a, Sfio_t* sp)
{
	PTSCAN(a, x, b, m, sfprintf(sp, "%s\n", fmtip4(x, b)));
	return sfsync(sp);
}