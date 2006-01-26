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
#include	"vcmeth.h"

/*	Burrows-Wheeler transform.
**
**	Written by Kiem-Phong Vo (kpv@research.att.com)
*/

#if __STD_C
static ssize_t vcbwt(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t vcbwt(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	Vcsfx_t		*sfx;
	ssize_t		hd, sp, sz, *idx, *endidx;
	Vcio_t		io;
	Vcchar_t	*dt, *output, *bw;
	ssize_t		rv = -1;
	Vcctxt_t	*ctxt = VCGETCTXT(vc, Vcctxt_t*);

	if(size == 0)
		return 0;

	/* compute suffix array */
	if(!(sfx = vcsfxsort(data,size)) )
		return -1;

	/* compute the location of the sentinel */
	for(endidx = (idx = sfx->idx)+size; idx < endidx; ++idx)
		if(*idx == 0)
			break;
	sp = idx - sfx->idx;

	hd = VCSIZEU(sp); /* encoding size of the 0th position after sorting */
	sz = size + 1; /* size of transformed data */
	if(!(output = vcsetbuf(vc, NIL(Vcchar_t*), sz, hd)) )
		goto done;

	/* compute the transform */
	dt = (Vcchar_t*)data; bw = output;
	for(idx = sfx->idx; idx < endidx; ++idx, ++bw)
	{	if(*idx == 0) /* special coding of the 0th position */
			*bw = idx == sfx->idx ? 0 : *(bw-1);
		else	*bw = dt[*idx - 1];
	}
	*bw = dt[size-1];

	/* filter data thru the continuation coder */
	if(vc->coder)
	{	if(VCSETCTXT(vc->coder, ctxt->ctxt) < 0)
			goto done;
		if(VCCODER(vc, vc->coder, hd, output, sz) < 0 )
			goto done;
	}

	/* write out the sorted location of position-0 in data */
	output -= hd;
	vcioinit(&io, output, hd);
	vcioputu(&io, sp);
	rv = hd+sz;

	if(out)
		*out = output;

done:	free(sfx);
	return rv;
}

#if __STD_C
static ssize_t vcunbwt(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t vcunbwt(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	ssize_t		n, sp, sz;
	Vcchar_t	*dt, *output, *o;
	ssize_t		base[256], *offset;
	Vcio_t		io;
	Vcctxt_t	*ctxt = VCGETCTXT(vc, Vcctxt_t*);

	if(size == 0)
		return 0;

	vcioinit(&io, data, size);

	if((sp = vciogetu(&io)) < 0) /* index of 0th position */
		return -1;

	/* retrieve transformed data */
	sz = vciomore(&io);
	dt = vcionext(&io);

	/* invert continuation coder if there was one */
	if(vc->coder)
	{	if(VCSETCTXT(vc->coder, ctxt->ctxt) < 0)
			return -1;
		if((sz = vcapply(vc->coder, dt, sz, &dt)) <= 0)
			return -1;
	}
	sz -= 1; /* actual data size */

	if(sp >= sz) /* corrupted data */
		return -1;

	/* get space to decode */
	if(!(output = vcsetbuf(vc, NIL(Vcchar_t*), sz, 0)) )
		return -1;

	if(!(offset = (ssize_t*)malloc((sz+1)*sizeof(ssize_t))) )
		return -1;

	/* base and offset vector for bytes */
	for(n = 0; n < 256; ++n)
		base[n] = 0;
	for(n = 0; n <= sz; ++n)
	{	if(n == sp)
			offset[n] = 0;
		else
		{	offset[n] = base[dt[n]];
			base[dt[n]] += 1;
		}
	}
	for(sp = 0, n = 0; n < 256; ++n)
	{	ssize_t	c = base[n];
		base[n] = sp;
		sp += c;
	}

	/* now invert the transform */
	for(n = sz, o = output+sz-1; o >= output; --o)
	{	*o = dt[n];
		n = base[*o] + offset[n];
	}

	free(offset);

	if(out)
		*out = output;

	return sz;
}


Vcmethod_t _Vcbwt =
{	vcbwt,
	vcunbwt,
	0,
	0,
	0,
	"bwt", "\142\167\164", "Burrows-Wheeler transform",
	NIL(Vcmtarg_t*),
	4*1024*1024,
	VCNEXT(Vcbwt)
};

VCLIB(Vcbwt)
