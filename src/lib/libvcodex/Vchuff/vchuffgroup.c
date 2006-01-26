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
#include	"vchhdr.h"

/*	Group segments of data for more effective Huffman coding.
**
**	Written by Binh Dao Vo and Kiem-Phong Vo
*/

#define GRPMIN(sz)	(3)
#define GRPMAX(sz)	(9)
#define GRPPTSZ(sz)	(80)

#define GRP_NTBL	(GRPMAX(1024*1024)+1)	/* maximum number of tables	*/
#define GRP_ITER	3			/* default number of iterations	*/

typedef struct _table_s
{	ssize_t		size[VCH_SIZE]; /* code length table	*/
	int		maxs;	   /* max code length		*/
	int		runb;	   /* the run object if any	*/
	ssize_t		nblks;	   /* # of associated blocks	*/
	ssize_t		cost;	   /* cost of encoding		*/
} Table_t;

typedef struct _group_s
{	Vcodex_t*	huf;	/* Huffman coder/decoder	*/
	Vcodex_t*	mtf;	/* MTF coder/decoder		*/

	ssize_t		ptsz;	/* actual part size		*/
	ssize_t		npts;	/* number of parts		*/
	Vcchar_t*	part;	/* table index for each part	*/
	Vcchar_t*	work;	/* working space for indices	*/
	ssize_t*	ppos;	/* position of part[] in objs[]	*/
	ssize_t*	sort;	/* for sorting part positions	*/

	Vchobj_t*	obj;	/* distinct objs of each part	*/
	Vcchar_t*	ofr;	/* and their frequencies	*/

	ssize_t		cmpsz;	/* current best compressed size	*/
	ssize_t		ntbl;	/* number of coding tables	*/
	Table_t		tbl[GRP_NTBL]; /* best coding tables	*/
} Group_t;

/* Construct a list of distinct objects and frequencies from data[]. This list
** is used for fast computation of total frequencies, encoding lengths, etc.
** It is good for data transformed by a Burrows-Wheeler transform since the
** distribution is skewed toward a few small values.
*/
#if __STD_C
static int grpinit(Group_t* grp, Vchobj_t* data, size_t dtsz, ssize_t ptsz)
#else
static int grpinit(grp, data, dtsz, ptsz)
Group_t*	grp;
Vchobj_t*	data;
size_t		dtsz;
ssize_t		ptsz;
#endif
{
	ssize_t		freq[VCH_SIZE];
	ssize_t		i, k, max, d, p, npts;

 	if(ptsz >= (ssize_t)dtsz )
		ptsz = (ssize_t)dtsz;
	grp->ptsz = ptsz;
	grp->npts = npts = (dtsz+ptsz-1)/ptsz; /* guaranteed >= 1 */
	grp->cmpsz = (dtsz < VCH_SIZE ? VCH_SIZE : dtsz)*VC_BITSIZE; /* starting cost */
	grp->ntbl = 0;

	if(grp->part)
		free(grp->part);
	if(!(grp->part = (Vcchar_t*)calloc(2*npts, sizeof(Vcchar_t))) )
		return -1;
	grp->work = grp->part + npts;

	if(grp->ppos)
		free(grp->ppos);
	if(!(grp->ppos = (ssize_t*)calloc((2*npts+1), sizeof(ssize_t))) )
		return -1;
	grp->sort = grp->ppos + npts+1;

	if(grp->obj)
		free(grp->obj);
	if(!(grp->obj = (Vchobj_t*)calloc(dtsz, sizeof(Vchobj_t))) )
		return -1;

	if(grp->ofr)
		free(grp->ofr);
	if(!(grp->ofr = (Vcchar_t*)calloc(dtsz, sizeof(Vcchar_t))) )
		return -1;

	/* ptsz is such that a object frequency should fit in a byte */
	for(d = 0, p = 0, i = 0; i < npts; i += 1, d += ptsz)
	{	grp->ppos[i] = p;

		CLRTABLE(freq,VCH_SIZE);
		ADDFREQ(freq, Vchobj_t*, data+d, i == npts-1 ? dtsz-d : ptsz);
		for(max = -1, k = 0; k < VCH_SIZE; ++k)
		{	if(freq[k] != 0)
			{	grp->obj[p] = (Vchobj_t)k;
				grp->ofr[p] = (Vcchar_t)freq[k];
				if(max < 0 || (freq[k] > grp->ofr[max]) )
					max = p;
				p += 1;
			}
		}
		if(max > (k = grp->ppos[i]) ) /* swap heaviest object to front */
		{	int	t;
			t = grp->obj[k]; grp->obj[k] = grp->obj[max]; grp->obj[max] = t;
			t = grp->ofr[k]; grp->ofr[k] = grp->ofr[max]; grp->ofr[max] = t;
		}
	}
	grp->ppos[npts] = p;

	return 0;
}

