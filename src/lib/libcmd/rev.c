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
static const char id[] = "\n@(#)rev (AT&T Bell Laboratories) 11/11/92\0\n";

#include	<cmdlib.h>

/*
 * reverse the characters within a line
 */
static int rev_char __PARAM__((Sfio_t *in, Sfio_t *out), (in, out)) __OTORP__(Sfio_t *in; Sfio_t *out;){
	register int c;
	register char *ep, *bp, *cp;
	register int n;
	while(cp = bp = sfgetr(in,'\n',0))
	{
		ep = bp + (n=sfslen()) -1;
		while(ep > bp)
		{
			c = *--ep;
			*ep = *bp;
			*bp++ = c;
		}
		if(sfwrite(out,cp,n)<0)
			return(-1);
	}
	return(0);
}

int
b_rev __PARAM__((int argc, register char** argv), (argc, argv)) __OTORP__(int argc; register char** argv;){
	register Sfio_t *fp;
	register char *cp;
	register int n, line=0;
	NOT_USED(argc);

	NoP(id[0]);
	NoP(argc);
	cmdinit(argv);
	while (n = optget(argv, "l [file ...]")) switch (n)
	{
	    case 'l':
		line=1;
		break;
	    case ':':
		error(2, opt_info.arg);
		break;
	    case '?':
		error(ERROR_usage(2), opt_info.arg);
		break;
	}
	argv += opt_info.index;
	if(error_info.errors)
		error(ERROR_usage(2),optusage((char*)0));
	n=0;
	if(cp = *argv)
		argv++;
	do
	{
		if(!cp || streq(cp,"-"))
			fp = sfstdin;
		else if(!(fp = sfopen((Sfio_t*)0,cp,"r")))
		{
			error(ERROR_system(0),"%s: cannot open",cp);
			n=1;
			continue;
		}
		if(line)
			line = rev_line(fp,sfstdout,sftell(fp));
		else
			line = rev_char(fp,sfstdout);
		if(fp!=sfstdin)
			sfclose(fp);
		if(line < 0)
			error(ERROR_system(1),"write failed");
	}
	while(cp= *argv++);
	return(n);
}
