/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*           Copyright (c) 2003-2006 AT&T Knowledge Ventures            *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                      by AT&T Knowledge Ventures                      *
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

#include	<vcsfio.h>

/*	Sfio discipline to run Vcodex methods for compression.
**
**	Written by Kiem-Phong Vo, kpv@research.att.com.
*/

#define VCSF_BUFSIZE	(256*1024)	/* decoding buffer	*/
#define VCSF_BUFMIN	(64*1024)	/* surely affordable!	*/
#define VCSF_WINSIZE	(4*1024*1024)	/* dflt encoding window	*/
#define VCSF_SLACK	(16*sizeof(size_t)) /* for extra coding	*/
#define VCSFDTSZ(sz)	((sz) + ((sz)/2 >= 1024 ? (sz)/2 : 1024) )

/* note that this bit must be separate from VC_FLAGS */
#define VCD_HEADER	010000		/* output Vcdiff header	*/

typedef struct _sfdc_s
{
	Sfdisc_t	disc;	/* Sfio discipline must be 1st	*/
	Sfio_t*		sf;	/* stream to do IO with		*/

	unsigned int	flags;	/* states of the handle		*/
	int		save;	/* cannot free this yet		*/

	Sfoff_t		pos;	/* current stream position	*/
	Vcwindow_t*	vcw;	/* to match delta-comp windows 	*/
	Vcdisc_t	vcdc;	/* to specify source string	*/

	Vcodex_t*	vc;	/* Vcodex handle for de/coding	*/
	Vcio_t*		io;	/* to write integers, etc.	*/

	Vcchar_t*	data;	/* data buffer for external use	*/
	ssize_t		dtsz;
	Vcchar_t*	endd;	/* end of data buffer		*/
	Vcchar_t*	next;	/* where to read/write		*/

	Vcchar_t*	base;	/* buffer for encoded data	*/
	ssize_t		bssz;
	Vcchar_t*	endb;	/* end of base buffer		*/

	Vcchar_t*	code;	/* space to be used by coder	*/
	ssize_t		cdsz;

	char*		state;	/* store of compression states	*/

	Vcchar_t*	encoding;	/* encoding */
	size_t		encodesize;	/* encoding size */

	Vcsfmeth_t	meth[VCSF_MAX+1];
	int		nmeth;
} Sfdc_t;

#ifndef KPVDELETE /* to decode the VCDX format that uses method numbers */

typedef struct _map_s
{	int		mtid;
	Vcmethod_t*	meth;
	char*		name;
} Map_t;

#if __STD_C
static ssize_t hdrvcdx(Vcchar_t** datap, ssize_t dtsz, Vcchar_t* code, ssize_t cdsz)
#else
static ssize_t hdrvcdx(datap, dtsz, code, cdsz)
Vcchar_t**	datap;	/* old header data	*/
ssize_t		dtsz;
Vcchar_t*	code;	/* space for new code	*/
ssize_t		cdsz;
#endif
{
	int		k, n, id;
	Vcio_t		io;
	Vcsfmeth_t	mt[256];
	static Map_t	Map[16];

	if(!Map[0].meth) /* initialize mapping */
	{	k = 0;
		Map[k].mtid = 0;	Map[k++].meth = Vcdelta;
		Map[k].mtid = 1;	Map[k++].meth = Vchuffman;
		Map[k].mtid = 2;	Map[k++].meth = Vchuffgroup;
		Map[k].mtid = 4;	Map[k++].meth = Vcbwt;
		Map[k].mtid = 5;	Map[k++].meth = Vcrle;
		Map[k].mtid = 6;	Map[k++].meth = Vcmtf;
		Map[k].mtid = 7;	Map[k++].meth = Vctranspose;
		Map[k].mtid = 8;	Map[k++].name = "table";
		Map[k].mtid = 9;	Map[k++].meth = Vchuffpart;
		Map[k].mtid = 50;	Map[k++].meth = Vcmap;
		Map[k].mtid = 100;	Map[k++].name = "ama";
		Map[k].mtid = 101;	Map[k++].name = "ss7";
		Map[k].mtid = -1;	Map[k++].meth = NIL(Vcmethod_t*);
	}

	/* parse old header data */
	vcioinit(&io, *datap, dtsz);
	for(n = -1; vciomore(&io) > 0; )
	{	if((n += 1) >= sizeof(mt)/sizeof(mt[0]) )
			return -1; /* not enough space - bad! */

		id = vciogetc(&io);
		for(k = 0;; k++)
		{
			if(Map[k].mtid < 0)
				return dtsz; /* default to new type */
			if(Map[k].mtid == id)
			{
				if(Map[k].name)
				{	/* implemented as a plugin */
					Map[k].meth = vcgetmeth(Map[k].name, NIL(char*), NIL(char**));
					Map[k].name = 0;
				}
				if(!Map[k].meth)
					return -1; /* plugin not found */
				break;
			}
		}
		mt[n].meth = Map[k].meth;
		if(vciomore(&io) <= 0)
			return dtsz;
		mt[n].size = (ssize_t)vciogetu(&io);
		if(mt[n].size < 0 || vciomore(&io) < mt[n].size )
			return dtsz;
		mt[n].data = vcionext(&io); vcioskip(&io, mt[n].size);
	}

	/* reconstruct the right header */
	vcioinit(&io, code, cdsz);
	for(; n >= 0; --n)
	{	k = strlen(mt[n].meth->ident)+1 + VCSIZEU(mt[n].size) + mt[n].size;
		if(vciomore(&io) < k)
			return -1;

		vcioputs(&io, mt[n].meth->ident, strlen((char*)mt[n].meth->ident)+1);
		if(mt[n].size <= 0)
			vcioputu(&io, 0);
		else if(mt[n].meth == Vcrle || mt[n].meth == Vcmtf)
		{	if(*((Vcchar_t*)mt[n].data) == 0 )
			{	vcioputu(&io, 1);
				vcioputc(&io, '0');
			}
			else	vcioputu(&io, 0);
		}
		else
		{	vcioputu(&io, mt[n].size);
			vcioputs(&io, mt[n].data, mt[n].size);
		}
	}

	*datap = code;
	return vciosize(&io);
}

