/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1989-2002 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*******************************************************************/
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
