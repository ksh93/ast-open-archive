/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1990-2002 AT&T Corp.                *
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
 * system call message buffer support
 */

#include "msglib.h"

/*
 * get encoded unsigned long from buf
 */

unsigned long
msggetu(char** buf, char* end)
{
	register int		c;
	register unsigned char*	s;
	register unsigned char*	e;
	register unsigned long	v;

	v = 0;
	s = (unsigned char*)*buf;
	e = (unsigned char*)end;
	do
	{
		if (s >= e) return -1L;
		c = *s++;
		v = (v << 7) | (c & 0177);
	} while (c & 0200);
	*buf = (char*)s;
	return v;
}

/*
 * encode unsigned long into buf
 */

int
msgputu(char** buf, char* end, register unsigned long v)
{
	register char*	s;
	register char*	t;
	register int	n;
	char		tmp[2 * sizeof(unsigned long)];

	s = t = &tmp[elementsof(tmp) - 1];
	*t = v & 0177;
	while((v >>= 7) > 0)
		*--t = (v & 0177) | 0200;
	n = s - t + 1;
	if ((s = *buf) + n >= end)
		return 0;
	switch (n)
	{
	default: for (v = n - 7; v-- > 0;) *s++ = *t++;
	case 7:	*s++ = *t++;
	case 6:	*s++ = *t++;
	case 5:	*s++ = *t++;
	case 4:	*s++ = *t++;
	case 3:	*s++ = *t++;
	case 2:	*s++ = *t++;
	case 1:	*s++ = *t++;
	}
	*buf = s;
	return n;
}

/*
 * copy <data,size> from buf
 */

size_t
msggetz(char** buf, char* end, void* data, size_t size)
{
	register size_t	n;
	register size_t	i;

	i = end - *buf;
	n = msggetu(buf, end);
	if (n > i) n = i;
	if (size > n) size = n;
	if (size) memcpy(data, *buf, size);
	*buf += n;
	return n;
}

/*
 * copy <data,size> to buf
 */

int
msgputz(char** buf, char* end, void* data, size_t size)
{
	register char*	s;
	register size_t	n;

	s = *buf;
	n = end - s;
	if (n > size) n = size;
	msgputu(buf, end, n);
	if (n)
	{
		if (n > (end - *buf))
		{
			n = end - *buf;
			*buf = s;
			msgputu(buf, end, n);
		}
		memcpy(*buf, data, n);
		*buf += n;
	}
	return *buf - s;
}
