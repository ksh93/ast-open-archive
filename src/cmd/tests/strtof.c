/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1999-2004 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*    If you have copied or used this software without agreeing     *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
#pragma prototyped

/*
 * AT&T Labs Research
 *
 * test harness for
 *
 *	strtod		strtold
 */

#if _PACKAGE_ast
#include <ast.h>
#else
#define _ast_fltmax_t	long double
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <locale.h>

#ifndef ERANGE
#define ERANGE	EINVAL
#endif

#ifndef errno
extern int	errno;
#endif

#if !_PACKAGE_ast
#undef	printf
#endif

main(int argc, char** argv)
{
	char*			s;
	char*			p;
	double			d;
	_ast_fltmax_t		ld;
	int			sep = 0;

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

	}
	return 0;
}
