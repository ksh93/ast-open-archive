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

/*	Various run-length-encoding methods.
**
**	Written by Kiem-Phong Vo (kpv@research.att.com)
*/

typedef struct _rle_s	Rle_t;
typedef ssize_t		(*Rle_f)_ARG_((Rle_t*, int));

typedef struct _rlectxt_s
{	int	dctxt;	/* context of subcoder for data	*/
	int	rctxt;	/* context of subcoder for runs	*/
} Rlectxt_t;

struct _rle_s
{
	Rle_f		rlef;	/* rle function to call	*/
	Vcchar_t*	ibuf;	/* default input data	*/
	ssize_t		isiz;

	Vcchar_t*	obuf;	/* default output data	*/
	ssize_t		osiz;

	Vcchar_t*	abuf;	/* auxiliary buffer	*/
	ssize_t		asiz;
};

/* run secondary coder on the two parts of run-length coding */
#if __STD_C
ssize_t _vcrle2coder(Vcodex_t* vc, ssize_t hd,
		   Vcchar_t* chr, ssize_t csz,
		   Vcchar_t* run, ssize_t rsz,
		   Vcchar_t** out, ssize_t outsz)
#else
ssize_t _vcrle2coder(vc, hd, chr, csz, run, rsz, out, outsz)
Vcodex_t*	vc;
ssize_t		hd;	/* buffer header space	*/
Vcchar_t*	chr;	/* character data	*/
ssize_t		csz;
Vcchar_t*	run;	/* run length data	*/
ssize_t		rsz;
Vcchar_t**	out;	/* current output buf	*/
ssize_t		outsz;
#endif
{
	ssize_t		sz;
	Vcchar_t	*output;
	Vcio_t		io;
	Rlectxt_t	*ctxt = VCGETCTXT(vc, Rlectxt_t*);

	/* secondarily encode the data segment */
	if(VCSETCTXT(vc->coder,ctxt->dctxt) < 0 )
		return -1;
	if(VCCODER(vc, vc->coder, 0, chr, csz) < 0)
		return -1;

	/* secondarily encode the run-length segment */
	if(VCSETCTXT(vc->coder,ctxt->rctxt) < 0 )
		return -1;
	if(VCCODER(vc, vc->coder, 0, run, rsz) < 0)
		return -1;

	/* see if buffer needs reallocation */
	output = *out;
	sz = VCSIZEU(csz) + csz + VCSIZEU(rsz) + rsz;
	if(sz > outsz && !(output = vcsetbuf(vc, NIL(Vcchar_t*), sz, hd)) )
		return -1;
		
	vcioinit(&io, output, sz);
	vcioputu(&io, csz);
	vcioputs(&io, chr, csz);
	vcioputu(&io, rsz);
	vcioputs(&io, run, rsz);

	*out = output;
	return sz;
}

/* Encoding 0-run's only. Useful for sequences undergone entropy reduction
   transforms such as Burrows-Wheele + MTF or the column prediction method.
*/
#if __STD_C
static ssize_t rle0(Rle_t* rle, int encoding)
#else
static ssize_t rle0(rle, encoding)
Rle_t*	rle;
int	encoding;
#endif
{
	ssize_t		c, z;
	Vcchar_t	*dt, *enddt, *o;
	Vcio_t		io;

	enddt = (dt = rle->ibuf) + rle->isiz;
	o = rle->obuf; rle->osiz = 0;

	if(encoding)
	{	for(z = -1; dt < enddt; ++dt)
		{	if((c = *dt) == 0)
				z += 1;
			else
			{	if(z >= 0) /* encode the 0-run */
				{	/**/PRINT(9,"%d\n",z);
					vcioinit(&io, o, 8*sizeof(ssize_t));
					vcioput2(&io, z, 0, RL_ZERO);
					o = vcionext(&io);
					z = -1;
				}
				if(RLLITERAL(c)) /* literal encoding */
					*o++ = RL_ESC;
				*o++ = c;
			}
		}
		if(z >= 0) /* final 0-run if any */
		{	/**/PRINT(9,"%d\n",z);
			vcioinit(&io, o, 8*sizeof(ssize_t));
			vcioput2(&io, z, 0, RL_ZERO);
			o = vcionext(&io);
		}
	}
	else
	{	for(; dt < enddt; )
		{	if((c = *dt++) == RL_ESC)
			{	if(dt >= enddt) /* premature EOF */
					return -1;
				c = *dt++;
				if(!RLLITERAL(c)) /* corrupted data */
					return -1;
				*o++ = c;
			}
			else if(c == 0 || c == RL_ZERO)
			{	vcioinit(&io, dt-1, (enddt-dt)+1);
				z = vcioget2(&io, 0, RL_ZERO); /**/PRINT(9,"%d\n",z);
				dt = vcionext(&io);
				for(; z >= 0; --z)
					*o++ = 0;
			}
			else	*o++ = c;
		}
	}

	return (rle->osiz = o - rle->obuf);
}

