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

/* Generalized block-move parsing of a pair of strings.
**
** Written by Kiem-Phong Vo (kpv@research.att.com)
*/

/* hash HMIN bytes at a time. The below FK/RK functions assume HMIN == 4 */
#define HMIN		sizeof(Vchash_t)

#define FKINIT(s)	(((s)[0]<<16) | ((s)[1]<<8) | (s)[2] )
#define FKNEXT(s,k)	(((k)<<8) | (s)[0] )
#define RKINIT(s)	(((s)[0]<<8) | ((s)[1]<<16) | ((s)[2]<<24) )
#define RKNEXT(s,k)	(((k)>>8) | ((s)[0]<<24) )

#define FKINITM(s,m)	(((m)[(s)[0]]<<16) | ((m)[(s)[1]]<<8) | (m)[(s)[2]] )
#define FKNEXTM(s,k,m)	(((k)<<8) | (m)[(s)[0]] )
#define RKINITM(s,m)	(((m)[(s)[0]]<<8) | ((m)[(s)[1]]<<16) | ((m)[(s)[2]]<<24) )
#define RKNEXTM(s,k,m)	(((k)>>8) | ((m)[(s)[0]]<<24) )

#define FKTRUE(s)	(((s)[0]<<24) | ((s)[1]<<16) | ((s)[2]<<8) | ((s)[3]) )
#define RKTRUE(s)	(((s)[3]<<24) | ((s)[2]<<16) | ((s)[1]<<8) | ((s)[0]) )
#define FKTRUEM(s,m)	(((m)[(s)[0]]<<24) | ((m)[(s)[1]]<<16) | ((m)[(s)[2]]<<8) | ((m)[(s)[3]]) )
#define RKTRUEM(s,m)	(((m)[(s)[3]]<<24) | ((m)[(s)[2]]<<16) | ((m)[(s)[1]]<<8) | ((m)[(s)[0]]) )

typedef struct _hobj_s
{	struct _hobj_s*	next;
	size_t		cnt;	/* number of times matched */
} Hobj_t;

#define MINMATCH(vcpa)	((vcpa)->mmin > HMIN ? (vcpa)->mmin : HMIN)

#define INSERT(tb,ob)	((ob)->next = *(tb), *(tb) = (ob) )
#define DELETE(tb,p,ob) ((p) ? ((p)->next = (ob)->next) : (*(tb) = (ob)->next) )

