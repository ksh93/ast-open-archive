/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1999-2004 AT&T Corp.                  *
*                      and is licensed under the                       *
*          Common Public License, Version 1.0 (the "License")          *
*                        by AT&T Corp. ("AT&T")                        *
*      Any use, downloading, reproduction or distribution of this      *
*      software constitutes acceptance of the License.  A copy of      *
*                     the License is available at                      *
*                                                                      *
*         http://www.research.att.com/sw/license/cpl-1.0.html          *
*         (with md5 checksum 8a5e0081c856944e76c69a1cf29c2e8b)         *
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
 *	strtol		strtoul		strton
 *	strtoll		strtoull	strtonll
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
	char			b;
	int			sep = 0;

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
		if (sep)
			sfprintf(sfstdout, "\n");
		else
			sep = 1;

		errno = 0;
		l = strtol(s, &p, 0);
		sfprintf(sfstdout, "strtol   \"%s\" \"%s\" %I*d %s\n", s, p, sizeof(l), l, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");

		errno = 0;
		b = 0;
		l = strton(s, &p, &b, 0);
		sfprintf(sfstdout, "strton   \"%s\" \"%s\" %I*d %s %d\n", s, p, sizeof(l), l, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR", b);

		errno = 0;
		l = strtoul(s, &p, 0);
		sfprintf(sfstdout, "strtoul  \"%s\" \"%s\" %I*u %s\n", s, p, sizeof(l), l, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");

		errno = 0;
		ll = strtoll(s, &p, 0);
		sfprintf(sfstdout, "strtoll  \"%s\" \"%s\" %I*d %s\n", s, p, sizeof(ll), ll, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");

		errno = 0;
		b = 0;
		ll = strtonll(s, &p, &b, 0);
		sfprintf(sfstdout, "strtonll \"%s\" \"%s\" %I*d %s %d\n", s, p, sizeof(ll), ll, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR", b);

		errno = 0;
		ll = strtoull(s, &p, 0);
		sfprintf(sfstdout, "strtoull \"%s\" \"%s\" %I*u %s\n", s, p, sizeof(ll), ll, errno == 0 ? "OK" : errno == ERANGE ? "ERANGE" : errno == EINVAL ? "EINVAL" : "ERROR");
	}
	return 0;
}
