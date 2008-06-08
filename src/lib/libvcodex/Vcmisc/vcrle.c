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
#include	<vclib.h>

/*	Various run-length-encoding methods.
**
**	Written by Kiem-Phong Vo (kpv@research.att.com)
*/

typedef struct _rle_s	Rle_t;
typedef ssize_t		(*Rle_f)_ARG_((Rle_t*, int));
static ssize_t		rle0 _ARG_((Rle_t*, int));
static ssize_t		rle1 _ARG_((Rle_t*, int));
static ssize_t		rle2 _ARG_((Rle_t*, int));
static ssize_t		rleg _ARG_((Rle_t*, int));

struct _rle_s
{
	Rle_f		rlef;	/* rle function to call	*/
	Vcchar_t*	ibuf;	/* default input data	*/
	ssize_t		isiz;

	Vcchar_t*	obuf;	/* default output data	*/
	ssize_t		osiz;
	Vcchar_t*	endo;	/* upper bound of array	*/

	Vcchar_t*	abuf;	/* auxiliary buffer	*/
	ssize_t		asiz;

	Vcchar_t	onec;	/* 1st letter for rle2	*/
	Vcchar_t	twoc;	/* 2nd letter for rle2	*/
};

/* arguments to select type of run length coder */
static Vcmtarg_t _Rleargs[] =
{	{ "0", "Run-length-encoding: 0-sequences only.", (Void_t*)rle0 },
	{ "1", "Run-length-encoding: 0&1-sequences only.", (Void_t*)rle1 },
	{ "2", "Run-length-encoding: only 2 letters in data.", (Void_t*)rle2 },
	{  0 , "General run-length-encoding.", (Void_t*)rleg }
};

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
	Vcchar_t	*dt, *enddt, *o, *endo;
	Vcio_t		io;

	enddt = (dt = rle->ibuf) + rle->isiz;
	o = rle->obuf; rle->osiz = 0;

	if(encoding)
	{	for(z = -1; dt < enddt; ++dt)
		{	if((c = *dt) == 0)
				z += 1;
			else
			{	if(z >= 0) /* encode the 0-run */
				{	/**/DEBUG_PRINT(9,"%d\n",z);
					vcioinit(&io, o, 8*sizeof(ssize_t));
					vcioput2(&io, z, 0, RL_ZERO);
					o = vcionext(&io);
					z = -1;
				}
				if(c >= RL_ZERO) /* literal encoding */
					*o++ = RL_ESC;
				*o++ = c;
			}
		}
		if(z >= 0) /* final 0-run if any */
		{	/**/DEBUG_PRINT(9,"%d\n",z);
			vcioinit(&io, o, 8*sizeof(ssize_t));
			vcioput2(&io, z, 0, RL_ZERO);
			o = vcionext(&io);
		}
	}
	else
	{	for(endo = rle->endo; dt < enddt; )
		{	if((c = *dt++) == RL_ESC)
			{	if(dt >= enddt) /* premature EOF */
					return -1;
				c = *dt++;
				if(o >= endo || c < RL_ZERO) /* corrupted data */
					return -1;
				*o++ = c;
			}
			else if(c == 0 || c == RL_ZERO)
			{	vcioinit(&io, dt-1, (enddt-dt)+1);
				z = vcioget2(&io, 0, RL_ZERO); /**/DEBUG_PRINT(9,"%d\n",z);
				dt = vcionext(&io);
				if(o+z > endo)
					return -1;
				for(; z >= 0; --z)
					*o++ = 0;
			}
			else
			{	if(o >= endo)
					return -1;
				*o++ = c;
			}
		}
	}

	return (rle->osiz = o - rle->obuf);
}

