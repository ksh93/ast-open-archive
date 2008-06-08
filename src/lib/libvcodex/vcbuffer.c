/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2008 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#include	"vchdr.h"

/*	Managing buffered data for a handle. 
**
**	Written by Kiem-Phong Vo (kpv@research.att.com)
*/

#if __STD_C
Vcchar_t* _vcbuffer(Vcodex_t* vc, Vcchar_t* trunc, ssize_t size, ssize_t head)
#else
Vcchar_t* _vcbuffer(vc, trunc, size, head)
Vcodex_t*	vc;
Vcchar_t*	trunc;	/* if != NULL, a buffer to be truncated	*/
ssize_t		size;	/* the size needed for buffered data 	*/
ssize_t		head;	/* head room in front of buffer 	*/
#endif
{
	Vcbuffer_t	*b, *n;

	if(!vc)
		return NIL(Vcchar_t*);

	if(trunc) /* truncating a buffer */
	{	/* find the buffer */
		for(; vc; vc = vc->coder)
		{	for(n = NIL(Vcbuffer_t*), b = vc->list; b; n = b, b = b->next)
				if(trunc >= b->buf && trunc < b->buf+b->size)
					break;
			if(!b) /* not in this handle */
				continue;

			if(n) /* isolate b from buffer pool */
				n->next = b->next;
			else	vc->list = b->next;

			if(size < 0 ) /* just free the buffer */
			{	vc->busy -= b->size;
				vc->nbuf -= 1;
				free(b);
				return NIL(Vcchar_t*);
			}

			if(trunc+size > b->buf+b->size) /* no extension */
			{	b->next = vc->list;
				vc->list = b;
				return NIL(Vcchar_t*); 
			}

			size += (head = trunc - (Vcchar_t*)b->buf);
			if(size < 3*b->size/4 )
			{	if(!(n = (Vcbuffer_t*)realloc(b, sizeof(Vcbuffer_t)+size)) )
					return NIL(Vcchar_t*);
				vc->busy -= n->size - size; /* n->size is old b->size */
				n->size = size;
				if(n != b)
					b = n;
			}

			b->next = vc->list;
			vc->list = b;
			return (Vcchar_t*)(&b->buf[head]);
		}

		return NIL(Vcchar_t*);
	}
	else if(size < 0) /* free all buffers */
	{	for(; vc; vc = vc->coder)
		{	if(vc->meth->eventf)
				(*vc->meth->eventf)(vc, VC_FREEBUFFER, 0);

			for(b = vc->list; b; b = n)
			{	n = b->next;
				free(b);
			}

			vc->list = NIL(Vcbuffer_t*);
			vc->busy = 0;
			vc->nbuf = 0;
		}

		return NIL(Vcchar_t*);
	}
	else
	{	head = (head <= 0 ? 0 : head) + vc->head; /* required head room */
		if(!(b = (Vcbuffer_t*)malloc(sizeof(Vcbuffer_t)+head+size)) )
			return NIL(Vcchar_t*);
		b->size = head+size;
		b->next = vc->list;
		b->file = vc->file; vc->file = NIL(char*);
		b->line = vc->line; vc->line = 0;
		vc->list = b;
		vc->busy += b->size;
		vc->nbuf += 1;
		return (Vcchar_t*)(&b->buf[head]);
	}
}

/* default vctellbuf() prints buffer data */
#if __STD_C
static int printbuf(Vcodex_t* vc, Vcbuffer_t* list, Vcmethod_t* meth)
#else
static int printbuf(vc, list, meth)
Vcodex_t*	vc;
Vcbuffer_t*	list;	/* list of buffers	*/
Vcmethod_t*	meth;	/* meth being executed	*/
#endif
{
	char	buf[1024], *b, *endb;
	ssize_t	n;

	for(; list; list = list->next)
	{	b = buf; endb = buf+sizeof(buf) - 16;

		n = vcitoa(TYPECAST(Vcint_t,list), b, endb-b); b += n; *b++ = ':'; 

		n = vcitoa(TYPECAST(Vcint_t,list->size), b, endb-b); b += n; *b++ = ':'; 

		n = (n = strlen(meth->name)) < (endb-b) ? n : endb-b;
		memcpy(b, meth->name, n); b += n; *b++ = ':';

		n = (n = strlen(list->file)) < (endb-b) ? n : endb-b;
		memcpy(b, list->file, n); b += n; *b++ = ':';

		n = vcitoa(TYPECAST(Vcint_t,list->line), b, endb-b); b += n; *b++ = '\n'; 

		if(write(2, buf, b-buf) < 0 )
			return -1;
	}

	return 0;
}

/* This is mostly for debugging. bufferf is called on each list of buffers per handle */
#if __STD_C
int vctellbuf(Vcodex_t* vc, Vcbuffer_f bufferf)
#else
int vctellbuf(vc, bufferf)
Vcodex_t	*vc;
Vcbuffer_f	bufferf;
#endif
{
	if(!bufferf)
		bufferf = printbuf;

	for(; vc; vc = vc->coder)
	{	if(vc->list && (*bufferf)(vc, vc->list, vc->meth) < 0)
			return -1;
		if(vc->meth->eventf && (*vc->meth->eventf)(vc, VC_TELLBUFFER, (Void_t*)bufferf) < 0 )
			return -1;
	}

	return 0;
}
