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
 * dss generic magic number support
 *
 * Glenn Fowler
 * AT&T Research
 */

#include "dsslib.h"

#include <magicid.h>

ssize_t
dssmagic(Dss_t* dss, Sfio_t* sp, const char* name, const char* type, uint32_t version, size_t size)
{
	Magicid_t	magic;
	ssize_t		n;

	memset(&magic, 0, sizeof(magic));
	magic.magic = MAGICID;
	if (!name)
		name = dss->id;
	strncopy(magic.name, name, sizeof(magic.name));
	strncopy(magic.type, type, sizeof(magic.type));
	magic.version = version;
	magic.size = size;
	sfwrite(sp, &magic, sizeof(magic));
	while (size < sizeof(magic))
		size += magic.size;
	for (n = size; size > sizeof(magic); size--)
		sfputc(sp, 0);
	if (sferror(sp))
	{
		if (dss->disc->errorf)
			(*dss->disc->errorf)(dss, dss->disc, ERROR_SYSTEM|2, "magic header write error");
		return -1;
	}
	return n;
}
