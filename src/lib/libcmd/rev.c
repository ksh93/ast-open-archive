/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1992-2000 AT&T Corp.              *
*      and it may only be used by you under license from       *
*                     AT&T Corp. ("AT&T")                      *
*       A copy of the Source Code Agreement is available       *
*              at the AT&T Internet web site URL               *
*                                                              *
*     http://www.research.att.com/sw/license/ast-open.html     *
*                                                              *
*      If you have copied this software without agreeing       *
*      to the terms of the license you are infringing on       *
*         the license and copyright and are violating          *
*             AT&T's intellectual property rights.             *
*                                                              *
*               This software was created by the               *
*               Network Services Research Center               *
*                      AT&T Labs Research                      *
*                       Florham Park NJ                        *
*                                                              *
*             Glenn Fowler <gsf@research.att.com>              *
*              David Korn <dgk@research.att.com>               *
*                                                              *
***************************************************************/
#pragma prototyped
/*
 * rev [-l] [file ...]
 *
 * reverse the characters or lines of one or more files
 *
 *   David Korn
 *   AT&T Laboratories
 *   dgk@research.att.com
 *
 */

static const char usage[] =
"[-?\n@(#)rev (AT&T Labs Research) 1999-04-10\n]"
USAGE_LICENSE
"[+NAME?rev - reverse the characters or lines of one or more files]"
"[+DESCRIPTION?\brev\b copies one or more files to standard output "
	"reversing the order of characters on every line of the file "
	"or reversing the order of lines of the file if \b-l\b is specified.]"
"[+?If no \afile\a is given, or if the \afile\a is \b-\b, \brev\b "
        "copies from standard input starting at the current offset.]"
"[l:line?Reverse the lines of the file.]"

"\n"
"\n[file ...]\n"
"\n"
"[+EXIT STATUS?]{"
        "[+0?All files copied successfully.]"
        "[+>0?One or more files did not copy.]"
"}"
"[+SEE ALSO?\bcat\b(1), \btail\b(1)]"
;

#include	<cmdlib.h>

/*
 * reverse the characters within a line
 */
static int rev_char(Sfio_t *in, Sfio_t *out)
{
	register int c;
	register char *ep, *bp, *cp;
	register int n;
	while(cp = bp = sfgetr(in,'\n',0))
	{
		ep = bp + (n=sfvalue(in)) -1;
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
b_rev(int argc, register char** argv, void* context)
{
	register Sfio_t *fp;
	register char *cp;
	register int n, line=0;
	NOT_USED(argc);

	NoP(argc);
	cmdinit(argv, context, ERROR_CATALOG);
	while (n = optget(argv, usage)) switch (n)
	{
	    case 'l':
		line=1;
		break;
	    case ':':
		error(2, "%s", opt_info.arg);
		break;
	    case '?':
		error(ERROR_usage(2), "%s", opt_info.arg);
		break;
	}
	argv += opt_info.index;
	if(error_info.errors)
		error(ERROR_usage(2),"%s",optusage((char*)0));
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
