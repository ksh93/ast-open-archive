#include	"dthdr.h"

/*
**	Change the discipline structure.
**	Duplicates caused by the new functions will be removed.
**	dt :	dictionary
**	disc :	discipline
**
**	Written by Kiem-Phong Vo (7/15/95)
*/

#if __STD_C
static Void_t* dtmemory(Dt_t* dt,Void_t* addr,size_t size,Dtdisc_t* disc)
#else
static Void_t* dtmemory(dt, addr, size, disc)
Dt_t* 		dt;	/* dictionary			*/
Void_t* 	addr;	/* address to be manipulate	*/
size_t		size;	/* size to obtain		*/
Dtdisc_t* 	disc;	/* discipline			*/
#endif
{
	if(addr)
	{	free(addr);
		return NIL(Void_t*);
	}
	else	return size > 0 ? malloc(size) : NIL(Void_t*);
}

#if __STD_C
Dtdisc_t* dtdisc(Dt_t* dt, Dtdisc_t* disc, int type)
#else
Dtdisc_t* dtdisc(dt,disc,type)
Dt_t*		dt;
Dtdisc_t*	disc;
int		type;
#endif
{
	reg Dtsearch_f	searchf;
	reg Dtlink_t	*root, *t, *next, **slot, **eslot;
	reg char*	k;
	reg int		rehash;
	reg Dtdisc_t*	old;

	if(!(old = dt->disc) )	/* initialization call from dtopen() */
	{	dt->disc = disc;
		if(!(dt->memoryf = disc->memoryf) )
			dt->memoryf = dtmemory;
		return disc;
	}

	if(!disc)	/* only want to know current discipline */
		return old;

	searchf = dt->meth->searchf;

	UNFLATTEN(dt);

	if(old->eventf && (*old->eventf)(dt,DT_DISC,(Void_t*)disc,old) < 0)
		return NIL(Dtdisc_t*);

	dt->disc = disc;
	if(!(dt->memoryf = disc->memoryf) )
		dt->memoryf = dtmemory;

	/* no need to do anything further */
	if((dt->data->type&(DT_STACK|DT_QUEUE)) ||
	   (type&(DT_COMPARF|DT_HASHF)) == (DT_COMPARF|DT_HASHF) )
		return old;

	if(dt->data->type&DT_HASH)
	{	/* collect all elements into a list */
		root = next = NIL(Dtlink_t*);
		for(eslot = (slot = dt->data->htab)+dt->data->ntab; slot < eslot; ++slot)
		{	if(!(t = *slot) )
				continue;
			if(next)
				next->right = t;
			else	root = next = t;
			while((t = next->right) )
				next = t;
			*slot = NIL(Dtlink_t*);
		}

		/* reinsert them */
		dt->data->size = 0;
		dt->data->here = NIL(Dtlink_t*);
		while(root)
		{	next = root->right;
			if(!(type&DT_HASHF))	/* new hash value */
			{	k = (char*)OBJ(root,disc); k = KEY((Void_t*)k,disc);
				root->hash = HASH(dt,k,disc);
			}
			(void)(*searchf)(dt,(Void_t*)root,DT_RENEW);
			root = next;
		}
	}
	else if(!(type&DT_COMPARF) )
	{	dt->data->size = 0;
		if(dt->data->type&DT_TREE)
		{	root = dt->data->here;
			dt->data->here = NIL(Dtlink_t*);
			while(root)
			{	while((t = root->left) )
					RROTATE(root,t);
				next = root->right;
				(void)(*searchf)(dt,(Void_t*)root,DT_RENEW);
				root = next;
			}
		}
		else /*if(dt->data->type&DT_LIST)*/
		{	root = dt->data->head;
			dt->data->head = NIL(Dtlink_t*);
			while(root)
			{	t = root->right;
				(void)(*searchf)(dt,(Void_t*)root,DT_RENEW);
				root = t;
			}
		}
	}

	return old;
}
