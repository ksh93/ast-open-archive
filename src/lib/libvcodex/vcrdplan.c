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
#include	<graph.h>

/*	Transforming relational data for better compression.
**
**	Written by Binh Vo and Kiem-Phong Vo (05/17/2006).
*/

/* compute the length of the common prefix of two strings */
#if __STD_C
static ssize_t prefix(Vcchar_t* str1, ssize_t n1, Vcchar_t* str2, ssize_t n2)
#else
static ssize_t prefix(str1, n1, str2, n2)
Vcchar_t*	str1;
ssize_t		n1;
Vcchar_t*	str2;
ssize_t		n2;
#endif
{
	n1 = n1 > n2 ? n2 : n1;
	for(n2 = 0; n2 < n1; ++n2)
		if(str1[n2] != str2[n2])
			break;
	return n2;
}

/* Get the weight of a field after permuting records by some transform vector */
#if __STD_C
ssize_t vcrdweight(Vcrdtable_t* tbl, ssize_t f, ssize_t* vect, Vcrdweight_f wghtf)
#else
ssize_t vcrdweight(tbl, f, vect, wghtf)
Vcrdtable_t*	tbl;	/* table of all records and fields	*/
ssize_t		f;	/* field to compute weight		*/
ssize_t*	vect;	/* permutation vector, NULL for ident	*/
Vcrdweight_f	wghtf;	/* weight function, NULL for prefix()	*/
#endif
{
	ssize_t		psz, csz, wght, recn, r, k;
	Vcchar_t	*pdt, *cdt;
	Vcrdrecord_t	*rcrd;

	if(!tbl || f < 0 || f >= tbl->fldn)
		return -1;

	if(!wghtf) /* if no weight function specified, use prefix */
		wghtf = prefix;
	if((recn = tbl->recn) <= 0)
		return 0;

	pdt = NIL(Vcchar_t*); psz = 0;
	for(wght = 0, rcrd = tbl->fld[f].rcrd, r = 0; r < recn; ++r)
	{	/* compute current record and length */
		if((k = vect ? vect[r] : r) < 0 || k >= recn)
			return -1;
		cdt = rcrd[k].data; csz = rcrd[k].dtsz;
		wght += (*wghtf)(pdt, psz, cdt, csz);
		pdt = cdt; psz = csz;
	}

	return wght;
}

/* training to compute a plan for data transforming */
#if __STD_C
Vcrdplan_t* vcrdmakeplan(Vcrdtable_t* tbl, Vcrdweight_f wghtf)
#else
Vcrdplan_t* vcrdmakeplan(tbl, wghtf)
Vcrdtable_t*	tbl;	/* table of data	*/
Vcrdweight_t	wghtf;	/* weight function	*/
#endif
{
	Grnode_t	*nd, *pd;
	Gredge_t	*e;
	Graph_t		*gr;
	ssize_t		f, p, z;
	ssize_t		fldn, recn, rown;
	ssize_t		*wght; /* field weights */
	Vcrdplan_t	*pl;

	if(!tbl || (fldn = tbl->fldn) <= 0)  /* nothing to do */
		return NIL(Vcrdplan_t*);
	recn = tbl->recn;

#define FACTOR	10 /* use a small number of rows to construct the plan */
	if((rown = fldn*fldn) < recn/FACTOR)
		rown = recn/FACTOR;
	else if(rown > recn)
		rown = recn;

	/* construct the identity plan */
	if(!(pl = (Vcrdplan_t*)malloc(sizeof(Vcrdplan_t) + (fldn-1)*sizeof(ssize_t))) )
		return NIL(Vcrdplan_t*);
	pl->fldn = fldn;
	for(f = 0; f < fldn; ++f)
		pl->pred[f] = f;

	if(rown <= 0 || wghtf == VCRD_NOPLAN) /* no transformation */
		return pl;

	/* allocate space for intermediate computation */
	if(!(wght = (ssize_t*)malloc(fldn*sizeof(ssize_t))) )
	{	free(pl);
		return NIL(Vcrdplan_t*);
	}

	tbl->recn = rown; /* restrict data for training */

	/* compute the self-weight and transform vector of each field */
	for(f = 0; f < fldn; ++f)
	{	wght[f] = vcrdweight(tbl, f, NIL(ssize_t*), wghtf);
		if(vcrdvector(tbl, f, NIL(Vcchar_t*), 0, VC_ENCODE) == 1)
			wght[f] = 0; /* this won't transform anything */
	}

	/* compute the maximum branching that determines the optimum dependency relations */
	if(!(gr = gropen(NIL(Grdisc_t*), GR_DIRECTED)) )
		goto done;
	for(f = 0; f < fldn; ++f)
	{	/* create the node corresponding to this column */
		if(!(nd = grnode(gr, (Void_t*)f, 1)) )
			goto done;
		if(wght[f] <= 0) /* reorder won't do anything */
			continue;

		for(p = 0; p < f; ++p) /* build graph edges */
		{	if(wght[p] <= 0)
				continue;

			if(!(pd = grnode(gr, (Void_t*)p, 0)) )
				goto done;
			if((z = vcrdweight(tbl, f, tbl->fld[p].vect, wghtf)) > wght[f])
			{	if(!(e = gredge(gr, pd, nd, (Void_t*)0, 1)) )
					goto done;
				grbrweight(e, z);
			}
			if((z = vcrdweight(tbl, p, tbl->fld[f].vect, wghtf)) > wght[p])
			{	if(!(e = gredge(gr, nd, pd, (Void_t*)0, 1)) )
					goto done;
				grbrweight(e, z);
			}
		}
	}
	if((z = grbranching(gr)) < 0 )
		goto done; /**/DEBUG_PRINT(2,"Grbranching weight = %d\n", z);

	/* the transform plan */
	for(nd = dtfirst(gr->nodes); nd; nd = dtnext(gr->nodes,nd))
		if(nd->iedge)
			pl->pred[(ssize_t)nd->label] = (ssize_t)nd->iedge->tail->label;

done:	grclose(gr);
	free(wght);

	if(rown != recn) /* restore original data */
	{	tbl->recn = recn;
		for(f = 0; f < fldn; ++f)
			tbl->fld[f].type &= ~VCRD_VECTOR;
	}

	return pl;
}

