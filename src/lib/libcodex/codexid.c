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
 * identify encoding given first block of encoded data
 * not all encodings are expected to self-identify
 */

#include <codex.h>

Codexmeth_t*
codexid(const void* head, size_t headsize, char* name, size_t namesize)
{
	register Codexmeth_t*	meth;

	for (meth = codexlist(NiL); meth; meth = codexlist(meth))
		if (meth->identf && (*meth->identf)(meth, head, headsize, name, namesize))
			return meth;
	return 0;
}
