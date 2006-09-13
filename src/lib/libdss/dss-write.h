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
 * write query
 */

static const char write_usage[] =
"[-1l?\n@(#)$Id: dss write query (AT&T Labs Research) 2002-12-12 $\n]"
USAGE_LICENSE
"[+NAME?\findex\f]"
"[+DESCRIPTION?The \bdss\b \bwrite\b query writes the"
"	current record according to the method-specific \aformat\a."
"	If \aformat\a is omitted then the input record format is assumed."
"	The formats are:]{\fformats\f}"
"\n"
"\n [ format] \n"
"\n";

static int
write_beg(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	char**		argv = (char**)data;
	int		errors = error_info.errors;
	Dssformat_t*	format;

	for (;;)
	{
		switch (optget(argv, write_usage))
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
	if (expr->data = *argv++)
	{
		if (*argv)
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_USAGE|4, "%s", optusage(NiL));
			return -1;
		}
		if (!(format = dssformat(expr->data, disc, DSS(cx)->meth)))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%s: unknown format", (char*)expr->data);
			return -1;
		}
	}
	else
		format = 0;
	if (!(expr->data = dssfopen(DSS(cx), expr->file, expr->op, DSS_FILE_WRITE, format)))
		return -1;
	return 0;
}

static int
write_act(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	return dssfwrite((Dssfile_t*)expr->data, data);
}

static int
write_end(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	return dssfclose((Dssfile_t*)expr->data);
}

#define QUERY_write \
	{ \
		"write", \
		"write the current record", \
		{0}, \
		write_beg, \
		0, \
		write_act, \
		write_end \
	}