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

/* Compute the highest period in all quasi-cycles of a data set.
**
** Written by Kiem-Phong Vo
*/

#if __STD_C
ssize_t vcperiod(const Void_t* data, size_t dtsz)
#else
ssize_t vcperiod(data, dtsz)
Void_t*	data;
size_t	dtsz;
#endif
{
	Vcchar_t	*dt;
	ssize_t		*len, *peak, *lcp;
	ssize_t		i, n, k, p, m, pm, np, l, pl, pmin;
	Vcsfx_t		*sfx;

#define BOUND	4 /* bound to check for local peaks */
	if(!data || dtsz <= BOUND*BOUND)
		return -1;

	/* space for the length accumulator */
	if(!(lcp = (ssize_t*)calloc(1,dtsz*sizeof(ssize_t))) )
		return -1;

	/* suffix array and lcp array */
	if(!(sfx = vcsfxsort(data,dtsz)) )
	{	free(lcp);
		return -1;
	}
	for(dt = (Vcchar_t*)data, p = 0, i = 0; i < dtsz; ++i)
	{	if(sfx->inv[i] == 0)
			continue;
		k = sfx->idx[sfx->inv[i]-1];
		while(dt[i+p] == dt[k+p])
			p += 1;
		lcp[sfx->inv[i]] = p;
		if(p > 0)
			p -= 1;
	}

	/* populating the length array */
	len = sfx->inv; memset(len, 0, dtsz*sizeof(ssize_t));
	for(i = 0; i < dtsz-1; ++i)
	{	for(m = 0, k = i+1; k < dtsz; ++k)
		{	if(lcp[k] == 0)
				break;
			if((m = sfx->idx[k] - sfx->idx[i]) > 0 )
				break;
		}
		for(n = 0, p = i-1; p >= 0; --p)
		{	if(lcp[p] == 0)
				break;
			if((n = sfx->idx[p] - sfx->idx[i]) > 0 )
				break;
		}
		if(m > 0 && m < n)
			len[m] += 1;
		else if(n > 0 && n < m)
			len[n] += 1;
	}

	/* compute the peaks and the max peak */
	pmin = (ssize_t)vclog(dtsz);
#define PEAK(x,mt)	(len[x] > pmin && len[x] > (mt-len[x]) )
	peak = sfx->idx; memset(peak, 0, dtsz*sizeof(ssize_t));
	for(m = len[0]+len[1], i = 2; i < 2*BOUND; ++i)
	{	m += len[i]; /* running sum of a neighborhood */
		if((i%2) == 0 && (k = i/2) >= 2 )
			if(PEAK(k, m))
				peak[k] = 1;
	}
	for(n = dtsz-(BOUND+1), i = BOUND; i < n; ++i)
	{	m += len[i+BOUND];
		if(PEAK(i,m))
			peak[i] = 1;
		m -= len[i-BOUND];
	}

	p = -1; pm = pl = 0;
	for(n = dtsz/2, i = 2; i < n; ++i)
	{	if(!peak[i])
			continue;

#define PSKIP	2 /* allow skipping this many non-peaks */
		m = 0; l = 0; /* sum weights of peaks in progression */
		for(np = 0, k = i; k < dtsz; k += i)
		{	if(!peak[k])
			{	if((np += 1) > PSKIP)
					break;
			}
			else
			{	np = 0;
				m += 1;
				l += len[k];
			}
		}

		if(p <= 0 || (l > pl && (i%p != 0 || m > pm) ) )
		{	p = i;
			pm = m;
			pl = l;
		}
	}

#ifdef DEBUG
{	extern char* getenv _ARG_((char*)); char *period = getenv("PERIOD");
	 	if(period && period[0] && (i = atoi(period)) > 0) p = i;

	PRINT(3, "period=%d\n", p); PRINT(3, "dtsz=%d\n", dtsz);
	for(i = 2; i < dtsz; ++i)
		if(len[i] > 0)
			{ PRINT(3,"%d: ",i); PRINT(3,"%d\n",len[i]); }
}
#endif

	free(lcp);
	free(sfx);

	PRINT(4, "p=%d\n",p);
	return p;
}
