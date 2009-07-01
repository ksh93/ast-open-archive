/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2009 AT&T Intellectual Property          *
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

int	ilist[] = {3, 4, 4, 5, 1, 1, 2, 2, 7, 6};
int	isort[] = {1, 1, 2, 2, 3, 4, 4, 5, 6, 7};

int intcmp(Void_t* one, Void_t* two, Void_t* disc)
{
	int	i1 = *((int*)one), i2 = *((int*)two);
	return i1 - i2;
}

main()
{
	int	i;

	vcqsort(ilist, sizeof(ilist)/sizeof(int), sizeof(int), intcmp, 0);
	for(i = 0; i < sizeof(ilist)/sizeof(int); ++i)
	{	if(ilist[i] != isort[i])
			terror("vcqsort() failed");
	}

	exit(0);
}