#if __STD_C
static int hashparse(Vcparse_t* vcpa, ssize_t prune)
#else
static int hashparse(vcpa, prune)
Vcparse_t*	vcpa;
ssize_t		prune;	/* prune target by this amount	*/
#endif
{
	Vcchar_t	*ks, *ts, *et, *sm, *em;
	ssize_t		n, u, k, len, cnt;
	Hobj_t		*m, *p, *pos, *obj, *add, *src, *tar, **ht, **htab;
	int		type, tp, rvpa;
	Vchash_t	fk, rk, fm, rm, pk, mask; /* forward, reverse keys, hash mask */
	ssize_t		mmin = MINMATCH(vcpa); /* minimum match length */
	ssize_t		miss = vcpa->miss; /* amount of consecutive misses allowed */
	Vcchar_t	*map = vcpa->map; /* byte map table */

	/* allocate hash table to search for matches */
	n = vcpa->nsrc + vcpa->ntar;
	for(mask = 1<<10; mask < n/4; mask *= 2)
		;
	if(!(htab = (Hobj_t**)calloc(1, mask*sizeof(Hobj_t*) + n*sizeof(Hobj_t)) ) )
		return -1;
	src  = (Hobj_t*)(htab+mask);
	tar  = src + vcpa->nsrc;
	mask = mask-1;

	if(vcpa->nsrc >= mmin) /* add source data into hash table */
	{	et = (ks = vcpa->src) + vcpa->nsrc;
		fk = FKINIT(ks); pk = fk + (1<<16);
		for(m = src, ks += (HMIN-1); ks < et; pk = fk, ++ks, ++m)
			if((fk = FKNEXT(ks,fk)) != pk )
				{ ht = htab + VCINDEX(fk, mask); INSERT(ht, m); }
	}

	rvpa = 1; /* return value from vcpa->parsef */
	et = (ks = vcpa->tar) + vcpa->ntar; /* bounds of target string */
	fk = FKINIT(ks); fm = map ? FKINITM(ks,map) : fk;
	rk = RKINIT(ks); rm = map ? RKINITM(ks,map) : rk;
	for(add = obj = tar, ks += (HMIN-1); ks < et; ) /* parsing target string */
	{	fk = FKNEXT(ks,fk); fm = map ? FKNEXTM(ks,fm,map) : fk;
		rk = RKNEXT(ks,rk); rm = map ? RKNEXTM(ks,rm,map) : rk;

		pos = NIL(Hobj_t*); /* match position */
		len = 0; /* match length */
		cnt = 0; /* exact # of matching bytes */
		type = 0; /* match type */

		/* matching twice, once for plain data, once for mapped data */
		for(tp = 0; tp <= VC_MAP; tp += VC_MAP)
		{	if((tp == VC_MAP && !map) || (tp == 0 && (vcpa->type&VC_MAP)) )
				continue;

			ht = htab + (tp == 0 ? VCINDEX(fk,mask) : VCINDEX(fm,mask) );
			if(prune > 0) /* prune target data outside of search window */
			{	for(p = NIL(Hobj_t*), m = *ht; m && m >= tar; )
				{	if((obj - m) > prune)
						{ DELETE(ht,p,m); m = p ? p->next : *ht; }
					else	{ p = m; m = m->next; }
				}
			}

			for(m = *ht; m; m = m->next)
			{	ts = ks - (HMIN-1); /* starting point of string */

				if(m >= tar) /* possible target match */
				{	sm = vcpa->tar + (m - tar);
					em = vcpa->tar + vcpa->ntar;
				}
				else /* possible source match */
				{	sm = vcpa->src + (m - src);
					em = vcpa->src + vcpa->nsrc;
				}
				if((em-sm) > (et-ts) ) /* matchable bound */
					em = sm + (et-ts);
				if((em-sm) <= len )
					continue;

				if(!(vcpa->type&VC_APPROX) || miss <= 0) /* exact matching */
				{	if(tp == 0) /* match without mapping */
					{	if(sm[len] != ts[len] ||
						   sm[len>>1] != ts[len>>1])
							continue;
						for(; sm < em; ++sm, ++ts)
							if(*sm != *ts)
								break;
					}
					else /* match with mapping */
					{	if(sm[len] != map[ts[len]] ||
						   sm[len>>1] != map[ts[len>>1]])
							continue;
						for(; sm < em; ++sm, ++ts)
							if(*sm != map[*ts])
								break;
					}
					n = k = ts - (ks - (HMIN-1));
				}
				else /* approximate matching */
				{	if(tp == 0) /* no mapping */
					{	for(u = k = 0; sm < em; ++sm, ++ts)
						{	if(*sm == *ts )
								{ k += 1; u = 0; }
							else if(k > 0 && u < miss)
								{ u += 1; }
							else 	break;
						}
					}
					else /* with mapping */
					{	for(u = k = 0; sm < em; ++sm, ++ts)
						{	if(*sm == map[*ts] )
								{ k += 1; u = 0; }
							else if(k > 0 && u < miss)
								{ u += 1; }
							else 	break;
						}
					}
					n = ts - (ks - (HMIN-1)) - u;
				}

				if(n <= HMIN-1 || n < len || /* match too short */
				   (n == len && k < cnt) || /* #true matches too small */
				   (n == len && k == cnt && pos && m->cnt <= pos->cnt) )
					continue;
				pos = m;
				len = n;
				cnt = k;
				type = (k < n ? VC_APPROX : 0) | tp;
			}
		}

		/* reverse matching */
		for(tp = 0; (vcpa->type&VC_REVERSE) && tp <= VC_MAP; tp += VC_MAP)
		{	if((tp == VC_MAP && !map) || (tp == 0 && (vcpa->type&VC_MAP)) )
				continue;

			ht = htab + (tp == 0 ? VCINDEX(rk,mask) : VCINDEX(rm,mask) );
			if(prune > 0) /* prune target data outside of search window */
			{	for(p = NIL(Hobj_t*), m = *ht; m && m >= tar; )
				{	if((obj - m) > prune)
						{ DELETE(ht,p,m); m = p ? p->next : *ht; }
					else	{ p = m; m = m->next; }
				}
			}

			for(m = *ht; m; m = m->next)
			{	ts = ks - (HMIN-1); /* starting point of string */

				if(m >= tar) /* possible target match */
				{	sm = vcpa->tar + (m - tar) + (HMIN-1);
					em = vcpa->tar;
					if(sm >= ts)
						continue;
				}
				else /* possible source match */
				{	sm = vcpa->src + (m - src) + (HMIN-1);
					em = vcpa->src;
				}
				if((sm-em) > (et-ts) ) /* matchable bound */
					em = sm - (et-ts);
				if((sm-em+1) <= len )
					continue;

				if(!(vcpa->type&VC_APPROX) || miss <= 0) /* exact matching */
				{	if(tp == 0) /* match without mapping */
					{	if(sm[-len] != ts[len] ||
						   sm[-(len>>1)] != ts[len>>1])
							continue;	
						for(; sm >= em; --sm, ++ts)
							if(*sm != *ts)
								break;
					}
					else /* match with mapping */
					{	if(sm[-len] != map[ts[len]] ||
						   sm[-(len>>1)] != map[ts[len>>1]])
							continue;	
						for(; sm >= em; --sm, ++ts)
							if(*sm != map[*ts])
								break;
					}
					n = k = ts - (ks - (HMIN-1));
				}
				else /* approximate matching */
				{	if(tp == 0)
					{	for(u = k = 0; sm >= em; --sm, ++ts)
						{	if(*sm == *ts )
								{ k += 1; u = 0; }
							else if(k > 0 && u < miss)
								{ u += 1; }
							else 	break;
						}
					}
					else
					{	for(u = k = 0; sm >= em; --sm, ++ts)
						{	if(*sm == map[*ts] )
								{ k += 1; u = 0; }
							else if(k > 0 && u < miss)
								{ u += 1; }
							else 	break;
						}
					}
					n = ts - (ks - (HMIN-1)) - u;
				}

				if(n <= HMIN-1 || n < len || /* match too short */
				   (n == len && k < cnt) || /* #true matches too small */
				   (n == len && k == cnt && pos && m->cnt <= pos->cnt) )
					continue;
				pos = m + (HMIN-1);
				len = n;
				cnt = k;
				type = VC_REVERSE | (k < n ? VC_APPROX : 0) | tp;
			}
		}

		if(len >= mmin) /* a potential match */
		{	if((rvpa = (*vcpa->parsef)(vcpa,type,add-src,obj-src,len,pos-src)) < 0 )
				goto done;
			else if(rvpa == 0)
				goto nope;
			else	pos->cnt += 1; /* keep track of # of times matched */

			add = obj+len; /* new as yet unmatched position */

			for(pk = fk+1, em = ks+len;; ) /* add parsed data to htab */
			{	if(fk != pk )
					{ ht = htab+VCINDEX(fk,mask); INSERT(ht,obj); }
				pk = fk; obj += 1;
				if((ks += 1) >= em)
					break;
				fk = FKNEXT(ks,fk); fm = map ? FKNEXTM(ks,fm,map) : fk;
				rk = RKNEXT(ks,rk); rm = map ? RKNEXTM(ks,rm,map) : rk;
			}
		}
		else 
		{ nope:	rvpa = 1;
			ht = htab+VCINDEX(fk,mask); INSERT(ht,obj);
			obj += 1;
			ks  += 1;
		}
	}

	if(add < (tar+vcpa->ntar) )
		rvpa = (*vcpa->parsef)(vcpa, 0, add-src, vcpa->ntar+vcpa->nsrc, 0, -1);

done:	free(htab);
	return rvpa > 0 ? 0 : -1;
}

