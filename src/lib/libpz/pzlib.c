/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1998-2000 AT&T Corp.                *
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
*                                                                  *
*******************************************************************/
#pragma prototyped

/*
 * find and load a pzip dll
 */

#include "pzlib.h"

#include <dlldefs.h>

int
pzlib(register Pz_t* pz, register const char* name, int ignore)
{
	register Pzdll_t*	dll;
	register Pzdllpz_t*	pzs;
	register int		n;
	register char*		id;
	const char*		options;
	char			buf[64];

	/*
	 * prepend local part of state.id to name if not already there
	 */

	if (id = strchr((char*)state.id, ':'))
		id++;
	else
		id = (char*)state.id;
	if (strchr(name, '/') || strchr(name, '\\'))
		n = 0;
	else
	{
		n = strlen(id);
		if (strneq(name, id, n))
			n = 0;
		else
		{
			sfsprintf(buf, sizeof(buf), "%s%s", id, name);
			name = (const char*)buf;
		}
	}

	/*
	 * see if the dll is already loaded
	 */

	for (dll = state.dll; dll && !streq(dll->name, name); dll = dll->next);
	if (!dll)
	{
		/*
		 * add to the list and open
		 */

		if (!(dll = newof(0, Pzdll_t, 1, strlen(name) + 1)))
			return -1;
		dll->name = strcpy((char*)(dll + 1), name);
		dll->next = state.dll;
		state.dll = dll;
		if (ignore)
			return 0;
		if (!(dll->dll = dllfind(dll->name, NiL, RTLD_LAZY)) && (!n || !(dll->dll = dllfind(dll->name + n, NiL, RTLD_LAZY))))
		{
			if (pz->disc && pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "%s: cannot find library", dll->name + n);
			return -1;
		}

		/*
		 * get the initialization function
		 */

		sfsprintf(buf, sizeof(buf), "_%s_init", id);
		if (!(dll->initf = (Pzinit_f)dlsym(dll->dll, buf + 1)) && !(dll->initf = (Pzinit_f)dlsym(dll->dll, buf)))
		{
			if (pz->disc && pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, 2, "%s: %s: initialization function not found in library", dll->name + n, buf + 1);
			return -1;
		}
	}

	/*
	 * see if pz already initialized
	 */

	if (dll->initf)
	{
		for (pzs = dll->pzs; pzs && pzs->pz != pz; pzs = pzs->next);
		if (!pzs)
		{
			if (!(pzs = newof(0, Pzdllpz_t, 1, 0)))
				return -1;
			pzs->pz = pz;
			pzs->next = dll->pzs;
			dll->pzs = pzs;
			options = pz->disc->options;
			if (!(dll->usage = (*dll->initf)(pz, pz->disc)))
			{
				if (pz->disc && pz->disc->errorf)
					(*pz->disc->errorf)(pz, pz->disc, 2, "%s: %s_init: initialization function error", dll->name + n, id);
				return -1;
			}
			optget(NiL, dll->usage);
			if (pz->disc->options != options)
			{
				if (!pz->disc->options)
					pz->options = 0;
				else if (!(pz->options = vmstrdup(pz->vm, pz->disc->options)))
				{
					if (pz->disc && pz->disc->errorf)
						(*pz->disc->errorf)(pz, pz->disc, 2, "%s: out of space", dll->name + n);
					return -1;
				}
			}
			if (pz->options && pzoptions(pz, pz->part, pz->options, 0) < 0)
				return -1;
		}
	}
	return 0;
}
