/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1997-2004 AT&T Corp.                  *
*                      and is licensed under the                       *
*          Common Public License, Version 1.0 (the "License")          *
*                        by AT&T Corp. ("AT&T")                        *
*      Any use, downloading, reproduction or distribution of this      *
*      software constitutes acceptance of the License.  A copy of      *
*                     the License is available at                      *
*                                                                      *
*         http://www.research.att.com/sw/license/cpl-1.0.html          *
*         (with md5 checksum 8a5e0081c856944e76c69a1cf29c2e8b)         *
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
