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
#include	"vchdr.h"

/* Compute the suffix array of a string.
**
** Written by Kiem-Phong Vo (kpv@research.att.com)
*/

#ifdef DEBUG
static int		N_qsort,S_qsort, N_csort,S_csort;
#define DB_OK		0	/* 1 to skip hard checks, 0 to do all	*/

static int chkinversion(ssize_t* idx, ssize_t* inv, ssize_t n, int partial)
{	/* see if inv and idx are inverse permutations */
	ssize_t	i, endi, k;
	for(i = 0; i < n; i = k)
	{	endi = inv[idx[i]];
		/**/ASSERT((partial && endi >= i) || (!partial && endi == i) );
		for(k = i; k <= endi; ++k)
			/**/ASSERT(inv[idx[k]] == inv[idx[endi]]);
	} /**/ASSERT(i == n);
	return 1;
}
static int chkorder(Vcsfx_t* sfx)
{	/* see if suffixes are ordered correctly */
	ssize_t		k, n1, n2, p;
	Vcchar_t	*s1, *s2;
	Vcchar_t	*str = sfx->str;
	ssize_t		len  = sfx->nstr;
	ssize_t		*idx = sfx->idx;

	s1 = str + idx[0];
	n1 = len - idx[0];
	for(p = 1; p < len; ++p, s1 = s2, n1 = n2)
	{	s2 = str + idx[p];
		n2 = len - idx[p];
		for(k = 0; k < n1 && k < n2; ++k)
			if(s1[k] != s2[k])
				{ /**/ASSERT(s1[k] < s2[k]); break; }
	}
	return 1;
}
#endif /*DEBUG*/

#define SFX_INSERTION	20	/* insertion-sort small groups 	*/
#define SFX_INFINITE	((~((size_t)0)) >> 1) /* a large number	*/

#define SFX_MARK	(~SFX_INFINITE) /* mark sorted buckets	*/
#define SFXSETMARK(v)	((v) |= SFX_MARK)
#define SFXCHKMARK(v)	((v) &  SFX_MARK)
#define SFXCLRMARK(v)	((v) & ~SFX_MARK)

/* Sfxqsort recurses if there is progress or if the recursion is still shallow.
** The parameters to determine this are:
**	l: main loop number,
**	r: number of elements to sort,
**	n: the original size,
**	d: current depth, and
**	*dp: known uniform sort depth for all recursion branches.
*/
#define SFXMAXD(l,r,d,dp)	((l) > 0 ? 4 : (d) >= *(dp) ? 1 : \
				 (r) <  2*1024 ? SFX_INFINITE : \
				 (r) <  4*1024 ? 512 : (r) <  8*1024 ? 256 : \
				 (r) < 16*1024 ? 128 : (r) < 32*1024 ?  64 : 32 )
#define SFXPROGRESS(n,r)	((r) < (n)/2)
#define SFXRECURSE(l,n,r,d,dp)	(SFXPROGRESS(n,r) || (d) < SFXMAXD(l,r,d,dp) )

/* a cheap random number generator */
static unsigned int	_rand = 0xdeadbeef;
#define SFXRAND()	(_rand = _rand*16777617 + 3)

/* median of 3 */
#define SFXMEDIAN(i,ip,j,jp,k,kp) \
		(i <= j ? (j <= k ? (jp) : (i <= k ? (kp) : (ip)) ) : \
			  (j >= k ? (jp) : (i >= k ? (kp) : (ip)) ) )

/* swapping two elements or two adjacent vectors */
#define SFXSWAP(xp,yp,t)	(t = *(xp), *(xp) = *(yp), *(yp) = t)
#if __STD_C
static void sfxswap(ssize_t* xp, ssize_t x, ssize_t y)
#else
static void sfxswap(xp, x, y)
ssize_t*	xp;	/* base of vectors	*/
ssize_t		x;	/* size of left vector	*/
ssize_t		y;	/* size of right one	*/
#endif
{	ssize_t	t, *endxy, *xx = xp, *yy = xp+x;

 	if((t = y-x) > 0)
		for(endxy = yy, yy += t; xx < endxy; ++xx, ++yy)
			SFXSWAP(xx, yy, t);
  	else	for(endxy = yy+y; yy < endxy; ++xx, ++yy)
			SFXSWAP(xx, yy, t);
}

