/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1996-2000 AT&T Corp.              *
*      and it may only be used by you under license from       *
*                     AT&T Corp. ("AT&T")                      *
*       A copy of the Source Code Agreement is available       *
*              at the AT&T Internet web site URL               *
*                                                              *
*     http://www.research.att.com/sw/license/ast-open.html     *
*                                                              *
*     If you received this software without first entering     *
*       into a license with AT&T, you have an infringing       *
*           copy and cannot use it without violating           *
*             AT&T's intellectual property rights.             *
*                                                              *
*               This software was created by the               *
*               Network Services Research Center               *
*                      AT&T Labs Research                      *
*                       Florham Park NJ                        *
*                                                              *
*               Phong Vo <kpv@research.att.com>                *
*             Glenn Fowler <gsf@research.att.com>              *
*                                                              *
***************************************************************/
/*	Verify that records are sorted.
**
**	Written by Kiem-Phong Vo (09/18/96).
*/

#include	"rshdr.h"

typedef struct rsverify_s
{	Rsobj_t	obj;
	ulong	o;	/* obj's ordinal	*/
	ulong	n;	/* number of objects	*/
} Rsverify_t;

#if __STD_C
static int verifyinsert(Rs_t* rs, reg Rsobj_t* obj)
#else
static int verifyinsert(rs, obj)
Rs_t*		rs;
reg Rsobj_t*	obj;
#endif
{
	reg int		cmp, n;
	reg ulong	oh, ph;
	reg uchar*	k;
	reg Rsobj_t*	p;
	reg Rsverify_t*	verify = (Rsverify_t*)rs->methdata;

	p = &verify->obj; ph = p->order;

	OBJHEAD(obj); oh = obj->order;
	if(verify->n > 0)	/* compare with current obj */
	{	OBJCMP(obj,p,cmp);

		if(cmp == 0 && (rs->type&RS_DATA) )	/* compare by obj->data */
		{	k = obj->data; obj->data = obj->key; obj->key = k;
			n = obj->datalen; obj->datalen = obj->keylen; obj->keylen = n;
			OBJHEAD(obj);
			k = p->data; p->data = p->key; p->key = k;
			n = p->datalen; p->datalen = p->keylen; p->keylen = n;
			OBJHEAD(p);

			OBJCMP(obj,p,cmp);

			k = obj->data; obj->data = obj->key; obj->key = k;
			n = obj->datalen; obj->datalen = obj->keylen; obj->keylen = n;
			k = p->data; p->data = p->key; p->key = k;
			n = p->datalen; p->datalen = p->keylen; p->keylen = n;
			obj->order = oh; p->order = ph;
		}
		if(rs->type&RS_REVERSE)
			cmp = -cmp;

		/* out of order */
		if((cmp == 0 && (rs->type&RS_UNIQ)) || cmp < 0 )
		{	if(!rs->disc->eventf)
				return -1;

			obj->equal = obj->left = obj->right = NIL(Rsobj_t*);
			if(cmp == 0)
				obj->equal = p;
			else if(rs->type&RS_REVERSE)
				obj->left = p;
			else	obj->right = p;

			obj->order = verify->n;
			p->order = verify->o;

			n = (*rs->disc->eventf)(rs,RS_VERIFY,(Void_t*)obj,rs->disc);

			obj->order = oh; p->order = ph;

			if(n < 0)	/* abort now */
				return -1;
			else if(n == 0)	/* pretend this didn't exist */
			{	if(rs->disc->defkeyf && obj->key)
					vmfree(rs->vm,obj->key);
				goto done;
			}
			/* else if(n > 0); start comparing from this one */
		}
	}

	if(p->left == p) /* p->data was our allocation */
	{	if(p->data)
			vmfree(rs->vm,p->data);
		p->left = NIL(Rsobj_t*);
	}

	/* switch current object */
	p->order = obj->order;
	p->data = obj->data;
	p->datalen = obj->datalen;

	if(rs->disc->defkeyf && p->key)
		vmfree(rs->vm,p->key);
	p->key = obj->key;
	p->keylen = obj->keylen;

	verify->o = verify->n;

done:	/* free obj */
	obj->right = rs->free;
	rs->free = obj;

	verify->n += 1;
	return 0;
}

#if __STD_C
static Rsobj_t* verifylist(Rs_t* rs)
#else
static Rsobj_t* verifylist(rs)
Rs_t*		rs;
#endif
{
	reg Rsverify_t*	verify = (Rsverify_t*)rs->methdata;
	reg Rsobj_t*	p = &verify->obj;
	reg uchar*	k;

	if(verify->n > 0 && p->data)
	{	/* save data space for last object */
		if(!(k = (uchar*)vmalloc(rs->vm,p->datalen)) )
			return NIL(Rsobj_t*);
		memcpy(k,p->data,p->datalen);
		p->data = k;

		if(!rs->disc->defkeyf)
			p->key = p->data + rs->disc->key;

		p->left = p; /* so we know that p->data is our space */
	}

	return NIL(Rsobj_t*);
}

/* public method */
static Rsmethod_t _Rsverify =
{	verifyinsert,
	verifylist,
	sizeof(Rsverify_t),
	RS_MTVERIFY,
	"verify",
	"Verify that the input is sorted."
};

Rsmethod_t*	Rsverify = &_Rsverify;
