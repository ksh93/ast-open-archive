/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*Copyright (c) 1978-2004 The Regents of the University of California an*
*                                                                      *
*            Permission is hereby granted, free of charge,             *
*         to any person obtaining a copy of THIS SOFTWARE FILE         *
*              (the "Software"), to deal in the Software               *
*                without restriction, including without                *
*             limitation the rights to use, copy, modify,              *
*                  merge, publish, distribute, and/or                  *
*              sell copies of the Software, and to permit              *
*              persons to whom the Software is furnished               *
*            to do so, subject to the following disclaimer:            *
*                                                                      *
*THIS SOFTWARE IS PROVIDED BY The Regents of the University of Californ*
*           ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,           *
*              INCLUDING, BUT NOT LIMITED TO, THE IMPLIED              *
*              WARRANTIES OF MERCHANTABILITY AND FITNESS               *
*               FOR A PARTICULAR PURPOSE ARE DISCLAIMED.               *
*IN NO EVENT SHALL The Regents of the University of California and AT&T*
*           BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,            *
*             SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES             *
*             (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT              *
*            OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,             *
*             DATA, OR PROFITS; OR BUSINESS INTERRUPTION)              *
*            HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,            *
*            WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT            *
*             (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING              *
*             IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,              *
*          EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.          *
*                                                                      *
*              Information and Software Systems Research               *
*    The Regents of the University of California and AT&T Research     *
*                           Florham Park NJ                            *
*                                                                      *
*                          Kurt Shoens (UCB)                           *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#include	"dthdr.h"

/*
**	Change searching method.
**
**	Written by Kiem-Phong Vo (07/15/95)
*/

#if __STD_C
Dtmethod_t* dtmethod(Dt_t* dt, Dtmethod_t* meth)
#else
Dtmethod_t* dtmethod(dt, meth)
Dt_t*		dt;
Dtmethod_t*	meth;
#endif
{
	reg Dtlink_t	*list, *t, **slot, **eslot;
	reg int		nhash, n;
	reg Dtsearch_f	searchf;
	reg Dtdisc_t*	disc = dt->disc;
	reg Dtmethod_t*	oldmeth = dt->meth;

	if(!meth || meth->type == oldmeth->type)
		return oldmeth;

	/* these methods cannot be changed */
	if((dt->data->type&(DT_STACK|DT_QUEUE)) ||
	   (meth && (meth->type&(DT_STACK|DT_QUEUE))) )
		return NIL(Dtmethod_t*);

	if(disc->eventf &&
	   (*disc->eventf)(dt,DT_METH,(Void_t*)meth,disc) < 0)
		return NIL(Dtmethod_t*);

	/* get the list of elements */
	list = dtflatten(dt);
	if(dt->data->type&DT_LIST)
		dt->data->head = NIL(Dtlink_t*);
	else if(dt->data->type&DT_HASH)
	{	if(dt->data->ntab > 0)
			(*dt->memoryf)(dt,(Void_t*)dt->data->htab,0,disc);
		dt->data->ntab = 0;
		dt->data->htab = NIL(Dtlink_t**);
	}

	dt->data->here = NIL(Dtlink_t*);
	nhash = dt->data->size;
	dt->data->size = 0;
	dt->data->type = (dt->data->type&~(DT_METHODS|DT_FLATTEN)) | meth->type;
	dt->meth = meth;
	searchf = meth->searchf;
	if(dt->searchf == oldmeth->searchf)
		dt->searchf = searchf;

	if((meth->type&DT_HASH) && nhash > 0)
	{	/* set hash table to proper size */
		for(n = HSLOT; HLOAD(n) < nhash; )
			n = HRESIZE(n);
		slot = (Dtlink_t**)(*dt->memoryf)
				(dt,NIL(Void_t*),n*sizeof(Dtlink_t*),disc);
		if(!slot)
			return NIL(Dtmethod_t*);
		dt->data->ntab = n;
		dt->data->htab = slot;
		for(eslot = slot+n; slot < eslot; ++slot)
			*slot = NIL(Dtlink_t*);
	}

	/* reinsert */
	n = meth->type&DT_HASH;
	while(list)
	{	t = list->right;
		if(n)
		{	reg char* key = (char*)OBJ(list,disc);
			key = KEY((Void_t*)key,disc);
			list->hash = HASH(dt,key,disc);
		}
		(void)(*searchf)(dt,(Void_t*)list,DT_RENEW);
		list = t;
	}

	return oldmeth;
}
