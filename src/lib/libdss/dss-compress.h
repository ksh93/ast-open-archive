/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 2002-2006 AT&T Corp.                  *
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
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * compress query
 */

static const char compress_usage[] =
"[-1l?\n@(#)$Id: dss compress query (AT&T Labs Research) 2003-05-05 $\n]"
USAGE_LICENSE
"[+NAME?\findex\f]"
"[+DESCRIPTION?The \bdss\b \bcompress\b query compress the parent output"
"	stream according to \amethod\a. If \amethod\a is omitted then"
"	the method preferred compression, if specified, is used, otherwise"
"	\bgzip\b is assumed. The methods are:]{"
"		[+bzip?\bbzip\b(1) (a.k.a. bzip2) compression using the"
"			\bsfdcbzip\b(3) discipline for \b-lbz\b.]"
"		[+gzip?\bgzip\b(1) compression using the \bsfdcgzip\b(3)"
"			discipline for \b-lz\b.]"
"		[+lzw?\bcompress\b(1) LZW compression using the \bsfdclzw\b(3)"
"			discipline.]"
"		[+pzip?\bpzip\b(1) compression using the \bsfdcpzip\b(3)"
"			discipline for \b-lpz\b. \aoperand\a must specify"
"			the \bpzip\b(1) partition file.]"
"}"
"\n"
"\n[ method [ operand ] ]\n"
"\n"
;

static int
compress_beg(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	char**		argv = (char**)data;
	int		errors = error_info.errors;
	char*		meth;

	for (;;)
	{
		switch (optget(argv, compress_usage))
		{
		case '?':
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_USAGE|4, "%s", opt_info.arg);
			continue;
		case ':':
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%s", opt_info.arg);
			continue;
		}
		break;
	}
	if (error_info.errors > errors)
		return -1;
	argv += opt_info.index;
	if (expr->pass || expr->fail || expr->parent->pass != expr)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "can only compress parent output stream");
		return -1;
	}
	expr->parent->pass = 0;
	if (expr->parent->op != expr->op)
	{
		if (expr->parent->op != sfstdout)
			sfclose(expr->parent->op);
		expr->parent->op = expr->op;
	}
	if (!(meth = *argv))
		sfdisc(expr->op, &dssstate(disc)->compress_preferred);
	else
	{
		if (*++argv)
		{
			sfprintf(cx->buf, "%s", meth);
			while (meth = *argv++)
				sfprintf(cx->buf, " %s", meth);
			meth = sfstruse(cx->buf);
		}
		if (sfdczip(expr->op, expr->file, meth, disc->errorf) < 0)
			return -1;
	}
	return 0;
}

#define QUERY_compress \
	{ \
		"compress", \
		"compress parent output stream", \
		{0}, \
		compress_beg, \
		0, \
		0, \
		0 \
	}