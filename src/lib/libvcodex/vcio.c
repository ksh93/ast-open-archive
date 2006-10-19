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

/*	Functions to read/write integers in various portable formats.
**	Some of these are stolen from the Sfio library and modified 
**	to deal only with memory buffers.
**
**	Written by Kiem-Phong Vo (kpv@research.att.com)
*/

/* base-128 unsigned coding */
#if __STD_C
ssize_t _vcioputu(Vcio_t* io, Vcint_t v)
#else
ssize_t _vcioputu(io, v)
Vcio_t*	io;
Vcint_t	v;
#endif
{
	Vcchar_t	*ptr, *code;
	ssize_t		n;
	Vcchar_t	data[2*sizeof(Vcint_t)];

	code = &data[sizeof(data)-1];
	*code = v&127;
	while((v >>= 7) > 0)
		*--code = (v&127) | 128;
	n = &data[sizeof(data)] - code;

	ptr = io->next;
	switch(n)
	{ default:	memcpy(ptr, code, n); ptr += n; break;
	  case 7 :	*ptr++ = *code++;
	  case 6 :	*ptr++ = *code++;
	  case 5 :	*ptr++ = *code++;
	  case 4 :	*ptr++ = *code++;
	  case 3 :	*ptr++ = *code++;
	  case 2 :	*ptr++ = *code++;
	  case 1 :	*ptr++ = *code++;
	}
	io->next = ptr;

	return n;
}

#if __STD_C
Vcint_t _vciogetu(Vcio_t* io)
#else
Vcint_t _vciogetu(io)
Vcio_t*	io;
#endif
{
	int		n;
	Vcint_t		v;
	Vcchar_t	*ptr;

	ptr = io->next;
	v = (n = *ptr++)&127;
	while(n & 128)
		v = (v << 7) | ((n = *ptr++)&127);
	io->next = ptr;

	return v;
}

/* base 256 coding */
#if __STD_C
ssize_t _vcioputm(Vcio_t* io, Vcint_t v, Vcint_t max)
#else
ssize_t _vcioputm(io, v, max)
Vcio_t*	io;
Vcint_t	v;
Vcint_t	max;
#endif
{
	ssize_t		n;
	Vcchar_t	data[sizeof(Vcint_t)];
	Vcchar_t	*code = &data[sizeof(data) - 1];
	Vcchar_t	*ptr;

	*code = v&255;
	while((max >>= 8) > 0)
		*--code = (v >>= 8)&255;
	n = &data[sizeof(data)] - code;

	if(io)
	{	ptr = io->next;
		switch(n)
		{ default:	memcpy(ptr, code, n); ptr += n; break;
		  case 7 :	*ptr++ = *code++;
		  case 6 :	*ptr++ = *code++;
		  case 5 :	*ptr++ = *code++;
		  case 4 :	*ptr++ = *code++;
		  case 3 :	*ptr++ = *code++;
		  case 2 :	*ptr++ = *code++;
		  case 1 :	*ptr++ = *code++;
		}
		io->next = ptr;
	}

	return n;
}

#if __STD_C
Vcint_t _vciogetm(Vcio_t* io, Vcint_t max)
#else
Vcint_t _vciogetm(io, max)
Vcio_t*	io;
Vcint_t	max;
#endif
{
	Vcint_t		v;
	Vcchar_t	*ptr;

	ptr = io->next;
	v = *ptr++;
	while((max >>= 8) > 0)
		v = (v << 8) | *ptr++;
	io->next = ptr;

	return v;
}

/* A value v can be coded using two letters A and Z by treating v-1
** as the rank of a string of A's and Z's listed in lexicographic order.
** This coding is useful for coding runs of a letter, say A, using just
** another companion letter. Below are the codes of the first fourteen
** (2+4+8) integers using A and Z:
**
**	0	A		2	AA		6	AAA
**	1	Z		3	ZA		7	ZAA
**				4	AZ		8	AZA
**				5	ZZ		9	ZZA
**							10	AAZ
**							11	ZAZ
**							12	AZZ
**							13	ZZZ
*/