#endif /* KPVDELETE */

#if __STD_C
static ssize_t hdrvcdiff(Vcio_t* iodt, int indi, Vcchar_t* code, ssize_t cdsz)
#else
static ssize_t hdrvcdiff(iodt, indi, code, cdsz)
Vcio_t*		iodt;	/* data to be decoded	*/
int		indi;	/* indicator byte	*/
Vcchar_t*	code;	/* code buffer		*/
ssize_t		cdsz;	/* size of code buffer	*/
#endif
{
	Vcio_t	io;

	if(indi&VCD_COMPRESSOR) /* no secondary compressor */
		return -1;

	vcioinit(&io, code, cdsz);

	/* write out the method identification string */
	if((cdsz = strlen(Vcdelta->ident)+1) > vciomore(&io))
		return -1;
	vcioputs(&io, Vcdelta->ident, cdsz);

	if(indi&VCD_CODETABLE)
	{	if((cdsz = vciogetu(iodt)) < 0 )
			return -1;

		if(vciomore(&io) < VCSIZEU(cdsz))
			return -1;
		vcioputu(&io, cdsz);

		if(vciomore(&io) < cdsz)
			return -1;
		vciomove(iodt, &io, cdsz);
	}
	else
	{	if(vciomore(&io) <= 0)
			return -1;
		vcioputu(&io, 0);
	}

	return vciosize(&io);
}

#if __STD_C
static int putheader(Sfdc_t* sfdc)
#else
static int putheader(sfdc)
Sfdc_t*		sfdc;
#endif
{
	Vcodex_t	*vc, *coder;
	Vcchar_t	*code;
	ssize_t		sz;
	int		k, flags;

	/* continuation means the handles must be constructed in reverse order */
	flags = sfdc->flags | VC_CLOSECODER;
	coder = NIL(Vcodex_t*);
	for(k = sfdc->nmeth-1; k >= 0; --k)
	{	if(!(vc = vcopen(0,sfdc->meth[k].meth,sfdc->meth[k].data,coder,flags)) )
			return -1;
		coder = vc;
	}
	sfdc->vc = vc;

	if(sfdc->state) /* reinitialize from saved states */
	{	Sfio_t* sf;
		if((sf = sfopen(0, sfdc->state, "r")) )
		{	if((sz = (ssize_t)sfsize(sf)) > 0 &&
			   (code = sfreserve(sf, sz, -1)) )
				vcrestore(sfdc->vc, code, sz);
			sfclose(sf);
		}
	}

	/* get the string that codes the methods */
	if((sz = vcextract(vc, (Void_t**)&code, 0)) <= 0 )
		return -1;

	/* output data that can be used to recreate handles */
	vcioputc(sfdc->io, VC_HEADER0);
	vcioputc(sfdc->io, VC_HEADER1);
	vcioputc(sfdc->io, VC_HEADER2);
	vcioputc(sfdc->io, (sfdc->flags&VCSF_VCDIFFHEADER) ? 0 : VC_HEADER3);

	/* indicator byte - see also compatibility with RFC3284 */
	vcioputc(sfdc->io, 0);

	/* data encoding the methods used */
	if(!(sfdc->flags&VCSF_VCDIFFHEADER) )
	{	vcioputu(sfdc->io, sz);
		vcioputs(sfdc->io, code, sz);
	}

	return 0;
}

#if __STD_C
static int makebuf(Sfdc_t* sfdc, ssize_t size)
#else
static int makebuf(sfdc, size)
Sfdc_t*	sfdc;
ssize_t	size;
#endif
{
	Vcchar_t	*base, *oldbase;

	size = ((size + VCSF_BUFMIN - 1)/VCSF_BUFMIN)*VCSF_BUFMIN;
	if(sfdc->bssz >= size)
		return 0;

	oldbase = sfdc->base;
	if(!(base = (Vcchar_t*)malloc(size)) )
		return -1;

	memcpy(base, oldbase, sfdc->endb - oldbase);
	sfdc->endb = base + (sfdc->endb - oldbase);
	sfdc->code = base + (sfdc->code - oldbase);
	sfdc->base = base;
	sfdc->bssz = size;
	free(oldbase);

	return 0;
}

#if __STD_C
static ssize_t fillbuf(Sfdc_t* sfdc, Sfio_t* f, Sfdisc_t* disc)
#else
static ssize_t fillbuf(sfdc, f, disc)
Sfdc_t*		sfdc;
Sfio_t*		f;
Sfdisc_t*	disc;
#endif
{
	ssize_t	sz, n;

	if((sz = sfdc->endb - sfdc->code) <= 0 )
	{	sfdc->endb = sfdc->code = sfdc->base;
		sz = 0;
	}
	else if(sfdc->code > sfdc->base)
	{	memcpy(sfdc->base, sfdc->code, sz);
		sfdc->endb = (sfdc->code = sfdc->base) + sz;
	}

	for(; sz < sfdc->bssz; sz += n, sfdc->endb += n)
		if((n = sfrd(f, sfdc->endb, sfdc->bssz-sz, disc)) <= 0 )
			break;

	return sz;
}

