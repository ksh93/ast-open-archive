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
#include	"vcdhdr.h"

/*	Delta compression based on the Vcdiff data format.
**
**	Written by Kiem-Phong Vo (kpv@research.att.com)
*/

#define SLOP		64	/* max overhead space needed for output */

#define SFXSORT		00001	/* use suffix sorting for lz-parsing	*/
#define FREETABLE	00010	/* free the coding table on closing	*/

static Vcmtarg_t _Diffargs[] =
{	{ "s", "\060", "Suffix sorting for string matching", (Void_t*)SFXSORT },
	{  0 , 0, "Hashing for string matching", 0 }
};

#if __STD_C
static void vcdflsadd(Vcdiff_t* vcd, ssize_t dt, ssize_t az)
#else
static void vcdflsadd(vcd, dt, az)
Vcdiff_t*	vcd;
ssize_t		dt;
ssize_t		az;
#endif
{
	int		cd;
	Vcchar_t	*data;
	Vcdtable_t*	tbl = vcd->table;
	Vcdsize_t*	siz = vcd->size;
	Vcdindex_t*	idx = vcd->index;

	cd = idx->add[az <= siz->add ? az : 0];
	vcioputc(vcd->inst, cd);

	if(tbl->code[cd].inst1.size == 0)
		vcioputu(vcd->inst, az);

	data = dt < vcd->vcpa.nsrc ? vcd->vcpa.src + dt :
				     vcd->vcpa.tar + (dt - vcd->vcpa.nsrc);
	vcioputs(vcd->data, data, az);
}

#if __STD_C
static void vcdflsaddr(Vcdiff_t* vcd, ssize_t ad, ssize_t md)
#else
static void vcdflsaddr(vcd, ad, md)
Vcdiff_t*	vcd;
ssize_t		ad;
ssize_t		md;
#endif
{
	if(md <= VCD_HERE+vcd->cache->s_near)
		vcioputu(vcd->addr, ad);
	else	vcioputc(vcd->addr, ad); /* must be same[] cache */
}

#if __STD_C
static void vcdflscopy(Vcdiff_t* vcd, ssize_t ad, ssize_t cz, ssize_t md)
#else
static void vcdflscopy(vcd, ad, cz, md)
Vcdiff_t*	vcd;
ssize_t		ad;
ssize_t		cz;
ssize_t		md;
#endif
{
	int		cd;
	Vcdtable_t*	tbl = vcd->table;
	Vcdsize_t*	siz = vcd->size;
	Vcdindex_t*	idx = vcd->index;

	cd = idx->copy[md][cz <= siz->copy[md] ? cz : 0];
	vcioputc(vcd->inst, cd);
	if(tbl->code[cd].inst1.size == 0)
		vcioputu(vcd->inst, cz);

	vcdflsaddr(vcd, ad, md);
}

#if __STD_C
static int vcdputcopy(Vcdiff_t* vcd, ssize_t dt, ssize_t cz, ssize_t mt)
#else
static int vcdputcopy(vcd, dt, cz, mt)
Vcdiff_t*	vcd;
ssize_t		dt;	/* data position	*/
ssize_t		cz;	/* size of matched data	*/
ssize_t		mt;	/* matched position	*/
#endif
{
	ssize_t		ad, md;
	int		cd;
	Vcchar_t	*data;
	Vcdsave_t*	sav = vcd->save;
	Vcdsize_t*	siz = vcd->size;
	Vcdindex_t*	idx = vcd->index;

	if(cz > 0) /* compute values to be encoded */
		ad = vcdkasetaddr(vcd->cache, mt, dt, &md);
	else	ad = md = -1;

	if(sav->dtsz > 0)
	{	if(sav->mode >= 0) /* saved COPY, no merged COPY+COPY */
			vcdflscopy(vcd, sav->addr, sav->dtsz, sav->mode);
		else /* saved ADD */
		{	if(cz <= 0 || sav->dtsz > siz->add1[md] || cz > siz->copy2[md])
				vcdflsadd(vcd, sav->addr, sav->dtsz);
			else
			{	cd = idx->addcopy[sav->dtsz][md][cz];
				vcioputc(vcd->inst, cd);

				if(sav->addr < vcd->vcpa.nsrc)
					data = vcd->vcpa.src + sav->addr;
				else	data = vcd->vcpa.tar + (sav->addr - vcd->vcpa.nsrc);
				vcioputs(vcd->data, data, sav->dtsz);

				vcdflsaddr(vcd, ad, md); cz = 0;
			}
		}
	}

	sav->dtsz = cz; /* save current COPY instruction */
	sav->addr = ad;
	sav->mode = md;

	return 1;
}

