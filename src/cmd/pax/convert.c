/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1987-2005 AT&T Corp.                  *
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
 * Glenn Fowler
 * AT&T Bell Laboratories
 *
 * pax conversion support
 */

#include "pax.h"

/*
 * set char code conversion for section
 */

void
convert(Archive_t* ap, int section, int from, int to)
{
	if (ap->convert[section].on = (ap->convert[section].f2t = ccmap(from, to)) != 0)
	{
		ap->convert[section].t2f = ccmap(to, from);
		ap->convert[section].f2a = ccmap(from, CC_ASCII);
		ap->convert[section].t2a = ccmap(to, CC_ASCII);
	}
}

/*
 * convert string to lower case in place
 */

char*
strlower(register char* s)
{
	register int	c;
	register char*	t;

	for (t = s; c = *t; t++)
		if (isupper(c))
			*t = tolower(c);
	return s;
}

/*
 * convert string to upper case in place
 */

char*
strupper(register char* s)
{
	register int	c;
	register char*	t;

	for (t = s; c = *t; t++)
		if (islower(c))
			*t = toupper(c);
	return s;
}

/*
 * return 1 if s is a portable string
 */

int
portable(Archive_t* ap, const char* s)
{
	register unsigned char*	u = (unsigned char*)s;
	register unsigned char*	m;
	register int		c;

	if (!ap->convert[SECTION_CONTROL].on)
	{
		while (c = *u++)
			if (c > 0177)
				return 0;
	}
	else
	{
		m = ap->convert[SECTION_CONTROL].f2t;
		while (c = m[*u++])
			if (c > 0177)
				return 0;
	}
	return 1;
}

/*
 * this is an obsolete version of the ast library implementation
 */

#undef	OHASHPART
#define OHASHPART(b,h,c,l,r)	(h = ((h = ((unsigned)h << (l)) ^ (h >> (r)) ^ (c)) & ((unsigned)1 << (b)) ? ~h : h) & (((((unsigned)1 << ((b) - 1)) - 1) << 2) | 3))

#undef	OHASHLPART
#define OHASHLPART(h,c)		OHASHPART(31, h, c, 3, 2)

unsigned long
omemsum(const void* b, int n, register unsigned long c)
{
	register unsigned char*	p;
	register unsigned char*	e;

	p = (unsigned char*)b;
	e = p + n;
	while (p < e) OHASHLPART(c, *p++ + 1);
	return c;
}