#if __STD_C
static ssize_t getheader(Sfdc_t* sfdc, Sfio_t* f, int peek)
#else
static ssize_t getheader(sfdc, f, peek)
Sfdc_t*		sfdc;
Sfio_t*		f;
int		peek;
#endif
{
	ssize_t		cdsz, sz;
	Vcchar_t	*code, cdbuf[4*1024];
	int		indi, head, loop;
	Vcodex_t	*vc;
	Vcio_t		io;

	if(peek)
	{	/* peek and verify header magic -- ignore if no magic */
		if (!(code = sfreserve(f, 4, SF_LOCKR)))
			return 0;
		memcpy(cdbuf, code, 4);
		sfread(f, code, 0);
		if(cdbuf[0]!=VC_HEADER0 || cdbuf[1]!=VC_HEADER1 || cdbuf[2]!=VC_HEADER2 || cdbuf[3]!=VC_HEADER3 && cdbuf[3]!=0)
			return 0;
	}
	for(loop = 0;; ++loop)
	{	
		/* buffer was too small for header data */
		if(loop > 0 && (sfdc->endb - sfdc->base) >= sfdc->bssz &&
		   makebuf(sfdc, sfdc->bssz+VCSF_BUFMIN) < 0)
			return -1;

		/* read header data as necessary */
		sz = sfdc->endb - sfdc->code;
		if(loop > 0 || sz <= 0)
		{	if(fillbuf(sfdc, f, sfdc->vc ? &sfdc->disc : NIL(Sfdisc_t*)) <= 0 )
				return loop ? -1 : 0;
			if((sz = sfdc->endb - sfdc->code) <= 0 )
				return loop ? -1 : 0;
		}

		vcioinit(&io, sfdc->code, sz);

		if(vciomore(&io) < 5) /* need 4-byte header + indicator byte */
			continue;

		if(vciogetc(&io) != VC_HEADER0)
			return loop ? -1 : 0;
		if(vciogetc(&io) != VC_HEADER1)
			return loop ? -1 : 0;
		if(vciogetc(&io) != VC_HEADER2)
			return loop ? -1 : 0;
		if((head = vciogetc(&io)) != 0 /* RFC3284 Vcdiff header */ &&
		    head != VC_HEADER3 /* normal Vcodex header */ )
			return loop ? -1 : 0;

		if((indi = vciogetc(&io)) & VC_EXTRAHEADER )
		{	if((sz = vciogetu(&io)) < 0) /* skip app-specific data */
				continue;
			vcioskip(&io, sz);
		}

		if(head == 0) /* RFC3284 Vcdiff format */
		{	if((cdsz = hdrvcdiff(&io, indi, cdbuf, sizeof(cdbuf))) < 0)
				return -1;
			else if(cdsz == 0)
				continue;
			else	code = cdbuf;
		}
		else /* Vcodex encoding */
		{	if((cdsz = vciogetu(&io)) < 0 )
				continue;
			else if(cdsz == 0)
				return -1;
			if(vciomore(&io) < cdsz)
				continue;
			code = vcionext(&io);
			vcioskip(&io, cdsz);

#ifndef KPVDELETE /* upward compatibility stuff for most recent VCDX change */
			cdsz = hdrvcdx(&code, cdsz, cdbuf, sizeof(cdbuf));
#endif
		}

		/* successfully read the header data */
		sfdc->code = vcionext(&io);
		break;
	}

	/* construct new decoding handle and reset */
	if(cdsz <= 0 )
		return -1;
	if(!(vc = vcrestore(NIL(Vcodex_t*),code,cdsz)) )
		return -1;
	if(sfdc->vc)
		vcclose(sfdc->vc);
	sfdc->vc = vc;
	if(sfdc->encoding)
		free(sfdc->encoding);
	if(!(sfdc->encoding = (Vcchar_t*)malloc(cdsz*sizeof(Vcchar_t))))
		return -1;
	memcpy(sfdc->encoding, code, cdsz);
	sfdc->encodesize = cdsz;
	return 1;
}

