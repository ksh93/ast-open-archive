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
