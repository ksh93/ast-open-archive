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
