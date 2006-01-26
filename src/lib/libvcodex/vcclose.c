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

/*	Close a Vcodex_t handle
**
**	Written by Kiem-Phong Vo (kpv@research.att.com)
*/

#if __STD_C
int vcclose(Vcodex_t* vc)
#else
int vcclose(vc)
Vcodex_t*	vc;
#endif
{
	int		i;

	if(!vc)
		return -1;

	vcsetbuf(vc, NIL(Vcchar_t*), -1, -1); /* free all cached buffers */

	if(vc->disc && vc->disc->eventf &&
	   (*vc->disc->eventf)(vc, VC_CLOSING, NIL(Void_t*), vc->disc) < 0 )
		return -1;

	if(vc->meth && vc->meth->eventf &&
	   (*vc->meth->eventf)(vc, VC_CLOSING, NIL(Void_t*)) < 0 )
		return -1;

	if(vc->coder && (vc->flags&VC_CLOSECODER) )
	{	vcclose(vc->coder);
		vc->coder = NIL(Vcodex_t*);
	}

	if(vc->ctxt ) /* free context data */
	{	if(vc->meth && vc->meth->eventf)
		{	Vcchar_t *ctxt = (Vcchar_t*)vc->ctxt;
			for(i = 0; i < vc->ctop; ++i, ctxt += vc->csize)
				(*vc->meth->eventf)(vc,VC_ENDCTXT,(Void_t*)ctxt);
		}

		free(vc->ctxt);
	}

	free(vc);

	return 0;
}
