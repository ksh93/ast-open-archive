/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 2003-2006 AT&T Corp.                  *
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
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#include	"vcwhdr.h"

/*	Open a handle for computing matching windows.
**
**	Written by Kiem-Phong Vo (kpv@research.att.com)
*/

#if __STD_C
Vcwindow_t* vcwopen(Vcwdisc_t* disc, Vcwmethod_t* meth)
#else
Vcwindow_t* vcwopen(disc, meth)
Vcwdisc_t*	disc;
Vcwmethod_t*	meth;	/* window matching method	*/
#endif
{
	Vcwindow_t*	vcw;

	if(!disc || (!disc->srcf && !disc->tarf) )
		return NIL(Vcwindow_t*);
	if(disc->srcf && sfseek(disc->srcf, (Sfoff_t)0, 0) < 0)
		return NIL(Vcwindow_t*);
	if(disc->tarf && sfseek(disc->tarf, (Sfoff_t)0, 0) < 0)
		return NIL(Vcwindow_t*);

	if(!meth)
		meth = Vcwdecode;

	if(!(vcw = (Vcwindow_t*)calloc(1,sizeof(Vcwindow_t))) )
		return NIL(Vcwindow_t*);

	vcw->disc = disc;
	vcw->meth = meth;
	vcw->mtdata = NIL(Void_t*);

	if(disc->eventf && (*disc->eventf)(vcw, VCW_OPENING, 0, disc) < 0)
	{	vcwclose(vcw);
		return NIL(Vcwindow_t*);
	}

	if(meth->eventf && (*meth->eventf)(vcw, VCW_OPENING) < 0)
	{	vcwclose(vcw);
		return NIL(Vcwindow_t*);
	}

	return vcw;
}