/* sorting parts by heaviest elements */
#if __STD_C
static int partcmp(Void_t* one, Void_t* two, Void_t* disc)
#else
static int partcmp(one, two, disc)
Void_t*	one;
Void_t*	two;
Void_t*	disc;
#endif
{
	int		p1, p2, d;
	Group_t		*grp = (Group_t*)disc;

	p1 = (int)grp->ppos[*((ssize_t*)one)];
	p2 = (int)grp->ppos[*((ssize_t*)two)];
	if((d = (int)grp->obj[p1] - (int)grp->obj[p2]) != 0 )
		return d;
	if((d = (int)grp->ofr[p1] - (int)grp->ofr[p2]) != 0 )
		return d;
	else	return p1 - p2;
}

/* compute an optimal clustering with ntbl clusters */
#if __STD_C
static void grppart(Group_t* grp, ssize_t ntbl, int niter)
#else
static void grppart(grp, ntbl, niter)
Group_t*	grp;
ssize_t		ntbl;	/* # of tables aiming for	*/
int		niter;	/* # of iterations to run	*/
#endif
{
	ssize_t		i, k, p, q, z, n, t, iter;
	Vcchar_t	*dt, tmp[VCH_SIZE];
	Table_t		tbl[GRP_NTBL];
	ssize_t		freq[GRP_NTBL][VCH_SIZE], pfr[VCH_SIZE], psz[VCH_SIZE], *fr, *sz;
	Vcchar_t	*part = grp->part, *work = grp->work;
	ssize_t		npts = grp->npts, *ppos = grp->ppos, *sort = grp->sort;
	Vchobj_t	*obj = grp->obj;
	Vcchar_t	*ofr = grp->ofr;

	if(ntbl > npts)
		ntbl = npts;
	if(ntbl > GRP_NTBL)
		ntbl = GRP_NTBL;

	if(grp->ntbl <= 0 || ntbl < grp->ntbl) /* making new tables */
	{	/* sort parts so that similar prefixes group together */
		for(k = 0; k < npts; ++k)
			sort[k] = k;
		vcqsort(sort, npts, sizeof(ssize_t), partcmp, grp);

		/* now make tables */
		for(z = npts/ntbl, p = 0, t = 0; t < ntbl; t += 1)
		{	fr = freq[t]; CLRTABLE(fr, VCH_SIZE);

			for(n = p+z > npts ? (npts-p) : z; n > 0; --n, ++p)
			{	k = sort[p];
				GRPFREQ(fr, obj+ppos[k], ofr+ppos[k], ppos[k+1]-ppos[k]);
			}

			tbl[t].maxs = vchsize(VCH_SIZE, fr, tbl[t].size, &tbl[t].runb);
			tbl[t].nblks = 0;
			tbl[t].cost = 0;
		}
	}
	else /* increasing number of tables */
	{	/**/ASSERT(ntbl <= GRP_NTBL && grp->ntbl <= GRP_NTBL);
		memcpy(tbl,grp->tbl,grp->ntbl*sizeof(Table_t));
		n = ntbl - grp->ntbl; ntbl = grp->ntbl;
		for(; n > 0; --n)
		{	for(z = 0, p = -1, i = 0; i < grp->ntbl; ++i)
			{	if(tbl[i].cost <= z)
					continue;
				z = tbl[p = i].cost;
			}
			if(p < 0) /* if p >= 0, it's the highest cost table */
				break;

			/* split blocks of table p into two tables, p and q */
			q = ntbl; ntbl += 1;
			z = tbl[p].nblks/2 - 1; fr = freq[p]; CLRTABLE(fr, VCH_SIZE);
			for(i = 0; i < npts; ++i)
			{	if(work[i] != p)
					continue;
				GRPFREQ(fr, obj+ppos[i], ofr+ppos[i], ppos[i+1]-ppos[i]);
				if((z -= 1) == 0) /* start 2nd table */
					{ fr = freq[q]; CLRTABLE(fr, VCH_SIZE); }
			}

			/* make sure neither table will considered for further splitting */
			tbl[p].maxs = vchsize(VCH_SIZE, freq[p], tbl[p].size, &tbl[p].runb);
			tbl[q].maxs = vchsize(VCH_SIZE, freq[q], tbl[q].size, &tbl[q].runb);
			tbl[p].cost = tbl[q].cost = 0;
			tbl[p].nblks = tbl[q].nblks = 0;
		}
	}

	/**/PRINT(2,"\t#table aiming for=%d\n",ntbl);
	for(iter = 1;; iter++)
	{	/**/PRINT(2,"\t\titer=%d ", iter); PRINT(2,"cmpsz=%d ", (grp->cmpsz+7)/8);

		for(k = 0; k < ntbl; ++k)
		{	fr = freq[k]; sz = tbl[k].size;
			if((z = tbl[k].maxs) > 0)
			{	z += (z <= 4 ? 15 : z <= 8 ? 7 : z <= 16 ? 1 : 0);
				for(p = 0; p < VCH_SIZE; ++p)
				{	if(fr[p] != 0)
						fr[p] = 0; /* clear frequency table */
					else	sz[p] = z; /* 0-freq obj gets dflt length */
				}
			}
			tbl[k].cost  = 0;
			tbl[k].nblks = 0;
		}

		for(i = 0; i < npts; i += 1)
		{	ssize_t		bestz, bestt;

			/* find the table best matching this part */
			p = ppos[i]; n = ppos[i+1] - ppos[i];
			for(bestz = VC_LARGE, bestt = -1, k = 0; k < ntbl; ++k)
			{	if(tbl[k].maxs == 0) /* representing a run */
					z = (n == 1 && obj[p] == tbl[k].runb) ? 0 : VC_LARGE;
				else /* normal table, tally up the lengths */
					{ sz = tbl[k].size; GRPSIZE(z, sz, obj+p, ofr+p, n); }
				if(z < bestz || bestt < 0)
				{	bestz = z;
					bestt = k;
				}
			}

			work[i] = bestt; /* assignment, then add frequencies */
			fr = freq[bestt]; GRPFREQ(fr, obj+p, ofr+p, n );
			tbl[bestt].nblks += 1;
		}

		z = 0; /* recompute encoding cost given new grouping */
		for(k = 0; k < ntbl; ++k)
		{	if(tbl[k].nblks <= 0) /* empty table */
			{	ntbl -= 1;
				for(p = k; p < ntbl; ++p)
				{	memcpy(tbl+p, tbl+p+1, sizeof(Table_t));
					memcpy(freq[p], freq[p+1], VCH_SIZE*sizeof(ssize_t));
				}
				for(i = 0; i < npts; ++i)
				{	/**/ ASSERT(work[i] != k);
					if(work[i] > k)
						work[i] -= 1;
				}
				k -= 1;
				continue;
			}

			fr = freq[k]; sz = tbl[k].size;
			tbl[k].maxs = vchsize(VCH_SIZE, fr, sz, &tbl[k].runb);
			if(tbl[k].maxs > 0)
			{	DOTPRODUCT(p,fr,sz,VCH_SIZE); /* encoding size */
				n = vchputcode(VCH_SIZE, sz, tbl[k].maxs, tmp, sizeof(tmp));
				p += (n + VCSIZEU(n))*8;
				tbl[k].cost = p;
				z += p; /* add to total cost */
			}
			else
			{	/**/ASSERT(tbl[k].runb >= 0);
				z += (tbl[k].cost = 2*8); /* one 0-byte and the run byte */
			}
		}

		if(ntbl > 1) /* add the cost of encoding the indices */
		{	n = vcapply(grp->mtf,work,npts,&dt); /* mtf transform */
			CLRTABLE(pfr,VCH_SIZE); ADDFREQ(pfr, Vcchar_t*, dt, n);
			k = vchsize(VCH_SIZE, pfr, psz, NIL(int*));
			DOTPRODUCT(p, pfr, psz, VCH_SIZE);
			n = vchputcode(VCH_SIZE, psz, k, tmp, sizeof(tmp));
			z += p + (n + VCSIZEU(n))*8;
		}

		/**/PRINT(2,"z=%d\n", (z+7)/8);
		if(z < (p = grp->cmpsz) )
		{	grp->ntbl = ntbl;
			grp->cmpsz = z;
			memcpy(part, work, npts);
			memcpy(grp->tbl, tbl, ntbl*sizeof(Table_t));
		}

		if(ntbl == 1 || iter >= niter || (iter > 1 && z >= p-64) )
		{	/**/PRINT(2,"\t\t#table=%d ",grp->ntbl);
			/**/PRINT(2,"cmpsz=%d\n", (grp->cmpsz+7)/8);
			return;
		}
	}
}

