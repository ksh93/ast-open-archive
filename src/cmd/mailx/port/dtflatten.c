#include	"dthdr.h"

/*	Flatten a dictionary into a linked list.
**	This may be used when many traversals are likely.
**
**	Written by Kiem-Phong Vo (07/15/95).
*/

#if __STD_C
Dtlink_t* dtflatten(Dt_t* dt)
#else
Dtlink_t* dtflatten(dt)
Dt_t*	dt;
#endif
{
	reg Dtlink_t	*r, *t, *list, *last, **slot, **eslot;

	/* already flattened */
	if(dt->data->type&DT_FLATTEN )
		return dt->data->here;

	list = last = NIL(Dtlink_t*);
	if(dt->data->type&DT_HASH)
	{	eslot = (slot = dt->data->htab) + dt->data->ntab;
		for(; slot < eslot; ++slot)
		{	if(!(t = *slot) )
				continue;
			if(last)
				last->right = t;
			else	list = last = t;
			while((t = last->right) )
				last = t;
			*slot = last;
		}
	}
	else if(dt->data->type&DT_TREE)/* eliminate left children */
	{	for(r = dt->data->here; r; )
		{	while((t = r->left) )
				RROTATE(r,t);
			last = last ? (last->right = r) : (list = r);
			r = last->right;
		}
	}
	else if(dt->data->type&(DT_LIST|DT_STACK|DT_QUEUE) )
		list = dt->data->head;

	dt->data->here = list;
	dt->data->type |= DT_FLATTEN;

	return list;
}
