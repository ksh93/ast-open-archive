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
**	Stack&queue.
**	dtsearch() moves an element to head of list.
**	dtdelete() always deletes from head of list.
**	List is doubly linked with head->left points to last elt.
**
**	Written by Kiem-Phong Vo (07/15/95)
*/

#if __STD_C
static Void_t* dtstkq(Dt_t* dt, reg Void_t* obj, reg int type)
#else
static Void_t* dtstkq(dt, obj, type)
Dt_t*		dt;
reg Void_t*	obj;
reg int		type;
#endif
{
	reg Dtdisc_t*	disc = dt->disc;
	reg Dtlink_t	*r, *t;

	if(type&DT_DELETE)
	{	/* always happens at head of list */
		t = dt->data->head;
		if(!t || (obj && obj != OBJ(t,disc)) )
			return NIL(Void_t*);
		dt->data->head = t->right;
		if(t->right) /* head->left always points to last element */
			t->right->left = t->left;

		if((dt->data->size -= 1) < 0)
			dt->data->size = -1;
		obj = OBJ(t,disc);
		if(disc->freef)
			(*disc->freef)(dt,obj,disc);
		if(disc->link < 0)
			(*dt->memoryf)(dt,(Void_t*)t,0,disc);
		return obj;
	}

	if(!obj)
	{	if(!(r = dt->data->head) || !(type&(DT_CLEAR|DT_FIRST|DT_LAST)) )
			return NIL(Void_t*);
		else if(type&DT_CLEAR)
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
				r = r->left;
			dt->data->here = r;
			return OBJ(r,disc);
		}
	}

	if(type&DT_INSERT) /* always allow inserts */
	{	if(disc->makef && !(obj = (*disc->makef)(dt,obj,disc)) )
			return NIL(Void_t*);
		if(disc->link < 0)
		{	t = (Dtlink_t*)(*dt->memoryf)
					(dt,NIL(Void_t*),sizeof(Dthold_t),disc);
			if(!t)
			{	if(disc->freef && disc->makef)
					(*disc->freef)(dt,obj,disc);
				return NIL(Void_t*);
			}
			else	((Dthold_t*)t)->obj = obj;
		}
		else	t = ELT(obj,disc);

		if(!(r = dt->data->head) )
		{	dt->data->head = t;
			t->right = NIL(Dtlink_t*);
			t->left = t;
		}
		else if(dt->data->type&DT_STACK) /* stack: add to start of list */
		{	t->right = r;
			t->left = r->left;
			r->left = t;
			dt->data->head = t;
		}
		else /*if(dt->data->type&DT_QUEUE)*/ /* queue: add to end of list */
		{	r = r->left;
			r->right = t;
			t->right = NIL(Dtlink_t*);
			t->left = r;
			dt->data->head->left = t;
		}
		if((dt->data->size += 1) <= 0)
			dt->data->size = -1;
		return obj;
	}

	/* DT_MATCH|DT_SEARCH|DT_NEXT|DT_PREV: see if it exists */
	if(!(t = dt->data->here) || OBJ(t,disc) != obj)
	{	reg char	*k, *key = KEY(obj,disc);
		for(t = dt->data->head; t; t = t->right)
		{	k = (char*)OBJ(t,disc); k = KEY((Void_t*)k,disc);
			if(CMP(dt,key,k,disc) == 0)
				break;
		}
		if(!t)
			return NIL(Void_t*);
	}

	if(type&DT_NEXT)
		t = t->right;
	else if(type&DT_PREV)
		t = t == dt->data->head ? NIL(Dtlink_t*) : t->left;
	else if(type&(DT_SEARCH|DT_MATCH))
	{	if(t != dt->data->head) /* move to head of list */
		{	t->left->right = t->right;
			if(t->right)
				t->right->left = t->left;
			t->right = dt->data->head;
			t->left = dt->data->head->left;
			dt->data->head->left = t;
			dt->data->head = t;
		}
	}
	else	t = NIL(Dtlink_t*);

	dt->data->here = t;
	return t ? OBJ(t,disc) : NIL(Void_t*);
}

/* exporting method */
Dtmethod_t _Dtstack = { dtstkq, DT_STACK };
Dtmethod_t _Dtqueue = { dtstkq, DT_QUEUE };