#if __STD_C
static int vcdputadd(Vcdiff_t* vcd, ssize_t dt, ssize_t az)
#else
static int vcdputadd(vcd, dt, az)
Vcdiff_t*	vcd;
ssize_t		dt;
ssize_t		az;
#endif
{
	int		cd;
	Vcchar_t	*data;
	Vcdsave_t*	sav = vcd->save;
	Vcdsize_t*	siz = vcd->size;
	Vcdindex_t*	idx = vcd->index;

	if(sav->dtsz > 0) /* there is a saved instruction */
	{	if(sav->mode < 0) /* parsing error? no two ADDs in a row possible */
			return -1;

		if(sav->dtsz <= siz->copy1[sav->mode] && az <= siz->add2[sav->mode])
		{	cd = idx->copyadd[sav->mode][sav->dtsz][az];
			vcioputc(vcd->inst, cd);

			vcdflsaddr(vcd, sav->addr, sav->mode);

			if(dt < vcd->vcpa.nsrc)
				data = vcd->vcpa.src + dt;
			else	data = vcd->vcpa.tar + (dt - vcd->vcpa.nsrc);
			vcioputs(vcd->data, data, az); az = 0;
		}
		else	vcdflscopy(vcd, sav->addr, sav->dtsz, sav->mode);
	}

	sav->dtsz = az; /* save current ADD instruction */
	sav->addr = dt;
	sav->mode = -1;

	return 1;
}

#if __STD_C
static int vcdputinst(Vcparse_t* vcpa, int type, ssize_t ad, ssize_t dt, ssize_t sz, ssize_t mt)
#else
static int vcdputinst(vcpa, type, ad, dt, sz, mt)
Vcparse_t*	vcpa;
int		type;	/* type of instruction		*/
ssize_t		ad;	/* start of unmatchable data	*/
ssize_t		dt;	/* current position of data	*/
ssize_t		sz;	/* matchable length		*/
ssize_t		mt;	/* matched addr if any		*/
#endif
{
	Vcdiff_t	*vcd = (Vcdiff_t*)vcpa;

	if(ad < dt && vcdputadd(vcd, ad, (dt-ad)) < 0 )
		return -1;

	if(sz > 0 && vcdputcopy(vcd, dt, sz, mt) < 0 )
		return -1;

	return 1; /* instruction was processed */
}

#if __STD_C
static ssize_t vcddiff(Vcodex_t* vc, const Void_t* tar, size_t ntar, Void_t** del)
#else
static ssize_t vcddiff(vc, tar, ntar, del)
Vcodex_t*	vc;
Void_t* 	tar;
size_t		ntar;
Void_t**	del;
#endif
{
	ssize_t		i, a, d, n, nsrc;
	Vcchar_t	*rd, *ri, *ra, *p, ctrl, *output;
	Vcio_t		inst, addr, data;
	Vcdsave_t	save;
	Vcdiff_t	*vcd = VCGETMETH(vc, Vcdiff_t*);
	Vcdctxt_t	*ctxt = VCGETCTXT(vc, Vcdctxt_t*);
	Vcdisc_t	*disc = VCGETDISC(vc);

	if(ntar == 0)
		return 0;

	nsrc = disc ? disc->size : 0;
	vcd->vcpa.src  = nsrc == 0 ? NIL(Vcchar_t*) : (Vcchar_t*)disc->data;
	vcd->vcpa.nsrc = nsrc;
	vcd->vcpa.tar  = (Vcchar_t*)tar;
	vcd->vcpa.ntar = ntar;
	vcd->vcpa.type = 0;
	vcd->vcpa.mmin = COPYMIN;
	vcd->vcpa.miss = 0;

	/* obtain output buffer */
	if(!(output = vcsetbuf(vc, NIL(Vcchar_t*), 3*(ntar+SLOP), 0)) )
		return -1;
	vcd->data = &data; vcioinit(&data, output+SLOP, ntar);
	vcd->inst = &inst; vcioinit(&inst, vcionext(&data)+ntar, ntar+SLOP);
	vcd->addr = &addr; vcioinit(&addr, vcionext(&inst)+ntar+SLOP, ntar+SLOP);

	/* initialize address caches and saved COPY instruction */
	vcdkaclear(vcd->cache);
	vcd->save = &save; save.dtsz = save.addr = save.mode = 0;

	vcparse(&vcd->vcpa, (vcd->flags&SFXSORT) ? -1 : 32*1024);
	vcdputcopy(vcd, 0, 0, 0);

	ctrl = 0;
	d = vciosize(vcd->data); rd = vciodata(vcd->data);
	i = vciosize(vcd->inst); ri = vciodata(vcd->inst);
	a = vciosize(vcd->addr); ra = vciodata(vcd->addr);
	if(vc->coder) /* continuation coding */
	{	if(VCSETCTXT(vc->coder, ctxt->dctxt) < 0)
			return -1;
		if((n = vcapply(vc->coder, rd, d, &p)) >= 0 && n < d )
		{	d = n; rd = p;
			ctrl |= VCD_DATACOMPRESS;
		}

		if(VCSETCTXT(vc->coder, ctxt->ictxt) < 0)
			return -1;
		if((n = vcapply(vc->coder, ri, i, &p)) >= 0 && n < i )
		{	i = n; ri = p;
			ctrl |= VCD_INSTCOMPRESS;
		}

		if(VCSETCTXT(vc->coder, ctxt->actxt) < 0)
			return -1;
		if((n = vcapply(vc->coder, ra, a, &p)) >= 0 && n < a )
		{	a = n; ra = p;
			ctrl |= VCD_ADDRCOMPRESS;
		}
	}

	/* output data */
	vcioinit(&data, output, d+i+a+SLOP);
	vcioputu(&data, ntar);
	vcioputc(&data, ctrl);
	vcioputu(&data, d);
	vcioputu(&data, i);
	vcioputu(&data, a);
	vcioputs(&data, rd, d);
	vcioputs(&data, ri, i);
	vcioputs(&data, ra, a);
	n = vciosize(&data);

	if(del)
		*del = output;

	return n;
}


