#include	"dthdr.h"

/*	Extract objects of a dictionary.
**
**	Written by Kiem-Phong Vo (7/15/95).
*/

#if __STD_C
Dtlink_t* dtextract(reg Dt_t* dt)
#else
Dtlink_t* dtextract(dt)
reg Dt_t*	dt;
#endif
{
	reg Dtlink_t	*t, *last, *list, **slot, **eslot;

	UNFLATTEN(dt);

	if(dt->data->type&DT_TREE)
		list = dt->data->here;
	else if(dt->data->type&DT_HASH) /* make a big list of everything */
	{	list = last = NIL(Dtlink_t*);
		eslot = (slot = dt->data->htab) + dt->data->ntab;
		for(; slot < eslot; ++slot)
		{	if(!(t = *slot) )
				continue;
			*slot = NIL(Dtlink_t*);
			if(last)
				last->right = t;
			else	list = last = t;
			while((t = last->right) )
				last = t;
		}
	}
	else /*if(dt->data->type&(DT_LIST|DT_STACK|DT_QUEUE))*/
	{	list = dt->data->head;
		dt->data->head = NIL(Dtlink_t*);
	}

	dt->data->size = 0;
	dt->data->here = NIL(Dtlink_t*);

	return list;
}