#if __STD_C
static ssize_t grphuff(Vcodex_t* vc, const Void_t* data, size_t dtsz, Void_t** out)
#else
static ssize_t grphuff(vc, data, dtsz, out)
Vcodex_t*	vc;	/* Vcodex handle		*/
Void_t*		data;	/* target data to be compressed	*/
size_t		dtsz;	/* data size			*/
Void_t**	out;	/* to return output buffer 	*/
#endif
{
	ssize_t		n, i, p, k, s, better;
	ssize_t		*sz, npts, ntbl, ptsz, gmin, gmax;
	Vcchar_t	*part;
	Table_t		*tbl;
	Vcbit_t		b, *bt, bits[GRP_NTBL][VCH_SIZE];
	Vcchar_t	*output, *dt;
	ssize_t		n_output;
	Vcio_t		io;
	Vcctxt_t	*ctxt = VCGETCTXT(vc, Vcctxt_t*);
	Group_t		*grp = VCGETMETH(vc, Group_t*);

	if(dtsz == 0)
		return 0;
	vcsetbuf(grp->huf, NIL(Vcchar_t*), -1, -1);
	vcsetbuf(grp->mtf, NIL(Vcchar_t*), -1, -1);

	/* set desired part size and bounds for number of groups */
	gmin = GRPMIN(dtsz);
	gmax = GRPMAX(dtsz);
	ptsz = GRPPTSZ(dtsz);

	/* initialize data structures for fast frequency calculations */
	if(grpinit(grp, (Vcchar_t*)data, dtsz, ptsz) < 0)
		return -1;

	/* compute optimal number of tables */
	grppart(grp, (gmin+gmax)/2, 1);
	for(k = grp->ntbl+1, i = grp->ntbl-1; i >= gmin || k <= gmax; --i, ++k)
	{	better = -1;
		if(i >= gmin)
		{	s = grp->cmpsz;
			grppart(grp, i, 1);
			if(grp->cmpsz < s)
				better = i;
		}
		if(k <= gmax)
		{	s = grp->cmpsz;
			grppart(grp, k, 1);
			if(grp->cmpsz < s)
				better = k;
		}
		if(better < 0)
			break;
		else if(better == i)
			k = gmax+1;
		else	i = gmin-1;
	}

	/* now make the best grouping */
	if(grp->ntbl > 1)
		grppart(grp, grp->ntbl, GRP_ITER);

	/* short-hands */
	part = grp->part; npts = grp->npts; ptsz = grp->ptsz;
	tbl = grp->tbl; ntbl = grp->ntbl;

	/* get space for output */
	n_output = (ntbl+1)*(VCH_SIZE+8) + (grp->cmpsz+7)/8;
	if(!(output = vcsetbuf(vc, NIL(Vcchar_t*), n_output, 0)) )
		return -1;
	vcioinit(&io, output, n_output);

	vcioputu(&io, dtsz);
	vcioputu(&io, ntbl);
	vcioputu(&io, ptsz); /* the part size used */

	/* output the coding tables and compute the coding bits */
	for(k = 0; k < ntbl; ++k)
	{	vcioputc(&io, tbl[k].maxs);
		if(tbl[k].maxs == 0) /* coding a run */
			vcioputc(&io,tbl[k].runb);
		else /* coding a code table */
		{	dt = vcionext(&io); n = vciomore(&io);
			if((n = vchputcode(VCH_SIZE, tbl[k].size, tbl[k].maxs, dt, n)) < 0)
				return -1;
			else	vcioskip(&io, n);
			vchbits(VCH_SIZE, tbl[k].size, bits[k]);
		}
	}

	/* compress and output the indices */
	if((n = vcapply(grp->mtf, part, npts, &dt)) < 0 )
		return -1;
	if((n = vcapply(grp->huf, dt, n, &dt)) < 0 )
		return -1;
	vcioputu(&io,n);
	vcioputs(&io,dt,n);

	/* now write out the encoded data */
	vciosetb(&io, b, n, VC_ENCODE);
	for(p = 0, i = 0; i < npts; i += 1, p += ptsz)
	{	if(tbl[part[i]].maxs == 0)
			continue;

		sz = tbl[part[i]].size;
		bt = bits[part[i]];
		dt = ((Vcchar_t*)data)+p;
		for(k = i == npts-1 ? dtsz-p : ptsz; k > 0; --k, ++dt)
			vcioaddb(&io, b, n, bt[*dt], sz[*dt]);
	}
	vcioendb(&io, b, n, VC_ENCODE);

	n = vciosize(&io); /**/ ASSERT(n <= n_output);
	if(vc->coder)
	{	if(VCSETCTXT(vc->coder, ctxt->ctxt) < 0)
			return -1;
		if(VCCODER(vc, vc->coder, 0, output, n) < 0 )
			return -1;
	}

	if(out)
		*out = output;
	return n;
}

