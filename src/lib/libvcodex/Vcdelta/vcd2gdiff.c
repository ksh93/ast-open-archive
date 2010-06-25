/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2010 AT&T Intellectual Property          *
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
#include	"vcdhdr.h"

/*	Transform Vcdiff format to Gdiff format.
**	This is currently used for testing only.
**
**	Written by Kiem-Phong Vo (kpv@research.att.com)
*/

#if __STD_C
static ssize_t vcd2gdiff(Vcodex_t* vc, const Void_t* del, size_t ndel, Void_t** out)
#else
static ssize_t vcd2gdiff(vc, del, ndel, out)
Vcodex_t*	vc;
Void_t*		del;
size_t		ndel;
Void_t**	out;
#endif
{
	ssize_t		d, i, a, n, nsrc, here;
	Vcchar_t	*output, *rd, *ri, *ra;
	Vcdcode_t*	code;
	Vcdcache_t*	ka;
	Vcio_t		data, inst, addr, gdiff;
	int		ctrl;
	Vcdiff_t*	vcd = vcgetmtdata(vc, Vcdiff_t*);

	ka = vcd->cache;

	/* read size of data buffers */
	vcioinit(&data, del, ndel);
	n = (ssize_t)vciogetu(&data);	/* buffer size for target data		*/
	ctrl = (int)vciogetc(&data);	/* to see if datasets were compressed	*/
	d = (ssize_t)vciogetu(&data); 	/* size of unmatched data		*/
	i = (ssize_t)vciogetu(&data); 	/* size of instruction set		*/
	a = (ssize_t)vciogetu(&data); 	/* size of COPY addresses		*/

	/* buffer for reconstructed data */
	n = 2*n + 64;
	if(!(output = vcbuffer(vc, NIL(Vcchar_t*), n, 0)) )
		return -1;
	vcioinit(&gdiff, output, n);

	/* make sure we have enough data for decoding */
	if((d+i+a) != vciomore(&data) )
		return -1;

	/* data, instructions and COPY addresses */
	rd = vcionext(&data);
	ri = rd+d;
	ra = ri+i;

	/* recompute the data, instruction and address streams if encoded */
	if(vc->coder)
	{	if(ctrl&VCD_DATACOMPRESS)
		{	if((d = vcapply(vc->coder, rd, d, &rd)) < 0 )
				return -1;
		}
		if(ctrl&VCD_INSTCOMPRESS)
		{	if((i = vcapply(vc->coder, ri, i, &ri)) < 0 )
				return -1;
		}
		if(ctrl&VCD_ADDRCOMPRESS)
		{	if((a = vcapply(vc->coder, ra, a, &ra)) < 0 )
				return -1;
		}
	}

	vcioinit(&data, rd, d);
	vcioinit(&inst, ri, i);
	vcioinit(&addr, ra, a);

	/* clear address caches and set code table */
	vcdkaclear(ka);
	code = vcd->table->code; /**/ DEBUG_SET(Tab, vcd->table);

	for(here = nsrc = vc->disc->size; ; )
	{	Vcdcode_t*	cd;
		Vcdinst_t*	in;
		unsigned int	size;

		if(vciomore(&gdiff) <= 0)
			return -1;

		/* get the pair of instructions */
		if(vciomore(&inst) <= 0)
			break;
		cd = code + vciogetc(&inst);

		for(i = 0; i < 2; ++i)
		{	in = i == 0 ? &cd->inst1 : &cd->inst2;
			if(in->type == VCD_NOOP)
				continue;

			if((size = in->size) == 0)
			{	if(vciomore(&inst) <= 0)
					return -1;
				size = vciogetu(&inst);
			}

#define MAXUCHAR	(unsigned int)((1 << 8) - 1)
#define MAXUSHORT	(unsigned int)((1 << 16) - 1)
#define MAXUINT		(unsigned int)(~((unsigned int)0))

			if(in->type == VCD_ADD)
			{	if(vciomore(&data) < size)
					return -1;

				if(vciomore(&gdiff) < 8)
					return -1;

				if(size <= 246)
					vcioputc(&gdiff,size);
				else if(size <= MAXUSHORT)
				{	vcioputc(&gdiff,247);
					vcioputm(&gdiff,size,MAXUSHORT);
				}
				else
				{	vcioputc(&gdiff,248);
					vcioputm(&gdiff,size,MAXUINT);
				}

				if(vciomore(&gdiff) < size)
					return -1;
				vciomove(&data, &gdiff, size);
				here += size;
			}
			else if(in->type == VCD_COPY)
			{	if(vciomore(&addr) <= 0)
					return -1;

				d = vcdkagetaddr(ka,&addr,here,in->mode);
				here += size;

				if(d >= nsrc || (d+size) > nsrc || vciomore(&gdiff) < 16)
					return -1;

				if(d <= MAXUSHORT )
				{	if(size <= MAXUCHAR)
					{	vcioputc(&gdiff,249);
						vcioputm(&gdiff,d,MAXUSHORT);
						vcioputm(&gdiff,size,MAXUCHAR);
					}
					else if(size <= MAXUSHORT)
					{	vcioputc(&gdiff,250);
						vcioputm(&gdiff,d,MAXUSHORT);
						vcioputm(&gdiff,size,MAXUSHORT);
					}
					else
					{	vcioputc(&gdiff,251);
						vcioputm(&gdiff,d,MAXUSHORT);
						vcioputm(&gdiff,size,MAXUINT);
					}
				}
				else
				{	if(size <= MAXUCHAR)
					{	vcioputc(&gdiff,252);
						vcioputm(&gdiff,d,MAXUINT);
						vcioputm(&gdiff,size,MAXUCHAR);
					}
					else if(size <= MAXUSHORT)
					{	vcioputc(&gdiff,253);
						vcioputm(&gdiff,d,MAXUINT);
						vcioputm(&gdiff,size,MAXUSHORT);
					}
					else
					{	vcioputc(&gdiff,254);
						vcioputm(&gdiff,d,MAXUINT);
						vcioputm(&gdiff,size,MAXUINT);
					}
				}
			}
			else	return -1;
		}
	}

	if(vciomore(&data) != 0 || vciomore(&inst) != 0 || vciomore(&addr) != 0 )
		return -1;

	n = vciosize(&gdiff);
	if(out)
		*out = output;

	return n;
}

