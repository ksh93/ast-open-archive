/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1989-2001 AT&T Corp.                *
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
*                 Eduardo Krell <ekrell@adexus.cl>                 *
*******************************************************************/
#pragma prototyped

/*
 * 3d buffer io
 */

#include "3d.h"

/*
 * stripped down printf -- only { %c %[l[l]][dopux] %s }
 */

int
bvprintf(char** buf, char* end, register const char* format, va_list ap)
{
	register int	c;
	register char*	p;
	register char*	e;
	int		w;
	int		l;
	int		f;
	int		g;
	int		r;
	long		n;
	unsigned long	u;
#ifdef int_8
	int_8		q;
	unsigned int_8	v;
#endif
	char*		s;
	char*		b;
	char*		x;
	char		num[32];

	static char	digits[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLNMOPQRSTUVWXYZ@_";

	if (buf)
	{
		p = *buf;
		e = end;
	}
	else e = (p = end) + SHRT_MAX;
	for (;;)
	{
		switch (c = *format++)
		{
		case 0:
			goto done;
		case '%':
			if (*format == '-')
			{
				format++;
				l = 1;
			}
			else l = 0;
			if (*format == '0')
			{
				format++;
				f = l ? ' ' : '0';
			}
			else f = ' ';
			if ((c = *format) == '*')
			{
				format++;
				w = va_arg(ap, int);
			}
			else
			{
				w = 0;
				while (c >= '0' && c <= '9')
				{
					w = w * 10 + c - '0';
					c = *++format;
				}
			}
			r = 0;
			if (c == '.')
			{
				if ((c = *++format) == '*')
				{
					format++;
					va_arg(ap, int);
				}
				else while (c >= '0' && c <= '9') c = *++format;
				if (c == '.')
				{
					if ((c = *++format) == '*')
					{
						format++;
						r = va_arg(ap, int);
					}
					else while (c >= '0' && c <= '9')
					{
						r = r * 10 + c - '0';
						c = *++format;
					}
				}
			}
			if ((c = *format++) != 'l')
				n = 0;
			else if ((c = *format++) != 'l')
				n = 1;
			else
			{
				n = 2;
				c = *format++;
			}
			g = 0;
			b = num;
			switch (c)
			{
			case 0:
				break;
			case 'c':
				*b++ = va_arg(ap, int);
				break;
			case 'd':
				switch (n)
				{
				case 0:
					n = va_arg(ap, int);
					break;
				default:
#ifdef int_8
					q = va_arg(ap, int_8);
					if (q < 0)
					{
						g = '-';
						q = -q;
					}
					v = q;
					goto dec_8;
				case 1:
#endif
					n = va_arg(ap, long);
					break;
				}
				if (n < 0)
				{
					g = '-';
					n = -n;
				}
				u = n;
				goto dec;
			case 'o':
				switch (n)
				{
				case 0:
					u = va_arg(ap, unsigned int);
					break;
				default:
#ifdef int_8
					v = va_arg(ap, unsigned int_8);
					goto oct_8;
				case 1:
#endif
					u = va_arg(ap, unsigned long);
					break;
				}
				do *b++ = (u & 07) + '0'; while (u >>= 3);
				break;
			case 's':
				s = va_arg(ap, char*);
				if (!s) s = "(null)";
				if (w)
				{
					n = w - strlen(s);
					if (l)
					{
						while (w-- > 0)
						{
							if (p >= e) goto done;
							if (!(*p = *s++)) break;
							p++;
						}
						while (n-- > 0)
						{
							if (p >= e) goto done;
							*p++ = f;
						}
						continue;
					}
					while (n-- > 0)
					{
						if (p >= e) goto done;
						*p++ = f;
					}
				}
				for (;;)
				{
					if (p >= e) goto done;
					if (!(*p = *s++)) break;
					p++;
				}
				continue;
			case 'u':
				switch (n)
				{
				case 0:
					u = va_arg(ap, unsigned int);
					break;
				default:
#ifdef int_8
					v = va_arg(ap, unsigned int_8);
					goto dec_8;
				case 1:
#endif
					u = va_arg(ap, unsigned long);
					break;
				}
			dec:
				if (r <= 0 || r >= sizeof(digits)) r = 10;
				do *b++ = digits[u % r]; while (u /= r);
				break;
			case 'p':
				if (x = va_arg(ap, char*))
				{
					g = 'x';
					w = 10;
					f = '0';
					l = 0;
				}
#ifdef int_8
				if (sizeof(char*) == sizeof(int_8))
				{
					v = (unsigned int_8)x;
					goto hex_8;
				}
#endif
				u = (unsigned long)x;
				goto hex;
			case 'x':
				switch (n)
				{
				case 0:
					u = va_arg(ap, unsigned int);
					break;
				default:
#ifdef int_8
					v = va_arg(ap, unsigned int_8);
					goto hex_8;
				case 1:
#endif
					u = va_arg(ap, unsigned long);
					break;
				}
			hex:
				do *b++ = ((n = (u & 0xf)) >= 0xa) ? n - 0xa + 'a' : n + '0'; while (u >>= 4);
				break;
			default:
				if (p >= e) goto done;
				*p++ = c;
				continue;
#ifdef int_8
			dec_8:
				if (r <= 0 || r >= sizeof(digits)) r = 10;
				do *b++ = digits[v % r]; while (v /= r);
				break;
			hex_8:
				do *b++ = ((n = (v & 0xf)) >= 0xa) ? n - 0xa + 'a' : n + '0'; while (v >>= 4);
				break;
			oct_8:
				do *b++ = (v & 07) + '0'; while (v >>= 3);
				break;
#endif
			}
			if (w)
			{
				if (g == 'x') w -= 2;
				else if (g) w -= 1;
				n = w - (b - num);
				if (!l)
				{
					if (g && f != ' ')
					{
						if (g == 'x')
						{
							if (p >= e) goto done;
							*p++ = '0';
							if (p >= e) goto done;
							*p++ = 'x';
						}
						else if (p >= e) goto done;
						else *p++ = g;
						g = 0;
					}
					while (n-- > 0)
					{
						if (p >= e) goto done;
						*p++ = f;
					}
				}
			}
			if (g == 'x')
			{
				if (p >= e) goto done;
				*p++ = '0';
				if (p >= e) goto done;
				*p++ = 'x';
			}
			else if (g)
			{
				if (p >= e) goto done;
				*p++ = g;
			}
			while (b > num)
			{
				if (p >= e) goto done;
				*p++ = *--b;
			}
			if (w && l) while (n-- > 0)
			{
				if (p >= e) goto done;
				*p++ = f;
			}
			continue;
		default:
			if (p >= e) goto done;
			*p++ = c;
			continue;
		}
		break;
	}
 done:
	if (p < e) *p = 0;
	if (buf)
	{
		c = p - *buf;
		*buf = p;
	}
	else c = p - end;
	return(c);
}

int
bprintf(char** buf, char* end, const char* format, ...)
{
	va_list	ap;
	int	n;

	va_start(ap, format);
	n = bvprintf(buf, end, format, ap);
	va_end(ap);
	return(n);
}

int
sfsprintf(char* buffer, int size, const char* format, ...)
{
	va_list	ap;
	char**	buf;
	char*	end;
	int	n;

	va_start(ap, format);
	if (size)
	{
		buf = &buffer;
		end = buffer + size;
	}
	else
	{
		buf = 0;
		end = buffer;
	}
	n = bvprintf(buf, end, format, ap);
	va_end(ap);
	return(n);
}