#if __STD_C
static ssize_t grpunhuff(Vcodex_t* vc, const Void_t* data, size_t dtsz, Void_t** out)
#else
static ssize_t grpunhuff(vc, data, dtsz, out)
Vcodex_t*	vc;	/* Vcodex handle		*/
Void_t*		data;	/* data to be uncompressed	*/
size_t		dtsz;	/* data size			*/
Void_t**	out;	/* to return output buffer 	*/
#endif
{
	reg Vcbit_t	b;
	ssize_t		k, p, sz, n, ntop;
	short		*node, *size;
	ssize_t		npts, ptsz, ntbl;
	Vcchar_t	*part, *dt, *output, *o, *endo;
	Table_t		tbl[GRP_NTBL];
	Vcbit_t		bits[GRP_NTBL][VCH_SIZE];
	Vchtrie_t	*trie[GRP_NTBL];
	Vcio_t		io;
	Vcctxt_t	*ctxt = VCGETCTXT(vc, Vcctxt_t*);
	Group_t		*grp = VCGETMETH(vc, Group_t*);
	int		rv = -1;

	if(dtsz == 0)
		return 0;
	vcsetbuf(grp->huf, NIL(Vcchar_t*), -1, -1);
	vcsetbuf(grp->mtf, NIL(Vcchar_t*), -1, -1);

	if(vc->coder)
	{	if(VCSETCTXT(vc->coder, ctxt->ctxt) < 0)
			return -1;
		if((n = vcapply(vc->coder, data, dtsz, &data)) <= 0)
			return -1;
		dtsz = n;
	}

	for(k = 0; k < GRP_NTBL; ++k)
		trie[k] = NIL(Vchtrie_t*);

	vcioinit(&io,data,dtsz);

	if((sz = (ssize_t)vciogetu(&io)) < 0)	/* size of decoded data */
		goto done;
	if((ntbl = (ssize_t)vciogetu(&io)) < 0)	/* # of coding tables	*/
		goto done;
	if((ptsz = (ssize_t)vciogetu(&io)) < 0)	/* size of each part	*/
		goto done;

	for(k = 0; k < ntbl; ++k)
	{	if((tbl[k].maxs = vciogetc(&io)) < 0) /* max code size	*/
			goto done;
		else if(tbl[k].maxs == 0) /* this is a run */
			tbl[k].runb = vciogetc(&io);
		else /* construct code table and trie for fast matching	*/
		{	dt = vcionext(&io); n = vciomore(&io);
			if((n = vchgetcode(VCH_SIZE, tbl[k].size, tbl[k].maxs, dt, n)) < 0)
				goto done;
			else	vcioskip(&io,n);

			vchbits(VCH_SIZE, tbl[k].size, bits[k]);
			if(!(trie[k] = vchbldtrie(VCH_SIZE, tbl[k].size, bits[k])) )
				goto done;
		}
	}

	/* reconstruct the array of part indices */
	if((n = vciogetu(&io)) < 0)
		goto done;
	dt = vcionext(&io); vcioskip(&io,n);
	if((n = vcapply(grp->huf, dt, n, &dt)) < 0)
		goto done;
	if((npts = vcapply(grp->mtf, dt, n, &part)) < 0)
		goto done;

	/* buffer to reconstruct the original data */
	if(!(output = vcsetbuf(vc, NIL(Vcchar_t*), sz, 0)) )
		goto done;
	endo = (o = output) + sz;

	vciosetb(&io, b, n, VC_DECODE);
	for(k = 0; k < npts; ++k)
	{	dt = o + (k == npts-1 ? endo-o : ptsz); /* end of this part */
		if(tbl[part[k]].maxs == 0) /* reconstruct a run */
		{	p = tbl[part[k]].runb;
			while(o < dt)
				*o++ = (Vcchar_t)p;
		}
		else /* reconstructing a Huffman-coded set of bytes */
		{	node = trie[part[k]]->node;
			size = trie[part[k]]->size;
			ntop = trie[part[k]]->ntop;
			for(sz = ntop, p = 0;;)
			{	vciofilb(&io, b, n, sz);

				p += (b >> (VC_BITSIZE-sz));
				if(size[p] > 0)
				{	vciodelb(&io, b, n, size[p]);
					*o = (Vcchar_t)node[p];
					if((o += 1) >= dt)
						break;
					sz = ntop; p = 0;
				}
				else if(size[p] == 0)
					return -1;
				else
				{	vciodelb(&io, b, n, sz);
					sz = -size[p]; p = node[p];
				}
			}
		}
	} /**/ASSERT(o == endo);
	vcioendb(&io, b, n, VC_DECODE);

	if(out)
		*out = output;
	rv = o-output;

done:	for(k = 0; k < GRP_NTBL; ++k)
		if(trie[k])
			vchdeltrie(trie[k]);
	return rv;
}

