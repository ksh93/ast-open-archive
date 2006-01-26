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

#define MMIN			4
typedef unsigned _ast_int4_t	Hkey_t;
typedef struct _hobj_s		Hobj_t;
typedef struct _htab_s		Htab_t;
typedef struct _hash_s		Hash_t;

struct _hobj_s
{	Hobj_t*		next;
	Hkey_t		key;
};

struct _htab_s
{	Hobj_t*		list;
};

struct _hash_s
{	Vclz_t*		vclz;
	size_t		mask;	/* mask to index into htab[]	*/
	Htab_t*		htab;
	Hobj_t*		obj;
};

#define MAXDISTANCE	(64*1024)
#define INSERT(tb,ob)	\
	((ob)->next = (tb)->list, (tb)->list = (ob) )
#define DELETE(tb,p,ob) \
	((p) ? ((p)->next = (ob)->next) : ((tb)->list = (ob)->next) )

#if __STD_C
static int hashfold(Hash_t* hsh, ssize_t cmin, int target)
#else
static int hashfold(hsh, cmin, target)
Hash_t*		hsh;
ssize_t		cmin;
int		target;
#endif
{
	ssize_t		n, r;
	Hobj_t		*oe, *lo, *endo, *endl, *cm, *lm, *endm;
	Hobj_t		*obj, *tar, *src, *ad, *mt;
	Htab_t		*ht;
	Vcchar_t	*s;
	Hkey_t		key;
	size_t		mask;
	Vclz_t		*vclz = hsh->vclz;

	s = target ? vclz->tar  : vclz->src; /* string to be folded */
	n = target ? vclz->ntar : vclz->nsrc; /**/ASSERT(n >= MMIN);

	tar = (src = hsh->obj) + vclz->nsrc; /* source and target data */
	obj = target ? tar : src; /* data to be folded */
	endl = (endo = obj+n) - (MMIN-1);

	/* initialize keys for all positions */
	for(key = 0, n = 0; n < (MMIN-1); ++n, ++s)
		key = (key << 8) | s[0];
	for(oe = obj; oe < endl; ++oe, ++s)
		oe->key = key = (key << 8) | s[0];

	mask = hsh->mask; /* mask to index hash table */
	if(!target) /* source string */
	{	for(oe = obj; oe < endl; oe += 2)
		{	if(oe > obj && oe->key == (oe-1)->key )
				continue;
			ht = hsh->htab + VCINDEX(oe->key, mask);
			INSERT(ht, oe);
		}
		return 0;
	}

	for(ad = obj; obj < endo; )
	{	/* prune target data outside of search window */
		ht = hsh->htab + VCINDEX(obj->key, mask);
		for(lm = NIL(Hobj_t*), cm = ht->list; cm && cm >= tar; )
		{	if((obj-cm) < MAXDISTANCE)
				{ lm = cm; cm = cm->next; }
			else	{ lo = cm->next; DELETE(ht,lm,cm); cm = lo; }
		}

		for(n = 0, oe = obj, cm = ht->list; cm; cm = cm->next)
		{	endm = (cm < tar ? tar : endo) - (MMIN-1);
			if((cm+n) >= endm || /* quick filtering of unmatchables */
			   (cm+n)->key != oe->key || (cm+n/2)->key != (obj+n/2)->key )
				continue;

			/* match forward as far as possible */
			if((endl-obj) < (endm-cm) )
				endm -= (endm-cm) - (endl-obj);
			for(lm = cm, lo = obj; lm < endm; lm += MMIN, lo += MMIN )
				if(lm->key != lo->key)
					break;
			if(lo > oe) /* good one, get last few bytes too */
			{	lm -= (MMIN-1); lo -= (MMIN-1);
				for(; lm < endm; lm += 1, lo += 1)
					if(lm->key != lo->key)
						break;

				mt = cm; /* matching object */
				oe = lo; /* matched to here */
				n = oe-obj;
			}
		}

		if(n > 0) /* a potential match */
		{	if((n += (MMIN-1)) < cmin) /* not long enough */
				goto nope;

			if((obj-ad) > 0)
			{	if((r = (*vclz->lzf)(vclz, ad-src, obj-ad, -1)) < 0 )
					return -1;
				else if(r == 0)
					goto nope;
			}

			if((r = (*vclz->lzf)(vclz, obj-src, n, mt-src)) < 0 )
				return -1;
			else if(r == 0)
				goto nope;

			/* add positions at start and end of match to search table */
			for(endm = obj+MMIN; obj < endm; ++obj)
			{	ht = hsh->htab + VCINDEX(obj->key, mask);
				INSERT(ht, obj);
			}

			obj = obj > oe ? obj : oe;
			endm = (oe += MMIN-1) < endl ? oe : endl;
			for(; obj < endm; obj++)
			{	ht = hsh->htab + VCINDEX(obj->key, mask);
				INSERT(ht, obj);
			}

			ad = obj = oe; /* restart search at end of match */
		}
		else 
		{ nope: ht = hsh->htab + VCINDEX(obj->key, mask);
			INSERT(ht, obj); 
			obj += 1; /* restart search at next position */
		}
	} /**/ASSERT(obj == endo);

	if((obj-ad) > 0 && (*vclz->lzf)(vclz, ad-src, obj-ad, -1) <= 0 )
		return -1;

	return 0;
}

