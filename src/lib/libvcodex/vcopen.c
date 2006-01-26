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
#include	"vchdr.h"

static char*	Version = "\r\n@(#)vcodex (AT&T Research - kpv) 2006-01-24\0\r\n";

/*	Open a handle for data coding
**
**	Written by Kiem-Phong Vo (kpv@research.att.com)
*/

#if __STD_C
Vcodex_t* vcopen(Vcdisc_t* disc, Vcmethod_t* meth, Void_t* init,
		 Vcodex_t* coder, int flags)
#else
Vcodex_t* vcopen(disc, meth, init, coder, flags)
Vcdisc_t*	disc;	/* discipline to describe data	*/
Vcmethod_t*	meth;	/* method to process given data	*/
Void_t*		init;	/* method initialization params	*/
Vcodex_t*	coder;	/* continuation processor	*/
int		flags;	/* control flags		*/
#endif
{
	Vcodex_t*	vc = (Vcodex_t*)Version; /* stop compiler warning */

	/* exactly one of VC_EN/DECODE  is allowed */
	if((flags&VC_ENCODE) && (flags&VC_DECODE) )
		return NIL(Vcodex_t*);
	if(!(flags&VC_ENCODE) && !(flags&VC_DECODE) )
		return NIL(Vcodex_t*);

	if(!meth || !(vc = (Vcodex_t*)calloc(1,sizeof(Vcodex_t))) )
		return NIL(Vcodex_t*);

	if(!(vc->applyf = (flags&VC_ENCODE) ? meth->encodef : meth->decodef) )
	{	free(vc);
		return NIL(Vcodex_t*);
	}

	vc->disc = disc;
	vc->meth = meth;
	vc->coder = coder;
	vc->undone = 0;

	vc->flags = flags & VC_FLAGS;

	vc->csize = 0;
	vc->ctop = 0;
	vc->cnow = -1;
	vc->ctxt = NIL(Void_t*);

	vc->head = 0;
	vc->buf = NIL(Void_t*);
	vc->bsize = vc->bhead = 0;
	vc->blist = NIL(Vcbuf_t*);

	vc->mtdata = NIL(Void_t*);

	if(disc && disc->eventf &&
	   (*disc->eventf)(vc, VC_OPENING, NIL(Void_t*), disc) < 0)
	{	if(vc->coder && vc->coder != coder)
			vcclose(vc->coder);
		free(vc);
		return NIL(Vcodex_t*);
	}

	if(meth->eventf && (*meth->eventf)(vc, VC_OPENING, init) < 0)
	{	if(disc && disc->eventf)
			(void)(*disc->eventf)(vc, VC_OPENING, meth, disc);
		if(vc->coder && vc->coder != coder)
			vcclose(vc->coder);
		free(vc);
		return NIL(Vcodex_t*);
	}

	/* initialize context structure */
	if(vc->csize <= 0)
		vc->csize = sizeof(Vcctxt_t);
	if(vccontext(vc, -1) < 0)
	{	vcclose(vc);
		return NIL(Vcodex_t*);
	}

	return vc;
}
