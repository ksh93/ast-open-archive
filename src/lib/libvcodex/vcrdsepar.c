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

/* Algorithm to guess the field and record separators in a flat file database.
** The basic observation is that if the data is indeed from a table, the data
** diversity should be less in each field than in the overall table due to fields
** having different types. Thus, the entropy coding sizes over all fields should
** be less than the entropy coding size over all data. The method is then to look
** for candidate field &| record separators that minimize entropy coding. 
**
** Written by Kiem-Phong Vo.
*/

typedef struct _fsep_s
{	ssize_t		recf;	/* # records with separator	*/
	ssize_t		recm;	/* # of records with max# occ	*/
	ssize_t		maxf;	/* max # occurence in a record	*/
} Fsep_t;

typedef struct _entropy_s
{	ssize_t		size;	/* total number of elements	*/
	ssize_t		freq[256]; /* their frequencies		*/
} Entropy_t;

typedef struct _separ_s
{	Dtlink_t	link;
	Vcrdsepar_t	rds;
} Separ_t;

#if __STD_C
int separcmp(Dt_t* sepdt, Void_t* one, Void_t* two, Dtdisc_t* sepdc)
#else
int separcmp(Dt_t* sepdt, Void_t* one, Void_t* two, Dtdisc_t* sepdc)
#endif
{
	Separ_t	*s1 = (Separ_t*)one;
	Separ_t	*s2 = (Separ_t*)two;
	double	r1, r2;

	/* empty slots are always worse */
	if(s1->rds.allz == 0.)
	{	if(s2->rds.allz == 0.)
			return s1 < s2 ? -1 : 1;
		else	return 1;
	}
	if(s2->rds.allz == 0.)
		return -1;

	/* prefer average small record size */
	r1 = ((double)s1->rds.size) / s1->rds.recn;
	r2 = ((double)s2->rds.size) / s2->rds.recn;
	if(2*r1 < r2)
		return -1;
	if(r1 > 2*r2)
		return 1;

	/* prefer field separators to appear uniformly in records */
	r1 = ((double)(s1->rds.recn - s1->rds.recf)) / s1->rds.recn;
	r2 = ((double)(s2->rds.recn - s2->rds.recf)) / s2->rds.recn;
	if(2*r1 < r2)
		return -1;
	if(r1 > 2*r2)
		return 1;

	/* prefer better compression rates */
	r1 = s1->rds.fldz/s1->rds.allz;
	r2 = s2->rds.fldz/s2->rds.allz;
	if(r1 == r2)
		return s1 < s2 ? -1 : 1;
	else	return r1 < r2 ? -1 : 1;
}

