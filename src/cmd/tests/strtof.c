/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1999-2008 AT&T Intellectual Property          *
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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * AT&T Research
 *
 * test harness for
 *
 *	strtod		strtold
 *	strntod		strntold
 */

#if _PACKAGE_ast
#include <ast.h>
#else
#ifndef _ISOC99_SOURCE
#define _ISOC99_SOURCE	1
#endif
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <locale.h>
#include <float.h>

#ifndef ERANGE
#define ERANGE	EINVAL
#endif

#ifndef errno
extern int	errno;
#endif

#if !_PACKAGE_ast
#define _ast_fltmax_t	long double
#endif

#ifndef LDBL_DIG
#define LDBL_DIG	DBL_DIG
#endif

int
main(int argc, char** argv)
{
	char*			s;
	char*			p;
	double			d;
	_ast_fltmax_t		ld;
	int			sep = 0;
#if _PACKAGE_ast
	int			n;
#endif

	if (argc <= 1)
	{
		printf("%u/%u\n", DBL_DIG, LDBL_DIG);
		return 0;
	}
	while (s = *++argv)
	{
		if (!strncmp(s, "LC_ALL=", 7))
		{
			if (!setlocale(LC_ALL, s + 7))
			{
				printf("%s failed\n", s);
				return 0;
			}
			continue;
		}
		if (sep)
			printf("\n");
		else
			sep = 1;

		errno = 0;
		d = strtod(s, &p);
		printf("strtod   \"%s\" \"%s\" %.15e %s\n", s, p, d, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");

		errno = 0;
		ld = strtold(s, &p);
		printf("strtold  \"%s\" \"%s\" %.31Le %s\n", s, p, ld, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");

#if _PACKAGE_ast
		n = strlen(s);

		errno = 0;
		d = strntod(s, n, &p);
		printf("strntod  %2d \"%-.*s\" \"%s\" %.15e %s\n", n, n, s, p, d, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");

		errno = 0;
		d = strntod(s, n - 1, &p);
		printf("strntod  %2d \"%-.*s\" \"%s\" %.15e %s\n", n - 1, n - 1, s, p, d, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");

		errno = 0;
		ld = strntold(s, n, &p);
		printf("strntold %2d \"%-.*s\" \"%s\" %.31Le %s\n", n, n, s, p, ld, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");

		errno = 0;
		ld = strntold(s, n - 1, &p);
		printf("strntold %2d \"%-.*s\" \"%s\" %.31Le %s\n", n - 1, n - 1, s, p, ld, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");

#endif

	}
	return 0;
}
