/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*           Copyright (c) 1999-2007 AT&T Knowledge Ventures            *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                      by AT&T Knowledge Ventures                      *
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
 *	strntol		strntoul
 *	strntoll	strntoull
 */

#include <ast.h>
#include <error.h>

#ifndef ERANGE
#define ERANGE	EINVAL
#endif

int
main(int argc, char** argv)
{
	char*			s;
	char*			p;
	unsigned long		l;
	uintmax_t		ll;
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
