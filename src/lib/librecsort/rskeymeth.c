/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1996-2002 AT&T Corp.                *
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
*                 Phong Vo <kpv@research.att.com>                  *
*               Glenn Fowler <gsf@research.att.com>                *
*******************************************************************/
#pragma prototyped

/*
 * rskey method by name
 */

#include "rskeyhdr.h"

static Rsmethod_t**	methods[] =
{
	&Rsrasp,
	&Rsradix,
	&Rssplay,
	&Rsverify,
};

/*
 * return rs method given name
 */

Rsmethod_t*
#if __STD_C
rskeymeth(register Rskey_t* kp, const char* name)
#else
rskeymeth(kp, name)
register Rskey_t*	kp;
char*			name;
#endif
{
	register int	n;

	if (!name || !*name || streq(name, "-") || streq(name, "default"))
		return Rsrasp;
	for (n = 0; n < elementsof(methods); n++)
		if (streq(name, (*methods[n])->name))
			return *methods[n];
	return 0;
}

/*
 * list rs method names on fp
 */

int
#if __STD_C
rskeylist(register Rskey_t* kp, Sfio_t* fp, int usage)
#else
rskeylist(kp, fp, usage)
register Rskey_t*	kp;
Sfio_t*			fp;
int			usage;
#endif
{
	register int	i;
	register int	n;

	n = 0;
	for (i = 0; i < elementsof(methods); i++)
		if (usage)
			n += sfprintf(fp, "[+%s?%s]", (*methods[i])->name, (*methods[i])->desc);
		else
			n += sfprintf(fp, "%-10s %s\n", (*methods[i])->name, (*methods[i])->desc);
	return n;
}