/* insertion sort for small groups */
#if __STD_C
static ssize_t sfxisort(Vcsfx_t* sfx, ssize_t* lp, ssize_t* rp, ssize_t pfx, ssize_t incr)
#else
static ssize_t sfxisort(sfx, lp, rp, pfx, incr)
Vcsfx_t*	sfx;
ssize_t*	lp;	/* left-most element	*/
ssize_t*	rp;	/* right-most element	*/
ssize_t		pfx;	/* common sorted prefix	*/
ssize_t		incr;	/* skip on starting	*/
#endif
{
	ssize_t	*ip, *jp, *kp, *pp, *qp, v;
	ssize_t	*inv = sfx->inv;

#define CMP(c,x,y) \
{	pp = inv + *x + incr; qp = inv + *y + incr; \
	for(;; pp += pfx, qp += pfx) \
		if((c = *pp - *qp) != 0) break; \
}
	for(ip = rp-1; ip >= lp; --ip) /* inserting ip into [ip+1,rp] */
	{	for(jp = ip+1; jp <= rp; ++jp)
		{	CMP(v,ip,jp); /**/ASSERT(v != 0);
			if(v < 0)
				break;
		}
		if((kp = ip+1) < jp) /* ip is to be inserted at jp-1 */
		{	v = *ip;
			do *(kp-1) = *kp;
				while((kp += 1) < jp);
			*(kp-1) = v;
		}
	}
	for(kp = sfx->idx, ip = lp; ip <= rp; ++ip) /* update inversion numbers */
		inv[*ip] = ip-kp;
	return (rp-lp+1);
}

/* Bentley-Sedgewick quicksort parameterized with prefix/depth. */
#if __STD_C
static ssize_t sfxqsort(Vcsfx_t* sfx, ssize_t loop,
			ssize_t* lp, ssize_t* rp, ssize_t rank,
			ssize_t pfx, ssize_t d, ssize_t* depth)