/* Event handler */
#if __STD_C
static int grpevent(Vcodex_t* vc, int type, Void_t* params)
#else
static int grpevent(vc, type, params)
Vcodex_t*	vc;
int		type;
Void_t*		params;
#endif
{
	Group_t	*grp;
	int	rv = -1;

	if(type == VC_OPENING)
	{	if(!(grp = (Group_t*)calloc(1, sizeof(Group_t))) )
			return -1;

		/* open the entropy coder handle */
		grp->huf = vcopen(NIL(Vcdisc_t*), Vchuffman, 0, 0, vc->flags);
		grp->mtf = vcopen(NIL(Vcdisc_t*), Vcmtf, 0, 0, vc->flags);
		if(!grp->huf || !grp->mtf )
			goto do_closing;

		VCSETMETH(vc, grp);
		return 0;
	}
	else if(type == VC_CLOSING)
	{	rv = 0;
	do_closing:
		if((grp = VCGETMETH(vc, Group_t*)) )
		{	if(grp->huf)
				vcclose(grp->huf);
			if(grp->mtf)
				vcclose(grp->mtf);
			if(grp->part)
				free(grp->part);
			if(grp->ppos)
				free(grp->ppos);
			if(grp->obj)
				free(grp->obj);
			if(grp->ofr)
				free(grp->ofr);
			free(grp);
		}

		VCSETMETH(vc, NIL(Group_t*));
		return rv;
	}
	else if(type == VC_FREEBUF)
	{	if((grp = VCGETMETH(vc, Group_t*)) )
		{	if(grp->mtf)
				vcsetbuf(grp->mtf, NIL(Vcchar_t*), -1, -1);
			if(grp->huf)
				vcsetbuf(grp->huf, NIL(Vcchar_t*), -1, -1);
		}
		return 0;
	}
	else	return 0;
}

Vcmethod_t	_Vchuffgroup =
{	grphuff,
	grpunhuff,
	0,
	0,
	grpevent,
	"huffgroup", "\150\165\146\146\147\162\157\165\160", "Huffman encoding by groups",
	NIL(Vcmtarg_t*),
	1024*1024,
	VCNEXT(Vchuffgroup)
};

VCLIB(Vchuffgroup)
