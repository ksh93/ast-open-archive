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
*        THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF        *
*                    AT&T BELL LABORATORIES                    *
*         AND IS NOT TO BE DISCLOSED OR USED EXCEPT IN         *
*            ACCORDANCE WITH APPLICABLE AGREEMENTS             *
*                                                              *
*                Copyright (c) 1995 AT&T Corp.                 *
*              Unpublished & Not for Publication               *
*                     All Rights Reserved                      *
*                                                              *
*       The copyright notice above does not evidence any       *
*      actual or intended publication of such source code      *
*                                                              *
*               This software was created by the               *
*           Advanced Software Technology Department            *
*                    AT&T Bell Laboratories                    *
*                                                              *
*               For further information contact                *
*                    {research,attmail}!dgk                    *
*                                                              *
***************************************************************/

/* : : generated by proto : : */
                  
#ifndef fcgetc
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

/*
 * David Korn
 * AT&T Bell Laboratories
 *
 * Fast character input with sfio text streams and strings
 *
 */

#include	<sfio.h>

typedef struct _fcin
{
	Sfio_t		*_fcfile;	/* input file pointer */
	unsigned char	*fcbuff;	/* pointer to input buffer */
	unsigned char	*fclast;	/* pointer to end of input buffer */
	unsigned char	*fcptr;		/* pointer to next input char */
	unsigned char	fcchar;		/* saved character */
	void (*fcfun) __PROTO__((Sfio_t*,const char*,int));	/* advance function */
} Fcin_t;

#define fcfile()	(_Fcin._fcfile)
#define fcgetc(c)	(((c=fcget()) || (c=fcfill())), c)
#define	fcget()		((int)(*_Fcin.fcptr++))
#define	fcpeek(n)	((int)_Fcin.fcptr[n])
#define	fcseek(n)	((char*)(_Fcin.fcptr+=(n)))
#define fcfirst()	((char*)_Fcin.fcbuff)
#define fcsopen(s)	(_Fcin._fcfile=(Sfio_t*)0,_Fcin.fcbuff=_Fcin.fcptr=(unsigned char*)(s))
#define fcsave(x)	(*(x) = _Fcin)
#define fcrestore(x)	(_Fcin = *(x))
extern __MANGLE__ int		fcfill __PROTO__((void));
extern __MANGLE__ int		fcfopen __PROTO__((Sfio_t*));
extern __MANGLE__ int		fcclose __PROTO__((void));
void			fcnotify __PROTO__((void(*)(Sfio_t*,const char*,int)));

extern __MANGLE__ Fcin_t		_Fcin;		/* used by macros */

#endif /* fcgetc */