#if __STD_C
static ssize_t vcdundiff(Vcodex_t* vc, const Void_t* del, size_t ndel, Void_t** out)
#else
static ssize_t vcdundiff(vc, del, ndel, out)
Vcodex_t*	vc;
Void_t*		del;
size_t		ndel;
Void_t**	out;
#endif
{
	Vcchar_t	*t, *s;
	ssize_t		d, i, a, ntar, nsrc;
	Vcchar_t	*tar, *etar, *src, *rd, *ri, *ra;
	Vcio_t		data, inst, addr;
	Vcdcode_t	*code;
	Vcdcache_t	*ka;
	int		ctrl;
	Vcdiff_t	*vcd = VCGETMETH(vc, Vcdiff_t*);
	Vcdctxt_t	*ctxt = VCGETCTXT(vc, Vcdctxt_t*);
	Vcdisc_t	*disc = VCGETDISC(vc);

	/* read size of data buffers */
	vcioinit(&data, del, ndel);
	ntar = (ssize_t)vciogetu(&data);/* buffer size for target data		*/
	ctrl = (int)vciogetc(&data);	/* to see if datasets were compressed	*/
	d = (ssize_t)vciogetu(&data); 	/* size of unmatched data		*/
	i = (ssize_t)vciogetu(&data); 	/* size of instruction set		*/
	a = (ssize_t)vciogetu(&data); 	/* size of COPY addresses		*/

	/* make sure we have enough data for decoding */
	if((d+i+a) != vciomore(&data) )
		RETURN(-1);

	/* data, instructions and COPY addresses */
	rd = vcionext(&data);
	ri = rd+d;
	ra = ri+i;

	/* recompute the data, instruction and address streams if encoded */
	if(vc->coder)
	{	if(ctrl&VCD_DATACOMPRESS)
		{	if(VCSETCTXT(vc->coder, ctxt->dctxt) < 0)
				RETURN(-1);
			if((d = vcapply(vc->coder, rd, d, &rd)) < 0 )
				RETURN(-1);
		}
		if(ctrl&VCD_INSTCOMPRESS)
		{	if(VCSETCTXT(vc->coder, ctxt->ictxt) < 0)
				RETURN(-1);
			if((i = vcapply(vc->coder, ri, i, &ri)) < 0 )
				RETURN(-1);
		}
		if(ctrl&VCD_ADDRCOMPRESS)
		{	if(VCSETCTXT(vc->coder, ctxt->actxt) < 0)
				RETURN(-1);
			if((a = vcapply(vc->coder, ra, a, &ra)) < 0 )
				RETURN(-1);
		}
	}

	vcioinit(&data, rd, d);
	vcioinit(&inst, ri, i);
	vcioinit(&addr, ra, a);

	code = vcd->table->code;
	if((ka = vcd->cache) )
		vcdkaclear(ka);

	/* buffer to reconstruct data */
	if(!(tar = vcsetbuf(vc, NIL(Vcchar_t*), ntar, 0)) )
		RETURN(-1);
	etar = tar + ntar;
	nsrc = disc ? disc->size : 0;
	src  = disc ? (Vcchar_t*)disc->data : NIL(Vcchar_t*);

	for(t = tar; t < etar; )
	{	Vcdcode_t*	cd;
		Vcdinst_t*	in;
		ssize_t		sz;

		/* get the pair of instructions */
		if(vciomore(&inst) <= 0)
			RETURN(-1);
		cd = code + vciogetc(&inst);

		for(i = 0; i < 2; ++i)
		{	in = i == 0 ? &cd->inst1 : &cd->inst2;
			if(in->type == VCD_NOOP)
				continue;

			if((sz = in->size) == 0)
			{	if(vciomore(&inst) <= 0)
					RETURN(-1);
				sz = vciogetu(&inst);
			}

			if(t+sz > etar)
				RETURN(-1);

			if(in->type == VCD_BYTE)
			{	d = in->mode;
				for(; sz > 0; --sz)
					*t++ = (Vcchar_t)d;
			}
			else if(in->type == VCD_COPY)
			{	if(vciomore(&addr) <= 0)
					RETURN(-1);
				d = vcdkagetaddr(ka,&addr,(t-tar)+nsrc,in->mode);
				if(d < nsrc)
				{	if(d+sz > nsrc)
						RETURN(-1);
					s = src+d;
				}
				else
				{	if((d -= nsrc) >= (t-tar) || (d+sz) > ntar)
						RETURN(-1);
					s = tar+d;
				}
				for(; sz > 0; --sz)
					*t++ = *s++;
			}
			else if(in->type == VCD_ADD)
			{	if(vciomore(&data) < sz)
					RETURN(-1);
				vciogets(&data, t, sz);
				t += sz;
			}
			else if(in->type == VCD_RUN)
			{	if(vciomore(&data) <= 0)
					RETURN(-1);
				d = vciogetc(&data);
				for(; sz > 0; --sz)
					*t++ = (Vcchar_t)d;
			}
			else	RETURN(-1);
		}
	}

	if(vciomore(&data) != 0 || vciomore(&inst) != 0 || vciomore(&addr) != 0)
		return -1;

	if(out)
		*out = (Void_t*)tar;

	return ntar;
}

