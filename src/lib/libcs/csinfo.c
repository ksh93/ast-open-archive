/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1990-2003 AT&T Corp.                *
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
 *
 * return Sfio_t stream pointer to host info file
 * if line!=0 then it points to current line number in file
 */

#include "cslib.h"

Sfio_t*
csinfo(register Cs_t* state, const char* file, int* line)
{
	int		n;
	Sfio_t*		sp = 0;
	char		buf[PATH_MAX];
	char		tmp[PATH_MAX];
	struct stat	st;

	messagef((state->id, NiL, -8, "info(%s) call", file));
	if (!file || streq(file, "-")) file = CS_SVC_INFO;
	if (strmatch(file, "*[ \t\n=]*")) sp = tokline(file, SF_STRING, line);
	else if (!strchr(file, '/') || stat(file, &st) || S_ISDIR(st.st_mode) || !(sp = tokline(file, SF_READ, line)))
		for (n = 0; n <= 1; n++)
		{
			sfsprintf(tmp, sizeof(tmp), "%s/%s", n ? csvar(state, CS_VAR_SHARE, 0) : CS_SVC_DIR, file);
			if (pathpath(buf, tmp, "", PATH_REGULAR|PATH_READ))
			{
				sp = tokline(buf, SF_READ, line);
				break;
			}
		}
	if (!sp) messagef((state->id, NiL, -1, "info: %s: not found", file));
	return sp;
}

Sfio_t*
_cs_info(const char* file, int* line)
{
	return csinfo(&cs, file, line);
}