#if __STD_C
static ssize_t encode(Sfdc_t* sfdc, Vcchar_t* data, size_t dtsz)
#else
static ssize_t encode(sfdc, data, dtsz)
Sfdc_t*		sfdc;
Vcchar_t*	data;
size_t		dtsz;
#endif
{
	Vcchar_t	*code, ctrl, *dt;
	ssize_t		cdsz, size, sz, bssz, dosz;
	Vcchar_t	*base;
	Vcwmatch_t	*wm;
	Vcio_t		io;

	vcioinit(&io, sfdc->base, sfdc->bssz);
	sfdc->io = &io;

	if(!sfdc->vc && putheader(sfdc) < 0)
		return -1;

	sfdc->code = NIL(Vcchar_t*); sfdc->cdsz = 0;

	for(size = 0, dosz = dtsz, dt = data; size < dtsz; )
	{	/* control data */
		ctrl = 0;
		sfdc->vcdc.data = NIL(Void_t*);
		sfdc->vcdc.size = 0;

		/* compute a matching window to enhance compression */
		wm = NIL(Vcwmatch_t*);
		if(sfdc->vcw)
			wm = vcwapply(sfdc->vcw, dt, dosz, sfdc->pos);
		if(wm)
		{	/**/ASSERT(wm->msize <= dosz);
			if(wm->wsize > 0 && wm->wpos >= 0)
			{	sfdc->vcdc.data = wm->wdata;
				sfdc->vcdc.size = wm->wsize;
			}

			ctrl = wm->type & (VCD_SOURCEFILE|VCD_TARGETFILE);
			dosz = wm->msize; /* amount doable now */
			/**/ PRINT(2,"dtpos=%d ", sfdc->pos);
			/**/ PRINT(2,"dtsz=%d ", dosz);
			/**/ PRINT(2,"wpos=%d ", wm->wpos);
			/**/ PRINT(2,"wsz=%d ", wm->wsize);
			/**/ PRINT(2,"mtsz=%d\n", wm->msize);
		}
		if(sfdc->vcw) /* set window data */
			vcdisc(sfdc->vc, &sfdc->vcdc);

		vcsetbuf(sfdc->vc, NIL(Vcchar_t*), -1, 0); /* free buffers */
		if((cdsz = vcapply(sfdc->vc, dt, dosz, &code)) <= 0 ||
		   (sz = vcundone(sfdc->vc)) >= dosz )
		{	ctrl = VC_RAW; /* coder failed, output raw data */
			code = dt;
			cdsz = dosz;
		}
		else
		{	dosz -= (sz > 0 ? sz : 0); /* true processed amount */
			if(sfdc->vcw) /* tell window matcher compressed result */
				vcwfeedback(sfdc->vcw, cdsz);
		}

		vcioputc(&io, ctrl);
		if(ctrl&(VCD_SOURCEFILE|VCD_TARGETFILE) )
		{	vcioputu(&io, wm->wsize);
			vcioputu(&io, wm->wpos);
		}
		vcioputu(&io, cdsz);
		if(vciomore(&io) < cdsz) /* buffer too small */
		{	sz = vciosize(&io);
			bssz = ((cdsz+sz+1023)/1024)*1024;
			if(!(base = (Vcchar_t*)malloc(bssz)) )
				return -1;
			memcpy(base, sfdc->base, vciosize(&io));
			if(sfdc->base)
				free(sfdc->base);
			sfdc->base = base;
			sfdc->bssz = bssz;

			vcioinit(&io, sfdc->base, sfdc->bssz);
			vcioskip(&io, sz);
		}
		vcioputs(&io, code, cdsz);

		sfdc->pos += dosz; /* advance by amount consumed */
		dt += dosz;
		size += dosz;

		if((dosz = dtsz-size) > 0 )
		{	if(wm && wm->more) /* more subwindows to do */
				continue;
			else /* need fresh data */
			{	if(data == sfdc->data) /* shift undone data */
				{	memcpy(data, data+size, dosz);
					sfdc->next = data + dosz;
				}
				break;
			}
		}
	}

	sfdc->code = sfdc->base; sfdc->cdsz = vciosize(&io);

	return size; /* return amount processed */
}

/* read data from the filter */
#if __STD_C
static ssize_t vcsfdcread(Sfio_t* f, Void_t* buf, size_t n, Sfdisc_t* disc)
#else
static ssize_t vcsfdcread(f, buf, n, disc)
Sfio_t*		f;	/* stream reading from		*/
Void_t*		buf;	/* buffer to read data into	*/
size_t		n;	/* number of bytes requested	*/
Sfdisc_t*	disc;	/* discipline structure		*/
#endif
{
	Vcchar_t	*dt, *text;
	int		ctrl;
	ssize_t		sz, r, d, m;
	Sfoff_t		pos;
	Vcwmatch_t	*wm;
	Vcio_t		io;
	Sfdc_t		*sfdc = (Sfdc_t*)disc;

	if(!(sfdc->flags&VC_DECODE) )
		return -1;

	for(sz = 0, dt = (Vcchar_t*)buf; sz < n; sz += r, dt += r)
	{	/* copy already decoded data */
		if((r = sfdc->endd - sfdc->next) > 0 )
		{	r = r > (n-sz) ? (n-sz) : r;
			memcpy(dt, sfdc->next, r);
			sfdc->next += r;
		}
		else /* need to decode a new batch of data */
		{
			if((d = (sfdc->endb - sfdc->code)) < 2*sizeof(size_t))
			{	if(fillbuf(sfdc, f, disc) <= 0 )
					break;
				d = sfdc->endb - sfdc->code;
			}

			vcioinit(&io, sfdc->code, d);

			sfdc->vcdc.data = NIL(Void_t*);
			sfdc->vcdc.size = 0;

			/* get the control byte */
			ctrl = vciogetc(&io);

			/* let upper level write and flush first */
			if((ctrl&VCD_TARGETFILE) && sz > 0)
				break;

			if(ctrl & (VCD_SOURCEFILE|VCD_TARGETFILE) )
			{	if(!sfdc->vcw)
					BREAK;
				if((d = (ssize_t)vciogetu(&io)) < 0)
					BREAK;
				if((pos = (Sfoff_t)vciogetu(&io)) < 0)
					BREAK;
				if(!(wm = vcwapply(sfdc->vcw, (Void_t*)ctrl, d, pos)) )
					BREAK;
				sfdc->vcdc.data = wm->wdata;
				sfdc->vcdc.size = wm->wsize;
			}
			else if(ctrl == VC_EOF) /* a new decoding context */
			{	sfdc->code = vcionext(&io);
				if(getheader(sfdc, f, 0) < 0 )
					break;
				else	continue;
			}
			else if(ctrl != 0 && ctrl != VC_RAW)
				BREAK;

			if(sfdc->vcw)
				vcdisc(sfdc->vc, &sfdc->vcdc);

			/* size of coded data */
			if((d = vciogetu(&io)) <= 0)
				BREAK;

			/* make sure all the data is available */
			if((vcionext(&io) + d) > sfdc->endb)
			{	sfdc->code = vcionext(&io);
				if((m = d+VCSF_SLACK) > sfdc->bssz &&
				   makebuf(sfdc, m) < 0 )
					RETURN(-1);
				if(fillbuf(sfdc, f, disc) <= 0)
					RETURN(-1);
				if((m = sfdc->endb - sfdc->code) < d )
					RETURN(-1);
				vcioinit(&io, sfdc->code, m);
			}

			/* decode data */
			sfdc->code = vcionext(&io);
			if(ctrl == VC_RAW)
			{	text = sfdc->code;
				m = d;
			}
			else
			{	vcsetbuf(sfdc->vc, NIL(Vcchar_t*), -1, 0);
				if((m = vcapply(sfdc->vc, sfdc->code, d, &text)) <= 0)
					BREAK;
			}

			sfdc->code += d; /* advance passed processed data */

			/* set plaintext data buffer */
			sfdc->data = sfdc->next = text;
			sfdc->endd = text+m;
			sfdc->dtsz = m;
		}
	}

	return sz;
}

