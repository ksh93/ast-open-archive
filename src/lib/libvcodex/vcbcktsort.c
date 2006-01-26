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

/*	Counting bucket sort.
**	Return the number of distinct bytes. The bckt[] argument returns
**	the cumulative counts of all bytes. Thus, bckt[0] is the count
**	of byte 0, bckt[1] is the cumulative count of 0 and 1, and so on.
**
**	Written by Kiem-Phong Vo.
*/

#if __STD_C
ssize_t vcbcktsort(ssize_t* idx, ssize_t* list, ssize_t n, Vcchar_t* data, ssize_t* bckt)
#else
ssize_t vcbcktsort(idx, list, n, data, bckt)
ssize_t*	idx;	/* output sorted idxes	*/
ssize_t*	list;	/* indices to be sorted	*/
ssize_t		n;	/* # of indices		*/
Vcchar_t*	data;	/* data used to sort	*/
ssize_t*	bckt;	/* [256] buckets	*/
#endif
{
	ssize_t		i, p, c;
	ssize_t		distinct = 0;

	/* count byte frequencies */
	memset(bckt, 0, 256*sizeof(ssize_t));
	if(list) /* sort using secondary predictor */
	{	for(p = 0; p < n; ++p)
			bckt[data[list[p]]] += 1;
	}
	else /* unsorted permutation was the identity */
	{	for(p = 0; p < n; ++p)
			bckt[data[p]] += 1;
	}

	for(p = 0, i = 0; i < 256; ++i) /* starting positions */
	{	if((c = bckt[i]) > 0)
			distinct += 1;
		bckt[i] = p;
		p += c;
	}

	if(list) /* sorting a sublist of indices */
	{	for(p = 0; p < n; ++p)
			idx[bckt[data[list[p]]]++] = list[p];
	}
	else /* sorting all indices */
	{	for(p = 0; p < n; ++p)
			idx[bckt[data[p]]++] = p;
	}

	return distinct;
}
