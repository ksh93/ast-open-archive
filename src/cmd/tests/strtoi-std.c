/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1999-2005 AT&T Corp.                  *
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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * AT&T Labs Research
 *
 * test harness for
 *
 *	strtol		strtoul
 *	strtoll		strtoull
 */

#include <stdio.h>
#include <errno.h>

#ifndef ERANGE
#define ERANGE	EINVAL
#endif

main(int argc, char** argv)
{
	char*			s;
	char*			p;
	unsigned long		l;
	unsigned long long	ll;
	int			sep = 0;

	while (s = *++argv)
	{
		if (sep)
			printf("\n");
		else
			sep = 1;

		errno = 0;
		l = strtol(s, &p, 0);
		printf("strtol   \"%s\" \"%s\" %ld %s\n", s, p, l, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");

		errno = 0;
		l = strtoul(s, &p, 0);
		printf("strtoul  \"%s\" \"%s\" %lu %s\n", s, p, l, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");

		errno = 0;
		ll = strtoll(s, &p, 0);
		printf("strtoll  \"%s\" \"%s\" %lld %s\n", s, p, ll, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");

		errno = 0;
		ll = strtoull(s, &p, 0);
		printf("strtoull \"%s\" \"%s\" %llu %s\n", s, p, ll, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");
	}
	return 0;
}
