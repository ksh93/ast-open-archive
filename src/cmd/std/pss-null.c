/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1989-2003 AT&T Corp.                *
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
*                                                                  *
*******************************************************************/
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