#if __STD_C
static ssize_t vcsfdcwrite(Sfio_t* f, const Void_t* buf, size_t n, Sfdisc_t* disc)
#else
static ssize_t vcsfdcwrite(f, buf, n, disc)
Sfio_t*		f;	/* stream writing to		*/
Void_t*		buf;	/* buffer of data to write out	*/
size_t		n;	/* number of bytes requested	*/
Sfdisc_t*	disc;	/* discipline structure		*/
#endif
{
	Vcchar_t	*dt;
	ssize_t		sz, w;
	Sfdc_t		*sfdc = (Sfdc_t*)disc;

	if(!(sfdc->flags & VC_ENCODE) )
		return -1;

	for(sz = 0, dt = (Vcchar_t*)buf; sz < n; sz += w, dt += w)
	{	if(buf == (Void_t*)sfdc->data)
		{	/* final flush */
			w = sfdc->next - sfdc->data;
			sfdc->next = sfdc->data;

			if((w = encode(sfdc, sfdc->data, w)) < 0 )
				break;
			if(sfwr(f, sfdc->code, sfdc->cdsz, disc) != sfdc->cdsz)
				break;

			if(sfdc->next > sfdc->data)
			{	sz += w;
				w   = 0; /* back to flushing */
				continue;
			}
		}

		if((w = sfdc->endd - sfdc->next) == 0)
		{	/* flush a full buffer */
			sfdc->next = sfdc->data;
			if(encode(sfdc, sfdc->data, sfdc->dtsz) < 0)
				break;
			if(sfwr(f, sfdc->code, sfdc->cdsz, disc) != sfdc->cdsz)
				break;

			w = sfdc->endd - sfdc->next; /* bufferable space */
		}

		/* process data directly if buffer is empty and data is large */
		if(w == sfdc->dtsz && (n-sz) >= w)
		{	if((w = encode(sfdc, dt, n-sz)) < 0)
				break;
			if(sfwr(f, sfdc->code, sfdc->cdsz, disc) != sfdc->cdsz)
				break;
		}
		else /* accumulating data into buffer */
		{	w = w > (n-sz) ? (n-sz) : w;
			memcpy(sfdc->next, dt, w);
			sfdc->next += w;
		}
	}

	return sz;
}

/* for the duration of this discipline, the stream is unseekable */
#if __STD_C
static Sfoff_t vcsfdcseek(Sfio_t* f, Sfoff_t addr, int offset, Sfdisc_t* disc)
#else
static Sfoff_t vcsfdcseek(f, addr, offset, disc)
Sfio_t*		f;
Sfoff_t		addr;
int		offset;
Sfdisc_t*	disc;
#endif
{	
	return (Sfoff_t)(-1);
}