#else
static ssize_t sfxqsort(sfx, loop, lp, rp, rank, pfx, d, depth)
Vcsfx_t*	sfx;	/* handle of string to be sorted	*/
ssize_t		loop;	/* # of Sadakane-Larsson loop		*/
ssize_t*	lp;	/* left end of the area to be sorted	*/
ssize_t*	rp;	/* right end of the area to be sorted	*/
ssize_t		rank;	/* starting inversion or rank number	*/
ssize_t		pfx;	/* the prefix length known to be sorted	*/
ssize_t		d;	/* current depth to be sorted		*/
ssize_t*	depth;	/* to return maximmally sorted index  	*/
#endif
{
	ssize_t		i, j, k, s, r;
	ssize_t		*ip, *jp, *kp, *pp, *qp;
	ssize_t		*idx = sfx->idx, *inv = sfx->inv;
	ssize_t		incr = d*pfx, sz = rp-lp+1;

	if(sz < SFX_INSERTION)
		return sfxisort(sfx,lp,rp,pfx,incr);

q_sort:	/* compute a quasi-median to use as the pivot */
#define RANK(x)		inv[*(x) + incr]
	ip = lp + SFXRAND()%sz; i = RANK(ip);
	jp = rp - SFXRAND()%sz; j = RANK(jp);
	kp = lp + SFXRAND()%sz; k = RANK(kp);
	pp = SFXMEDIAN(i,ip,j,jp,k,kp);

	if(sz > 4*SFX_INSERTION) /* quasi-median-of-9 */
	{	ip = lp + SFXRAND()%sz; i = RANK(ip);
		jp = rp - SFXRAND()%sz; j = RANK(jp);
		kp = lp + SFXRAND()%sz; k = RANK(kp);
		qp = SFXMEDIAN(i,ip,j,jp,k,kp);
		ip = lp + SFXRAND()%sz; i = RANK(ip);
		jp = rp - SFXRAND()%sz; j = RANK(jp);
		kp = lp + SFXRAND()%sz; k = RANK(kp);
		kp = SFXMEDIAN(i,ip,j,jp,k,kp);
		i = RANK(pp); j = RANK(qp); k = RANK(kp);
		pp = SFXMEDIAN(i,pp,j,qp,k,kp);
	}

	SFXSWAP(pp,rp,s); r = RANK(rp); /* pivot around rp */
	for(ip = pp = lp-1, jp = qp = rp;;)
	{	while((ip += 1) < jp && (k = RANK(ip) - r) <= 0 )
			if(k == 0 && (pp += 1) < ip )
				SFXSWAP(pp,ip,s);
		while((jp -= 1) > ip && (k = RANK(jp) - r) >= 0 )
			if(k == 0 && (qp -= 1) > jp )
				SFXSWAP(qp,jp,s);
		if(ip >= jp)
			break;
		SFXSWAP(ip,jp,s);
	}

	if((i = ip-pp-1) > 0 && (k = pp-lp+1) > 0)
		sfxswap(lp,k,i); /* swap elts < pivot to the left */
	if((j = qp - ip) > 0 && (k = rp-qp+1) > 0)
		sfxswap(ip,j,k); /* swap elts > pivot to the right */
	ip = lp+i; jp = rp-j; k = jp-ip+1; /* [ip,jp] is the group = pivot */
	/**/ASSERT(i >= 0 && j >= 0 && k > 0);

	s = 0; /* s counts the completely sorted elements */

	if(i > 1) /* sort the segment of objects < pivot */
		s += sfxqsort(sfx, loop, lp, ip-1, rank, pfx, d, depth);
	else if(i > 0)
		{ s += 1; inv[*lp] = lp-idx; }

	if(k > 1) /* sort the segment of objects == pivot */
	{	if(SFXRECURSE(loop,sz,k,d,depth) )
		{	if(i == 0 && j == 0) /* tail-recursion */
			{	d += 1; incr += pfx;
				goto q_sort;
			}
			else	s += sfxqsort(sfx, loop, ip, jp, rank, pfx, d+1, depth);
		}
		else /* not sorting further, update sorted depth and ranks */
		{	if(*depth > d)
				*depth = d;
			if((r = jp-idx) != rank)
				for(; ip <= jp; ++ip)
					inv[*ip] = r;
		}
	}
	else	{ s += 1; inv[*ip] = ip-idx; }

	if(j > 1) /* sort the segment of objects > pivot */
		s += sfxqsort(sfx, loop, jp+1, rp, rank, pfx, d, depth);
	else if(j > 0)
		{ s += 1; inv[*rp] = rp-idx; }

	return s;
}

