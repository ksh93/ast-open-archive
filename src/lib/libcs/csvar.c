/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1990-2000 AT&T Corp.                *
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
 * Glenn Fowler
 * AT&T Research
 *
 * run time cs var support
 */

#include "cslib.h"

#ifndef TRUST
#define TRUST		"/usr/local:/usr" /* trusted root dirs		*/
#endif

typedef struct
{
	const char*	name;
	const char*	trust;
	const char*	value;
} Var_t;

static Var_t	var[] =
{

{ /* CS_VAR_LOCAL */ "CS_MOUNT_LOCAL", "/tmp/cs"			},
{ /* CS_VAR_PROXY */ "CS_MOUNT_PROXY", "/dev/tcp/proxy/inet.proxy"	},
{ /* CS_VAR_SHARE */ "CS_MOUNT_SHARE", "share/lib/cs"			},
{ /* CS_VAR_TRUST */ "CS_MOUNT_TRUST", TRUST				},

};

char*
csvar(Cs_t* state, int index, int trust)
{
	register char*	s;
	register Var_t*	p;

	if (index < 0 || index >= elementsof(var))
		return 0;
	p = &var[index];
	if (!p->name || trust)
		return (char*)p->trust;
	if (!p->value)
		p->value = ((s = getenv(p->name)) && *s) ? (const char*)s : p->trust;
	return (char*)p->value;
}

char*
_cs_var(int index, int trust)
{
	return csvar(&cs, index, trust);
}
