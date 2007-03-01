/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1985-2005 AT&T Corp.                  *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                            by AT&T Corp.                             *
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
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * posix regex error message handler
 */

static const char id[] = "\n@(#)$Id: regex (AT&T Research) 2004-05-24 $\0\n";

#include "reglib.h"

static const char*	reg_error[] =
{
	/* REG_ENOSYS	*/	"not supported",
	/* REG_SUCCESS	*/	"success",
	/* REG_NOMATCH	*/	"no match",
	/* REG_BADPAT	*/	"invalid regular expression",
	/* REG_ECOLLATE	*/	"invalid collation element",
	/* REG_ECTYPE	*/	"invalid character class",
	/* REG_EESCAPE	*/	"trailing \\ in pattern",
	/* REG_ESUBREG	*/	"invalid \\digit backreference",
	/* REG_EBRACK	*/	"[...] imbalance",
	/* REG_EPAREN	*/	"\\(...\\) or (...) imbalance",
	/* REG_EBRACE	*/	"\\{...\\} or {...} imbalance",
	/* REG_BADBR	*/	"invalid {...} digits",
	/* REG_ERANGE	*/	"invalid [...] range endpoint",
	/* REG_ESPACE	*/	"out of space",
	/* REG_BADRPT	*/	"unary op not preceeded by re",
	/* REG_ENULL	*/	"empty subexpr in pattern",
	/* REG_ECOUNT	*/	"re component count overflow",
	/* REG_BADESC	*/	"invalid \\char escape",
	/* REG_VERSIONID*/	&id[10],
	/* REG_EFLAGS	*/	"conflicting flags",
	/* REG_EDELIM	*/	"invalid or omitted delimiter",
	/* REG_PANIC	*/	"unrecoverable internal error",
};

size_t
regerror(int code, const regex_t* p, char* buf, size_t size)
{
	const char*	s;

	NoP(p);
	if (code++ == REG_VERSIONID)
		s = (const char*)fmtident(&id[1]);
	else if (code >= 0 && code < elementsof(reg_error))
		s = reg_error[code];
	else
		s = (const char*)"unknown error";
	if (size)
	{
		strncpy(buf, s, size);
		buf[size - 1] = 0;
	}
	else
		buf = (char*)s;
	return strlen(buf) + 1;
}

/*
 * discipline error intercept
 */

int
fatal(regdisc_t* disc, int code, const char* pattern)
{
	if (disc->re_errorf)
	{
		if (pattern)
			(*disc->re_errorf)(NiL, disc, disc->re_errorlevel, "regular expression: %s: %s", pattern, reg_error[code+1]);
		else
			(*disc->re_errorf)(NiL, disc, disc->re_errorlevel, "regular expression: %s", reg_error[code+1]);
	}
	return code;
}