/* Suffixes starting with x (except perhaps the xx's) have been sorted.
** This algorithm (J. Seward) spreads the sort order to other groups.
*/
#if __STD_C
static void sfxcsort(Vcsfx_t* sfx, ssize_t* bckt, ssize_t* high, ssize_t x)
#else
static void sfxcsort(sfx, bckt, high, x)
Vcsfx_t*	sfx;
ssize_t*	bckt;
ssize_t*	high;
ssize_t		x;
#endif
{
	ssize_t		p, q, *ip, *cp;
	ssize_t		lcopy[256], rcopy[256];
	ssize_t		*idx = sfx->idx, *inv = sfx->inv;
	Vcchar_t	*str = sfx->str;
#define HEAD(z,zp)	((z = *(zp)-1) < 0)
#define BYTE(z)		(str[z])

	/**/COUNT(N_csort);
	for(p = 0; p < 256; ++p) /* compute the bounds of buckets to be sorted */
	{	q = (p<<8) + x;
		if(p != x && SFXCHKMARK(bckt[q]))
			lcopy[p] = rcopy[p] = -1;
		else /* bucket xx must be well-defined for the loops below */
		{	lcopy[p] = SFXCLRMARK(bckt[q]);
			rcopy[p] = (x == 255 ? high[p] : SFXCLRMARK(bckt[q+1])) - 1;
			SFXSETMARK(bckt[q]);
		}
	}

	p = SFXCLRMARK(bckt[x<<8]);
	for(ip = idx+p; p < lcopy[x]; ++p, ++ip)
	{	if(HEAD(q,ip))
			continue;
		if(*(cp = &lcopy[BYTE(q)]) >= 0)
			{ idx[*cp] = q; inv[q] = *cp; *cp += 1; /**/COUNT(S_csort); }
	}

	p = (x == 255 ? sfx->nstr : SFXCLRMARK(bckt[(x+1)<<8])) - 1;
	for(ip = idx+p; p > rcopy[x]; --p, --ip)
	{	if(HEAD(q,ip))
			continue;
		if(*(cp = &rcopy[BYTE(q)]) >= 0)
			{ idx[*cp] = q; inv[q] = *cp; *cp -= 1; /**/COUNT(S_csort); }
	} /**/ASSERT(DB_OK || chkinversion(idx,inv,sfx->nstr,1));
}

/* bucket-sort using first two bytes */
#if __STD_C
static ssize_t sfxbsort(Vcsfx_t* sfx, ssize_t* bckt, ssize_t* high)
#else
static ssize_t sfxbsort(sfx, bckt, high)
Vcsfx_t*	sfx;
ssize_t*	bckt;	/* buckets of sfx sorted by two bytes	*/
ssize_t*	high;	/* highest pos in idx[] of the buckets	*/
#endif
{
	ssize_t		n, i, j, v, lasts;
	ssize_t		*b, *endb, *r, rank[256*256];
	Vcchar_t	*s, *ends;
	Vcchar_t	*str = sfx->str;
	ssize_t		len  = sfx->nstr, *idx = sfx->idx, *inv = sfx->inv;

	/* count frequency of suffixes with same first 2 bytes */
	for(endb = (b = bckt)+256*256; b < endb; ++b)
		*b = 0;

	ends = (s = str) + len - 1;
	for(i = *s; s < ends; i = j)
		bckt[(i<<8) + (j = *++s)] += 1;

	/* allocate space in the index array */
	lasts = len > 0 ? str[len-1] : -1;
	for(n = 0, b = bckt, r = rank, i = 0; i < 256; ++i)
	{	for(endb = b+256; b < endb; ++b, ++r)
		{	*b = (n += *b);
			*r = n - 1; /* rank for this bucket */
		}
		high[i] = n; /* the upper bound of this bucket */

		if(i == lasts) /* last sfx of string */
		{	idx[n] = len-1;
			inv[len-1] = n++;
		}
	}

	/* sort suffixes into their buckets */
	ends = (s = str) + len - 1;
	for(n = 0, i = *s; s < ends; i = j)
	{	v = (i << 8) + (j = *++s);
		idx[bckt[v] -= 1] = n;
		inv[n++] = rank[v];
	} /**/ASSERT(DB_OK || chkinversion(idx,inv,len,1));

	return 2;
}

/* groups are sorted in order by # of remaining unsorteds */
typedef struct _order_s
{	ssize_t	unsrt;	/* #unsorted suffixes in group	*/
	ssize_t	group;	/* group sorted by first byte	*/
} Order_t;

#if __STD_C
static int ordercmp(const Void_t* arg1, const Void_t* arg2)
#else
static int ordercmp(arg1, arg2)
Void_t*	arg1;
Void_t* arg2;
#endif
{	return ((Order_t*)arg1)->unsrt - ((Order_t*)arg2)->unsrt;
}

