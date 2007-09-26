/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*           Copyright (c) 2002-2007 AT&T Knowledge Ventures            *
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
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * print query
 */

static const char print_usage[] =
"[-1l?\n@(#)$Id: dss print query (AT&T Research) 2003-02-19 $\n]"
USAGE_LICENSE
"[+NAME?\findex\f]"
"[+DESCRIPTION?The \bdss\b \bprint\b query formats and prints the"
"	current record according to \aformat\a. If \aformat\a is omitted"
"	then the default method \bprint\b format is used; an error occurs if"
"	there is no default \bprint\b format.]"
"[+?\aformat\a follows \bprintf\b(3) conventions, except"
"	that \bsfio\b(3) inline ids are used instead of arguments:"
"	%[-+]][\awidth\a[.\aprecis\a[.\abase\a]]]]]](\aid\a[:\asubformat\a]])\achar\a."
"	If \achar\a is \bs\b then the string form of the item is listed,"
"	otherwise the corresponding numeric form is listed. If \achar\a is"
"	\bq\b then the string form of the item is $'...' quoted if it contains"
"	space or non-printing characters. If \awidth\a is omitted then the"
"	default width is assumed. \asubformat\a optionally specifies field"
"	specific subformats. For example, a \bstrftime\b(3) format for time"
"	fields, or a field separator for array fields. The \bdf\b(1),"
"	\bls\b(1) and \bpax\b(1) commands have \b--format\b options in this"
"	same style.]"
"[+?The default print format is \fprint\f.]"
"\n"
"\n[ format ]\n"
"\n";

static int
print_beg(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	char**	argv = (char**)data;
	int	errors = error_info.errors;

	for (;;)
	{
		switch (optget(argv, print_usage))
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
	if (!(expr->data = *argv++))
	{
		if (!(expr->data = (char*)DSS(cx)->meth->print))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%s: no default method print format", DSS(cx)->meth->name);
			return -1;
		}
	}
	else if (*argv)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_USAGE|4, "%s", optusage(NiL));
		return -1;
	}
	return 0;
}

static int
print_act(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	return dssprintf(DSS(cx), expr->op, (char*)expr->data, data);
}

#define QUERY_print \
	{ \
		"print", \
		"format and print the current record", \
		CXH, \
		print_beg, \
		0, \
		print_act, \
		0 \
	}