/* on close, remove the discipline */
#if __STD_C
static int vcsfdcexcept(Sfio_t* f, int type, Void_t* data, Sfdisc_t* disc)
#else
static int vcsfdcexcept(f,type,data,disc)
Sfio_t*		f;
int		type;
Void_t*		data;
Sfdisc_t*	disc;
#endif
{
	ssize_t		sz;
	Sfdc_t		*sfdc = (Sfdc_t*)disc;

#define VCSF_DISC	((((int)('V'))<<7) | ((int)('C')) )
	switch(type)
	{
	case VCSF_DISC: /* get the discipline */
		if(data)
			*((Sfdc_t**)data) = sfdc;
		return VCSF_DISC;
	case SF_SYNC:
	case SF_DPOP:
	case SF_CLOSING:
	case SF_ATEXIT:
		if(sfdc->flags & VC_ENCODE)
		{	if((sz = sfdc->next - sfdc->data) > 0 )
			{	
#if _SFIO_H == 1		/* Sfio: this will wind up calling vcsfdcwrite() */
				sfset(f, SF_IOCHECK, 0); /* to avoid recursion in sfsync() */
				if(sfwr(f, sfdc->data, sz, disc) != sz)
					return -1;
				sfset(f, SF_IOCHECK, 1); /* restore for future sfsync() calls */
#else				/* Stdio: must call vcsfdcwrite() directly to encode */
				if(vcsfdcwrite(f, sfdc->data, sz, disc) != sz)
					return -1;
#endif
				sfdc->next = sfdc->data;
			}

			if(type == SF_CLOSING || type == SF_ATEXIT)
			{	Vcio_t		io;

				/* back to plain text mode */
				sfdc->save = 1;
				sfdisc(f, NIL(Sfdisc_t*));
				sfdc->save = 0;

				vcioinit(&io, sfdc->base, sfdc->bssz);
				if(!sfdc->vc) /* write header if not done yet */
				{	sfdc->io = &io;
					if(putheader(sfdc) < 0 )
						return -1;
					vcioputu(&io,0);
				}
				vcioputc(&io, VC_EOF); /* write the eof marker */

				sz = vciosize(&io); /* output to stream */
				if(sfwr(f, sfdc->base, sz, NIL(Sfdisc_t*)) != sz)
					return -1;

				if(sfdc->state ) /* save current states */
				{	Sfio_t	*sf;
					if((sf = sfopen(0, sfdc->state, "w")) )
					{	ssize_t	cdsz; Void_t *code;
						cdsz = vcextract(sfdc->vc, &code, 1);
						if(cdsz > 0)
							sfwrite(sf, code, cdsz);
						sfclose(sf);
					}
					sfdc->state = NIL(char*);
				}
			}
		}

		if(!sfdc->save && (type == SF_CLOSING || type == SF_DPOP) )
		{	if(sfdc->vc)
				vcclose(sfdc->vc);
			if(sfdc->vcw && (sfdc->flags&VCSF_CLOSEWINDOW) )
				vcwclose(sfdc->vcw);
			if((sfdc->flags&VC_ENCODE) && sfdc->data)
				free(sfdc->data);
			if(sfdc->base)
				free(sfdc->base);
			if(sfdc->encoding)
				free(sfdc->encoding);
			free(sfdc);
		}

		break;
	}

	return 0;
}

/* return the method list for data transformation */
#if __STD_C
Vcsfmeth_t* vcsfmeth(Vcsfio_t* vcf, ssize_t* nmeth)
#else
Vcsfmeth_t* vcsfmeth(vcf, nmeth)
Vcsfio_t*	vcf;
ssize_t*	nmeth;
#endif
{
	Sfdc_t*		sfdc;

#if _SFIO_H == 1
	if(sfraise((Sfio_t*)vcf, VCSF_DISC, (Void_t*)(&sfdc)) != VCSF_DISC)
		return NIL(Vcsfmeth_t*);
#else
	sfdc = (Sfdc_t*)vcf;
#endif 
	if(!sfdc->nmeth)
	{	ssize_t		sz, k, n;
		char		*mt;
		Vcio_t		io;

		if(!sfdc->encoding || !sfdc->encodesize)
			return 0;
		vcioinit(&io, sfdc->encoding, sfdc->encodesize);
		n = 0;
		while(vciomore(&io) > 0)
		{	if(n >= VCSF_MAX)
				return 0;
			mt = (char*)vcionext(&io);
			for(sz = vciomore(&io), k = 0; k < sz; ++k)
				if(mt[k] == 0)
					break;
			if(k >= sz)
				return 0;
			if(!(sfdc->meth[n].meth = vcgetmeth(NIL(char*), mt, NIL(char**))) )
				return 0;
			vcioskip(&io, k+1);

			/* get the initialization data, if any */
			if((sz = (ssize_t)vciogetu(&io)) < 0 || sz > vciomore(&io))
				return 0;
			if(sfdc->meth[n].size = sz)
			{	sfdc->meth[n].data = (Void_t*)vcionext(&io);
				vcioskip(&io, sz);
			}
			else
				sfdc->meth[n].data = 0;
			n++;
		}
		if(!n)
			return 0;
		sfdc->nmeth = n;
	}
	if(nmeth)
		*nmeth = sfdc->nmeth;
	return sfdc->meth;
}

