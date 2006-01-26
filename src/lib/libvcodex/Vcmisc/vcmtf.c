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

/*	Move-to-front transformers.
**
**	Written by Kiem-Phong Vo (kpv@research.att.com)
*/

typedef ssize_t	(*Mtf_f)_ARG_((Vcchar_t*, Vcchar_t*, Vcchar_t*, int));

#define MTFC(c,p,m,n) /* know byte, need to compute position */ \
	{ m = mtf[p = 0]; \
	  while(m != c) \
	  	{ n = mtf[p += 1]; mtf[p] = m; m = n; } \
	  mtf[0] = c; \
	}

#define MTFP(c,p,m,n) /* know position, need to compute byte */ \
	{ c = mtf[m = 0]; \
	  while(m != p) \
	  	{ n = mtf[m += 1]; mtf[m] = c; c = n; } \
	  mtf[0] = c; \
	}

#if __STD_C
static ssize_t mtfp(Vcchar_t* dt, Vcchar_t* enddt, Vcchar_t* output, int encoding)
#else
static ssize_t mtfp(dt, enddt, output, encoding)
Vcchar_t*	dt;		/* data to be un/mtf-ed	*/
Vcchar_t*	enddt;
Vcchar_t*	output;		/* output array		*/
int		encoding;	/* !=0 if encoding	*/
#endif
{
	reg int		c, p, m, n, predc;
	reg Vcchar_t	*o;
	Vcchar_t	mtf[256], succ[256];
	size_t		wght[256];

	predc = 0;
	for(p = 0; p < 256; ++p)
		mtf[p] = succ[p] = p;
	memset(wght,0,sizeof(wght));

/* weight increases slightly but decreases quickly */
#define SUCC(c,p,m,n) \
	{ if(succ[predc] == c)  wght[predc] += 4; \
	  else if((wght[predc] >>= 1) <= 0) \
		{ succ[predc] = c; wght[predc] = 1; } \
	  predc = c; \
	  if(wght[c] > 1 && (c = succ[c]) != mtf[0] ) \
		MTFC(c,p,m,n); /* predicting succ[c] is next */ \
	}

	if(encoding)
	{	for(o = output; dt < enddt; ++dt, ++o )
		{	c = *dt; MTFC(c,p,m,n); *o = p;
			SUCC(c,p,m,n);
		}
	}
	else
	{	for(o = output; dt < enddt; ++dt, ++o )
		{	p = *dt; MTFP(c,p,m,n); *o = c;
			SUCC(c,p,m,n);
		}
	}

	return o-output;
}


/* move to zeroth location */
#if __STD_C
static ssize_t mtf0(Vcchar_t* dt, Vcchar_t* enddt, Vcchar_t* output, int encoding)
#else
static ssize_t mtf0(dt, enddt, output, encoding)
Vcchar_t*	dt;	/* data to be un/mtf-ed	*/
Vcchar_t*	enddt;
Vcchar_t*	output;	/* output array		*/
int		encoding; /* !=0 if encoding	*/
#endif
{
	reg int		c, p, m, n;
	reg Vcchar_t	*o;
	Vcchar_t	mtf[256];

	for(p = 0; p < 256; ++p)
		mtf[p] = p;
	if(encoding)
	{	for(o = output; dt < enddt; ++dt, ++o )
		{	c = *dt; MTFC(c,p,m,n); *o = p;
		}
	}
	else
	{	for(o = output; dt < enddt; ++dt, ++o )
		{	p = *dt; MTFP(c,p,m,n); *o = c;
		}
	}

	return o-output;
}

