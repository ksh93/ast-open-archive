/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1986-2001 AT&T Corp.                *
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
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * preprocessor data
 *
 * intended to be a conforming implementation of the translation phases
 * (2.1.1.2) 1,2,3,4 and 6 of the "American National Standard for
 * Information Systems -- Programming Language -- C", ANSI X3.159-1989.
 *
 * STANDARD INTERPRETATION:
 *
 *	include files are forced to preserve #if nesting levels
 *	support for this is found in the recursive description for
 *	include file processing in the translation phases
 *
 *	ID"..." produces two tokens: {ID}{"..."}
 *	ID'...' produces two tokens: {ID}{'...'}
 *
 * COMPATIBILITY:
 *
 *	only sane Reiser compatibility is implemented
 *
 *	strange handling of `\newline', especially in directives,
 *	is not implemented
 *
 *	dissappearing comments used as concatenation operators work
 *	only within macro bodies
 */

static const char id[] = "\n@(#)$Id: libpp (AT&T Research) 2001-02-06 $\0\n";

#include "pplib.h"

#ifndef IDNAME
#define IDNAME	"pp"
#endif

static char	addbuf[MAXTOKEN+1];	/* ADD buffer			*/
static char	argsbuf[MAXTOKEN+1];	/* predicate args		*/
static char	catbuf[MAXTOKEN+1];	/* catenation buffer		*/
static char	hidebuf[MAXTOKEN+1];	/* pp:hide buffer		*/
static char	outbuf[2*(PPBUFSIZ+MAXTOKEN)];/* output buffer		*/
static char	pathbuf[MAXTOKEN+1];	/* full path of last #include	*/
static char	tmpbuf[MAXTOKEN+1];	/* very temporary buffer	*/
static char	tokbuf[2*MAXTOKEN+1];	/* token buffer			*/
static char	valbuf[MAXTOKEN+1];	/* builtin macro value buffer	*/

static char	null[1];

static struct ppinstk	instack =	/* input stream stack		*/
{
	&null[0]			/* nextchr			*/
};

static struct ppdirs	stddir =	/* standard include directory	*/
{
	PPSTANDARD,	0,		1, 1, INC_STANDARD, 0
};

static struct ppdirs	firstdir =	/* first include directory	*/
{
	"",		&stddir,	0, 0, INC_PREFIX, TYPE_DIRECTORY
};

struct ppglobals pp =
{
	/* public globals */

	&id[10],			/* version			*/
	"",				/* lineid			*/
	"/dev/stdout",			/* outfile			*/
	IDNAME,				/* pass				*/
	&tokbuf[0],			/* token			*/
	0,				/* symbol			*/

	/* exposed for the output macros */

	&outbuf[0],			/* outb				*/
	&outbuf[0],			/* outbuf			*/
	&outbuf[0],			/* outp				*/
	&outbuf[PPBUFSIZ],		/* oute				*/
	0,				/* offset			*/

	/* public context */

	&firstdir,			/* lcldirs			*/
	&firstdir,			/* stddirs			*/
	0,				/* flags			*/
	0,				/* symtab			*/

	/* private context */

	0,				/* context			*/
	0,				/* state			*/
	ALLMULTIPLE|CATLITERAL,		/* mode				*/
	PREFIX,				/* option			*/
	0,				/* test				*/
	0,				/* filedeps			*/
	&firstdir,			/* firstdir			*/
	&firstdir,			/* lastdir			*/
	0,				/* hide				*/
	0,				/* column			*/
	-1,				/* pending			*/
	0,				/* firstfile			*/
	0,				/* lastfile			*/
	0,				/* ignore			*/
	0,				/* probe			*/
	0,				/* filtab			*/
	0,				/* prdtab			*/
	0,				/* date				*/
	0,				/* time				*/
	0,				/* maps				*/
	0,				/* ro_state			*/
	0,				/* ro_mode			*/
	0,				/* ro_option			*/
	0,				/* cdir				*/
	0,				/* hostdir			*/
	0,				/* ppdefault			*/
	0,				/* firstindex			*/
	0,				/* lastindex			*/
	0,				/* firstop			*/
	0,				/* lastop			*/
	0,				/* firsttx			*/
	0,				/* lasttx			*/
	0,				/* arg_file			*/
	0,				/* arg_mode			*/
	0,				/* arg_style			*/
	0,				/* c				*/
	0,				/* hosted			*/
	0,				/* ignoresrc			*/
	0,				/* initialized			*/
	0,				/* standalone			*/
	0,				/* spare_1			*/

	/* library private globals */

	"\"08/11/94\"",			/* checkpoint (with quotes!)	*/
	128,				/* constack			*/
	&instack,			/* in				*/
	&addbuf[0],			/* addp				*/
	&argsbuf[0],			/* args				*/
	&addbuf[0],			/* addbuf			*/
	&catbuf[0],			/* catbuf			*/
	0,				/* hdrbuf			*/
	&hidebuf[0],			/* hidebuf			*/
	&pathbuf[0],			/* path				*/
	&tmpbuf[0],			/* tmpbuf			*/
	&valbuf[0],			/* valbuf			*/

	/* the rest are implicitly initialized */
};

char	ppctype[UCHAR_MAX];
