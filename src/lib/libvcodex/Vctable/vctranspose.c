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

/*	Transpose a table of some given row length.
**
**	Written by Kiem-Phong Vo (kpv@research.att.com)
*/

typedef struct _transctxt_s
{	size_t	ncols;	/* number of columns in table	*/
	int	ctxt;	/* context # of secondary coder	*/
} Transctxt_t;

/* compute the # of columns from training data */
#if __STD_C
static ssize_t transtrain(const Void_t* train, size_t trsz)
#else
static ssize_t transtrain(train, trsz)
Void_t*	train;	/* training data */
size_t	trsz;
#endif
{
	ssize_t		ncols, sz;

	if(!train || trsz <= 0)
		return 1;

#define SAMPLE	(64*1024)
	for(sz = trsz < SAMPLE ? trsz : SAMPLE; sz <= trsz; sz *= 2)
		if((ncols = vcperiod(train, sz)) > 0)
			break;

	return ncols <= 0 ? 1 : ncols;
}

/* combining transpose and run-length-encoding - an optimization  */
#if __STD_C
static ssize_t transrle(Vcodex_t* vc, const Void_t* data,
			size_t ncols, size_t nrows, Void_t** out)
#else
static ssize_t transrle(vc, data, ncols, nrows, out)
Vcodex_t*	vc;
Void_t*		data;
ssize_t		ncols, nrows;
Void_t**	out;
#endif
{
	reg Vcchar_t	*run, *chr, *dt, ch;
	Vcchar_t	*output, *enddt;
	reg ssize_t	c, r;
	ssize_t		hd, sz, size;
	Vcio_t		io;

	size = nrows*ncols;
	hd = VCSIZEU(size) + (VCGETMETH(vc,Void_t*) ? 0 : VCSIZEU(ncols));
	if(!(output = vcsetbuf(vc, NIL(Vcchar_t*), 2*(size + VCSIZEU(size)), hd)) )
		return -1;

	chr = output+VCSIZEU(size);
	run = chr+size+VCSIZEU(size);

	ch = -1; r = 0;
	for(enddt = (Vcchar_t*)data+size, c = 0; c < ncols; c += 1)
	for(dt = (Vcchar_t*)data + c; dt < enddt; dt += ncols)
	{	if(*dt == ch)
			r += 1;
		else
		{	if(r > 0)
			{	if(r >= 3 || ch == RL_ESC)
				{	if(r < (1<<7) )
						*run++ = r;
					else if(r < (1<<14) )
					{	*run++ = (r>>7)|128;
						*run++ = r&127;
					}
					else
					{	vcioinit(&io, run, 2*sizeof(ssize_t));	
						vcioputu(&io, r);
						run = vcionext(&io);
					}

					*chr++ = RL_ESC;
					if(ch != RL_ESC || r > 1)
						*chr++ = ch;
				}
				else
				{	*chr++ = ch;
					if(r == 2)
						*chr++ = ch;
				}
			}

			ch = *dt; r = 1;
		}
	}

	if(r > 0)
	{	if(r >= 3 || ch == RL_ESC)
		{	vcioinit(&io, run, 2*sizeof(ssize_t));
			vcioputu(&io, r);
			run = vcionext(&io);

			*chr++ = RL_ESC;
			if(ch != RL_ESC || r > 1)
				*chr++ = ch;
		}
		else
		{	*chr++ = ch;
			if(r == 2)
				*chr++ = ch;
		}
	}

	c = chr - (output+VCSIZEU(size)); chr = output+VCSIZEU(size);
	r = run - (chr+size+VCSIZEU(size)); run = chr+size+VCSIZEU(size);

	if(vc->coder->coder) /* note that vc->coder is Vcrle */
	{	sz = 2*(size + VCSIZEU(size));
		if((sz = _vcrle2coder(vc->coder,hd,chr,c,run,r,&output,sz)) < 0)
			return -1;
	}
	else
	{	vcioinit(&io, output, 2*(size+hd));
		vcioputu(&io, c);
		vcioputs(&io, chr, c);
		vcioputu(&io, r);
		vcioputs(&io, run, r);
		sz = vciosize(&io);
	}

	output -= hd;
	vcioinit(&io, output, hd);
	if(VCGETMETH(vc, Void_t*) == NIL(Void_t*))
		vcioputu(&io, ncols);
	vcioputu(&io, size);

	if(out)
		*out = output;
	return sz+hd;
}

