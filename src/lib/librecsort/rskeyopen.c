/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1996-2004 AT&T Corp.                *
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
*                 Phong Vo <kpv@research.att.com>                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
#pragma prototyped

/*
 * keyed data support for recsort
 */

#include "rskeyhdr.h"

static const char id[] = "\n@(#)$Id: rskey library (AT&T Research) 2004-02-11 $\0\n";

static const char lib[] = "librecsort:rskey";

static State_t	state;

/*
 * initialize the global readonly tables
 */

static void
#if __STD_C
initialize(void)
#else
initialize()
#endif
{
	register int	i;

	for (i = 0; i <= UCHAR_MAX; i++)
	{
		state.all[i] = 1;
		state.ident[i] = i;
		state.fold[i] = islower(i) ? toupper(i) : i;
		if (blank(i))
			state.dict[i] = 1;
		if (isalnum(i))
		{
			state.dict[i] = 1;
			state.print[i] = 1;
		}
		else if (isprint(i))
			state.print[i] = 1;
	}
}

/*
 * open a recsort key discipline handle
 */

Rskey_t*
#if __STD_C
rskeyopen(Rskeydisc_t* disc)
#else
rskeyopen(disc)
Rskeydisc_t*	disc;
#endif
{
	register Rskey_t*	kp;

	if (!state.dict[' '])
		initialize();
	if (!(kp = vmnewof(Vmheap, 0, Rskey_t, 1, sizeof(Rsdisc_t))))
		return 0;
	kp->id = lib;
	kp->disc = (Rsdisc_t*)(kp + 1);
	kp->disc->version = RS_VERSION;
	kp->disc->keylen = -1;
	kp->disc->data = '\n';
	kp->keydisc = disc;
	kp->state = &state;
	kp->insize = INSIZE;
	kp->outsize = OUTSIZE;
	kp->procsize = PROCSIZE;
	kp->field.head = kp->field.tail = &kp->field.global;
	kp->field.global.end.field = MAXFIELD;
	kp->meth = Rsrasp;
	return kp;
}

/*
 * close an rskeyopen() handle
 */

int
#if __STD_C
rskeyclose(Rskey_t* kp)
#else
rskeyclose(kp)
Rskey_t*	kp;
#endif
{
	register Field_t*	fp;
	register Field_t*	np;

	if (!kp)
		return -1;
	np = kp->field.global.next;
	while (fp = np)
	{
		np = fp->next;
		if (fp->freetrans)
			vmfree(Vmheap, fp->trans);
		vmfree(Vmheap, fp);
	}
	np = kp->accumulate.head;
	while (fp = np)
	{
		np = fp->next;
		vmfree(Vmheap, fp);
	}
	if (kp->field.positions)
		vmfree(Vmheap, kp->field.positions);
	vmfree(Vmheap, kp);
	return 0;
}
