/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1985-2000 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*                 This software was created by the                 *
*                 Network Services Research Center                 *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                David Korn <dgk@research.att.com>                 *
*                 Phong Vo <kpv@research.att.com>                  *
*                                                                  *
*******************************************************************/
#include	"sfhdr.h"

/*	Push back one byte to a given SF_READ stream
**
**	Written by Kiem-Phong Vo.
*/
#if __STD_C
static int _uexcept(reg Sfio_t* f, reg int type, Void_t* val, reg Sfdisc_t* disc)
#else
static int _uexcept(f,type,val,disc)
reg Sfio_t	*f;
reg int		type;
Void_t*		val;
reg Sfdisc_t	*disc;
#endif
{	
	NOTUSED(val);

	/* hmm! This should never happen */
	if(disc != _Sfudisc)
		return -1;

	/* close the unget stream */
	if(type != SF_CLOSE)
		(void)sfclose((*_Sfstack)(f,NIL(Sfio_t*)));

	return 1;
}

#if __STD_C
int sfungetc(reg Sfio_t* f, reg int c)
#else
int sfungetc(f,c)
reg Sfio_t*	f;	/* push back one byte to this stream */
reg int		c;	/* the value to be pushed back */
#endif
{
	reg Sfio_t*	uf;

	SFMTXSTART(f, -1)

	if(c < 0 || (f->mode != SF_READ && _sfmode(f,SF_READ,0) < 0))
		SFMTXRETURN(f, -1);
	SFLOCK(f,0);

	/* fast handling of the typical unget */
	if(f->next > f->data && f->next[-1] == (uchar)c)
	{	f->next -= 1;
		goto done;
	}

	/* make a string stream for unget characters */
	if(f->disc != _Sfudisc)
	{	if(!(uf = sfnew(NIL(Sfio_t*),NIL(char*),(size_t)SF_UNBOUND,
				-1,SF_STRING|SF_READ)))
		{	c = -1;
			goto done;
		}
		_Sfudisc->exceptf = _uexcept;
		sfdisc(uf,_Sfudisc);
		SFOPEN(f,0); (void)sfstack(f,uf); SFLOCK(f,0);
	}

	/* space for data */
	if(f->next == f->data)
	{	reg uchar*	data;
		if(f->size < 0)
			f->size = 0;
		if(!(data = (uchar*)malloc(f->size+16)))
		{	c = -1;
			goto done;
		}
		f->flags |= SF_MALLOC;
		if(f->data)
			memcpy((char*)(data+16),(char*)f->data,f->size);
		f->size += 16;
		f->data  = data;
		f->next  = data+16;
		f->endb  = data+f->size;
	}

	*--f->next = (uchar)c;
done:
	SFOPEN(f,0);
	SFMTXRETURN(f, c);
}
