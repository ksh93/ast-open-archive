/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1990-2005 AT&T Corp.                  *
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