/* Encoding runs with escape codes in two streams */
#if __STD_C
static ssize_t rle2(Rle_t* rle, int encoding)
#else
static ssize_t rle2(rle, encoding)
Rle_t*	rle;
int	encoding;
#endif
{
	Vcchar_t	c, *chr, *run, *dt, *endb, *nextb;
	ssize_t		r;
	Vcio_t		io;

	if(encoding)
	{	endb = (dt = rle->ibuf) + rle->isiz;

		/* set buffers for runs and data */
		chr = rle->obuf;
		run = rle->abuf;
		for(; dt < endb; dt = nextb)
		{	for(c = *dt, nextb = dt+1; nextb < endb; ++nextb)
				if(*nextb != c)
					break;
			if((r = nextb-dt) >= 3 || c == RL_ESC)
			{	/* in-line small cases here for speed */
				if(r < (1<<7))
					*run++ = r;
				else if(r < (1<<14))
				{	*run++ = (r>>7)|128;
					*run++ = r&127;
				}
				else
				{	vcioinit(&io, run, 2*sizeof(ssize_t));
					vcioputu(&io, r);
					run = vcionext(&io);
				}

				*chr++ = RL_ESC;
				if(c != RL_ESC || r > 1)
					*chr++ = c;
			}
			else
			{	*chr++ = c;
				if(r == 2)
					*chr++ = c;
			}
		}

		return (rle->osiz = chr - rle->obuf) + (rle->asiz = run - rle->abuf);
	}
	else
	{	dt = rle->obuf;
		vcioinit(&io, rle->abuf, rle->asiz);
		for(endb = (chr = rle->ibuf) + rle->isiz; chr < endb; )
		{	if((c = *chr++) != RL_ESC)
				*dt++ = c;
			else
			{	r = vciogetu(&io);
				if(r == 1)
					*dt++ = RL_ESC;
				else for(c = *chr++; r > 0; --r)
					*dt++ = c;
			}
		}

		return (rle->osiz = dt - rle->obuf);
	}
}


