/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1986-2003 AT&T Corp.                *
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
*                David Korn <dgk@research.att.com>                 *
*                 Phong Vo <kpv@research.att.com>                  *
*                                                                  *
*******************************************************************/
#include <ast.h>
#include <ardir.h>
#include <error.h>

main(int argc, char** argv)
{
	Ardir_t*	dir;
	Ardirent_t*	ent;
	long		touch;
	char*		file;

	touch = 0;
	while (file = *++argv)
	{
		if (!strcmp(file, "-t") && *(argv + 1))
			touch = strtol(*++argv, NiL, 0);
		else if (dir = ardiropen(file, NiL, touch ? ARDIR_UPDATE : 0))
		{
			sfprintf(sfstdout, "%s: type=%s truncate=%d%s\n", file, dir->meth->name, dir->truncate, (dir->flags & ARDIR_RANLIB) ? " ranlib" : "");
			while (ent = ardirnext(dir))
			{
				if (touch)
				{
					ent->mtime = touch;
					ardirchange(dir, ent);
					sfprintf(sfstdout, "touch %s\n", ent->name);
				}
				else
					sfprintf(sfstdout, "%s %8u %8u %8llu %8llu %s %s\n", fmtmode(ent->mode, 1), ent->uid, ent->gid, ent->size, ent->offset, fmttime("%k", ent->mtime), ent->name);
			}
			if (ardirclose(dir))
				error(2, "%s: archive read error", file);
		}
		else
			error(ERROR_SYSTEM|2, "%s: not an archive", file);
	}
	return 0;
}
