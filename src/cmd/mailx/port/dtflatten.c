/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*Copyright (c) 1978-2004 The Regents of the University of Californi*
*                                                                  *
*          Permission is hereby granted, free of charge,           *
*       to any person obtaining a copy of THIS SOFTWARE FILE       *
*            (the "Software"), to deal in the Software             *
*              without restriction, including without              *
*           limitation the rights to use, copy, modify,            *
*                merge, publish, distribute, and/or                *
*            sell copies of the Software, and to permit            *
*            persons to whom the Software is furnished             *
*          to do so, subject to the following disclaimer:          *
*                                                                  *
*THIS SOFTWARE IS PROVIDED BY The Regents of the University of Cali*
*         ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,         *
*            INCLUDING, BUT NOT LIMITED TO, THE IMPLIED            *
*            WARRANTIES OF MERCHANTABILITY AND FITNESS             *
*             FOR A PARTICULAR PURPOSE ARE DISCLAIMED.             *
*IN NO EVENT SHALL The Regents of the University of California and *
*         BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,          *
*           SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES           *
*           (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT            *
*          OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,           *
*           DATA, OR PROFITS; OR BUSINESS INTERRUPTION)            *
*          HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,          *
*          WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT          *
*           (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING            *
*           IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,            *
*        EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.        *
*                                                                  *
*            Information and Software Systems Research             *
*The Regents of the University of California and AT&T Labs Research*
*                         Florham Park NJ                          *
*                                                                  *
*                        Kurt Shoens (UCB)                         *
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
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