/* Encoding 0&1-run's only. Useful for sequences undergone entropy reduction
   transforms such as Burrows-Wheele + MTF or the column prediction method.
*/
#if __STD_C
static ssize_t rle1(Rle_t* rle, int encoding)
#else
static ssize_t rle1(rle, encoding)
Rle_t*	rle;
int	encoding;
#endif
{
	ssize_t		c, rc, rz;
	Vcchar_t	*dt, *enddt, *o, *endo;
	Vcio_t		io;

	enddt = (dt = rle->ibuf) + rle->isiz;
	o = rle->obuf; rle->osiz = 0;

	if(encoding)
	{	for(rc = rz = -1; dt < enddt; ++dt)
		{	if((c = *dt) == 0 || c == 1)
			{	if(c == rc)
					rz += 1;
				else
				{	if(rz >= 0)
					{	vcioinit(&io, o, 8*sizeof(ssize_t));
						if(rc == 0)
							vcioput2(&io, rz, 0, RL_ZERO);
						else	vcioput2(&io, rz, 1, RL_ONE);
						o = vcionext(&io);
					}

					rc = c; rz = 0;
				}
			}
			else
			{	if(rz >= 0) /* encode the 0/1-run */
				{	vcioinit(&io, o, 8*sizeof(ssize_t));
					if(rc == 0)
						vcioput2(&io, rz, 0, RL_ZERO);
					else	vcioput2(&io, rz, 1, RL_ONE);
					o = vcionext(&io);

					rc = rz = -1;
				}

				if(c >= RL_ONE) /* literal encoding */
					*o++ = RL_ESC;
				*o++ = c;
			}
		}
		if(rz >= 0) /* final 0/1-run if any */
		{	vcioinit(&io, o, 8*sizeof(ssize_t));
			if(rc == 0)
				vcioput2(&io, rz, 0, RL_ZERO);
			else	vcioput2(&io, rz, 1, RL_ONE);
			o = vcionext(&io);
		}
	}
	else
	{	for(endo = rle->endo; dt < enddt; )
		{	if((c = *dt++) == RL_ESC)
			{	if(dt >= enddt) /* premature EOF */
					return -1;
				c = *dt++;
				if(o >= endo || c < RL_ONE) /* corrupted data */
					return -1;
				*o++ = c;
			}
			else if(c == 0 || c == RL_ZERO || c == 1 || c == RL_ONE)
			{	vcioinit(&io, dt-1, (enddt-dt)+1);
				if(c == 0 || c == RL_ZERO)
					rz = vcioget2(&io, 0, RL_ZERO);
				else	rz = vcioget2(&io, 1, RL_ONE);
				dt = vcionext(&io);
				if(o+rz > endo)
					return -1;
				if(c == 0 || c == RL_ZERO)
				{	for(; rz >= 0; --rz)
						*o++ = 0;
				}
				else
				{	for(; rz >= 0; --rz)
						*o++ = 1;
				}
			}
			else
			{	if(o >= endo)
					return -1;
				*o++ = c;
			}
		}
	}

	return (rle->osiz = o - rle->obuf);
}

/* rl-encoding data consisting of runs of two or less letters */
#if __STD_C
static ssize_t rle2(Rle_t* rle, int encoding)
#else
static ssize_t rle2(rle, encoding)
Rle_t*	rle;
int	encoding;
#endif
{
	Vcchar_t	*dt, *enddt;
	ssize_t		sz, n, k, c, c1, c2;
	Vcio_t		io1, io2;

	if(encoding)
	{	dt = rle->ibuf; sz = rle->isiz;
		vcioinit(&io1, rle->obuf, rle->osiz);
		vcioinit(&io2, rle->abuf, rle->asiz);

		/* compute the letters of the two runs */
		for(c1 = dt[0], k = 1; k < sz; ++k)
			if(dt[k] != c1)
				break;
		c2 = k < sz ? dt[k] : c1;

		for(c = c1, n = k-1; k < sz; ++k)
		{	if(dt[k] == c)
				n += 1;
			else /* end of current run, output */
			{	if(c == c1)
				{	if((c = dt[k]) != c2)
						return -1;
					vcioputu(&io1, n);
				}
				else
				{	if((c = dt[k]) != c1)
						return -1;
					vcioputu(&io2, n);
				}
				n = 0;
			}
		}
		if(c == c1)
			vcioputu(&io1, n);
		else	vcioputu(&io2, n);

		rle->onec = c1;
		rle->twoc = c2;
		return (rle->osiz = vciosize(&io1)) + (rle->asiz = vciosize(&io2));
	}
	else
	{	dt = rle->obuf; enddt = rle->endo;
		c1 = rle->onec; vcioinit(&io1, rle->ibuf, rle->isiz);
		c2 = rle->twoc; vcioinit(&io2, rle->abuf, rle->asiz);
		for(;;)
		{	if(vciomore(&io1) <= 0)
				break;
			for(n = vciogetu(&io1); n >= 0 && dt < enddt; --n)
				*dt++ = c1;
			if(vciomore(&io2) <= 0)
				break;
			for(n = vciogetu(&io2); n >= 0 && dt < enddt; --n)
				*dt++ = c2;
		}
		if(vciomore(&io1) > 0 || vciomore(&io2) > 0 || n >= 0)
			return -1;

		return (rle->osiz = dt - rle->obuf);
	}
}

