/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1997-2005 AT&T Corp.                  *
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
 * find and load a cdb dll
 */

#include "cdblib.h"

#include <dlldefs.h>

typedef int (*Init_f)(void);

int
cdblib(register const char* name, Cdbdisc_t* disc)
{
	register Cdbdll_t*	dll;
	register char*		id;
	Init_f			initf;
	char			buf[64];
	char			path[PATH_MAX];

	/*
	 * prepend local part of state.id to name if not already there
	 */

	if (id = strchr((char*)state.id, ':'))
		id++;
	else
		id = (char*)state.id;

	/*
	 * see if the dll is already loaded
	 */

	for (dll = state.dll; dll; dll = dll->next)
		if (streq(dll->name, name))
			return 0;

	/*
	 * add to the list and open
	 */

	if (!(dll = newof(0, Cdbdll_t, 1, strlen(name) + 1)))
		return -1;
	dll->name = strcpy((char*)(dll + 1), name);
	if (!(dll->dll = dllplug(id, dll->name, NiL, RTLD_LAZY, path, sizeof(path))))
	{
		if (disc && disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: %s", dll->name, dlerror());
		return -1;
	}
	dll->next = state.dll;
	state.dll = dll;

	/*
	 * get and call the initialization function
	 */

	sfsprintf(buf, sizeof(buf), "%s_lib", id);
	if (!(initf = (Init_f)dlllook(dll->dll, buf)))
	{
		if (disc && disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: %s: initialization function not found in library", path, buf);
		return -1;
	}
	if ((*initf)() < 0)
	{
		if (disc && disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: %s: initialization function error", path, buf);
		return -1;
	}
	return 0;
}
