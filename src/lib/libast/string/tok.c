/*
 * CDE - Common Desktop Environment
 *
 * Copyright (c) 1993-2012, The Open Group. All rights reserved.
 *
 * These libraries and programs are free software; you can
 * redistribute them and/or modify them under the terms of the GNU
 * Lesser General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * These libraries and programs are distributed in the hope that
 * they will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with these librararies and programs; if not, write
 * to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
 * Floor, Boston, MA 02110-1301 USA
 */
/***************************************************************
*                                                              *
*                      AT&T - PROPRIETARY                      *
*                                                              *
*         THIS IS PROPRIETARY SOURCE CODE LICENSED BY          *
*                          AT&T CORP.                          *
*                                                              *
*                Copyright (c) 1995 AT&T Corp.                 *
*                     All Rights Reserved                      *
*                                                              *
*           This software is licensed by AT&T Corp.            *
*       under the terms and conditions of the license in       *
*       http://www.research.att.com/orgs/ssr/book/reuse        *
*                                                              *
*               This software was created by the               *
*           Software Engineering Research Department           *
*                    AT&T Bell Laboratories                    *
*                                                              *
*               For further information contact                *
*                     gsf@research.att.com                     *
*                                                              *
***************************************************************/

/* : : generated by proto : : */

#if !defined(__PROTO__)
#if defined(__STDC__) || defined(__cplusplus) || defined(_proto) || defined(c_plusplus)
#if defined(__cplusplus)
#define __MANGLE__	"C"
#else
#define __MANGLE__
#endif
#define __STDARG__
#define __PROTO__(x)	x
#define __OTORP__(x)
#define __PARAM__(n,o)	n
#if !defined(__STDC__) && !defined(__cplusplus)
#if !defined(c_plusplus)
#define const
#endif
#define signed
#define void		int
#define volatile
#define __V_		char
#else
#define __V_		void
#endif
#else
#define __PROTO__(x)	()
#define __OTORP__(x)	x
#define __PARAM__(n,o)	o
#define __MANGLE__
#define __V_		char
#define const
#define signed
#define void		int
#define volatile
#endif
#if defined(__cplusplus) || defined(c_plusplus)
#define __VARARG__	...
#else
#define __VARARG__
#endif
#if defined(__STDARG__)
#define __VA_START__(p,a)	va_start(p,a)
#else
#define __VA_START__(p,a)	va_start(p)
#endif
#endif
#include <ast.h>

struct tok				/* token stream state		*/
{
	union
	{
	char*		end;		/* end ('\0') of last token	*/
	struct tok*	nxt;		/* next in free list		*/
	}		ptr;
	char		chr;		/* replace *end with this	*/
	char		flg;		/* ==0 => don't restore string	*/
};

typedef struct tok TOK;

static struct tok*	freelist;

/*
 * open a new token stream on s
 * if f==0 then string is not restored
 */

char*
tokopen __PARAM__((register char* s, int f), (s, f)) __OTORP__(register char* s; int f;){
	register TOK*	p;

	if (p = freelist) freelist = freelist->ptr.nxt;
	else if (!(p = newof(0, TOK, 1, 0))) return(0);
	p->chr = *(p->ptr.end = s);
	p->flg = f;
	return((char*)p);
}

/*
 * close a token stream
 * restore the string to its original state
 */

void
tokclose __PARAM__((char* u), (u)) __OTORP__(char* u;){
	register TOK*	p = (TOK*)u;

	if (p->flg) *p->ptr.end = p->chr;
	p->ptr.nxt = freelist;
	freelist = p;
}

/*
 * return next space separated token
 * "\n" is returned as a token
 * 0 returned when no tokens remain
 * "..." and '...' quotes are honored with \ escapes
 */

char*
tokread __PARAM__((char* u), (u)) __OTORP__(char* u;){
	register TOK*	p = (TOK*)u;
	register char*	s;
	register char*	r;
	register int	q;
	register int	c;

	/*
	 * restore string on each call
	 */

	if (!p->chr) return(0);
	s = p->ptr.end;
	if (p->flg) *s = p->chr;
	else if (!*s) s++;

	/*
	 * skip leading space
	 */

	while (*s == ' ' || *s == '\t') s++;
	if (!*s)
	{
		p->ptr.end = s;
		p->chr = 0;
		return(0);
	}

	/*
	 * find the end of this token
	 */

	r = s;
	q = 0;
	for (;;) switch (c = *r++)
	{
	case '\n':
		if (s == (r - 1) && !q) r++;
		/*FALLTHROUGH*/
	case ' ':
	case '\t':
		if (q) break;
		/*FALLTHROUGH*/
	case 0:
		if (s == --r)
		{
			p->ptr.end = r;
			p->chr = 0;
		}
		else
		{
			p->chr = *(p->ptr.end = r);
			*r = 0;
		}
		return(s);
	case '\\':
		if (*r) r++;
		break;
	case '"':
	case '\'':
		if (c == q) q = 0;
		else if (!q) q = c;
		break;
	}
}