/* common code for vcsfio() and sfdcsfio() */
#if __STD_C
static int push(Sfio_t* f, Vcsfmeth_t* meth, ssize_t nmeth, unsigned int flags, Vcsfio_t** handle)
#else
static int push(f, meth, nmeth, flags, handle)
Sfio_t*		f;	/* stream to be conditioned	*/
Vcsfmeth_t*	meth;	/* list of methods to use	*/
ssize_t		nmeth;
unsigned int	flags;	/* VCD_[ENCODE|DECODE] ...	*/
Vcsfio_t**	handle; /* Vcsfio_t* return		*/
#endif
{
	int		i, k, status = -1;
	ssize_t		windsize;
	Sfdc_t		*sfdc = NIL(Sfdc_t*);

	if(!f || !(flags & (VC_ENCODE|VC_DECODE)) || nmeth >= VCSF_MAX )
		goto error;

	if(!(sfdc = (Sfdc_t*)calloc(1,sizeof(Sfdc_t))) )
		goto error;

	sfdc->sf = f; /* stream to do IO on */

#if _SFIO_H == 1
	sfdc->disc.readf   = vcsfdcread;
	sfdc->disc.writef  = vcsfdcwrite;
	if(!handle)
		sfdc->disc.seekf   = vcsfdcseek;
	sfdc->disc.exceptf = vcsfdcexcept;
#endif

	windsize = 0;
	if((sfdc->flags = flags) & VC_ENCODE)
	{	if(!meth || nmeth == 0 )
			goto error;

		/* copy the method list */
		for(k = 0, i = 0; i < nmeth; ++i)
		{	if(!meth[i].meth)
				goto error;
			if(meth[i].meth == VCSF_WINDOW)
			{	windsize = meth[i].size;
				sfdc->vcw = (Vcwindow_t*)meth[i].data;
			}
			else if(meth[i].meth == VCSF_STATES)
				sfdc->state = (char*)meth[i].data;
			else if(meth[i].meth == VCSF_CHAIN)
			{	nmeth = meth[i].size;
				meth = (Vcsfmeth_t*)meth[i].data;
				i = -1;
			}
			else
			{	sfdc->meth[k].meth = meth[i].meth;
				sfdc->meth[k].data = meth[i].data;
				sfdc->meth[k].size = meth[i].size;
				k += 1;
			}
		}
		if((sfdc->nmeth = k) == 0)
			goto error;

		/* we only support Vcdelta when RFC3284 header is requested */
		if((sfdc->flags&VCSF_VCDIFFHEADER) &&
		   (sfdc->nmeth != 1 || sfdc->meth[0].meth != Vcdelta) )
			goto error;

		/* buffer to accumulate data before encoding */
		if(windsize <= 0)
			windsize = VCSF_WINSIZE;
		if(!(sfdc->data = (Vcchar_t*)malloc(windsize)) )
			goto error;
		sfdc->dtsz = windsize;
		sfdc->next = sfdc->data;
		sfdc->endd = sfdc->data + windsize;

		/* buffer for the encoder to output results */
		sfdc->bssz = VCSFDTSZ(windsize);
		if(!(sfdc->base = (Vcchar_t*)malloc(sfdc->bssz)) )
			goto error;
	}
	else /* VC_DECODE */
	{
		for(k = 0; k < nmeth; ++k)
		{	if(meth[k].meth != VCSF_WINDOW)
				continue;
			sfdc->bssz = meth[k].size;
			sfdc->vcw  = (Vcwindow_t*)meth[k].data;
		}
		if(sfdc->bssz <= 0)
			sfdc->bssz = VCSF_BUFSIZE;
		else if(sfdc->bssz < VCSF_BUFMIN)
			sfdc->bssz = VCSF_BUFMIN;

		if(!(sfdc->base = (Vcchar_t*)malloc(sfdc->bssz)) )
			goto error;
		sfdc->code = sfdc->endb = sfdc->base;

		/* reconstruct handle to decode data */
		if((k = getheader(sfdc, f, 1)) <= 0 )
		{	status = k;
			goto error;
		}
	}

	/* the discipline was successfully created, now apply it to f */
	if(!sfdisc(f, &sfdc->disc) )
		goto error;

#if _SFIO_H == 1
	if (handle)
		*handle = (Vcsfio_t*)f;
	sfset(f, SF_IOCHECK, 1); /* so that sfsync() will call vcsfdcexcept() */
	return 1;
#else
	if (handle)
		*handle = (Vcsfio_t*)sfdc;
	return 1;
#endif

error:	
	if(sfdc)
	{	if((sfdc->flags&VC_ENCODE) && sfdc->data)
			free(sfdc->data);
		if(sfdc->base)
			free(sfdc->base);
		if(sfdc->encoding)
			free(sfdc->encoding);
		free(sfdc);
	}

	if (handle)
		*handle = NIL(Vcsfio_t*);
	return status;
}

#if __STD_C
Vcsfio_t* vcsfio(Sfio_t* f, Vcsfmeth_t* meth, ssize_t nmeth, unsigned int flags)
#else
Vcsfio_t* vcsfio(f, meth, nmeth, flags)
Sfio_t*		f;	/* stream to be conditioned	*/
Vcsfmeth_t*	meth;	/* list of methods to use	*/
ssize_t		nmeth;
unsigned int	flags;	/* VCD_[ENCODE|DECODE] ...	*/
#endif
{
	Vcsfio_t*	handle;

	push(f, meth, nmeth, flags, &handle);
	return handle;
}

#if __STD_C
int sfdcvcodex(Sfio_t* f, Vcsfmeth_t* meth, ssize_t nmeth, unsigned int flags)
#else
int sfdcvcodex(f, meth, nmeth, flags)
Sfio_t*		f;	/* stream to be conditioned	*/
Vcsfmeth_t*	meth;	/* list of methods to use	*/
ssize_t		nmeth;
unsigned int	flags;	/* VCD_[ENCODE|DECODE] ...	*/
#endif
{
	return push(f, meth, nmeth, flags, NIL(Vcsfio_t**));
}

#if _SFIO_H != 1

typedef struct _rsrv_s	Rsrv_t;
struct _rsrv_s
{	Rsrv_t*	next;
	Sfio_t*	f;	/* file stream for I/O	*/
	Void_t*	data;	/* reserved data	*/
	ssize_t	dtsz;	/* amount of data	*/
	ssize_t	size;	/* allocated buf size	*/
};

