/*
 * CDE - Common Desktop Environment
 *
 * Copyright (c) 1993-2012, The Open Group. All rights reserved.
 *
 * These libraries and programs are free software; you can
 * redistribute them and/or modify them under the terms of the GNU
 * Lesser General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * These libraries and programs are distributed in the hope that
 * they will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with these librararies and programs; if not, write
 * to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
 * Floor, Boston, MA 02110-1301 USA
 */
/***************************************************************
*                                                              *
*                      AT&T - PROPRIETARY                      *
*                                                              *
*         THIS IS PROPRIETARY SOURCE CODE LICENSED BY          *
*                          AT&T CORP.                          *
*                                                              *
*                Copyright (c) 1995 AT&T Corp.                 *
*                     All Rights Reserved                      *
*                                                              *
*           This software is licensed by AT&T Corp.            *
*       under the terms and conditions of the license in       *
*       http://www.research.att.com/orgs/ssr/book/reuse        *
*                                                              *
*               This software was created by the               *
*           Software Engineering Research Department           *
*                    AT&T Bell Laboratories                    *
*                                                              *
*               For further information contact                *
*                     gsf@research.att.com                     *
*                                                              *
***************************************************************/

/* : : generated by proto : : */

#if !defined(__PROTO__)
#if defined(__STDC__) || defined(__cplusplus) || defined(_proto) || defined(c_plusplus)
#if defined(__cplusplus)
#define __MANGLE__	"C"
#else
#define __MANGLE__
#endif
#define __STDARG__
#define __PROTO__(x)	x
#define __OTORP__(x)
#define __PARAM__(n,o)	n
#if !defined(__STDC__) && !defined(__cplusplus)
#if !defined(c_plusplus)
#define const
#endif
#define signed
#define void		int
#define volatile
#define __V_		char
#else
#define __V_		void
#endif
#else
#define __PROTO__(x)	()
#define __OTORP__(x)	x
#define __PARAM__(n,o)	o
#define __MANGLE__
#define __V_		char
#define const
#define signed
#define void		int
#define volatile
#endif
#if defined(__cplusplus) || defined(c_plusplus)
#define __VARARG__	...
#else
#define __VARARG__
#endif
#if defined(__STDARG__)
#define __VA_START__(p,a)	va_start(p,a)
#else
#define __VA_START__(p,a)	va_start(p)
#endif
#endif
#include <ast.h>
#include <preroot.h>

#if FS_PREROOT

#include "dirlib.h"
#include <ls.h>
#include <error.h>
#include <stdio.h>

#ifndef ERANGE
#define ERANGE		E2BIG
#endif

#define ERROR(e)	{errno=e;goto error;}

char*
getpreroot __PARAM__((char* path, const char* cmd), (path, cmd)) __OTORP__(char* path; const char* cmd;){
	register int	c;
	register FILE*	fp;
	register char*	p;
	char		buf[PATH_MAX];

	if (!path) path = buf;
	if (cmd)
	{
		sfsprintf(buf, sizeof(buf), "set x `%s= %s - </dev/null 2>&1`\nwhile :\ndo\nshift\ncase $# in\n[012]) break ;;\nesac\ncase \"$1 $2\" in\n\"+ %s\")	echo $3; exit ;;\nesac\ndone\necho\n", PR_SILENT, cmd, PR_COMMAND);
		if (!(fp = popen(buf, "rug"))) return(0);
		for (p = path; (c = getc(fp)) != EOF && c != '\n'; *p++ = c);
		*p = 0;
		pclose(fp);
		if (path == p) return(0);
		return(path == buf ? strdup(path) : path);
	}
	else
	{
		char*		d;
		DIR*		dirp = 0;
		int		namlen;
		int		euid;
		int		ruid;
		struct dirent*	entry;
		struct stat*	cur;
		struct stat*	par;
		struct stat*	tmp;
		struct stat	curst;
		struct stat	parst;
		struct stat	tstst;
		char		dots[PATH_MAX];

		cur = &curst;
		par = &parst;
		if ((ruid = getuid()) != (euid = geteuid())) setuid(ruid);
		if (stat(PR_REAL, cur) || stat("/", par) || cur->st_dev == par->st_dev && cur->st_ino == par->st_ino) ERROR(ENOTDIR);

		/*
		 * like getcwd() but starting at the preroot
		 */

		d = dots;
		*d++ = '/';
		p = path + PATH_MAX - 1;
		*p = 0;
		for (;;)
		{
			tmp = cur;
			cur = par;
			par = tmp;
			if ((d - dots) > (PATH_MAX - 4)) ERROR(ERANGE);
			*d++ = '.';
			*d++ = '.';
			*d = 0;
			if (!(dirp = opendir(dots))) ERROR(errno);
#if !_dir_ok || _mem_dd_fd_DIR
			if (fstat(dirp->dd_fd, par)) ERROR(errno);
#else
			if (stat(dots, par)) ERROR(errno);
#endif
			*d++ = '/';
			if (par->st_dev == cur->st_dev)
			{
				if (par->st_ino == cur->st_ino)
				{
					closedir(dirp);
					*--p = '/';
					if (ruid != euid) setuid(euid);
					if (path == buf) return(strdup(p));
					if (path != p)
					{
						d = path;
						while (*d++ = *p++);
					}
					return(path);
				}
#if _mem_d_fileno_dirent || _mem_d_ino_dirent
#if !_mem_d_fileno_dirent
#define d_fileno	d_ino
#endif
				while (entry = readdir(dirp))
					if (entry->d_fileno == cur->st_ino)
					{
#if _mem_d_namlen_dirent
						namlen = entry->d_namlen;
#else
						namlen = strlen(entry->d_name);
#endif
						goto found;
					}
	
				/*
				 * this fallthrough handles logical naming
				 */

				rewinddir(dirp);
#endif
			}
			do
			{
				if (!(entry = readdir(dirp))) ERROR(ENOENT);
#if _mem_d_namlen_dirent
				namlen = entry->d_namlen;
#else
				namlen = strlen(entry->d_name);
#endif
				if ((d - dots) > (PATH_MAX - 1 - namlen)) ERROR(ERANGE);
				memcpy(d, entry->d_name, namlen + 1);
				if (stat(dots, &tstst)) ERROR(errno);
			} while (tstst.st_ino != cur->st_ino || tstst.st_dev != cur->st_dev);
		found:
			if (*p) *--p = '/';
			if ((p -= namlen) <= (path + 1)) ERROR(ERANGE);
			memcpy(p, entry->d_name, namlen);
			closedir(dirp);
			dirp = 0;
		}
	error:
		if (dirp) closedir(dirp);
		if (ruid != euid) setuid(euid);
	}
	return(0);
}

#else

NoN(getpreroot)

#endif
