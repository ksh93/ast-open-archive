/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1985-2001 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*                 This software was created by the                 *
*                 Network Services Research Center                 *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                David Korn <dgk@research.att.com>                 *
*                 Phong Vo <kpv@research.att.com>                  *
*******************************************************************/
#pragma prototyped
/*
 * AT&T Research
 *
 * convert string to long integer
 * if non-null e will point to first unrecognized char in s
 * if basep!=0 it points to the default base on input and
 * will point to the explicit base on return
 * a default base of 0 will determine the base from the input
 * a default base of 1 will determine the base from the input using bb#*
 * a base prefix in the string overrides *b
 * *b will not be set if the string has no base prefix
 * if m>1 and no multipler was specified then the result is multiplied by m
 * if m<0 then multipliers are not consumed
 *
 * integer numbers are of the form:
 *
 *	[sign][base][number[qualifier]][multiplier]
 *
 *	base:		nnn#		base nnn (no multiplier)
 *			0[xX]		hex
 *			0		octal
 *			[1-9]		decimal
 *
 *	number:		[0-9a-zA-Z]*
 *
 *	qualifier:	[lL]
 *			[uU]
 *			[uU][lL] 
 *			[lL][uU]
 *			[lL][lL][uU]
 *			[uU][lL][lL]
 *
 *	multiplier:	.		pseudo-float (100) + subsequent digits
 *			[bB]		block (512)
 *			[cC]		char (1)
 *			[gG]		giga (1024*1024*1024)
 *			[kK]		kilo (1024)
 *			[mM]		mega (1024*1024)
 */

#include <ast.h>
#include <ctype.h>

#include "sfhdr.h"

#define QL		01
#define QU		02

long
strton(const char* a, char** e, char* basep, int m)
{
	register unsigned char*	s = (unsigned char*)a;
	register long		n;
	register int		c;
	register int		base;
	register int		shift;
	register unsigned char*	p;
	register unsigned char*	cv;
	int			negative;

	if (!basep || (base = *basep) < 0 || base > 64)
		base = 0;
	while (isspace(*s))
		s++;
	if ((negative = (*s == '-')) || *s == '+')
		s++;
	p = s;
	if (base <= 1)
	{
		if ((c = *p++) >= '0' && c <= '9')
		{
			n = c - '0';
			if ((c = *p) >= '0' && c <= '9')
			{
				n = n * 10 + c - '0';
				p++;
			}
			if (*p == '#')
			{
				if (n >= 2 && n <= 64)
				{
					s = p + 1;
					base = n;
				}
			}
			else if (base)
				base = 0;
			else if (*s == '0')
			{
				if ((c = *(s + 1)) == 'x' || c == 'X')
				{
					s += 2;
					base = 16;
				}
				else if (c >= '0' && c <= '7')
				{
					s++;
					base = 8;
				}
			}
		}
		if (basep && base)
			*basep = base;
	}
	if (base >= 2 && base <= SF_RADIX)
		m = -1;
	else
		base = 10;

	/*
	 * this part transcribed from sfvscanf()
	 */

	n = 0;
	if (base == 10)
	{
		while ((c = *s++) >= '0' && c <= '9')
			n = (n << 3) + (n << 1) + (c - '0');
	}
	else
	{
		SFCVINIT();
		cv = base <= 36 ? _Sfcv36 : _Sfcv64;
		if ((base & ~(base - 1)) == base)
		{	
			if (base < 8)
				shift = base <  4 ? 1 : 2;
			else if (base < 32)
				shift = base < 16 ? 3 : 4;
			else
				shift = base < 64 ? 5 : 6;
			while ((c = cv[*s++]) < base)
				n = (n << shift) + c;
		}
		else while ((c = cv[*s++]) < base)
			n = (n * base) + c;
		c = *(s - 1);
	}
	if (s > (unsigned char*)(a + 1))
	{
		/*
		 * gobble the optional qualifier
		 */

		base = 0;
		for (;;)
		{
			if (!(base & QL) && (c == 'l' || c == 'L'))
			{
				base |= QL;
				c = *++s;
				if (c == 'l' || c == 'L')
					c = *++s;
				continue;
			}
			if (!(base & QU) && (c == 'u' || c == 'U'))
			{
				base |= QU;
				c = *++s;
				continue;
			}
			break;
		}
	}

	/*
	 * apply suffix multiplier
	 */

	if (m < 0 || s == (unsigned char*)(a + 1))
		s--;
	else switch (c)
	{
	case 'b':
	case 'B':
		n *= 512;
		break;
	case 'c':
	case 'C':
		break;
	case 'g':
	case 'G':
		n *= 1024 * 1024 * 1024;
		break;
	case 'k':
	case 'K':
		n *= 1024;
		break;
	case 'l':
	case 'L':
		n *= 4;
		break;
	case 'm':
	case 'M':
		n *= 1024 * 1024;
		break;
	case 'q':
	case 'Q':
		n *= 8;
		break;
	case 'w':
	case 'W':
		n *= 2;
		break;
	case '.':
		n *= 100;
		for (m = 10; *s >= '0' && *s <= '9'; m /= 10) 
			n += m * (*s++ - '0');
		break;
	default:
		s--;
		if (m > 1)
			n *= m;
		break;
	}
	if (e)
		*e = (char*)s;
	return negative ? -n : n;
}
