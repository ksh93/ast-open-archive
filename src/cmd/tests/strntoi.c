/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1999-2002 AT&T Corp.                *
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
 *	strntol		strntoul
 *	strntoll	strntoull
 */

#include <ast.h>
#include <error.h>

#ifndef ERANGE
#define ERANGE	EINVAL
#endif

main(int argc, char** argv)
{
	char*			s;
	char*			p;
	unsigned long		l;
	unsigned _ast_intmax_t	ll;
	int			sep = 0;
	size_t			size = 0;

	while (s = *++argv)
	{
		if (strneq(s, "LC_ALL=", 7))
		{
			if (!setlocale(LC_ALL, s + 7))
			{
				sfprintf(sfstdout, "%s failed\n", s);
				return 0;
			}
			continue;
		}
		if (!size)
		{
			size = atoi(s);
			continue;
		}
		if (sep)
			sfprintf(sfstdout, "\n");
		else
			sep = 1;

		errno = 0;
		l = strntol(s, size, &p, 0);
		sfprintf(sfstdout, "strntol   \"%s\" \"%s\" %I*d %s\n", s, p, sizeof(l), l, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");

		errno = 0;
		l = strntoul(s, size, &p, 0);
		sfprintf(sfstdout, "strntoul  \"%s\" \"%s\" %I*u %s\n", s, p, sizeof(l), l, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");

		errno = 0;
		ll = strntoll(s, size, &p, 0);
		sfprintf(sfstdout, "strntoll  \"%s\" \"%s\" %I*d %s\n", s, p, sizeof(ll), ll, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");

		errno = 0;
		ll = strntoull(s, size, &p, 0);
		sfprintf(sfstdout, "strntoull \"%s\" \"%s\" %I*u %s\n", s, p, sizeof(ll), ll, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");

		size = 0;
	}
	return 0;
}
