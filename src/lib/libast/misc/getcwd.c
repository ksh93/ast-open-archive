/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1985-2000 AT&T Corp.                *
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
*                 This software was created by the                 *
*                 Network Services Research Center                 *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                David Korn <dgk@research.att.com>                 *
*                 Phong Vo <kpv@research.att.com>                  *
*                                                                  *
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * pwd library support
 */

#include <ast.h>

#if _UWIN

NoN(getcwd)

#else

#include "dirlib.h"

#include <fs3d.h>

#ifndef ERANGE
#define ERANGE		E2BIG
#endif

#define ERROR(e)	{ errno = e; goto error; }

struct dirlist				/* long path chdir(2) component	*/
{
	struct dirlist*	next;		/* next component		*/
	int		index;		/* index from end of buf	*/
};

/*
 * pop long dir component chdir stack
 */

static int
popdir(register struct dirlist* d, register char* end)
{
	register struct dirlist*	dp;
	int				v;

	v = 0;
	while (dp = d)
	{
		d = d->next;
		if (!v)
		{
			if (d) *(end - d->index - 1) = 0;
			v = chdir(end - dp->index);
			if (d) *(end - d->index - 1) = '/';
		}
		free(dp);
	}
	return v;
}

/*
 * push long dir component onto stack
 */

static struct dirlist*
pushdir(register struct dirlist* d, char* dots, char* path, char* end)
{
	register struct dirlist*	p;

	if (!(p = newof(0, struct dirlist, 1, 0)) || chdir(dots))
	{
		if (p) free(p);
		if (d) popdir(d, end);
		return 0;
	}
	p->index = end - path;
	p->next = d;
	return p;
}

/*
 * return a pointer to the absolute path name of .
 * this path name may be longer than PATH_MAX
 *
 * a few environment variables are checked before the search algorithm
 * return value is placed in buf of len chars
 * if buf is 0 then space is allocated via malloc() with
 * len extra chars after the path name
 * 0 is returned on error with errno set as appropriate
 */

char*
getcwd(char* buf, size_t len)
{
	register char*	d;
	register char*	p;
	register char*	s;
	DIR*		dirp = 0;
	int		n;
	int		x;
	size_t		namlen;
	ssize_t		extra = -1;
	struct dirent*	entry;
	struct dirlist*	dirstk = 0;
	struct stat*	cur;
	struct stat*	par;
	struct stat*	tmp;
	struct stat	curst;
	struct stat	parst;
	struct stat	tstst;
	char		dots[PATH_MAX];

	static struct
	{
		char*	name;
		char*	path;
		dev_t	dev;
		ino_t	ino;
	}		env[] =
	{
		{ /*previous*/0	},
		{ "PWD"		},
		{ "HOME"	},
	};

	if (buf && !len) ERROR(EINVAL);
	if (fs3d(FS3D_TEST) && (namlen = mount(".", dots, FS3D_GET|FS3D_VIEW|FS3D_SIZE(sizeof(dots)), NiL)) > 1 && namlen < sizeof(dots))
	{
		p = dots;
	easy:
		namlen++;
		if (buf)
		{
			if (len < namlen) ERROR(ERANGE);
		}
		else if (!(buf = newof(0, char, namlen, len))) ERROR(ENOMEM);
		return (char*)memcpy(buf, p, namlen);
	}
	cur = &curst;
	par = &parst;
	if (stat(".", par)) ERROR(errno);
	for (n = 0; n < elementsof(env); n++)
	{
		if ((env[n].name && (p = getenv(env[n].name)) || (p = env[n].path)) && *p == '/' && !stat(p, cur))
		{
			env[n].path = p;
			env[n].dev = cur->st_dev;
			env[n].ino = cur->st_ino;
			if (cur->st_ino == par->st_ino && cur->st_dev == par->st_dev)
			{
				namlen = strlen(p);
				goto easy;
			}
		}
	}
	if (!buf)
	{
		extra = len;
		len = PATH_MAX;
		if (!(buf = newof(0, char, len, extra))) ERROR(ENOMEM);
	}
	d = dots;
	p = buf + len - 1;
	*p = 0;
	n = elementsof(env);
	for (;;)
	{
		tmp = cur;
		cur = par;
		par = tmp;
		if ((d - dots) > (PATH_MAX - 4))
		{
			if (!(dirstk = pushdir(dirstk, dots, p, buf + len - 1))) ERROR(ERANGE);
			d = dots;
		}
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
			pop:
				if (p != buf)
				{
					d = buf;
					while (*d++ = *p++);
					len = d - buf;
					if (extra >= 0 && !(buf = newof(buf, char, len, extra))) ERROR(ENOMEM);
				}
				if (dirstk && popdir(dirstk, buf + len - 1))
				{
					dirstk = 0;
					ERROR(errno);
				}
				if (env[0].path)
					free(env[0].path);
				env[0].path = strdup(buf);
				return buf;
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
#endif

			/*
			 * this fallthrough handles logical naming
			 */

			rewinddir(dirp);
		}
		do
		{
			if (!(entry = readdir(dirp))) ERROR(ENOENT);
#if _mem_d_namlen_dirent
			namlen = entry->d_namlen;
#else
			namlen = strlen(entry->d_name);
#endif
			if ((d - dots) > (PATH_MAX - 1 - namlen))
			{
				*d = 0;
				if (namlen >= PATH_MAX || !(dirstk = pushdir(dirstk, dots + 3, p, buf + len - 1))) ERROR(ERANGE);
				d = dots + 3;
			}
			memcpy(d, entry->d_name, namlen + 1);
		} while (stat(dots, &tstst) || tstst.st_ino != cur->st_ino || tstst.st_dev != cur->st_dev);
	found:
		if (*p) *--p = '/';
		while ((p -= namlen) <= (buf + 1))
		{
			x = (buf + len - 1) - (p += namlen);
			s = buf + len;
			if (extra < 0 || !(buf = newof(buf, char, len += PATH_MAX, extra))) ERROR(ERANGE);
			p = buf + len;
			while (p > buf + len - 1 - x) *--p = *--s;
		}
		if (n < elementsof(env))
		{
			memcpy(p, env[n].path, namlen);
			goto pop;
		}
		memcpy(p, entry->d_name, namlen);
		closedir(dirp);
		dirp = 0;
		for (n = 0; n < elementsof(env); n++)
			if (env[n].ino == par->st_ino && env[n].dev == par->st_dev)
			{
				namlen = strlen(env[n].path);
				goto found;
			}
	}
 error:
	if (buf)
	{
		if (dirstk) popdir(dirstk, buf + len - 1);
		if (extra >= 0) free(buf);
	}
	if (dirp) closedir(dirp);
	return 0;
}

#endif