#if __STD_C
ssize_t _vcioput2(Vcio_t* io, Vcint_t v, Vcchar_t a, Vcchar_t z)
#else
ssize_t _vcioput2(io, v, a, z)
Vcio_t*		io;
Vcint_t		v;	/* value to encode	*/
Vcchar_t	a;	/* 1st coding letter	*/
Vcchar_t	z;	/* 2nd coding letter	*/
#endif
{
	Vcchar_t	*ptr = io->next;
	ssize_t		n;

	for(;;)
	{	*ptr++ = (v&1) == 0 ? a : z;
		if((v -= 2) < 0)
			break;
		else	v >>= 1;
	}
	n = ptr - io->next;
	io->next = ptr;

	return n;
}

#if __STD_C
Vcint_t _vcioget2(Vcio_t* io, Vcchar_t a, Vcchar_t z)
#else
Vcint_t _vcioget2(io, a, z)
Vcio_t*		io;
Vcchar_t	a;	/* 1st coding letter	*/
Vcchar_t	z;	/* 2nd coding letter	*/
#endif
{
	int		d;
	Vcint_t		v;
	Vcchar_t	*ptr, *endp;

	v = -1; d = 1;
	for(ptr = io->next, endp = io->endb; ptr < endp; ++ptr)
	{	if(*ptr == a)
		{	v += d;
			d <<= 1;
		}
		else if(*ptr == z)
		{	d <<= 1;
			v += d;
		}
		else	break;
	}

	io->next = ptr;

	return v;
}


/* Elias Gamma code for POSITIVE integers
** Gamma code	Value	Base-2 bits
** 1		1	1
** 00 1		2	10
** 01 1		3	11
** 00 00 1	4	100
** 01 00 1	5	101
** ...
*/
static Vcbit_t Gfour[16] = /* table of gamma codes mapping 4 bits at a time */
{	/* 0000 -> 00 00 00 00	*/	0x00000000,
	/* 0001 -> 01 00 00 00	*/	0x40000000,
	/* 0010 -> 00 01 00 00	*/	0x10000000,
	/* 0011 -> 01 01 00 00	*/	0x50000000,
	/* 0100 -> 00 00 01 00	*/	0x04000000,
	/* 0101 -> 01 00 01 00	*/	0x44000000,
	/* 0110 -> 00 01 01 00	*/	0x14000000,
	/* 0111 -> 01 01 01 00	*/	0x54000000,
	/* 1000 -> 00 00 00 01	*/	0x01000000,
	/* 1001 -> 01 00 00 01	*/	0x41000000,
	/* 1010 -> 00 01 00 01	*/	0x11000000,
	/* 1011 -> 01 01 00 01	*/	0x51000000,
	/* 1100 -> 00 00 01 01	*/	0x05000000,
	/* 1101 -> 01 00 01 01	*/	0x45000000,
	/* 1110 -> 00 01 01 01	*/	0x15000000,
	/* 1111 -> 01 01 01 01	*/	0x55000000
};
static Vcbit_t Glast[16] = /* table of gamma codes for last <=4 bits */
{	/* 0    -> 0		*/	0x00000000,
	/* 1    -> 1    	*/	0x80000000,
	/* 10   -> 00 1 	*/	0x20000000,
	/* 11   -> 01 1  	*/	0x60000000,
	/* 100  -> 00 00 1	*/	0x08000000,
	/* 101  -> 01 00 1	*/	0x48000000,
	/* 110  -> 00 01 1	*/	0x18000000,
	/* 111  -> 01 01 1	*/	0x58000000,
	/* 1000 -> 00 00 00 1	*/	0x02000000,
	/* 1001 -> 01 00 00 1	*/	0x42000000,
	/* 1010 -> 00 01 00 1	*/	0x12000000,
	/* 1011 -> 01 01 00 1	*/	0x52000000,
	/* 1100 -> 00 00 01 1	*/	0x06000000,
	/* 1101 -> 01 00 01 1	*/	0x46000000,
	/* 1110 -> 00 01 01 1	*/	0x16000000,
	/* 1111 -> 01 01 01 1	*/	0x56000000
};

