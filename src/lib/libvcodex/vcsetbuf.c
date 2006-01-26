/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 2003-2006 AT&T Corp.                  *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                            by AT&T Corp.                             *
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

/*	Allocate a buffer for a handle. 
**
**	Written by Kiem-Phong Vo (kpv@research.att.com)
*/

#if __STD_C
Vcchar_t* vcsetbuf(Vcodex_t* vc, Vcchar_t* here, ssize_t bsize, ssize_t head)
#else
Vcchar_t* vcsetbuf(vc, here, bsize, head)
Vcodex_t*	vc;
Vcchar_t*	here;	/* if != 0, points somewhere in vc->buf	*/
ssize_t		bsize;	/* the size needed to buffer data 	*/
ssize_t		head;	/* initial head room (here == NULL)	*/
#endif
{
	Vcbuf_t		*b, *next;

	if(bsize < 0) /* free all buffers */
	{	for(; vc; vc = vc->coder)
		{	if(vc->meth->eventf)
				(*vc->meth->eventf)(vc, VC_FREEBUF, 0);

			for(b = vc->blist; b; b = next)
			{	next = b->next;
				free(b);
			}
			vc->blist = NIL(Vcbuf_t*);

			if(vc->buf)
			{	free(vc->buf - vc->bhead);
				vc->buf = NIL(Vcchar_t*);
				vc->bsize = vc->bhead = 0;
			}
		}

		return NIL(Vcchar_t*);
	}

	if(here) /* resizing the current buffer */
	{	Vcchar_t *buf, *obuf = vc->buf;

		if(here < obuf || here > obuf+vc->bsize)
			return NIL(Vcchar_t*);
		if(bsize <= vc->bsize)
			return here;

		if(!(buf = (Vcchar_t*)malloc(bsize+vc->bhead)) )
			return NIL(Vcchar_t*);	
		memcpy(buf, obuf - vc->bhead, vc->bsize + vc->bhead);
		free(vc->buf - vc->bhead); /* reclaim old space */
		vc->buf = buf + vc->bhead;
		vc->bsize = bsize;

		return vc->buf + (here - obuf); /* return the same offset */
	}

	if(vc->buf) /* save current buffer in the inactive but busy list */
	{	b = (Vcbuf_t*)(vc->buf - vc->bhead);
		b->next = vc->blist; vc->blist = b;
		vc->buf = NIL(Vcchar_t*);
		vc->bsize = vc->bhead = 0;
	}

	head  = (head < 0 ? 0 : head) + vc->head; /* required head room */
	if(!(b = (Vcbuf_t*)malloc(sizeof(Vcbuf_t)+ head + bsize)) )
		return NIL(Vcchar_t*);
	vc->bsize = bsize;
	vc->bhead = sizeof(Vcbuf_t) + head;
	vc->buf   = ((Vcchar_t*)b) + vc->bhead;

	return vc->buf;
}