#if __STD_C
static int sfxparse(Vcparse_t* vcpa)
#else
static int sfxparse(vcpa)
Vcparse_t*	vcpa;
#endif
{
	ssize_t		p, r, ad;
	ssize_t		lp, lz, rp, rz, savp, savlp, savlz;
	ssize_t		*inv, *idx, nsrc, nstr;
	Vcchar_t	*s1, *s2, *ends, *str = NIL(Vcchar_t*);
	Vcsfx_t		*sfx = NIL(Vcsfx_t*);
	ssize_t		mmin = MINMATCH(vcpa);
	int		rv = -1;

	/* catenate source and target strings into a superstring */
	if((nsrc = vcpa->nsrc) == 0)
	{	nstr = vcpa->ntar;
		str = vcpa->tar;
	}
	else if(vcpa->tar == (vcpa->src + nsrc) )
	{	nstr = nsrc + vcpa->ntar;
		str = vcpa->src;
	}
	else
	{	nstr = nsrc + vcpa->ntar;
		if(!(str = (Vcchar_t*)malloc(nstr)) )
			return -1;
		memcpy(str, vcpa->src, nsrc);
		memcpy(str+nsrc, vcpa->tar, vcpa->ntar);
	}
	ends = str+nstr;

	if(!(sfx = vcsfxsort(str,nstr)) ) /* compute suffix array */
		goto done;
	idx = sfx->idx; inv = sfx->inv;

	for(savlz = savlp = savp = 0, ad = p = nsrc; p < nstr; )
	{	for(lz = lp = 0, r = inv[p]-1; r >= 0; --r)
		{	if((lp = idx[r]) < p) /* best left match */
			{	s1 = str+p; s2 = str+lp;
				for(; s1 < ends && *s1 == *s2; ++s1, ++s2 )
					lz += 1;
				if(lp < nsrc && (lp+lz) > nsrc)
					if((lz = nsrc-lp) < mmin )
						lp = lz = 0;
				break;
			}
		}

		for(rz = rp = 0, r = inv[p]+1; r < nstr; ++r)
		{	if((rp = idx[r]) < p) /* best right match */
			{	s1 = str+p; s2 = str+rp;
				for(; s1 < ends && *s1 == *s2; ++s1, ++s2 )
					rz += 1;
				if(rp < nsrc && (rp+rz) > nsrc)
					if((rz = nsrc-rp) < mmin )
						rp = rz = 0;
				break;
			}
		}

		if(rz > lz || (rz == lz && rp > lp) )
			{ lp = rp; lz = rz; } /* best match over all */
			
#define SEARCH	4 /* neighborhood to search for an improved match */ 
		if(savlz == 0 || (p-savp) < SEARCH)
		{	if(lz > savlz )
			{	savlp = lp; savlz = lz; savp = p;
				p += 1; /* improve! go again */
				continue;
			}
			if(savlz > 0 && lz >= (savlz-2) )
			{	p += 1; /* not a bad short, go again */
				continue;
			}
		}
		if(savlz > 0) /* no more improvement, use current best */
		{	lz = savlz; lp = savlp; p = savp;
			savlz = savlp = savp = 0;
		}

		if(lz >= mmin)
		{	if((r = (*vcpa->parsef)(vcpa, ad, p, lz, lp, 0)) < 0 )
				goto done;
			else if(r == 0)
				goto nope;
			else	ad = (p += lz); /* advance by match length */
		}
		else
		{ nope: p += 1; /* skip over an unmatched position */
		}
	} /**/ASSERT(p == nstr);

	if(ad < p && (r = (*vcpa->parsef)(vcpa, ad, p, 0, 0, 0)) <= 0 )
		goto done;

	rv = 0; /* if got here, data successfully parsed */

done:	if(str && str != vcpa->tar && str != vcpa->src)
		free(str);
	if(sfx)
		free(sfx);
	return rv;
}

#if __STD_C
int vcparse(Vcparse_t* vcpa, ssize_t prune)
#else
int vcparse(vcpa, prune )
Vcparse_t*	vcpa;
ssize_t		prune;	/* <0: sfxsort, >=0: hashing with pruning if >0 */
#endif
{
	/* error conditions */
	if(!vcpa->parsef)
		return -1;
	if(vcpa->nsrc < 0 || (vcpa->nsrc > 0 && !vcpa->src) )
		return -1;
	if(vcpa->ntar < 0 || (vcpa->ntar > 0 && !vcpa->tar) )
		return -1;

	if(vcpa->ntar == 0 ) /* nothing to do */
		return 0;

	if(vcpa->ntar < MINMATCH(vcpa) ) /* no match possible */
	{	if((*vcpa->parsef)(vcpa,vcpa->nsrc,vcpa->nsrc+vcpa->ntar,0,0,0) <= 0 )
			return -1;
		else	return 0;
	}

	if((vcpa->type & (VC_REVERSE|VC_APPROX|VC_MAP)) || prune >= 0 || vcpa->map )
		return hashparse(vcpa, prune);
	else	return sfxparse(vcpa);
}