#if __STD_C
static Rsrv_t* vcsfrsrv(Sfio_t* f, ssize_t n)
#else
static Rsrv_t* vcsfrsrv(f, n)
Sfio_t*		f;	/* stream to to create reserve buffer	*/
ssize_t		n;	/* <0: remove, 0: find, >0: buffer size	*/
#endif
{
	Rsrv_t		*p, *r;
	static Rsrv_t	*Rsrv;	/* linked list	*/

	for(p = NIL(Rsrv_t*), r = Rsrv; r; p = r, r = r->next)
		if(r->f == f)
			break;

	if(!r) /* create a new reserve structure if requested */
	{	if(n <= 0)
			return NIL(Rsrv_t*);
		if(!(r = (Rsrv_t*)calloc(1,sizeof(Rsrv_t))) )
			return NIL(Rsrv_t*);
		r->f = f;
	}
	else
	{	if(p) /* remove from list */
			p->next = r->next;
		else	Rsrv = r->next;

		if(n < 0) /* remove all together */
		{	if(r->data && r->size > 0)
				free(r->data);
			free(r);	
			return NIL(Rsrv_t*);
		}
	}

	if(n > r->size) /* allocate buffer as necessary */
	{	if(r->data)
			free(r->data);
		r->size = r->dtsz = 0;
		if(!(r->data = malloc(n)) )
		{	free(r);
			return NIL(Rsrv_t*);
		}
		else	r->size = n;
	}

	r->next = Rsrv; Rsrv = r;
	return r;
}

#if __STD_C
Void_t* sfreserve(Sfio_t* f, ssize_t n, int type)
#else
Void_t* sfreserve(f, n, type)
Sfio_t*		f;
ssize_t		n;
int		type;
#endif
{
	Rsrv_t	*r;
	Sfoff_t	here = 0;

	n = n < 0 ? -n : n; 

	if(type == SF_LASTR)
		return (!(r = vcsfrsrv(f,0)) || r->dtsz <= 0 ) ? NIL(Void_t*) : r->data;

	if(!(r = vcsfrsrv(f, n)) ) /* find/create reserve structure */
		return NIL(Void_t*);

	if(type == SF_LOCKR)
		if((here = sfseek(f, (Sfoff_t)0, 1)) < 0 )
			return NIL(Void_t*);

	if((r->dtsz = sfread(f, r->data, n)) > 0 )
		if(type == SF_LOCKR)
			sfseek(f, here, 0);

	return r->dtsz >= n ? r->data : NIL(Void_t*);
}

#if __STD_C
ssize_t sfvalue(Sfio_t* f)
#else
ssize_t sfvalue(f)
Sfio_t*		f;
#endif
{
	Rsrv_t	*r;
	return (r = vcsfrsrv(f, 0)) ? r->dtsz : (Sfoff_t)(-1);
}

#if __STD_C
char* sfgetr(Sfio_t* f, int nl, int type)
#else
char* sfgetr(f, nl, type)
Sfio_t* 	f;
int		nl;
int		type;
#endif
{
	Rsrv_t	*r;

	if(!(r = vcsfrsrv(f, 1024)) )
		return NIL(char*);
	if(!fgets(r->data, 1024, f) )
		return NIL(char*);
	if(type > 0)
	{	nl = strlen(r->data);
		((char*)r->data)[nl-1] = 0;
	}
	return (char*)r->data;
}

#if __STD_C
int sfclose(Sfio_t* f)
#else
int sfclose(f)
Sfio_t*		f;
#endif
{	vcsfrsrv(f, -1);
	fclose(f);
	return 0;
}

#if __STD_C
Sfoff_t sfsize(Sfio_t* f)
#else
Sfoff_t sfsize(f)
Sfio_t*		f;
#endif
{
	Sfoff_t	pos, siz;
	if(fseek(f, (long)0, 1) < 0 )
		return -1;
	pos = (Sfoff_t)ftell(f);
	fseek(f, (long)0, 2);
	siz = (Sfoff_t)ftell(f);
	fseek(f, (long)pos, 0);
	return siz;
}

#if __STD_C
ssize_t vcsfread(Vcsfio_t* vcf, Void_t* buf, size_t n)
#else
ssize_t vcsfread(vcf, buf, n)
Vcsfio_t*	vcf;
Void_t*		buf;
size_t		n;
#endif
{
	return vcsfdcread(((Sfdc_t*)vcf)->sf, buf, n, (Sfdisc_t*)vcf);
}

#if __STD_C
ssize_t vcsfwrite(Vcsfio_t* vcf, const Void_t* buf, size_t n)
#else
ssize_t vcsfwrite(vcf, buf, n)
Vcsfio_t*	vcf;
Void_t*		buf;
size_t		n;
#endif
{
	return vcsfdcwrite(((Sfdc_t*)vcf)->sf, buf, n, (Sfdisc_t*)vcf);
}

#if __STD_C
int vcsfsync(Vcsfio_t* vcf)
#else
int vcsfsync(vcf)
Vcsfio_t*	vcf;
#endif
{
	return vcsfdcexcept(((Sfdc_t*)vcf)->sf, SF_SYNC, 0, (Sfdisc_t*)vcf);
}

#if __STD_C
int vcsfclose(Vcsfio_t* vcf)
#else
int vcsfclose(vcf)
Vcsfio_t*	vcf;
#endif
{
	Sfio_t	*sf = ((Sfdc_t*)vcf)->sf;

	if(vcsfdcexcept(sf, SF_CLOSING, 0, (Sfdisc_t*)vcf) < 0)
		return -1;
	if(sfclose(sf) != 0)
		return -1;
	return 0;
}

#endif /*!SFIO_H*/
