#include	"dthdr.h"


/*	Renew a single object which must be at current finger.
**
**	Written by Kiem-Phong Vo (08/01/95)
*/

#if __STD_C
Void_t* dtrenew(Dt_t* dt, reg Void_t* obj)
#else
Void_t* dtrenew(dt, obj)
Dt_t*		dt;
reg Void_t*	obj;
#endif
{
	reg char*	key;
	reg Dtlink_t	*e, *r, *t = NIL(Dtlink_t*);
	reg Dtlink_t**	slot;
	reg Dtdisc_t*	disc = dt->disc;

	UNFLATTEN(dt);

	if(!(e = dt->data->here) || OBJ(e,disc) != obj)
		return NIL(Void_t*);

	if(dt->data->type&DT_TREE)
	{	if(!(r = e->right) )
			dt->data->here = e->left;
		else
		{	while(r->left)
				RROTATE(r,t);
			r->left = e->left;
			dt->data->here = r;
		}
	}
	else if(dt->data->type&DT_HASH)
	{	slot = dt->data->htab + HINDEX(dt->data->ntab,e->hash);
		if((t = *slot) == e)
			*slot = e->right;
		else
		{	for(; t->right != e; t = t->right)
				;
			t->right = e->right;
		}
		key = KEY(obj,disc);
		e->hash = HASH(dt,key,disc);
		dt->data->here = NIL(Dtlink_t*);
	}
	else if(dt->data->type&DT_LIST)
	{	/* set r to elt right before class of e */
		r = NIL(Dtlink_t*);
		for(t = dt->data->head->left; t != e->left; r = t, t = t->right->left)
			;

		if(e == (t = r ? r->right : dt->data->head) )
		{	if(r)
				r->right = e->right;
			else	dt->data->head = e->right;
		}
		else
		{	for(; t->right != e; t = t->right)
				;
			t->right = e->right;

			/* reset last element pointer */
			if(e == e->left)
			{	r = r ? r->right : dt->data->head;
				for(; r != t->right; r = r->right)
					r->left = t;
			}
		}
		dt->data->here = NIL(Dtlink_t*);
	}
	else /* if(dt->data->type&(DT_STACK|DT_QUEUE)) */
	{	dt->data->here = NIL(Dtlink_t*);
		return obj;
	}

	dt->data->size -= 1;
	return (*dt->meth->searchf)(dt,(Void_t*)e,DT_RENEW) ? obj : NIL(Void_t*);
}