#if __STD_C
Vcsfx_t* vcsfxsort(const Void_t* str, size_t len)
#else
Vcsfx_t* vcsfxsort(str, len)
Void_t*	str;	/* the string to be sorted	*/
size_t	len;	/* length of the string		*/
#endif
{
	ssize_t		p, q, d, depth, pfx, loop;
	ssize_t		o, endo, n_order;
	Order_t		order[256];
	ssize_t		bckt[256*256], high[256];
	ssize_t		*idx, *inv;
	Vcsfx_t		*sfx;

	if(!str || len <= 0)
		{ str = NIL(Void_t*); len = 0; }

	/* allocate space for the data structures */
	if(!(sfx = malloc(sizeof(Vcsfx_t) + 2*(len+1)*sizeof(ssize_t))) )
		return NIL(Vcsfx_t*);
	sfx->idx = idx = (ssize_t*)(sfx+1);
	sfx->inv = inv = idx + len;
	inv[len] = len+1;

	sfx->str = (Vcchar_t*)str;
	sfx->nstr = len;

	/* bucket-sort by first few bytes, then calculate unsorted groups */
	pfx = sfxbsort(sfx, bckt, high);
	for(n_order = 0, p = 0; p < 256; ++p)
	{	order[n_order].unsrt = 0;
		for(d = (q = p<<8)+255; q <= d; ++q)
		{	o = (q == d ? high[p] : bckt[q+1]) - bckt[q];
			if(o <= 1)
				SFXSETMARK(bckt[q]);
			else	order[n_order].unsrt += o;
		}
		if(order[n_order].unsrt > 0)
			order[n_order++].group = p; /* index of the group */
	}

	for(loop = 0; n_order > 0; pfx *= depth+1, ++loop) /* Sadakane-Larsson loop */
	{	depth = SFX_INFINITE; /* multiplier for next turn */
		/**/PRINT(2,"N_loop=%d ",loop); PRINT(2,"S_pfx=%d\n",pfx);

		/* do groups from fewest to most # of unsorteds */
		qsort(order, n_order, sizeof(Order_t), ordercmp);
		for(endo = n_order, o = n_order = 0; o < endo; ++o)
		{	p = order[o].group; /* big group to be sorted */
			order[n_order].unsrt = 0;
			for(q = 0; q <= 256; ++q) /* q == 256 represents pp suffixes */
			{	register ssize_t r, l, endl, u, b;

				/* pp-suffixes are done last - possibly by sfxcsort() */
				b = (p<<8) + (q == 256 ? p : q);
				if(SFXCHKMARK(bckt[b]) || q == p ||
				   (q == 256 && order[n_order].unsrt == 0) )
					continue;

				l = bckt[b];
				endl = (q == 255 || (q == 256 && p == 255)) ?
					 high[p] : SFXCLRMARK(bckt[b+1]);
				for(u = 0; l < endl; ++l)
				{	if((r = inv[idx[l]]) == l)
						continue;
					/**/COUNT(N_qsort); TALLY(1,S_qsort,r-l+1);
					d = SFX_INFINITE; /* to get depth sorted to */
					u += (r-l+1) -
					     sfxqsort(sfx,loop,idx+l,idx+r,r,pfx,1,&d);
					depth = d < depth ? d : depth; /* reset depth */
					l = r;
				}

				if(u == 0) /* this group is completely sorted */
					SFXSETMARK(bckt[b]);
				else	order[n_order].unsrt += u;
			}

			if(order[n_order].unsrt > 0) /* sort again with longer pfx */
				order[n_order++].group = p;
			else	sfxcsort(sfx, bckt, high, p); /* copy sort order */
		}
	}

	/**/ASSERT(chkinversion(idx,inv,len,0) && chkorder(sfx));
	/**/PRINT(2,"N_qsort=%d ",N_qsort); PRINT(2,"S_qsort=%d\n",S_qsort);
	/**/PRINT(2,"N_csort=%d ",N_csort); PRINT(2,"S_csort=%d\n",S_csort);
	/**/SET(N_qsort,0);SET(S_qsort,0); SET(N_csort,0);SET(S_csort,0);

	return sfx;
}