#if __STD_C
void vcrdfreeplan(Vcrdplan_t* pl)
#else
void vcrdfreeplan(pl)
Vcrdplan_t*	pl;
#endif
{
	if(pl)
		free(pl);
}

/* transform a field based on a transform vector */
#if __STD_C
static void fldtransform(Vcrdtable_t* tbl, ssize_t f, ssize_t* vect, Vcrdrecord_t* rtmp, int type)
#else
static void fldtransform(tbl, f, vect, rtmp, type)
Vcrdtable_t*	tbl;	/* table data		*/
ssize_t		f;	/* field to transform	*/
ssize_t*	vect;	/* transform vector	*/
Vcrdrecord_t*	rtmp;	/* temp record space	*/
int		type;	/* VC_ENCODE/DECODE	*/
#endif
{
	ssize_t		r, recn = tbl->recn;
	Vcrdrecord_t	*rcrd = tbl->fld[f].rcrd;

	if(type == VC_ENCODE)
	{	for(r = 0; r < recn; ++r)
		{	rtmp[r].data = rcrd[vect[r]].data; 
			rtmp[r].dtsz = rcrd[vect[r]].dtsz; 
		}
	}
	else
	{	for(r = 0; r < recn; ++r)
		{	rtmp[vect[r]].data = rcrd[r].data; 
			rtmp[vect[r]].dtsz = rcrd[r].dtsz; 
		}
	}

	memcpy(rcrd, rtmp, recn*sizeof(Vcrdrecord_t));
}

/* reconstruct a field based on its predictor. */
#if __STD_C
static int fldinvert(Vcrdtable_t* tbl, Vcrdplan_t* pl, ssize_t f, ssize_t level, Vcrdrecord_t* rtmp)
#else
static int fldinvert(tbl, pl, f, level, rtmp)
Vcrdtable_t*	tbl;	/* table data		*/
Vcrdplan_t*	pl;	/* transform plan	*/
ssize_t		f;	/* field to rebuild	*/
ssize_t		level;	/* recursion level	*/
Vcrdrecord_t*	rtmp;	/* temp record space	*/
#endif
{
	ssize_t		p;

	if(level >= pl->fldn) /* data was corrupted */
		return -1;

	if((p = pl->pred[f]) == f) /* field already rebuilt */
		return 0;

	/* recursively invert predictor of f */
	if(fldinvert(tbl, pl, p, level+1, rtmp) < 0 )
		return -1;

	/* construct transform vector */
	if(vcrdvector(tbl, p, NIL(Vcchar_t*), 0, VC_DECODE) < 0 )
		return -1;

	/* now invert field f to its original data order */
	fldtransform(tbl, f, tbl->fld[p].vect, rtmp, VC_DECODE);
	pl->pred[f] = f;

	return 0;
}

#if __STD_C
int vcrdexecplan(Vcrdtable_t* tbl, Vcrdplan_t* pl, int type)
#else
int vcrdexecplan(tbl, pl, type)
Vcrdtable_t*	tbl;	/* table data			*/
Vcrdplan_t*	pl;	/* transform plan for data	*/
int		type;	/* VC_ENCODE or VC_DECODE	*/
#endif
{
	ssize_t		fldn, recn, f, p;
	Vcrdrecord_t	*rtmp;

	if(!tbl || !pl || tbl->fldn != pl->fldn )
		return -1;
	if(type != VC_ENCODE && type != VC_DECODE)
		return -1;

	if((fldn = tbl->fldn) <= 0 || (recn = tbl->recn) <= 0)
		return 0;

	if(!(rtmp = (Vcrdrecord_t*)malloc(recn*sizeof(Vcrdrecord_t))) )
		return -1;

	for(f = 0; f < fldn; ++f) /* no transform vector done yet */
		tbl->fld[f].type &= ~VCRD_VECTOR;

	if(type == VC_ENCODE)
	{	for(f = 0; f < fldn; ++f) /* build needed transform vectors first */
			if((p = pl->pred[f]) != f)
				vcrdvector(tbl, p, NIL(Vcchar_t*), 0, VC_ENCODE );
		for(f = 0; f < fldn; ++f) /* now transform fields with predictors */
			if((p = pl->pred[f]) != f)
				fldtransform(tbl, f, tbl->fld[p].vect, rtmp, VC_ENCODE);
	}
	else
	{	for(f = 0; f < fldn; ++f)
			if(fldinvert(tbl, pl, f, 0, rtmp) < 0 )
				break;
	}

	free(rtmp);

	return f < fldn ? -1 : 0;
}
