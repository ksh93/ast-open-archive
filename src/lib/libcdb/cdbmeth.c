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
 * cdb method by name
 */

#include "cdblib.h"

/*
 * add meth to the front of the method list
 */

int
cdbaddmeth(Cdbmeth_t* meth)
{
	meth->next = state.methods;
	state.methods = meth;
	return 0;
}

/*
 * return cdb method list given name
 * the returned list is malloc'd for local modification
 */

Cdbmeth_t*
cdbgetmeth(const char* name)
{
	register char*	s = (char*)name;
	register char*	e;
	register int	n;
	int		lib;
	Cdbmeth_t*	mp;
	Cdbmeth_t*	hp;
	Cdbmeth_t*	tp;
	Cdbmeth_t*	xp;
	char		buf[64];

	if (!name)
		return state.methods;
	hp = tp = 0;
	for (;;)
	{
		n = (e = strchr(s, '.')) ? (e - s) : strlen(s);
		mp = state.methods;
		if (n != 0 && (n != 1 || !strneq(s, "-", 1)) && (n != 7 || !strneq(s, "default", 7)))
			for (lib = 0;;)
			{
				if (!mp)
				{
					if (lib || n >= sizeof(buf))
						goto nope;
					memcpy(buf, s, n);
					s = buf;
					s[n] = 0;
					if (cdblib(s, 0, NiL) < 0)
						goto nope;
					lib = 1;
					mp = state.methods;
				}
				if (strneq(s, mp->name, n) && !mp->name[n])
					break;
				mp = mp->next;
			}
		if (!(xp = newof(0, Cdbmeth_t, 1, n + 1)))
			goto nope;
		*xp = *mp;
		xp->next = 0;
		xp->name = (const char*)(xp + 1);
		strcpy((char*)xp->name, s);
		if (tp)
			tp = tp->next = xp;
		else
			tp = hp = xp;
		if (!e)
			break;
		s = e + 1;
	}
	return hp;
 nope:
	while (xp = hp)
	{
		hp = xp->next;
		free(xp);
	}
	return 0;
}

/*
 * add type to the front of the external type list
 */

int
cdbaddtype(Cdbtype_t* type)
{
	type->next = state.types;
	state.types = type;
	return 0;
}

/*
 * return cdb type given name
 */

Cdbtype_t*
cdbgettype(const char* name)
{
	register Cdbtype_t*	type;
	register int		lib;

	if (!name)
		return state.types;
	for (type = state.types, lib = 0; (type || !lib++ && cdblib(name, 0, NiL) >= 0 && (type = state.types)) && !streq(name, type->name); type = type->next);
	return type;
}

/*
 * add map to the front of the map list
 */

int
cdbaddmap(Cdbmapmeth_t* map)
{
	map->next = state.maps;
	state.maps = map;
	return 0;
}

/*
 * return cdb map given name
 */

Cdbmapmeth_t*
cdbgetmap(const char* name)
{
	register Cdbmapmeth_t*	map;
	register int		lib;

	if (!name)
		return state.maps;
	for (map = state.maps, lib = 0; (map || !lib++ && cdblib(name, 0, NiL) >= 0 && (map = state.maps)) && !streq(name, map->name); map = map->next);
	return map;
}