#if __STD_C
ssize_t _vcioputg(Vcio_t* io, Vcint_t v)
#else
ssize_t _vcioputg(io, v)
Vcio_t*		io;
Vcint_t		v;
#endif
{
	ssize_t	n;

	for(n = 0; v > 0xf; v >>= 4, n += 8)
		vcioaddb(io, io->bits, io->nbits, Gfour[v&0xf], 8);

	if(v <= 0x3)
	{	if(v <= 0x1)
		{	vcioaddb(io, io->bits, io->nbits, Glast[v], 1);
			return n+1;
		}
		else
		{	vcioaddb(io, io->bits, io->nbits, Glast[v], 3);
			return n+3;
		}
	}
	else
	{	if(v <= 0x7)
		{	vcioaddb(io, io->bits, io->nbits, Glast[v], 5);
			return n+5;
		}
		else
		{	vcioaddb(io, io->bits, io->nbits, Glast[v], 7);
			return n+7;
		}
	}
}

#if __STD_C
Vcint_t _vciogetg(Vcio_t* io)
#else
Vcint_t _vciogetg(io)
Vcio_t*		io;
#endif
{
	Vcint_t		v;
	int		k, b, g, s;
	static int	Ifour[256], Ilast[256];

	/* Bit strings of the below forms are terminal and map to # of sig bits.
	**	1xxx xxxx 	|-1| bits.
	**	xx1x xxxx	|-3| bits.
	**	xxxx 1xxx	|-5| bits.
	**	xxxx xx1x	|-7| bits.
	** Otherwise, they would be of the below form and inversely map Gfour[].
	**	0x0x 0x0x
	*/
	if(Ifour[255] == 0) /* initialize the inversion arrays */
	{	for(k = 0; k < 256; ++k)
		{	for(b = 7; b >= 1; b -= 2) /* find the high odd bit */
				if(k & (1<<b) )
					break;
			if(b >= 1)
			{	Ifour[k] = b-8; /* set # of bits needed */

				if((k & ~((1<<b)-1)) == k ) /* set value in Ilast[] */
				{	for(g = 0; g < 16; ++g)
						if((Glast[g]>>(VC_BITSIZE-8)) == k)
							break;
					for(s = (1<<b)-1; s >= 0; --s)
						Ilast[k|s] = g;
				}
			}
		}
		for(k = 0; k < 16; ++k) /* inverse of Gfour[] */
			Ifour[Gfour[k] >> (VC_BITSIZE-8)] = k;
	}

	for(v = 0, s = 0;; s += 4)
	{	vciofilb(io, io->bits, io->nbits, 8);
		if(io->nbits == 0)
			return -1;

		if((k = io->nbits) >= 8)
			b = (int)(io->bits >> (VC_BITSIZE-8));
		else	b = (int)((io->bits >> (VC_BITSIZE-k)) << (8-k));

		if((g = Ifour[b]) >= 0)
		{	if(io->nbits < 8)
				return -1;
			k = 8;
		}
		else
		{	k = -g;
			g = Ilast[b];
		}

		v |= ((Vcint_t)g) << s;
		vciodelb(io, io->bits, io->nbits, k);
		if(k < 8)
			break;
	}

	return v;
}


/* use binary search to get # of significant bits in a given integer */
static ssize_t _Nbits4[16] =
{	1,	/*  0: 0000	*/
	1,	/*  1: 0001	*/
	2,	/*  2: 0010	*/
	2,	/*  3: 0011	*/
	3,	/*  4: 0100	*/
	3,	/*  5: 0101	*/
	3,	/*  6: 0110	*/
	3,	/*  7: 0111	*/
	4,	/*  8: 1000	*/
	4,	/*  9: 1001	*/
	4,	/* 10: 1010	*/
	4,	/* 11: 1011	*/
	4,	/* 12: 1100	*/
	4,	/* 13: 1101	*/
	4,	/* 14: 1110	*/
	4	/* 15: 1111	*/
};
#define NBITS4(v)	_Nbits4[v]
#define NBITS8(v)	((v) > 0xf    ? (((v) >>=  4), (NBITS4(v)+4))   : NBITS4(v) )
#define NBITS16(v)	((v) > 0xff   ? (((v) >>=  8), (NBITS8(v)+8))   : NBITS8(v) )
#define NBITS(v)	((v) > 0xffff ? (((v) >>= 16), (NBITS16(v)+16)) : NBITS16(v) )

