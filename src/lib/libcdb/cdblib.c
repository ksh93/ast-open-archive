/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1997-2002 AT&T Corp.                *
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
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*******************************************************************/
#pragma prototyped

/*
 * find and load a cdb dll
 */

#include "cdblib.h"

#include <dlldefs.h>

typedef int (*Init_f)(void);

int
cdblib(register const char* name, int base, Cdbdisc_t* disc)
{
	register Cdbdll_t*	dll;
	register char*		id;
	register int		n;
	Init_f			initf;
	char			buf[64];

	/*
	 * prepend local part of state.id to name if not already there
	 */

	if (id = strchr((char*)state.id, ':'))
		id++;
	else
		id = (char*)state.id;
	n = strlen(id);
	if (!strneq(name, id, n))
	{
		sfsprintf(buf, sizeof(buf), "%s%s", id, name);
		name = (const char*)buf;
	}

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
	if (!(dll->dll = dllfind(dll->name, NiL, RTLD_LAZY)) && (!base || !(dll->dll = dllfind(dll->name + n, NiL, RTLD_LAZY))))
	{
		if (disc && disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: %s", dll->name + n, dlerror());
		return -1;
	}
	dll->next = state.dll;
	state.dll = dll;

	/*
	 * get and call the initialization function
	 */

	sfsprintf(buf, sizeof(buf), "%s_init", id);
	if (!(initf = (Init_f)dlllook(dll->dll, buf)))
	{
		if (disc && disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: %s: initialization function not found in library", dll->name + n, buf);
		return -1;
	}
	if ((*initf)() < 0)
	{
		if (disc && disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: %s: initialization function error", dll->name + n, buf);
		return -1;
	}
	return 0;
}