#if __STD_C
static int fsepar(Dt_t* sepdt, Vcchar_t* data, ssize_t dtsz, int rs)
#else
static int fsepar(sepdt, data, dtsz, rs)
Dt_t*		sepdt;	/* dictionary of separators	*/
Vcchar_t*	data;	/* data to guess separators	*/
ssize_t		dtsz;	/* size of data			*/
int		rs;	/* record separator to use	*/
#endif
{
	Fsep_t		fsep[256];
	ssize_t		freq[256], allf[256], recn, size, nsep, z, f;
	int		fs;
	double		ez, fldz, allz;
	Separ_t		*sep, tmp;
	Entropy_t	*et;

	if(!sepdt || !data || dtsz <= 0 || rs < 0 || rs >= 256)
		return -1;

	/* compute statistics for all possible separators */
	memset(fsep, 0, sizeof(fsep));
	memset(allf, 0, sizeof(allf));
	for(recn = 0, size = 0;;)
	{	for(z = size; z < dtsz; ++z)
			if(data[z] == rs)
				break;
		if(z >= dtsz) /* incomplete record */
			break;

		memset(freq, 0, sizeof(freq));	
		for(; size < z; ++size)
		{	freq[data[size]] += 1;
			allf[data[size]] += 1;
		}

		size += 1; /* amount of processed data */
		recn += 1; /* # of complete records */

		/* statistics of fsep occurences */
		for(fs = 0; fs < 256; ++fs)
		{	if(freq[fs] == 0 ) /* fs is not a separator in this record */
				continue;

			if(freq[fs] > fsep[fs].maxf) /* update max # of occurences */
			{	fsep[fs].maxf = freq[fs];
				fsep[fs].recm = 0;
			}
			fsep[fs].recf += 1; /* # of records containing fs */
			fsep[fs].recm += freq[fs] == fsep[fs].maxf ? 1 : 0; /* #records with maxf */
		}
	}

	/* see if worth further checking */
	if(recn <= 4 || size < 3*dtsz/4 )
		return 0;

	/* to reduce computation, consider only credible candidates */
	for(f = 0, fs = 0; fs < 256; ++fs)
	{	if(fsep[fs].recf*100 < recn*95) /* field separator must appear in most records */
			fsep[fs].recf = fsep[fs].maxf = 0;
		else if(fsep[fs].recm*100 < fsep[fs].recf*95) /* most records must be at maxf */
			fsep[fs].recf = fsep[fs].maxf = 0;
		else if(fsep[fs].maxf > f) /* max number of fields over all candidates fsep  */
			f = fsep[fs].maxf;
	}
	if(f <= 0) /* no separator found */
		return 0;

	/* compute the entropy coding size of all data */
	allf[rs] = recn;
	for(ez = 0., fs = 0; fs < 256; ++fs)
		if(allf[fs] > 0)
			ez += allf[fs] * vclog(allf[fs]);
	if((allz = size*vclog(size) - ez) <= 0.)
		return 0;

	/* proto element to search for */
	tmp.rds.rsep = rs;
	tmp.rds.size = size;
	tmp.rds.recn = recn;
	tmp.rds.allz = allz;

	/* compute nrds best candidates for entropy coding size */
	if(!(et = (Entropy_t*)malloc((f+1)*sizeof(Entropy_t))) )
		return -1;
	for(nsep = 0, fs = 0; fs < 256; ++fs)
	{	if(fsep[fs].maxf <= 0)
			continue;

		memset(et, 0, (fsep[fs].maxf+1)*sizeof(Entropy_t));
		for(f = 0, z = 0; z < size; ++z)
		{	et[f].freq[data[z]] += 1;
			et[f].size += 1;

			if(data[z] == rs) /* see record separator, reset field index */
				f = 0;
			else if(data[z] == fs) /* see field separator, advance field index */
				f += 1;
		}

		/* compute the total coding size for all fields if fs is a separator */
		for(fldz = 0., f = 0; f <= fsep[fs].maxf; ++f)
		{	for(ez = 0., z = 0; z < 256; ++z)
				if(et[f].freq[z] > 0)
					ez += et[f].freq[z] * vclog(et[f].freq[z]);
			fldz += et[f].size*vclog(et[f].size) - ez;
		}

		if(fldz*100 > 95*allz) /* see if entropy coding sufficiently improves */
			continue;

		/* replace the current worst entry if better than it */
		tmp.rds.fsep = fs;
		tmp.rds.recf = fsep[fs].recf;
		tmp.rds.fldz = fldz;
		sep = (Separ_t*)dtlast(sepdt);
		if(separcmp(sepdt, &tmp, sep, 0) < 0 )
		{	dtdelete(sepdt, sep);
			memcpy(sep, &tmp, sizeof(Separ_t));
			dtinsert(sepdt, sep);
			nsep += 1;
		}
	}
	free(et); /* deallocate temporary memory */

	return nsep;
}

#if __STD_C
ssize_t vcrdsepar(Vcrdsepar_t* rds, ssize_t nrds, Vcchar_t* data, ssize_t dtsz, int rs)
#else
ssize_t vcrdsepar(rds, nrds, data, dtsz, rs)
Vcrdsepar_t*	rds;	/* array to return separators	*/
ssize_t		nrds;	/* # of slots in rds[]		*/
Vcchar_t*	data;	/* data to check for separators	*/
ssize_t		dtsz;	/* size of data			*/
int		rs;	/* candidate record separator	*/
#endif
{
	Separ_t		*sep, *list;
	ssize_t		k;
	Dt_t		*sepdt;
	Dtdisc_t	sepdc;

	if(!rds || nrds <= 0 || !data || dtsz <= 0)
		return -1;

	/* construct dictionary of separators to find the best one */
	if(!(list = (Separ_t*)calloc(nrds, sizeof(Separ_t))) )
		return -1;
	DTDISC(&sepdc, 0, 0, 0, 0, 0, separcmp, 0, 0, 0); 
	if(!(sepdt = dtopen(&sepdc, Dtoset)) )
	{	free(list);
		return -1;
	}
	for(k = 0; k < nrds; ++k)
		dtinsert(sepdt, list+k);

	if(rs >= 0)
		fsepar(sepdt, data, dtsz, rs);
	else 
	{	rs = '\n'; 
		do
		{	if(fsepar(sepdt, data, dtsz, rs) > 0)
			{	/* stop early if result is good */
				sep = (Separ_t*)dtfirst(sepdt);
				if(sep->rds.allz > 0. && sep->rds.fldz*100 < sep->rds.allz*75)
					break;
			}
		} while((rs = (rs+1)%256) != '\n');
	}

	for(k = 0; k < nrds; ++k) /* return the good candidates */
	{	if(!(sep = (Separ_t*)dtfirst(sepdt)) || sep->rds.allz <= 0. )
			break;

		/* if average record size dramatically increases, stop */
		if(k > 0 && sep->rds.size/sep->rds.recn > 2*rds[k-1].size/rds[k-1].recn)
			break;

		memcpy(rds+k, &sep->rds, sizeof(Vcrdsepar_t));
		dtdelete(sepdt, sep);
	}

	dtclose(sepdt);
	free(list);

	return k;
}
