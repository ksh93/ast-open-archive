/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1990-2002 AT&T Corp.                *
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
 * return host status given name
 */

#include "cslib.h"

#include <proc.h>

static int
initiate(register Cs_t* state, const char* name, char* buf)
{
	char*	av[3];

	if (!pathpath(buf, CS_STAT_DAEMON, "", PATH_REGULAR|PATH_EXECUTE))
	{
		messagef((state->id, NiL, -1, "stat: %s: %s: cannot find stat daemon", name, CS_STAT_DAEMON));
		return -1;
	}
	av[0] = buf;
	av[1] = (char*)name;
	av[2] = 0;
	procclose(procopen(av[0], av, NiL, NiL, 0));
	return 0;
}

int
csstat(register Cs_t* state, const char* name, register Csstat_t* sp)
{
	register int		n;
	register int		i;
	char*			s;
	unsigned long		a;
	unsigned long		m;
	struct stat		st;
	char			buf[PATH_MAX];

	static int		init;
	static char*		dir;

	messagef((state->id, NiL, -8, "stat(%s) call", name));
	CSTIME();
	if (!name || streq(name, CS_HOST_LOCAL))
		name = (const char*)csname(state, 0);
	if (!strchr(name, '/'))
	{
		if (!init)
		{
			init = 1;
			if (pathpath(buf, CS_STAT_DIR, "", PATH_EXECUTE))
				dir = strdup(buf);
		}
		if (!dir)
			return -1;
		sfsprintf(buf, sizeof(buf), "%s/%s", dir, name);
		if ((n = stat(buf, &st)) || (state->time - CS_STAT_DOWN) > (unsigned long)st.st_ctime)
		{
			if (initiate(state, name, buf))
				return -1;
			for (i = 1; i < CS_STAT_DOWN / 20; i <<= 1)
			{
				sleep(i);
				if (!(n = stat(buf, &st)))
					break;
			}
			if (n)
			{
				messagef((state->id, NiL, -1, "stat: %s: %s: stat error", name, buf));
				return -1;
			}
		}
	}
	else
		n = stat(name, &st);
	if (n)
	{
		memzero(sp, sizeof(*sp));
		sp->up = -CS_STAT_DOWN;
	}
	else
	{
		a = st.st_atime;
		m = st.st_mtime;
		if (!(sp->up = (((a >> 16) & 0x7ff) << ((a >> 27) & 0x1f))))
		{
			memzero(sp, sizeof(*sp));
			sp->up = -(((a & 0x7ff) << ((a >> 11) & 0x1f)) + (long)(state->time - (unsigned long)st.st_ctime));
			s = csname(state, 0);
			if (streq(name, s))
				initiate(state, name, buf);
		}
		else
		{
			sp->load = ((m >> 24) & 0xff) << 3;
			sp->pctsys = (m >> 16) & 0xff;
			sp->pctusr = (m >> 8) & 0xff;
			sp->users = m & 0xff;
			sp->idle = sp->users ? ((a & 0x7ff) << ((a >> 11) & 0x1f)) : ~0;
		}
	}
	return 0;
}

int
_cs_stat(const char* name, Csstat_t* sp)
{
	return csstat(&cs, name, sp);
}
