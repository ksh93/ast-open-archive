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
 * return matching prefix for addr
 */

Ptprefix_t*
ptmatch(Pt_t* tab, Ptaddr_t addr)
{
	Ptprefix_t	key;

	key.min = key.max = addr;
	return (Ptprefix_t*)dtsearch(tab->dict, &key);
}