/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1998-2001 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*                 This software was created by the                 *
*                 Network Services Research Center                 *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*******************************************************************/
#pragma prototyped

/*
 * initialize a pzip discipline library
 */

#include "pzlib.h"

const char*
pzinit(register Pz_t* pz, register const char* name, Pzinit_f initf)
{
	const char*		options;
	const char*		usage;
	char*			id;

	options = pz->disc->options;
	if (!(usage = (*initf)(pz, pz->disc)))
	{
		if (pz->disc && pz->disc->errorf)
		{
			if (id = strchr((char*)state.id, ':'))
				id++;
			else
				id = (char*)state.id;
			(*pz->disc->errorf)(pz, pz->disc, 2, "%s: %s_init: initialization function error", name, id);
		}
		return 0;
	}
	optget(NiL, usage);
	if (pz->disc->options != options)
	{
		if (!pz->disc->options)
			pz->options = 0;
		else if (!(pz->options = vmstrdup(pz->vm, pz->disc->options)))
		{
			if (pz->disc && pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, 2, "%s: out of space", name);
			return 0;
		}
	}
	if (pz->options && pzoptions(pz, pz->part, pz->options, 0) < 0)
		return 0;
	return usage;
}
