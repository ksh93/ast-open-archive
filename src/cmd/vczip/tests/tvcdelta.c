/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2010 AT&T Intellectual Property          *
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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#include	"vctest.h"

int main()
{
	char		*src, *tar, *del, *t;
	Vcdisc_t	disc;
	Vcodex_t	*vc;
	ssize_t		k, n;

	src = "the big cat and the little cats jump around the big sleeping dog";
	tar = "the little dogs and the big dog jump around the sleeping big cat";

	VCDISC(&disc, src, strlen(src), 0);

	if(!(vc = vcopen(&disc, Vcdelta, 0, 0, VC_ENCODE)) )
		terror("Cannot open Vcdelta handle");

	if((n = vcapply(vc, tar, strlen(tar), &del)) <= 0)
		terror("Vcdelta failed");
	if(n >= strlen(tar) )
		terror("Did not compress data");

	if(!(vc = vcopen(&disc, Vcdelta, 0, 0, VC_DECODE)) )
		terror("Cannot open decoding handle");
	if((n = vcapply(vc, del, n, &t)) != strlen(tar) )
		terror("Decoding returns wrong size");
	for(k = 0; k < n; ++k)
		if(t[k] != tar[k])
			terror("Decoding failed");

	exit(0);
}
