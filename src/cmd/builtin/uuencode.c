/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*           Copyright (c) 1992-2006 AT&T Knowledge Ventures            *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                      by AT&T Knowledge Ventures                      *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * uuencode
 */

static const char usage[] =
"[-?\n@(#)$Id: uuencode (AT&T Research) 2002-03-24 $\n]"
USAGE_LICENSE
"[+NAME?uuencode - encode a binary file]"
"[+DESCRIPTION?\buuencode\b writes an encoded version of the named input"
"	\afile\a, or the standard input if no file is specified, to the"
"	standard output. \adecode-file\a is the name stored in the"
"	encoded output header. This is the default name that will be used"
"	by \buudecode\b when decoding.]"

"[h!:header?Write header and trailer sequences to the encoded output."
"	The header for some encoding formats may contain file name and"
"	access infomation.]"
"[l:list?List the encoding method names on the standard output.]"
"[o:output?Write the output data into \afile\a instead of the standard"
"	output.]:[file]"
"[t:text?The input file is a text file that requires \\n => \\r\\n translation"
"	on encoding.]"
"[x:method?Specifies the encoding \amethod\a:]:[method]{"
"	[+posix|uuencode]"
"	[+ucb|bsd]"
"	[+mime|base64]"
"	[+quoted-printable|qp]"
"	[+binhex|mac-binhex]"
"}"
"[b?Equivalent to \b--method=binhex\b.]"
"[m?Equivalent to \b--method=mime\b.]"
"[q?Equivalent to \b--method=quoted-printable\b.]"
"[u?Equivalent to \b--method=ucb\b.]"

"\n"
"\n[ [ file ] decode-file ]\n"
"\n"

"[+SEE ALSO?\bmailx\b(1), \buudecode\b(1)]"
;

#include <ast.h>
#include <uu.h>
#include <error.h>
#include <option.h>

int
main(int argc, register char** argv)
{
	Uu_t*		uu;
	Uumeth_t*	meth;
	char*		encoding;
	char*		epath;
	char*		ipath;
	char*		opath;
	Sfio_t*		ip;
	Sfio_t*		op;
	char		buf[2];
	Uudisc_t	disc;

	error_info.id = "uuencode";
	memset(&disc, 0, sizeof(disc));
	disc.version = UU_VERSION;
	disc.flags = UU_HEADER;
	disc.errorf = errorf;
	encoding = 0;
	epath = ipath = opath = 0;
	ip = sfstdin;
	op = sfstdout;
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'b':
		case 'm':
		case 'q':
		case 'u':
			buf[0] = opt_info.option[1];
			buf[1] = 0;
			encoding = buf;
			continue;
		case 'h':
			disc.flags &= ~UU_HEADER;
			continue;
		case 'l':
			uulist(sfstdout);
			exit(0);
		case 'o':
			opath = opt_info.arg;
			continue;
		case 't':
			disc.flags |= UU_TEXT;
			continue;
		case 'x':
			encoding = opt_info.arg;
			if (streq(encoding, "?"))
			{
				uulist(sfstdout);
				exit(0);
			}
			continue;
		case ':':
			error(2, "%s", opt_info.arg);
			continue;
		case '?':
			error(ERROR_usage(2), "%s", opt_info.arg);
			continue;
		}
		break;
	}
	argv += opt_info.index;
	if ((epath = *argv) && *++argv)
	{
		ipath = epath;
		epath = *argv++;
	}
	if (error_info.errors || *argv)
		error(ERROR_usage(2), "%s", optusage(NiL));
	if (!(meth = uumeth(encoding)))
		error(ERROR_exit(1), "%s: unknown method", encoding);
	if (uu = uuopen(&disc, meth))
	{
		if (!ipath)
			ip = sfstdin;
		else if (!(ip = sfopen(NiL, ipath, "rt")))
			error(ERROR_system(1), "%s: cannot read", ipath);
		if (!opath)
			op = sfstdout;
		else if (!(op = sfopen(NiL, opath, "w")))
			error(ERROR_system(1), "%s: cannot write", opath);
		uuencode(uu, ip, op, SF_UNBOUND, epath);
		uuclose(uu);
	}
	return error_info.errors != 0;
}
