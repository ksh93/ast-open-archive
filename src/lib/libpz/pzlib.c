/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1998-2004 AT&T Corp.                *
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
 * find and load a pzip dll
 */

#include "pzlib.h"

#include <dlldefs.h>

int
pzlib(register Pz_t* pz, register const char* name, int ignore)
{
	register Pzdll_t*	dll;
	register Pzdllpz_t*	pzs;
	char*			id;
	char			buf[64];
	char			path[PATH_MAX];

	if (id = strrchr(state.id, ':'))
		id++;
	else
		id = (char*)state.id;

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
		if (!(dll->dll = dllplug(id, dll->name, NiL, RTLD_LAZY, path, sizeof(path))))
		{
			if (pz->disc && pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "%s: %s", dll->name, dlerror());
			return -1;
		}

		/*
		 * get the initialization function
		 */

		sfsprintf(buf, sizeof(buf), "%s_init", id);
		if (!(dll->initf = (Pzinit_f)dlllook(dll->dll, buf)))
		{
			if (pz->disc && pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, 2, "%s: %s: initialization function not found in library", path, buf);
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
			if (!(dll->usage = pzinit(pz, dll->name, dll->initf)))
				return -1;
		}
	}
	return 0;
}
