/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1984-2000 AT&T Corp.              *
*      and it may only be used by you under license from       *
*                     AT&T Corp. ("AT&T")                      *
*       A copy of the Source Code Agreement is available       *
*              at the AT&T Internet web site URL               *
*                                                              *
*     http://www.research.att.com/sw/license/ast-open.html     *
*                                                              *
*     If you received this software without first entering     *
*       into a license with AT&T, you have an infringing       *
*           copy and cannot use it without violating           *
*             AT&T's intellectual property rights.             *
*                                                              *
*               This software was created by the               *
*               Network Services Research Center               *
*                      AT&T Labs Research                      *
*                       Florham Park NJ                        *
*                                                              *
*              David Korn <dgk@research.att.com>               *
*                         Pat Sullivan                         *
*                                                              *
***************************************************************/
#include	<sys/types.h>
#undef direct
#ifndef FS_3D
#   define direct dirent
#endif /* FS_3D */
#ifdef _dirent_
#   include	<dirent.h>
#else
#   include	<ndir.h>
#endif /* _dirent_ */

/*
 * This routine checks for file named a.out in .
 * only define _ndir_ if ndir is there and opendir works
 */


main()
{
	DIR	 *dirf;
	struct dirent *ep;
	if(dirf=opendir("."))
	{
		while(ep=readdir(dirf))
		{
			if(strcmp("a.out",ep->d_name)==0)
			{
#ifndef _dirent_
				printf("#define _ndir_	1\n");
#endif /* _dirent_ */
				exit(0);
			}
		}
	}
	exit(1);
}