#if __STD_C
static int vcdevent(Vcodex_t* vc, int type, Void_t* init)
#else
static int vcdevent(vc, type, init)
Vcodex_t*	vc;
int		type;
Void_t*		init;
#endif
{
	Vcdiff_t	*vcd;
	Vcdctxt_t	*ctxt;
	char		*args;
	int		k, rv = -1;

	_vcdtblinit(); /* initialize default code tables */

	if(type == VC_OPENING)
	{	if(!(vcd = (Vcdiff_t*)calloc(1,sizeof(Vcdiff_t))) )
			return -1;
		VCDINIT(vcd);

		for(args = (char*)init; args && *args; ++args)
		{	for(k = 0; _Diffargs[k].name; ++k)
				if(*args == _Diffargs[k].name[0])
					vcd->flags |= (int)_Diffargs[k].data;
		}

		vcd->table = _Vcdtbl;
		vcd->index = &_Vcdindex;
		vcd->size  = &_Vcdsize;
		vcd->vcpa.parsef = vcdputinst;
		if(!(vcd->cache = vcdkaopen(vcd->table->s_near,vcd->table->s_same)))
			goto do_closing;

		VCINITCTXT(vc, Vcdctxt_t);
		VCSETMETH(vc, vcd);
		return 0;
	}
	else if(type == VC_CLOSING)
	{	rv = 0;
	do_closing:
		if((vcd = VCGETMETH(vc, Vcdiff_t*)) )
		{	if(vcd->cache)
				vcdkaclose(vcd->cache);
			if(vcd->table && (vcd->flags&FREETABLE) )
				free(vcd->table);
			free(vcd);
		}
		VCSETMETH(vc, NIL(Vcdiff_t*));
		return rv;
	}
	else if(type == VC_INITCTXT)
	{	if(!(ctxt = (Vcdctxt_t*)init))
			return -1;
		else
		{	ctxt->dctxt = ctxt->ictxt = ctxt->actxt = -1;
			return 1;
		}
	}
	else	return 0;
}

Vcmethod_t _Vcdelta =
{	vcddiff,
	vcdundiff,
	0,
	0,
	vcdevent,
	"delta", "\144\145\154\164\141", "Delta compression",
	_Diffargs,
	1024*1024,
	VCNEXT(Vcdelta)
};

VCLIB(Vcdelta)
