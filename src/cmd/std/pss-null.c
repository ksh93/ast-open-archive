/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1989-2005 AT&T Corp.                  *
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
/*
 * Glenn Fowler
 * AT&T Research
 *
 * process status stream PSS_METHOD_null implementation
 */

#include "psslib.h"

#if PSS_METHOD != PSS_METHOD_null

NoN(pss_null)

#else

static int
null_init(register Pss_t* pss)
{
	return 0;
}

static Pssmeth_t null_method =
{
	"null",
	"[-version?@(#)$Id: pss null (AT&T Labs Research) 2003-02-01 $\n]"
	"[-author?Glenn Fowler <gsf@research.att.com>]",
	0,
	null_init,
	0,
	0,
	0,
	0,
	0,
	0,
	0
};

Pssmeth_t*	_pss_method = &null_method;

#endif
