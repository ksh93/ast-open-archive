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