/* Coding a list of non-negative integers using a variable-length bit coding */
#if __STD_C
ssize_t vcioputl(Vcio_t* io, Vcint_t* list, ssize_t nlist)
#else
ssize_t vcioputl(io, list, nlist)
Vcio_t*		io;
Vcint_t*	list;
ssize_t		nlist;
#endif
{
	reg Vcbit_t	b, v, e;
	reg ssize_t	n, s, i;
	ssize_t		freq[VC_INTSIZE], size[VC_INTSIZE];
	Vcbit_t		bits[VC_INTSIZE];
	Vcchar_t	*begs;
	int		run;

	for(i = 0; i < VC_INTSIZE; ++i)
		freq[i] = 0;
	for(i = 0; i < nlist; ++i)
	{	v = list[i];
		freq[NBITS(v)-1] += 1;
	}

	if((s = vchsize(VC_INTSIZE, freq, size, &run)) < 0 ||
	   (s > 0 && vchbits(VC_INTSIZE, size, bits) < 0) )
		return -1;

	begs = vcionext(io);
	if(s == 0) /* all integers have the same size */
	{	s = run+1;
		vcioputc(io, s);

		vciosetb(io, b, n, VC_ENCODE);
		for(i = 0; i < nlist; ++i)
		{	v = ((Vcint_t)list[i]) << (VC_INTSIZE-s);
			vcioaddb(io, b, n, v, s);
		}
		vcioendb(io, b, n, VC_ENCODE);
	}
	else
	{	vcioputc(io, s|(1<<7)); /* the max size of any integer */	
		if((s = vchputcode(VC_INTSIZE, size, s, vcionext(io), vciomore(io))) < 0)
			return -1;
		else	vcioskip(io, s);

		vciosetb(io, b, n, VC_ENCODE);
		for(i = 0; i < nlist; ++i) 
		{	v = (Vcint_t)list[i]; s = NBITS(v)-1;
			vcioaddb(io, b, n, bits[s], size[s]);

			for(v = (Vcint_t)list[i], s += 1;; )
			{	if(s > 8)
				{	e = (v&0xff) << (VC_INTSIZE - 8);
					vcioaddb(io, b, n, e, 8);
					v >>= 8; s -= 8;
				}
				else
				{	e = v << (VC_INTSIZE - s);
					vcioaddb(io, b, n, e, s);
					break;
				}
			}
		}
		vcioendb(io, b, n, VC_ENCODE);
	}

	return vcionext(io)-begs;
}

#if __STD_C
ssize_t vciogetl(Vcio_t* io, Vcint_t* list, ssize_t nlist)
#else
ssize_t vciogetl(io, list, nlist)
Vcio_t*		io;
Vcint_t*	list;
ssize_t		nlist;
#endif
{
	reg Vcbit_t	b;
	reg ssize_t	n, s, p, ntop, nl, d;
	ssize_t		cdsz[VC_INTSIZE];
	Vcint_t		v;
	Vcbit_t		bits[VC_INTSIZE];
	Vchtrie_t	*trie;
	short		*node, *size;

	if((s = vciogetc(io)) < 0)
		return -1;

	vciosetb(io, b, n, VC_DECODE); /* start bit stream */

	if(!(s & (1<<7)) ) /* all integers have the same size */
	{	for(nl = 0; nl < nlist; ++nl)
		{	vciofilb(io, b, n, s);
			list[nl] = (Vcint_t)(b >> (VC_BITSIZE-s));
			vciodelb(io, b, n, s);
		}
	}
	else
	{	s &= ~(1<<7);
		if((s = vchgetcode(VC_INTSIZE, cdsz, s, vcionext(io), vciomore(io))) < 0 )
			return -1;
		else	vcioskip(io, s);
		if(vchbits(VC_INTSIZE, cdsz, bits) < 0)
			return -1;
		if(!(trie = vchbldtrie(VC_INTSIZE, cdsz, bits)) )
			return -1;
		node = trie->node;
		size = trie->size;
		ntop = trie->ntop;
		vciosetb(io, b, n, VC_DECODE);
		for(s = ntop, p = 0, nl = 0;; )
		{	vciofilb(io, b, n, s);

			p += (b >> (VC_BITSIZE-s)); /* slot to look into */
			if(size[p] > 0) /* length is found */
			{	s = (int)node[p] + 1; /* get the actual length */
				vciodelb(io, b, n, size[p]); /* consume bits */

				for(v = 0, d = 0;; )
				{	if(s > 8)
					{	vciofilb(io, b, n, 8);
						v |= (b >> (VC_BITSIZE-8)) << d;
						vciodelb(io, b, n, 8);
						d += 8; s -= 8;
					}
					else
					{	vciofilb(io, b, n, s);
						v |= (b >> (VC_BITSIZE-s)) << d;
						vciodelb(io, b, n, s);
						break;
					}
				}

				list[nl] = v;
				if((nl += 1) >= nlist)
					break;

				s = ntop; p = 0; /* restart at trie top for next integer */
			}
			else if(size[p] == 0) /* corrupted data */
				return -1;
			else 
			{	vciodelb(&io, b, n, s); /* consume bits */
				s = -size[p]; p = node[p]; /* trie recursion */
			}
		}
	}

	vcioendb(io, b, n, VC_DECODE); /* finish bit stream */

	return nl;
}