#if __STD_C
static ssize_t vcrle(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t vcrle(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	Vcchar_t	*output;
	ssize_t		hd, sz, outsz;
	Vcio_t		io;
	Rlectxt_t	*ctxt = VCGETCTXT(vc, Rlectxt_t*);
	Rle_t		*rle = VCGETMETH(vc, Rle_t*);

	if(size == 0)
		return 0;

	hd = VCSIZEU(size);
	outsz = 2*(size+VCSIZEU(size)) + 128; /* a little slack */
	if(!(output = vcsetbuf(vc, NIL(Vcchar_t*), outsz, hd)) )
		return 1;

	rle->ibuf = (Vcchar_t*)data;
	rle->isiz = (ssize_t)size;

	if(rle->rlef == rle2)
	{	rle->obuf = output + VCSIZEU(size);
		rle->abuf = rle->obuf + size + VCSIZEU(size);

		if((sz = (*rle->rlef)(rle, 1)) < 0 )
			return -1;

		if(vc->coder) /* run continuator on the two parts */
		{	sz = _vcrle2coder(vc, hd,
					  rle->obuf, rle->osiz,
					  rle->abuf, rle->asiz,
				          &output, outsz);
			if(sz < 0)
				return -1;
		}
		else
		{	output = rle->obuf - VCSIZEU(rle->osiz);
			vcioinit(&io, output, outsz);
			vcioputu(&io, rle->osiz); /* data */
			vcioskip(&io, rle->osiz);
			vcioputu(&io, rle->asiz); /* run lengths */
			vcioputs(&io, rle->abuf, rle->asiz);
			sz = vciosize(&io);
		}
	}
	else
	{	rle->obuf = output;

		if((sz = (*rle->rlef)(rle, 1)) < 0 )
			return -1;

		if(vc->coder)
		{	if(VCSETCTXT(vc->coder, ctxt->dctxt) < 0)
				return -1;
			if(VCCODER(vc, vc->coder, hd, output, sz) < 0 )
				return -1;
		}
	}

	output -= hd;
	vcioinit(&io, output, hd);
	vcioputu(&io, size);

	if(out)
		*out = output;

	return hd+sz;
}

#if __STD_C
static ssize_t vcunrle(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t vcunrle(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	Vcchar_t	*output;
	ssize_t		sz;
	Vcio_t		io;
	Rlectxt_t	*ctxt = VCGETCTXT(vc, Rlectxt_t*);
	Rle_t		*rle = VCGETMETH(vc, Rle_t*);

	if(size == 0)
		return 0;

	vcioinit(&io, data, size);
	if((sz = vciogetu(&io)) <= 0 )
		return -1;

	if(!(output = vcsetbuf(vc, NIL(Vcchar_t*), sz, 0)) )
		return -1;
	rle->obuf = output;

	if(rle->rlef == rle2)
	{	rle->isiz = vciogetu(&io); /* data */
		rle->ibuf = vcionext(&io);
		vcioskip(&io, rle->isiz);

		rle->asiz = vciogetu(&io); /* run lengths */
		rle->abuf = vcionext(&io);

		if(vc->coder)
		{	/* decode the data segment using secondary coder */
			if(VCSETCTXT(vc->coder, ctxt->dctxt) < 0 )
				return -1;
			if(VCCODER(vc, vc->coder, 0, rle->ibuf, rle->isiz) < 0)
				return -1;

			/* decode the run-length segment using secondary coder */
			if(VCSETCTXT(vc->coder, ctxt->rctxt) < 0 )
				return -1;
			if(VCCODER(vc, vc->coder, 0, rle->abuf, rle->asiz) < 0)
				return -1;
		}
	}
	else
	{	rle->isiz = vciomore(&io);
		rle->ibuf = vcionext(&io);
		if(vc->coder)
		{	if(VCSETCTXT(vc->coder, ctxt->dctxt) < 0)
				return -1;
			if(VCCODER(vc, vc->coder, 0, rle->ibuf, rle->isiz) < 0)
				return -1;
		}
	}

	if((*rle->rlef)(rle, 0) != sz)
		return -1;

	if(out)
		*out = output;

	return sz;
}


/* arguments to select type of run length coder */
static Vcmtarg_t _Rleargs[] =
{	{ "0", "\060", "Run-length-encoding only 0-sequences", (Void_t*)rle0 },
	{  0 , 0, "General run-length-encoding", (Void_t*)rle2 }
};

#if __STD_C
static ssize_t rleextract(Vcodex_t* vc, Void_t** datap, int state)
#else
static ssize_t rleextract(vc, datap, state)
Vcodex_t*	vc;
Void_t**	datap;	/* basis string for persistence	*/
int		state;	/* should always be 0 for this	*/
#endif
{
	Rle_t		*rle = VCGETMETH(vc, Rle_t*);
	int		k;

	if(state) /* we only deal with header data */
		return 0;

	for(k = 0; _Rleargs[k].name; ++k)
		if(_Rleargs[k].data == (Void_t*)rle->rlef)
			break;
	if(datap) /* return persistent data */
		*datap = (Void_t*)_Rleargs[k].ident;
	return _Rleargs[k].ident ? strlen((char*)_Rleargs[k].ident) : 0;
}

#if __STD_C
static Vcodex_t* rlerestore(Vcodex_t* vc, Void_t* data, ssize_t dtsz)
#else
static Vcodex_t* rlerestore(vc, data, dtsz)
Vcodex_t*	vc;	/* handle to restore	*/
Void_t*		data;	/* persistence data	*/
ssize_t		dtsz;
#endif
{
	int	k;

	if(vc)
		return vc;

	for(k = 0; _Rleargs[k].name; ++k)
		if(dtsz == strlen((char*)_Rleargs[k].ident) &&
		   strncmp((char*)_Rleargs[k].ident, (char*)data, dtsz) == 0)
			break;
	return vcopen(0, Vcrle, _Rleargs[k].name, 0, VC_DECODE);
}

#if __STD_C
static int rleevent(Vcodex_t* vc, int type, Void_t* params)
#else
static int rleevent(vc, type, params)
Vcodex_t*	vc;
int		type;
Void_t*		params;
#endif
{
	Rle_t		*rle;
	Rlectxt_t	*ctxt;
	int		k;

	if(type == VC_OPENING )
	{	if(!(rle = (Rle_t*)calloc(1,sizeof(Rle_t))) )
			return -1;

		for(k = 0; _Rleargs[k].name; ++k)
			if(params && strcmp((char*)params, _Rleargs[k].name) == 0)
				break;
		rle->rlef = (Rle_f)_Rleargs[k].data;

		VCINITCTXT(vc, Rlectxt_t);
		VCSETMETH(vc, rle);
	}
	else if(type == VC_INITCTXT) /* initialize a new context */
	{	if(!(ctxt = (Rlectxt_t*)params) )
			return -1;
		else
		{	ctxt->dctxt = ctxt->rctxt = -1;
			return 1;
		}
	}
	else if(type == VC_CLOSING)
	{	if((rle = VCGETMETH(vc, Rle_t*)) )
			free(rle);
	}
	else if(type == VC_GETARG)
	{	if(!(rle = VCGETMETH(vc, Rle_t*)))
			return -1;
		for(k = 0; _Rleargs[k].name; ++k)
		{	if((Rle_f)_Rleargs[k].data != rle->rlef)
				continue;
			if(params)
				*((char**)params) = _Rleargs[k].name;
			return 1;
		}
	}

	return 0;
}

Vcmethod_t _Vcrle =
{	vcrle,
	vcunrle,
	rleextract,
	rlerestore,
	rleevent,
	"rle", "\162\154\145", "Run-length encoding",
	_Rleargs,
	1024*1024,
	VCNEXT(Vcrle)
};

VCLIB(Vcrle)