#if __STD_C
static int vcdgevent(Vcodex_t* vc, int type, Void_t* init)
#else
static int vcdgevent(vc, type, init)
Vcodex_t*	vc;
int		type;
Void_t*		init;
#endif
{
	Vcdiff_t*	vcd;
	int		rv = 0;

	_vcdtblinit(); /* construct default code tables */

	if(type == VC_OPENING)
	{	if(!(vcd = (Vcdiff_t*)malloc(sizeof(Vcdiff_t))) )
			return -1;
		VCDINIT(vcd);
		if(init)
			vcd->table = (Vcdtable_t*)init;
		if(!vcd->table)
			vcd->table = _Vcdtbl;
		if(!(vcd->cache = vcdkaopen(vcd->table->s_near, vcd->table->s_same)) )
		{	rv = -1;
			goto do_closing;
		}

		vcsetmtdata(vc, vcd);
		return 0;
	}
	else if(type == VC_CLOSING)
	{	rv = 0;
	do_closing:
		if((vcd = vcgetmtdata(vc, Vcdiff_t*)) )
		{	if(vcd->cache)
				vcdkaclose(vcd->cache);
			free(vcd);
		}
		vcsetmtdata(vc, NIL(Vcdiff_t*));
		return rv;
	}
	else	return 0;
}

Vcmethod_t _Vc2gdiff =
{	vcd2gdiff,
	0,
	vcdgevent,
	"gdiff", "Translating from Vcdiff to gdiff.",
	"[-version?gdiff (AT&T Research) 2003-01-01]" USAGE_LICENSE,
	0,
	1024*1024,
	0
};

VCLIB(Vc2gdiff)
