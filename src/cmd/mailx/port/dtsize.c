#include	"dthdr.h"

/*	Return the # of objects in the dictionary
**
**	Written by Kiem-Phong Vo (07/15/95)
*/

#if __STD_C
static int dtcount(reg Dtlink_t* here)
#else
static int dtcount(here)
reg Dtlink_t*	here;
#endif
{	return !here ? 0 : dtcount(here->left) + dtcount(here->right) + 1;
}

#if __STD_C
int dtsize(Dt_t* dt)
#else
int dtsize(dt)
Dt_t*	dt;
#endif
{
	reg Dtlink_t*	t;
	reg int		size;

	UNFLATTEN(dt);

	if(dt->data->size < 0) /* !(dt->data->type&DT_HASH) */
	{	if(dt->data->type&DT_TREE)
			dt->data->size = dtcount(dt->data->here);
		else if(dt->data->type&(DT_LIST|DT_STACK|DT_QUEUE))
		{	t = (dt->data->type&DT_LIST) ? dt->data->head : dt->data->here;
			for(size = 0; t; t = t->right)
				size += 1;
			dt->data->size = size;
		}
	}

	return dt->data->size;
}
