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
tar_checksum(Archive_t* ap, int check, long sum)
{
	register unsigned char*		p;
	register unsigned char*		e;
	register unsigned char*		t;
	register unsigned long		u;
	register unsigned long		s;
	register long			c;
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
	u = 0;
	s = 0;
	p = (unsigned char*)tar_block;
	e = p + TAR_HEADER;
	if (!ap->convert[SECTION_CONTROL].on)
		while (p < e)
		{
			c = *p++;
			u += c;
			if (check)
			{
				if (c & 0x80)
					c |= (-1) << 8;
				s += c;
			}
		}
	else
	{
		map = (state.operation & IN) ? ap->convert[SECTION_CONTROL].t2f : ap->convert[SECTION_CONTROL].f2t;
		while (p < e)
		{
			c = map[*p++];
			u += c;
			if (check)
			{
				if (c & 0x80)
					c |= (-1) << 8;
				s += c;
			}
		}
	}
	p = (unsigned char*)tar_header.chksum;
	e = p + sizeof(tar_header.chksum);
	t = tmp;
	while (p < e)
		*p++ = *t++;
	u &= TAR_SUMASK;
	if (check)
	{
		if ((sum &= TAR_SUMASK) == u)
			return 1;
		if (sum == (s &= TAR_SUMASK))
		{
			if (!ap->old.warned)
			{
				ap->old.warned = 1;
				error(1, "%s: %s format archive generated with signed checksums", ap->name, format[ap->format].name);
			}
			return 1;
		}
		if (ap->entry > 1)
			error(state.keepgoing ? 1 : 3, "%s: %s format checksum error (%ld != %ld or %ld)", ap->name, format[ap->format].name, sum, u, s);
		return 0;
	}
	return u;
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
