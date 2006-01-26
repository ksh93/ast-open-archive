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

/*	A handle may be used to compress different types of data with
**	states carried thru different vcapply() calls per type. For example,
**	a handle using the method Vctable would compute a transform plan
**	on the first vcapply() call, then reuse it in subsequent vcapply()'s.
**	This would not work if different vcapply() calls are to process
**	tables with different number of columns or with different
**	characteristics. Assuming that the invoker of such a handle can
**	separately determine the data characteristics, it could then create
**	a different context per data type for processing. The computed
**	transforms would then be specific per context.
**
**	Note that this process is made complicated by the fact that handles
**	can be composed for secondary coding. In that case, contexts may need
**	to be carried through all continuation coders. Any implementer of
**	a method needs to be aware of this fact and code to anticipate
**	such compositions. See the Vcrle method for an example.
**
**	Written by Kiem-Phong Vo (kpv@research.att.com)
*/

#if __STD_C
int vccontext(Vcodex_t* vc, int c)
#else
int vccontext(vc, c)
Vcodex_t*	vc;
int		c;	/* < 0 to create a new context	*/
#endif
{
	int	init = vc->ctxt ? 0 : 1;

	if(vc->ctop < 0) /* effort to change context failed earlier */
		return -1;

	if(c < 0) /* creating a new context */
		c = vc->ctop;
	else if(c >= vc->ctop) /* cannot set an unknown context */
		return -1;

	if(init || (c > 0 && c == vc->ctop) )
	{	if(!(vc->ctxt = realloc(vc->ctxt, (c+1)*vc->csize)) )
		{	vc->ctop = -1;
			return -1;
		}
	}

	if(c == vc->ctop) /* a new context, initialize it */
	{	Void_t *ctxt = (Void_t*)(((Vcchar_t*)vc->ctxt) + c*vc->csize);

		memset(ctxt, 0, vc->csize);
		if(!vc->meth->eventf ||
		   (*vc->meth->eventf)(vc, VC_INITCTXT, ctxt) == 0 )
			((Vcctxt_t*)ctxt)->ctxt = -1;

		if(!init)
			vc->ctop += 1;
	}

	return (vc->cnow = c);
}
