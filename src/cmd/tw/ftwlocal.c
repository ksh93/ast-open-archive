/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1989-2004 AT&T Corp.                  *
*                      and is licensed under the                       *
*          Common Public License, Version 1.0 (the "License")          *
*                        by AT&T Corp. ("AT&T")                        *
*      Any use, downloading, reproduction or distribution of this      *
*      software constitutes acceptance of the License.  A copy of      *
*                     the License is available at                      *
*                                                                      *
*         http://www.research.att.com/sw/license/cpl-1.0.html          *
*         (with md5 checksum 8a5e0081c856944e76c69a1cf29c2e8b)         *
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