/* Encoding a list of integers by a few transforms to reduce space.
** 0. Transform the list by successive differences.
** 1. Keep a sign indicator and if the current element is of this sign,
**    code its absolute value; otherwise, code it as the negative value
**    with the same magnitude.
** 2. Then code all integers using only non-negative integers via
**    a proportional method. Suppose that we want n negatives for each p
**    positives, the codings for a negative x and a positive y would be:
**	x -> ((-x-1)/n)*(n+p) + (-x-1)%n + 1 + p
**	y -> ( (y-1)/p)*(n+p) + ( y-1)%p + 1
*/
#if __STD_C
ssize_t vcintlist(Vcint_t* list, ssize_t nlist, ssize_t* pos, ssize_t* neg, int type)
#else
ssize_t vcintlist(list, nlist, pos, neg, type)
Vcint_t*	list;
ssize_t		nlist;
ssize_t*	pos;
ssize_t*	neg;
int		type;
#endif
{
	ssize_t		k, p, n, s, g;
	Vcint_t		v;

	if(type != 0) /* encoding */
	{	/* transform to differences */
		for(k = nlist-1; k > 0; --k)
			list[k] -= list[k-1];

		/* transform runs to positives */
		for(type = 1, k = 0; k < nlist; ++k)
		{	if((v = list[k]) < 0)
			{	if(type > 0)
					type = -1;
				else	list[k] = -v;
			}
			else if(v > 0)
			{	if(type < 0)
				{	type = 1;	
					list[k] = -v;
				}
			}
		}

		/* count positives/negatives to decide proportion */
		for(n = p = 0, k = 0; k < nlist; ++k)
		{	if((v = list[k]) > 0)
				p += 1;
			else if(v < 0)
				n += 1;
		}

		if(n == 0) /* a sequence of non-negatives */
			p = 1;
		else if(p == 0) /* all negatives */
		{	for(k = 0; k < nlist; ++k)
				list[k] = -list[k];
			n = 1;
		}
		else
		{	/* reduce proportions */
			while(p >= 128 && n >= 128)
				{ p /= 2; n /= 2; }
			for(k = 127; k > 1; --k)
				if((p%k) == 0 && (n%k) == 0)
					{ p /= k; n /= k; }

			/* now do the coding */
			for(s = n+p, k = 0; k < nlist; ++k)
			{	if((v = list[k]) == 0 )
					continue;
				else if(v > 0)
					list[k] = (( v-1)/p)*s + ( v-1)%p + 1;
				else	list[k] = ((-v-1)/n)*s + (-v-1)%n + 1 + p;
			}
		}

		*pos = p; *neg = n;
	}
	else
	{	p = *pos; n = *neg;

		if(p == 0 && n > 0) /* all negatives */
		{	for(k = 0; k < nlist; ++k)
				list[k] = -list[k];
		}
		else if(p > 0 && n > 0) /* nontrivial coding */
		{	for(s = n+p, k = 0; k < nlist; ++k)
			{	if((v = list[k]) == 0)
					continue;
				if((g = (v-1)%s) < p)
					list[k] =  ( ((v-1)/s)*p + g + 1 );
				else	list[k] = -( ((v-1)/s)*n + g + 1 - p );
			}
		}

		/* undo the sign switching */
		for(type = 1, k = 0; k < nlist; ++k)
		{	v = list[k];
			if(type < 0)
				list[k] = -v;
			if(v < 0)
				type = -type;
		}

		/* undo the difference transformation */
		for(k = 1; k < nlist; ++k)
			list[k] += list[k-1];
	}

	return nlist;
}