#if __STD_C
static int hashparse(Vclz_t* vclz, ssize_t cmin)
#else
static int hashparse(vclz, cmin)
Vclz_t*		vclz;
ssize_t		cmin;
#endif
{
	ssize_t		k, n;
	Hash_t		*hsh;

	/* allocate table to process data */
	n = vclz->nsrc + vclz->ntar;
	for(k = 1<<10; k < n/4; k *= 2)
		;
	n = sizeof(Hash_t) + n*sizeof(Hobj_t) + k*sizeof(Htab_t);
	if(!(hsh = (Hash_t*)calloc(1, n)) )
		return -1;

	hsh->vclz = vclz;
	hsh->mask = k-1;
	hsh->htab = (Htab_t*)(hsh+1);
	hsh->obj  = (Hobj_t*)(hsh->htab + k);

	if(vclz->nsrc >= cmin && hashfold(hsh, cmin, 0) < 0)
		return -1;
	if(hashfold(hsh, cmin, 1) < 0)
		return -1;

	free(hsh);
	return 0;
}

#if __STD_C
static int sfxparse(Vclz_t* vclz, ssize_t cmin)
#else
static int sfxparse(vclz, cmin)
Vclz_t*		vclz;
ssize_t		cmin;
#endif
{
	ssize_t		p, r, ad;
	ssize_t		lp, lz, rp, rz, savp, savlp, savlz;
	ssize_t		*inv, *idx, nsrc, nstr;
	Vcchar_t	*s1, *s2, *ends, *str = NIL(Vcchar_t*);
	Vcsfx_t		*sfx = NIL(Vcsfx_t*);
	int		rv = -1;

	/* catenate source and target strings into a superstring */
	if((nsrc = vclz->nsrc) == 0)
	{	nstr = vclz->ntar;
		str = vclz->tar;
	}
	else if(vclz->tar == (vclz->src + nsrc) )
	{	nstr = nsrc + vclz->ntar;
		str = vclz->src;
	}
	else
	{	nstr = nsrc + vclz->ntar;
		if(!(str = (Vcchar_t*)malloc(nstr)) )
			return -1;
		memcpy(str, vclz->src, nsrc);
		memcpy(str+nsrc, vclz->tar, vclz->ntar);
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
					if((lz = nsrc-lp) < cmin )
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
					if((rz = nsrc-rp) < cmin )
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

		if(lz >= cmin)
		{	if((p-ad) > 0)
			{	if((r = (*vclz->lzf)(vclz, ad, p-ad, -1)) < 0 )
					goto done;
				else if(r == 0)
					goto nope;
			}
			if((r = (*vclz->lzf)(vclz, p, lz, lp)) < 0 )
				goto done;
			else if(r == 0)
				goto nope;
			ad = (p += lz); /* advance by match length */
		}
		else
		{ nope: p += 1; /* skip over an unmatched position */
		}
	} /**/ASSERT(p == nstr);

	if((p-ad) > 0 && (r = (*vclz->lzf)(vclz, ad, p-ad, -1)) <= 0 )
		goto done;

	rv = 0; /* if got here, data successfully parsed */

done:	if(str && str != vclz->tar && str != vclz->src)
		free(str);
	if(sfx)
		free(sfx);
	return rv;
}

#if __STD_C
int vclzparse(Vclz_t* vclz, ssize_t cmin, int type )
#else
int vclzparse(vclz, cmin, type )
Vclz_t*		vclz;
ssize_t		cmin;	/* minimum acceptable COPYs	*/
int		type;	/* != 0 for suffix-sorting	*/
#endif
{
	/* error conditions */
	if(!vclz->lzf)
		return -1;
	if(vclz->nsrc < 0 ||
	   (vclz->nsrc == 0 && vclz->src) ||
	   (vclz->nsrc > 0 && !vclz->src) )
		return -1;
	if(vclz->ntar > 0 && !vclz->tar)
		return -1;

	if(vclz->ntar <= 0 ) /* nothing to do */
		return 0;

	cmin = cmin > 0 ? cmin : 1;
	if(vclz->ntar < cmin ) /* no match possible */
		return (*vclz->lzf)(vclz, vclz->nsrc, vclz->ntar, -1) <= 0 ? -1 : 0;

	if(type != 0)
		return sfxparse(vclz, cmin);
	else	return hashparse(vclz, cmin);
}
