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

/* Construct/deconstruct data usable for persistent storage of a handle.
**
** Written by Kiem-Phong Vo.
*/

#define N_CODERS	1024	/* max #coders		*/

typedef struct _store_s
{	char*		ident;	/* method id string	*/
	int		idsz;	/* length of id		*/
	Void_t*		data;	/* associated data	*/
	ssize_t		dtsz;	/* length of data	*/
} Store_t;

#if __STD_C
ssize_t vcextract(Vcodex_t* vc, Void_t** datap, int state)
#else
ssize_t vcextract(vc, datap, state)
Vcodex_t*	vc;
Void_t**	datap;
int		state;	/* !=0 only extract states of top coder */
#endif
{
	ssize_t		n, k, dtsz;
	Store_t		store[N_CODERS];
	Vcchar_t	*data;
	Vcodex_t	*coder;
	Vcio_t		io;

	if(!(vc->flags&VC_ENCODE) )
		return -1;

	if(state) /* only extract states of top level coder */
	{	if(vc->meth->extractf)
			return (*vc->meth->extractf)(vc, datap, 1);
		else	return 0;
	}

	dtsz = 0; /* get all the encoding strings */
	for(n = -1, coder = vc; coder; coder = coder->coder)
	{	if((n += 1) >= N_CODERS)
			return -1; /* too many continuation coders */

		store[n].ident = coder->meth->ident;
		store[n].idsz = strlen(coder->meth->ident)+1; /* length + the 0-byte */

		if(coder->meth->extractf)
			store[n].dtsz = (*coder->meth->extractf)(coder, &store[n].data, 0);
		else	store[n].dtsz = 0;
		if(store[n].dtsz < 0)
			return -1;

		dtsz += store[n].idsz + VCSIZEU(store[n].dtsz) + store[n].dtsz;
	}

	if(!(data = vcsetbuf(vc, NIL(Vcchar_t*), dtsz, 0)) )
		return -1;

	/* write out data of all coders */
	vcioinit(&io, data, dtsz);
	for(k = 0; k <= n; ++k)
	{	vcioputs(&io, store[k].ident, store[k].idsz);
		vcioputu(&io, store[k].dtsz);
		vcioputs(&io, store[k].data, store[k].dtsz);
	}

	if(datap)
		*datap = (Void_t*)data;
	return dtsz;
}


#if __STD_C
Vcodex_t* vcrestore(Vcodex_t* vc, Void_t* data, size_t dtsz)
#else
Vcodex_t* vcrestore(vc, data, dtsz)
Vcodex_t*	vc;
Void_t*		data;
size_t		dtsz;
#endif
{
	Vcodex_t	*coder, *cdr;
	Vcmethod_t	*meth;
	ssize_t		sz, k;
	char		*mt;
	Void_t		*dt;
	Vcio_t		io;

	if(vc) /* only restore internal states of top-level coder */
	{	if(vc->meth->restoref )
			vc = (*vc->meth->restoref)(vc, data, dtsz);
		return vc;
	}

	if(!data || dtsz <= 0 )
		return NIL(Vcodex_t*);

	vcioinit(&io, data, dtsz);
	vc = coder = NIL(Vcodex_t*);

	while(vciomore(&io) > 0)
	{	
		mt = (char*)vcionext(&io);
		for(sz = vciomore(&io), k = 0; k < sz; ++k)
			if(mt[k] == 0)
				break;
		if(k >= sz)
			goto error;
		if(!(meth = vcgetmeth(mt, NIL(char*), NIL(char**))) )
			goto error;
		vcioskip(&io, k+1);

		/* get the initialization data, if any */
		if((sz = (ssize_t)vciogetu(&io)) < 0 || sz > vciomore(&io))
			goto error; 
		dt = vcionext(&io);
		vcioskip(&io, sz);

		if(meth->restoref )
			cdr = (*meth->restoref)(NIL(Vcodex_t*), dt, sz);
		else	cdr = vcopen(0, meth, 0, 0, VC_DECODE);
		if(!cdr)
		{ error:
			if(vc)
				vcclose(vc);
			return NIL(Vcodex_t*);
		}

		if(!coder)
			vc = cdr;
		else
		{	coder->coder = cdr;
			coder->flags |= VC_CLOSECODER;
		}
		coder = cdr;
	}

	return vc;
}
