/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*Copyright (c) 1978-2004 The Regents of the University of Californi*
*                                                                  *
*          Permission is hereby granted, free of charge,           *
*       to any person obtaining a copy of THIS SOFTWARE FILE       *
*            (the "Software"), to deal in the Software             *
*              without restriction, including without              *
*           limitation the rights to use, copy, modify,            *
*                merge, publish, distribute, and/or                *
*            sell copies of the Software, and to permit            *
*            persons to whom the Software is furnished             *
*          to do so, subject to the following disclaimer:          *
*                                                                  *
*THIS SOFTWARE IS PROVIDED BY The Regents of the University of Cali*
*         ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,         *
*            INCLUDING, BUT NOT LIMITED TO, THE IMPLIED            *
*            WARRANTIES OF MERCHANTABILITY AND FITNESS             *
*             FOR A PARTICULAR PURPOSE ARE DISCLAIMED.             *
*IN NO EVENT SHALL The Regents of the University of California and *
*         BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,          *
*           SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES           *
*           (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT            *
*          OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,           *
*           DATA, OR PROFITS; OR BUSINESS INTERRUPTION)            *
*          HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,          *
*          WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT          *
*           (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING            *
*           IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,            *
*        EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.        *
*                                                                  *
*            Information and Software Systems Research             *
*The Regents of the University of California and AT&T Labs Research*
*                         Florham Park NJ                          *
*                                                                  *
*                        Kurt Shoens (UCB)                         *
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
#include	"dthdr.h"

/*	Set a view path from dict to view.
**
**	Written by Kiem-Phong Vo (07/15/95)
*/


#if __STD_C
static Void_t* dtvsearch(Dt_t* dt, reg Void_t* obj, reg int type)
#else
static Void_t* dtvsearch(dt,obj,type)
Dt_t*		dt;
reg Void_t*	obj;
reg int		type;
#endif
{
	reg Dt_t	*d, *p;
	reg Void_t*	o;

	/* these operations only happen at the top level */
	if(type&(DT_INSERT|DT_DELETE|DT_CLEAR|DT_RENEW))
		return (*(dt->meth->searchf))(dt,obj,type);

	if(!obj && !(type&(DT_FIRST|DT_LAST)) )
		return NIL(Void_t*);

	if(type&(DT_MATCH|DT_SEARCH|DT_FIRST|DT_LAST))
	{	for(d = dt; d; d = d->view)
		{	if((o = (*(d->meth->searchf))(d,obj,type)) )
			{	dt->walk = d;
				return o;
			}
		}

		dt->walk = NIL(Dt_t*);
		return NIL(Void_t*);
	}

	/* must be (DT_NEXT|DT_PREV) */
	if(!dt->walk)
		dt->walk = dt;

	for(d = dt->walk, o = (*(d->meth->searchf))(d,obj,type); ; )
	{	while(o)
		{	for(p = dt; ; p = p->view)
			{	if(p == d) /* this object is uncovered */	
					return o;

				/* see if it is covered */
				if((*(p->meth->searchf))(p,o,DT_SEARCH) )
					break;
			}

			o = (*(d->meth->searchf))(d,o,type);
		}

		if(!(d = dt->walk = d->view) )
			return NIL(Void_t*);

		if(type&DT_NEXT)
			o = (*(d->meth->searchf))(d,NIL(Void_t*),DT_FIRST);
		else /* if(type&DT_PREV) */
			o = (*(d->meth->searchf))(d,NIL(Void_t*),DT_LAST);
	}
}

#if __STD_C
Dt_t* dtview(reg Dt_t* dt, reg Dt_t* view)
#else
Dt_t* dtview(dt,view)
reg Dt_t*	dt;
reg Dt_t*	view;
#endif
{
	reg Dt_t*	d;

	/* these types don't admit views */
	if((dt->data->type&(DT_LIST|DT_STACK|DT_QUEUE)) ||
	   (view && view->data->type&(DT_LIST|DT_STACK|DT_QUEUE)) )
		return NIL(Dt_t*);

	UNFLATTEN(dt);

	/* no more viewing lower dictionary */
	if((d = dt->view) )
		d->nview -= 1;
	dt->view = dt->walk = NIL(Dt_t*);

	if(!view)
	{	dt->searchf = dt->meth->searchf;
		return d;
	}

	UNFLATTEN(view);

	/* make sure there won't be a cycle */
	for(d = view; d; d = d->view)
		if(d == dt)
			return NIL(Dt_t*);

	/* ok */
	dt->view = view;
	dt->searchf = dtvsearch;
	view->nview += 1;

	return view;
}
