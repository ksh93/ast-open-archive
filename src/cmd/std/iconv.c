/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1989-2002 AT&T Corp.                *
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
 * AT&T Research
 */

static const char usage[] =
"[-?\n@(#)$Id: iconv (AT&T Labs Research) 2000-05-09 $\n]"
USAGE_LICENSE
"[+NAME?iconv - codeset conversion]"
"[+DESCRIPTION?\biconv\b converts the encoding of characters in the \afile\a"
"	operands from one codeset to another and writes the results to"
"	the standard output. If \afile\a is \b-\b or omitted then the"
"	standard input is read.]"
"[+?Character encodings in either codeset may include single-byte values"
"	(for example, for the ISO 8859-1:1987 standard characters) or"
"	multi-byte values (for example, for certain characters in the"
"	ISO 6937:1983 standard). Invalid characters in the input stream"
"	(either those that are not valid members of the input codeset or"
"	those that have no corresponding value in the output codeset) are"
"	output as the underscore character (\b_\b) in the output codeset.]"
"[+?The \bnative\b codeset is determined by the \bLANG\b, \bLC_ALL\b and"
"	\bLC_CTYPE\b environment variables. The supported codesets"
"	are matched by these case-insensitive patterns:]{\fcodesets\f}"
"[+?Conversion between certain codesets may not be supported. Also, since the"
"	standard(s) provide no support for listing the known codesets, the"
"	above list may be incomplete.]"

"[e:errors?Do not ignore conversion errors.]"
"[f:from?The input codeset is set to \acodeset\a.]:[codeset:=native]"
"[i:ignore?Ignore conversion errors. This is the default.]"
"[t:to?The output codeset is set to \acodeset\a.]:[codeset:=native]"

"\n"
"\n[ pid ... ]\n"
"\n"

"[+SEE ALSO?\bdd\b(1), \biconv\b(3), \bsetlocale\b(3)]"
;

#include <ast.h>
#include <error.h>
#include <iconv.h>

/*
 * optget() info discipline function
 */

static int
optinfo(Opt_t* op, Sfio_t* sp, const char* s, Optdisc_t* dp)
{
	register iconv_list_t*	ic;
	register const char*	p;
	register int		c;

	if (streq(s, "codesets"))
		for (ic = iconv_list(NiL); ic; ic = iconv_list(ic))
		{
			sfputc(sp, '[');
			sfputc(sp, '+');
			sfputc(sp, '\b');
			p = ic->match;
			if (*p == '(')
				p++;
			while (c = *p++)
			{
				if (c == ')' && !*p)
					break;
				if (c == '?' || c == ']')
					sfputc(sp, c);
				sfputc(sp, c);
			}
			sfputc(sp, '?');
			p = ic->desc;
			while (c = *p++)
			{
				if (c == ']')
					sfputc(sp, c);
				sfputc(sp, c);
			}
			sfputc(sp, ']');
		}
	return 0;
}

int
main(int argc, register char** argv)
{
	char*		file;
	char*		from;
	char*		to;
	iconv_t		cvt;
	size_t		errors;
	int		ignore;
	Sfio_t*		ip;
	Optdisc_t	od;

	NoP(argc);
	error_info.id = "iconv";
	from = to = "native";
	ignore = 1;
	setlocale(LC_ALL, "");

	/*
	 * set up the disciplines
	 */

	memset(&od, 0, sizeof(od));
	od.version = OPT_VERSION;
	od.infof = optinfo;
	opt_info.disc = &od;

	/*
	 * grab the options
	 */

	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'e':
			ignore = 0;
			continue;
		case 'f':
			from = opt_info.arg;
			continue;
		case 'i':
			ignore = 1;
			continue;
		case 't':
			to = opt_info.arg;
			continue;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			break;
		case ':':
			error(2, "%s", opt_info.arg);
			break;
		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	if ((cvt = iconv_open(to, from)) == (iconv_t)(-1))
	{
		if ((cvt = iconv_open(to, "utf-8")) == (iconv_t)(-1))
			error(3, "%s: unknown destination codeset", to);
		iconv_close(cvt);
		if ((cvt = iconv_open("utf-8", from)) == (iconv_t)(-1))
			error(3, "%s: unknown source codeset", from);
		iconv_close(cvt);
		error(3, "cannot convert from %s to %s", from, to);
	}
	if (file = *argv)
		argv++;
	do
	{
		if (!file || streq(file, "-"))
		{
			file = "/dev/stdin";
			ip = sfstdin;
		}
		else if (!(ip = sfopen(NiL, file, "r")))
		{
			error(ERROR_SYSTEM|2, "%s: cannot open", file);
			continue;
		}
		errors = 0;
		iconv_move(cvt, ip, sfstdout, SF_UNBOUND, &errors);
		if (!sfeof(ip) || sferror(ip))
			error(ERROR_SYSTEM|2, "%s: conversion read error", file);
		if (!ignore)
		{
			if (errors == 1)
				error(2, "%s: %d character conversion error", file, errors);
			else if (errors)
				error(2, "%s: %d character conversion errors", file, errors);
		}
		if (ip != sfstdin)
			sfclose(ip);
	} while (file = *argv++);
	if (sfsync(sfstdout))
		error(ERROR_SYSTEM|3, "conversion write error");
	return error_info.errors != 0;
}