/* General rl-encoding using escape codes */
#if __STD_C
static ssize_t rleg(Rle_t* rle, int encoding)
#else
static ssize_t rleg(rle, encoding)
Rle_t*	rle;
int	encoding;
#endif
{
	Vcchar_t	c, *chr, *run, *dt, *enddt, *endb, *nextb;
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
	{	dt = rle->obuf; enddt = rle->endo;
		vcioinit(&io, rle->abuf, rle->asiz);
		for(endb = (chr = rle->ibuf) + rle->isiz; chr < endb; )
		{	if((c = *chr++) != RL_ESC)
			{	if(dt >= enddt)
					return -1;
				*dt++ = c;
			}
			else
			{	r = vciogetu(&io);
				if(dt+r > enddt)
					return -1;
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
	Vcchar_t	*output, *space;
	ssize_t		hd, sz, outsz;
	Vcio_t		io;
	Rle_t		*rle = vcgetmtdata(vc, Rle_t*);

	if(size == 0)
		return 0;

	hd = vcsizeu(size);
	outsz = 2*vcsizeu(size) + 2*size + 128;
	if(!(output = space = vcbuffer(vc, NIL(Vcchar_t*), outsz, hd)) )
		return -1;

	rle->ibuf = (Vcchar_t*)data; /* input data to encode */
	rle->isiz = (ssize_t)size;

	if(rle->rlef == rle2 || rle->rlef == rleg)
	{	rle->obuf = output + vcsizeu(size) + (rle->rlef == rle2 ? 2 : 0);
		rle->endo = rle->obuf + (rle->osiz = size);
		rle->abuf = rle->endo + vcsizeu(size);
		rle->asiz = size;

		if((sz = (*rle->rlef)(rle, 1)) < 0 )
			return -1;

		if(vc->coder) /* run continuator on the two parts */
		{	if(vcrecode(vc, &rle->obuf, &rle->osiz, 0) < 0)
				return -1;
			if(vcrecode(vc, &rle->abuf, &rle->asiz, 0) < 0)
				return -1;
		}

		sz = vcsizeu(rle->osiz) + rle->osiz + vcsizeu(rle->asiz) + rle->asiz;
		if((sz += rle->rlef == rle2 ? 2 : 0) > outsz)
			output = vcbuffer(vc, NIL(Vcchar_t*), sz, hd);

		vcioinit(&io, output, sz);

		if(rle->rlef == rle2) /* put out the run bytes */
		{	vcioputc(&io, rle->onec);
			vcioputc(&io, rle->twoc);
		}

		vcioputu(&io, rle->osiz);
		if(rle->obuf != vcionext(&io))
			vcioputs(&io, rle->obuf, rle->osiz);
		else	vcioskip(&io, rle->osiz);

		vcioputu(&io, rle->asiz); 
		if(rle->abuf != vcionext(&io))
			vcioputs(&io, rle->abuf, rle->asiz);
		else	vcioskip(&io, rle->asiz);

		sz = vciosize(&io);
	}
	else
	{	rle->obuf = output;
		rle->endo = rle->obuf + outsz;

		if((sz = (*rle->rlef)(rle, 1)) < 0 )
			return -1;

		if(vcrecode(vc, &output, &sz, hd) < 0 )
			return -1;
	}

	if(space != output) /* free space if unused */
		vcbuffer(vc, space, -1, -1);

	output -= hd; sz += hd;
	vcioinit(&io, output, hd);
	vcioputu(&io, size);

	if(!(output = vcbuffer(vc, output, sz, -1)) ) /* truncate buffer to size */
		return -1;
	if(out)
		*out = output;
	return sz;
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
	Rle_t		*rle = vcgetmtdata(vc, Rle_t*);

	if(size == 0)
		return 0;

	vcioinit(&io, data, size);
	if((sz = vciogetu(&io)) <= 0 )
		return -1;

	if(!(output = vcbuffer(vc, (Vcchar_t*)0, sz, 0)) )
		return -1;
	rle->obuf = output;
	rle->endo = rle->obuf + sz;

	if(rle->rlef == rle2 || rle->rlef == rleg)
	{	if(rle->rlef == rle2)
		{	if(vciomore(&io) < 2)
				return -1;
			rle->onec = vciogetc(&io);
			rle->twoc = vciogetc(&io);
		}

		if(vciomore(&io) <= 0 || (rle->isiz = vciogetu(&io)) < 0)
			return -1;
		if(vciomore(&io) < rle->isiz)
			return -1;
		rle->ibuf = vcionext(&io);
		vcioskip(&io, rle->isiz);

		if(vciomore(&io) <= 0 || (rle->asiz = vciogetu(&io)) < 0)
			return -1;
		if(vciomore(&io) < rle->asiz)
			return -1;
		rle->abuf = vcionext(&io);

		/* decode data by secondary encoders */
		if(vcrecode(vc, &rle->ibuf, &rle->isiz, 0) < 0)
			return -1;
		if(vcrecode(vc, &rle->abuf, &rle->asiz, 0) < 0)
			return -1;
	}
	else
	{	if(vciomore(&io) <= 0 || (rle->isiz = vciomore(&io)) < 0 )
			return -1;
		if(vciomore(&io) < rle->isiz)
			return -1;
		rle->ibuf = vcionext(&io);
		if(vcrecode(vc, &rle->ibuf, &rle->isiz, 0) < 0)
			return -1;
	}

	if((*rle->rlef)(rle, 0) != sz) /* decode run data */
		return -1;

	if(rle->ibuf && (rle->ibuf < (Vcchar_t*)data || rle->ibuf >= (Vcchar_t*)data+size) )
		vcbuffer(vc, rle->ibuf, -1, -1);

	if(out)
		*out = output;
	return sz;
}

#if __STD_C
static ssize_t rleextract(Vcodex_t* vc, Vcchar_t** datap)
#else
static ssize_t rleextract(vc, datap)
Vcodex_t*	vc;
Vcchar_t**	datap;	/* basis string for persistence	*/
#endif
{
	Vcmtarg_t	*arg;
	char		*ident;
	ssize_t		n;
	Rle_t		*rle = vcgetmtdata(vc, Rle_t*);

	for(arg = _Rleargs;; ++arg)
		if(!arg->name || arg->data == (Void_t*)rle->rlef)
			break;
	if(!arg->name)
		return 0;

	n = strlen(arg->name);
	if(!(ident = (char*)vcbuffer(vc, NIL(Vcchar_t*), sizeof(int)*n+1, 0)) )
		return -1;
	if(!(ident = vcstrcode(arg->name, ident, sizeof(int)*n+1)) )
		return -1;
	if(datap)
		*datap = (Void_t*)ident;
	return n;
}

#if __STD_C
static Vcodex_t* rlerestore(Vcchar_t* data, ssize_t dtsz)
#else
static Vcodex_t* rlerestore(data, dtsz)
Vcchar_t*	data;	/* persistence data	*/
ssize_t		dtsz;
#endif
{
	Vcmtarg_t	*arg;
	char		*ident, buf[1024];

	for(arg = _Rleargs; arg->name; ++arg)
	{	if(!(ident = vcstrcode(arg->name, buf, sizeof(buf))) )
			return NIL(Vcodex_t*);
		if(dtsz == strlen(ident) && strncmp(ident, (Void_t*)data, dtsz) == 0)
			break;
	}
	return vcopen(0, Vcrle, (Void_t*)arg->name, 0, VC_DECODE);
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
	Vcmtarg_t	*arg;
	Vcmtcode_t	*mtcd;
	char		*data;

	if(type == VC_OPENING )
	{	for(arg = NIL(Vcmtarg_t*), data = (char*)params; data && *data; )
		{	data = vcgetmtarg(data, 0, 0, _Rleargs, &arg);
			if(arg && arg->name)
				break;
		}
		if(!arg)
			for(arg = _Rleargs;; ++arg)
				if(!arg->name)
					break;

		if(!(rle = (Rle_t*)calloc(1,sizeof(Rle_t))) )
			return -1;
		rle->rlef = (Rle_f)arg->data;

		vcsetmtdata(vc, rle);
		return 0;
	}
	else if(type == VC_CLOSING)
	{	if((rle = vcgetmtdata(vc, Rle_t*)) )
			free(rle);
		return 0;
	}
	else if(type == VC_EXTRACT)
	{	if(!(mtcd = (Vcmtcode_t*)params) )
			return -1;
		if((mtcd->size = rleextract(vc, &mtcd->data)) < 0 )
			return -1;
		return 1;
	}
	else if(type == VC_RESTORE)
	{	if(!(mtcd = (Vcmtcode_t*)params) )
			return -1;
		if(!(mtcd->coder = rlerestore(mtcd->data, mtcd->size)) )
			return -1;
		return 1;
	}

	return 0;
}

Vcmethod_t _Vcrle =
{	vcrle,
	vcunrle,
	rleevent,
	"rle", "Run-length encoding.",
	"[-version?rle (AT&T Research) 2003-01-01]" USAGE_LICENSE,
	_Rleargs,
	1024*1024,
	0
};

VCLIB(Vcrle)
