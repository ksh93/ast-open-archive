/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1998-2002 AT&T Corp.                *
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
 * sfio { lzw gzip pzip } deflate discipline wrapper
 */

#include "pzlib.h"

/*
 * push the sfio discipline named by meth:
 *
 *	lzw
 *	gzip
 *	pzip partition
 *
 * return:
 *	>0	discipline pushed
 *	 0	discipline not needed
 *	<0	error
 */

int
sfdczip(Sfio_t* sp, const char* path, register const char* meth, Error_f errorf)
{
	const char*	part;
	const char*	mesg;
	int		r;
	Pzdisc_t	disc;

	mesg = ERROR_dictionary("compress discipline error");
	if (!path)
	{
		if (sp == sfstdin)
			path = "/dev/stdin";
		else if (sp == sfstdout)
			path = "/dev/stdout";
		else if (sfset(sp, 0, 0) & SF_READ)
			path = "input";
		else
			path = "output";
	}
	if (sfset(sp, 0, 0) & SF_READ)
	{
		memset(&disc, 0, sizeof(disc));
		disc.version = PZ_VERSION;
		disc.errorf = errorf;
		r = sfdcpzip(sp, path, 0, &disc);
	}
	else if (strneq(meth, "gzip", 4))
		r = sfdcgzip(sp, 0);
	else if (strneq(meth, "lzw", 3))
		r = sfdclzw(sp, 0);
	else if (strneq(meth, "pzip", 4))
	{
		part = meth + 4;
		while (isspace(*part))
			part++;
		if (!*part)
		{
			mesg = ERROR_dictionary("partition file operand required");
			r = -1;
		}
		else
		{
			memset(&disc, 0, sizeof(disc));
			disc.version = PZ_VERSION;
			disc.partition = part;
			disc.errorf = errorf;
			r = sfdcpzip(sp, path, 0, &disc);
		}
	}
	else
	{
		mesg = ERROR_dictionary("unknown compress discipline method");
		r = -1;
	}
	if (r < 0 && errorf)
		(*errorf)(NiL, NiL, 2, "%s: %s: %s", path, meth, mesg);
	return r;
}
