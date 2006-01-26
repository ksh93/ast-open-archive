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

/*	Like qsort() but allows a struct to hold add'l data describing objects.
**
**	Written by Kiem-Phong Vo.
*/

#define SWAP(le, re, ne) \
do {	Vcchar_t *ll = (Vcchar_t*)(le); \
	Vcchar_t *rr = (Vcchar_t*)(re); \
	ssize_t   nn = (ne); \
	for(; nn > 0; --nn, ++ll, ++rr) \
		{ int ss = *ll; *ll = *rr; *rr = ss; } \
} while(0)

#if __STD_C
void vcqsort(Void_t* list, ssize_t n, ssize_t size, Vcqsort_f sortf, Void_t* disc)
#else
Void_t vcqsort(list, n, size, sortf, disc)
Void_t*		list;	/* list of objects to be sorted	*/
ssize_t		n;	/* number of objects in list[]	*/
ssize_t		size;	/* size in byte of each object	*/
Vcqsort_f	sortf;	/* comparison function		*/
Void_t*		disc;	/* adjunct struct for sortf()	*/
#endif
{
	ssize_t		l, r;
	Vcchar_t	*base = (Vcchar_t*)list;

	if(n <= 1)
		return;

	if(n == 2)
	{	if((*sortf)(base, base+size, disc) > 0)
			SWAP(base, base+size, size);
		return;
	}

	for(l = 1, r = n; l < r; ) /* pivot on element 0 */
	{	if((*sortf)(base, base + l*size, disc) >= 0)
			l += 1;
		else if((r -= 1) > l)
			SWAP(base + l*size, base + r*size, size);
	}

	if((l -= 1) > 0) /* move the pivot into its final place */
		SWAP(base, base + l*size, size);

	if(l > 1)
		vcqsort(base, l, size, sortf, disc);

	if((n -= r) > 1)
		vcqsort(base + r*size, n, size, sortf, disc);
}
