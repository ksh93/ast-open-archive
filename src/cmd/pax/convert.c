/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1987-2003 AT&T Corp.                *
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
	ap->convert[section].on =
		(ap->convert[section].from = from) !=
		(ap->convert[section].to = to);
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
 * convert binary header shorts to long
 */

long
cpio_long(register unsigned short* s)
{
	Integral_t	u;

	u.l = 1;
	if (u.c[0])
	{
		u.s[0] = s[1];
		u.s[1] = s[0];
	}
	else
	{
		u.s[0] = s[0];
		u.s[1] = s[1];
	}
	return u.l;
}

/*
 * convert long to binary header shorts
 */

void
cpio_short(register unsigned short* s, long n)
{
	Integral_t	u;

	u.l = 1;
	if (u.c[0])
	{
		u.l = n;
		s[0] = u.s[1];
		s[1] = u.s[0];
	}
	else
	{
		u.l = n;
		s[0] = u.s[0];
		s[1] = u.s[1];
	}
}

/*
 * compute tar_header checksum
 */

long
tar_checksum(Archive_t* ap)
{
	register unsigned char*		p;
	register unsigned char*		e;
	register unsigned char*		t;
	register unsigned long		n;
	register const unsigned char*	map;
	unsigned char			tmp[sizeof(tar_header.chksum)];

	p = (unsigned char*)tar_header.chksum;
	e = p + sizeof(tar_header.chksum);
	t = tmp;
	while (p < e)
	{
		*t++ = *p;
		*p++ = ' ';
	}
	n = 0;
	p = (unsigned char*)tar_block;
	e = p + TAR_HEADER;
	if (!ap->convert[SECTION_CONTROL].on)
		while (p < e)
			n += *p++;
	else
	{
		map = (state.operation & IN) ? CCMAP(ap->convert[SECTION_CONTROL].to, ap->convert[SECTION_CONTROL].from) : CCMAP(ap->convert[SECTION_CONTROL].from, ap->convert[SECTION_CONTROL].to);
		while (p < e)
			n += map[*p++];
	}
	p = (unsigned char*)tar_header.chksum;
	e = p + sizeof(tar_header.chksum);
	t = tmp;
	while (p < e)
		*p++ = *t++;
	return n & TAR_SUMASK;
}

/*
 * compute running s5r4 file content checksum
 */

long
asc_checksum(char* ab, int n, register unsigned long sum)
{
	register unsigned char* b = (unsigned char*)ab;
	register unsigned char*	e;

	e = b + n;
	while (b < e) sum += *b++;
	return sum;
}

/*
 * get label header number
 */

long
getlabnum(register char* p, int byte, int width, int base)
{
	register char*	e;
	register int	c;
	long		n;

	p += byte - 1;
	c = *(e = p + width);
	*e = 0;
	n = strtol(p, NiL, base);
	*e = c;
	return n;
}

/*
 * get label header string
 */

char*
getlabstr(register char* p, int byte, int width, register char* s)
{

	register char*	e;
	char*		v;

	v = s;
	p += byte - 1;
	e = p + width;
	while (p < e && (*s = *p++) != ' ') s++;
	*s = 0;
	return v;
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