#if __STD_C
static ssize_t transpose(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t transpose(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	reg ssize_t	r, rr, nr, c, cc, nc;
	Vcchar_t	*rdt, *cdt;
	Vcchar_t	*output;
	ssize_t		nrows, ncols, sz, hdsz;
	Vcio_t		io;
	Transctxt_t	*ctxt = VCGETCTXT(vc, Transctxt_t*);

	vc->undone = 0;
	if(size == 0)
		return 0;

	if(vc->disc)
		ncols = vc->disc->size;
	else if((ncols = ctxt->ncols) <= 1 )
		ncols = 0;
	hdsz = 0; /* amount of space for storing ncols if needed */

	if(vc->flags & VC_ENCODE)
	{	if(ncols <= 0) /* compute #columns from data */
			ncols = transtrain(data,size);
		ctxt->ncols = ncols;

		if(VCGETMETH(vc, Void_t*) == NIL(Void_t*))
			hdsz = VCSIZEU(ncols);

		nrows = size/ncols;
		vc->undone = size - ncols*nrows;
		if((sz = ncols*nrows) == 0)
			return 0;

		if(vc->coder && vc->coder->meth == Vcrle && 
		   /* this tests for rle2() in Vcrle */
		   (vcextract(vc->coder,(Void_t**)&rdt,0) <= 0 || !rdt || !rdt[0]) )
			return transrle(vc, data, ncols, nrows, out);
	}
	else /* if(vc->flags & VC_DECODE) */
	{	sz = size;

		if(VCGETMETH(vc, Void_t*) == NIL(Void_t*))
		{	vcioinit(&io, data, sz);
			if((ncols = vciogetu(&io)) <= 0)
				return -1;
			data = (Void_t*)vcionext(&io);
			sz = vciomore(&io);
		}

		if(vc->coder)
		{	if(VCSETCTXT(vc->coder,ctxt->ctxt) < 0)
				return -1;	
			if(VCCODER(vc, vc->coder, 0, data, sz) < 0 )
				return -1;
		}
		if(sz == 0)
			return 0;

		nrows = ncols; /* rows and columns switch roles */
		ncols = sz/nrows;
		if(nrows*ncols != sz)
			return -1;
	}

	if(!(output = vcsetbuf(vc, NIL(Vcchar_t*), sz, hdsz)) )
		return -1;

#define BLOCK	32	/* transposing small blocks to be cache-friendly */
	for(r = 0; r < nrows; r += BLOCK)
	{	nr = (nrows-r) < BLOCK ? (nrows-r) : BLOCK;
		for(c = 0; c < ncols; c += BLOCK)
		{	nc = (ncols-c) < BLOCK ? (ncols-c) : BLOCK;
			rdt = (Vcchar_t*)data + r*ncols + c;
			for(rr = 0; rr < nr; ++rr, rdt += ncols-nc)
			{	cdt = output + c*nrows + r+rr;
				for(cc = 0; cc < nc; ++cc, cdt += nrows)
					*cdt = *rdt++;
			}
		}
	}

	if(vc->flags & VC_ENCODE)
	{	if(vc->coder)
		{	if(VCSETCTXT(vc->coder, ctxt->ctxt) < 0)
				return -1;
			if(VCCODER(vc, vc->coder, hdsz, output, sz) < 0 )
				return -1;
		}

		if(VCGETMETH(vc,Void_t*) == NIL(Void_t*)) /* code the #columns */
		{	output -= hdsz;
			sz += hdsz;
			vcioinit(&io, output, hdsz);
			vcioputu(&io, ncols);
		}
	}

	if(out)
		*out = output;

	return sz;
}

static Vcmtarg_t _Transargs[] =
{	{ "0", "\060", "No coding of number of columns in data", (Void_t*)1 },
	{  0 , 0, "Number of columns encoded in data", (Void_t*)0 }
};

#if __STD_C
static ssize_t transextract(Vcodex_t* vc, Void_t** datap, int state)
#else
static ssize_t transextract(vc, datap, state)
Vcodex_t*	vc;
Void_t**	datap;	/* return the coding of #cols	*/
int		state;	/* get state only - unused	*/
#endif
{
	Vcchar_t	*output;
	ssize_t		sz, ncols;
	Vcio_t		io;
	Transctxt_t	*ctxt = VCGETCTXT(vc, Transctxt_t*);

	if(state)
		return 0;

	ncols = VCGETMETH(vc,Void_t*) ? ctxt->ncols : 0;

	sz = VCSIZEU(ncols);
	if(!(output = vcsetbuf(vc, NIL(Vcchar_t*), sz, 0)) )
		return -1;
	vcioinit(&io, output, sz);
	vcioputu(&io, ncols);
	if(datap)
		*datap = (Void_t*)output;
	return sz;
}

#if __STD_C
static Vcodex_t* transrestore(Vcodex_t* vc, Void_t* data, ssize_t dtsz)
#else
static Vcodex_t* transrestore(vc, data, dtsz)
Vcodex_t*	vc;
Void_t*		data;	/* #cols	*/
ssize_t		dtsz;
#endif
{
	ssize_t		sz;
	Vcio_t		io;
	Transctxt_t	*ctxt;

	if(vc)
		return vc;

	if(dtsz == 0) /* need column size */
		return NIL(Vcodex_t*);

	vcioinit(&io, data, dtsz);
	sz = vciogetu(&io);

	if(!(vc = vcopen(0, Vctranspose, sz ? (Void_t*)"0" : NIL(Void_t*), 0, VC_DECODE)) )
		return NIL(Vcodex_t*);
	if(!(ctxt = VCGETCTXT(vc, Transctxt_t*)) )
	{	vcclose(vc);
		return NIL(Vcodex_t*);
	}
	else	ctxt->ncols = sz;

	return vc;
}

#if __STD_C
static int transevent(Vcodex_t* vc, int type, Void_t* params)
#else
static int transevent(vc, type, params)
Vcodex_t*	vc;
int		type;
Void_t*		params;
#endif
{
	Transctxt_t	*ctxt;
	int		k;

	if(type == VC_OPENING)
	{	for(k = 0; _Transargs[k].name; ++k)
			if(params && strcmp((char*)params, _Transargs[k].name) == 0)
				break;
		VCINITCTXT(vc, Transctxt_t);
		VCSETMETH(vc, _Transargs[k].data);
		return 0;
	}
	else if(type == VC_INITCTXT)
	{	if(!(ctxt = (Transctxt_t*)params) )
			return -1;
		else
		{	ctxt->ncols = 0;
			ctxt->ctxt  = -1;
			return 1;
		}
	}
	else	return 0;
}

Vcmethod_t _Vctranspose =
{	transpose,
	transpose,
	transextract,
	transrestore,
	transevent,
	"transpose", "\164\162\141\156\163\160\157\163\145", "Transposing a table",
	_Transargs,
	256*1024,
	VCNEXT(Vctranspose)
};

VCLIB(Vctranspose)
