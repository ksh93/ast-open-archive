#include	"dthdr.h"

/*
**	Process a linked list.
**	Object that compares equal are kept together.
**
**	Written by Kiem-Phong Vo (07/15/95)
*/

#if __STD_C
static Void_t* dtlist(reg Dt_t* dt, reg Void_t* obj, reg int type)
#else
static Void_t* dtlist(dt, obj, type)
reg Dt_t*	dt;
reg Void_t*	obj;
reg int		type;
#endif
{
	reg char	*key, *k;
	reg Dtdisc_t*	disc = dt->disc;
	reg Dtlink_t	*r, *t, *prev, *renew;

	if(!obj)
	{	if(!(r = dt->data->head) || !(type&(DT_CLEAR|DT_FIRST|DT_LAST)) )
			return NIL(Void_t*);
		if(type&DT_CLEAR)
		{	if(disc->freef || disc->link < 0)
			{	do
				{	t = r->right;
					if(disc->freef)
						(*disc->freef)(dt,OBJ(r,disc),disc);
					if(disc->link < 0)
						(*dt->memoryf)(dt,(Void_t*)r,0,disc);
				} while((r = t) );
			}
			dt->data->head = dt->data->here = NIL(Dtlink_t*);
			dt->data->size = 0;
			return NIL(Void_t*);
		}
		else	/* either first or last elt */
		{	if(type&DT_LAST)
				while(r->right)
					r = r->right;
			dt->data->here = r;
			return OBJ(r,disc);
		}
	}

	if(type&DT_MATCH)
		key = (char*)obj;
	else
	{	if(type&DT_RENEW)
		{	renew = (Dtlink_t*)obj;
			obj = OBJ(renew,disc);
		}
		key = KEY(obj,disc);
	}

	/* see if it exists */
	prev = NIL(Dtlink_t*);
	if(!(t = dt->data->here) || OBJ(t,disc) != obj)
	{	for(t = dt->data->head; t; prev = t->left, t = prev->right)
		{	k = (char*)OBJ(t,disc); k = KEY((Void_t*)k,disc);
			if(CMP(dt,key,k,disc) == 0)
				break;
		}
	}

	if(t)
	{	if(type&DT_NEXT)
		{	dt->data->here = t = t->right;
			return t ? OBJ(t,disc) : NIL(Void_t*);
		}

		/* make sure prev is just before the equivalence class of t */
		if(!prev && (r = dt->data->head->left) != t->left)
			while((prev = r) )
				if((r = prev->right->left) == t->left)
					break;

		if(type&(DT_MATCH|DT_SEARCH|DT_DELETE|DT_INSERT|DT_RENEW))
		{	if(prev) /* move the whole class to the front */
			{	t = prev->right;
				prev->right = t->left->right;
				t->left->right = dt->data->head;
				dt->data->head = t;
			}
			else	t = dt->data->head;

			if(type&(DT_SEARCH|DT_MATCH) )
				return OBJ(t,disc);
			else if(type&DT_INSERT)
				goto do_insert;
			else if(type&DT_DELETE)
			{	dt->data->head = t->right;
				obj = OBJ(t,disc);
				if(disc->freef)
					(*disc->freef)(dt,obj,disc);
				if(disc->link < 0)
					(*disc->memoryf)(dt,(Void_t*)t,0,disc);
				if((dt->data->size -= 1) < 0)
					dt->data->size = -1;
				return obj;
			}
			else /*if(type&DT_RENEW)*/
			{	r = renew;
				goto do_renew;
			}
		}
		else /*if(type&DT_PREV)*/
		{	if(!prev && t != dt->data->head)
				prev = dt->data->head;
			if(prev)
				while(prev->right != t)
					prev = prev->right;
			dt->data->here = prev;
			return prev ? OBJ(prev,disc) : NIL(Void_t*);
		}
	}
	else
	{	if(type&DT_RENEW)
		{	r = renew;
			goto do_renew;
		}
		else if(type&DT_INSERT)
		{
		do_insert:
			if(disc->makef && !(obj = (*disc->makef)(dt,obj,disc)) )
				return NIL(Void_t*);
			if(disc->link < 0)
			{	r = (Dtlink_t*)(*dt->memoryf)
					(dt,NIL(Void_t*),sizeof(Dthold_t),disc);
				if(!r)
				{	if(disc->freef && disc->makef)
						(*disc->freef)(dt,obj,disc);
					return NIL(Void_t*);
				}
				((Dthold_t*)r)->obj = obj;
			}
			else	r = ELT(obj,disc);

		do_renew: /* point to last element of equivalence class */
			r->left = t ? t->left : r;
			r->right = dt->data->head;
			dt->data->head = r;
			if((dt->data->size += 1) <= 0)
				dt->data->size = -1;

			return obj;
		}
		else	return NIL(Void_t*);
	}
}

/* exporting method */
Dtmethod_t _Dtlist = { dtlist, DT_LIST };