#if __STD_C
static ssize_t vcmtf(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t vcmtf(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	Vcchar_t	*output;
	ssize_t		sz;
	Vcctxt_t	*ctxt = VCGETCTXT(vc, Vcctxt_t*);
	Mtf_f		mtff = VCGETMETH(vc, Mtf_f);

	if((sz = size) == 0)
		return 0;

	if(!(output = vcsetbuf(vc, NIL(Vcchar_t*), sz, 0)) )
		return -1;

	if((*mtff)((Vcchar_t*)data, ((Vcchar_t*)data)+sz, output, 1) != sz)
		return -1;

	if(vc->coder)
	{	if(VCSETCTXT(vc->coder, ctxt->ctxt) < 0)
			return -1;
		if(VCCODER(vc, vc->coder, 0, output, sz) < 0 )
			return -1;
	}

	if(out)
		*out = output;

	return sz;
}

#if __STD_C
static ssize_t vcunmtf(Vcodex_t* vc, const Void_t* data, size_t size, Void_t** out)
#else
static ssize_t vcunmtf(vc, data, size, out)
Vcodex_t*	vc;
Void_t*		data;
size_t		size;
Void_t**	out;
#endif
{
	Vcchar_t	*dt, *output;
	ssize_t		sz;
	Vcctxt_t	*ctxt = VCGETCTXT(vc, Vcctxt_t*);
	Mtf_f		mtff = VCGETMETH(vc, Mtf_f);

	if(size == 0)
		return 0;

	dt = (Vcchar_t*)data;
	sz = size;
	if(vc->coder)
	{	if(VCSETCTXT(vc->coder, ctxt->ctxt) < 0)
			return -1;
		if((sz = vcapply(vc->coder, dt, sz, &dt)) < 0)
			return -1;
	}

	if(!(output = vcsetbuf(vc, NIL(Vcchar_t*), sz, 0)) )
		return -1;

	if((*mtff)(dt, dt+sz, output, 0) != sz)
		return -1;

	if(out)
		*out = output;

	return sz;
}

/* arguments to select move-to-front coder */
static Vcmtarg_t _Mtfargs[] =
{	{ "0", "\060", "Pure move-to-front strategy", (Void_t*)mtf0 },
	{  0 , 0, "Move-to-front with prediction", (Void_t*)mtfp }
};

#if __STD_C
static ssize_t mtfextract(Vcodex_t* vc, Void_t** datap, int state)
#else
static ssize_t mtfextract(vc, datap, state)
Vcodex_t*	vc;
Void_t**	datap;	/* basis string for persistence	*/
int		state;	/* should always be 0 for this	*/
#endif
{
	int	k;
	Mtf_f	mtff = VCGETMETH(vc, Mtf_f);

	if(state) /* we only deal with header data here */
		return 0;

	for(k = 0; _Mtfargs[k].name; ++k)
		if(_Mtfargs[k].data == (Void_t*)mtff)
			break;
	if(datap)
		*datap = (Void_t*)_Mtfargs[k].ident;
	return _Mtfargs[k].ident ? strlen((char*)_Mtfargs[k].ident) : 0;
}

#if __STD_C
static Vcodex_t* mtfrestore(Vcodex_t* vc, Void_t* data, ssize_t dtsz)
#else
static Vcodex_t* mtfrestore(vc, data, dtsz)
Vcodex_t*	vc;	/* handle to restore	*/
Void_t*		data;	/* persistence data	*/
ssize_t		dtsz;
#endif
{
	int	k;

	if(vc)
		return vc;

	for(k = 0; _Mtfargs[k].name; ++k)
		if(dtsz == strlen(_Mtfargs[k].name) &&
		   strncmp((char*)_Mtfargs[k].ident, (char*)data, dtsz) == 0 )
			break;
	return vcopen(0, Vcmtf, _Mtfargs[k].name, 0, VC_DECODE);
}

#if __STD_C
static int mtfevent(Vcodex_t* vc, int type, Void_t* params)
#else
static int mtfevent(vc, type, params)
Vcodex_t*	vc;
int		type;
Void_t*		params;
#endif
{
	int	k;

	if(type == VC_OPENING )
	{	for(k = 0; _Mtfargs[k].name; ++k)
			if(params && strcmp((char*)params, _Mtfargs[k].name) == 0)
				break;
		VCSETMETH(vc, _Mtfargs[k].data);
	}
	else if(type == VC_GETARG)
	{	char	*tp;
		if((tp = VCGETMETH(vc, char*)) )
		{	for(k = 0; _Mtfargs[k].name; ++k)
			{	if(_Mtfargs[k].data == tp)
				{	if(params)
						*((char**)params) = _Mtfargs[k].name;
					return 1;
				}
			}
		}
	}

	return 0;
}

Vcmethod_t _Vcmtf =
{	vcmtf,
	vcunmtf,
	mtfextract,
	mtfrestore,
	mtfevent,
	"mtf", "\155\164\146", "Move-to-front transform",
	_Mtfargs,
	1024*1024,
	VCNEXT(Vcmtf)
};

VCLIB(Vcmtf)
