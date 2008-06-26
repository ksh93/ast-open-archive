/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2008 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
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

#include "ftwlocal.h"

#include <ls.h>

/*
 * return 1 if ftw is mounted on a local filesystem
 */

int
ftwlocal(Ftw_t* ftw)
{
#ifdef ST_LOCAL
	struct statvfs	fs;

	return statvfs(ftw->path, &fs) || (fs.f_flag & ST_LOCAL);
#else
	return !strgrpmatch(fmtfs(&ftw->statb), "([an]fs|samb)", NiL, 0, STR_LEFT|STR_ICASE);
#endif
}
