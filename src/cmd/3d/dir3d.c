/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1989-2001 AT&T Corp.                *
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
*                 Eduardo Krell <ekrell@adexus.cl>                 *
*******************************************************************/
#pragma prototyped

/*
 * 3d directory(3)
 *
 * NOTE: there are 2 limitations to this implementation
 *
 *	 (1) opendir() allocates one file descriptor for each directory
 *	     view and these remain open until closedir()
 *
 *	 (2) telldir() offsets are encoded with the directory view level
 *	     and TELLDIR() offset, and the directory view level takes
 *	     TABBITS bits, so TELLDIR() offsets are limited to (32-TABBITS)
 *	     bits, but that would be one big physical directory
 */

#include <ast_std.h>

#include "dir_3d.h"

static int	intercepted;

DIR*
opendir3d(const char* apath)
{
	register char*	path;
	register DIR*	dirp = 0;
	register int	n;
	int		oerrno;
	long		visits = 0;
	struct stat	st;

	if (dirp = (DIR*)state.freedirp) state.freedirp = 0;
	else if (!(dirp = newof(0, DIR, 1, 0))) return 0;
	intercepted++;
	dirp->viewp = dirp->view;
	n = state.in_2d;
	if (path = pathreal(apath, P_SLASH, NiL)) state.in_2d = 2;
	else path = (char*)apath;
	dirp->viewp->dirp = OPENDIR(path);
	state.in_2d = n;
	if (!dirp->viewp->dirp)
	{
		state.freedirp = (void*)dirp;
		intercepted = 0;
		return 0;
	}
	dirp->boundary = state.boundary;
	if (state.in_2d)
	{
		dirp->overlay = 0;
		(dirp->viewp++)->opaque = 0;
		dirp->viewp->dirp = 0;
	}
	else
	{
		oerrno = errno;
		n = strlen(path);
		path[n] = '/';
		strcpy(path + n + 1, state.opaque);
		dirp->viewp->level = state.path.level;
		state.in_2d++;
		(dirp->viewp++)->opaque = LSTAT(path, &st) ? 0 : st.st_ino;
		state.in_2d--;
		path[n] = 0;
		while (pathnext(state.path.name, NiL, &visits))
		{
			state.in_2d = 2;
			dirp->viewp->dirp = OPENDIR(state.path.name);
			state.in_2d = 0;
			if (dirp->viewp->dirp)
			{
				n = strlen(state.path.name);
				state.path.name[n] = '/';
				strcpy(state.path.name + n + 1, state.opaque);
				dirp->viewp->level = state.path.level;
				(dirp->viewp++)->opaque = LSTAT(state.path.name, &st) ? 0 : st.st_ino;
				state.path.name[n] = 0;
			}
		}
		errno = oerrno;
		(dirp->viewp--)->dirp = 0;
		if (dirp->viewp <= dirp->view) dirp->overlay = 0;
		else if (!(dirp->overlay = hashalloc(NiL, HASH_set, HASH_ALLOCATE, 0)))
		{
			closedir(dirp);
			intercepted = 0;
			return 0;
		}
	}
	dirp->viewp = dirp->view;
	CHEATDIR(dirp);
	intercepted = 0;
	return dirp;
}

int
closedir3d(register DIR* dirp)
{
	intercepted++;
	if (dirp)
	{
		for (dirp->viewp = dirp->view; dirp->viewp->dirp; dirp->viewp++) CLOSEDIR(dirp->viewp->dirp);
		if (dirp->overlay) hashfree(dirp->overlay);
		if (!state.freedirp) state.freedirp = (void*)dirp;
		else free((char*)dirp);
	}
	intercepted++;
	return 0;
}

struct dirent*
readdir3d(register DIR* dirp)
{
	register struct DIRdirent*	dp;
	register Dir_physical_t*	covered;

	if (intercepted++)
	{
		intercepted--;
		return READDIR(dirp);
	}
	for (;;)
	{
		while (!(dp = (struct DIRdirent*)READDIR(dirp->viewp->dirp)))
		{
			if (!(++dirp->viewp)->dirp)
			{
				intercepted--;
				return 0;
			}
			CHEATDIR(dirp);
		}
#if _mem_d_fileno_dirent
		if (!state.in_2d)
		{
			register char*	s = dp->d_name;

			/*
			 * skip opaque and hidden entries
			 */

			if (*s++ == '.' && *s++ == '.' && *s++ == '.')
			{
				/*
				 * the following for old opaque
				 */

				if (!*s && !dirp->viewp->opaque)
					dirp->viewp->opaque = dp->d_fileno;
				continue;
			}
		}
#endif

		/*
		 * NOTE: this (slimey) method assumes READDIR() returns .
		 */

		if (dirp->overlay && (!dirp->boundary || dp->d_name[0] != '.' || dp->d_name[1]))
		{
			if ((covered = (Dir_physical_t*)hashget(dirp->overlay, dp->d_name)) && covered < dirp->viewp) continue;
			if ((dirp->viewp + 1)->dirp) hashput(dirp->overlay, 0, (char*)dirp->viewp);
		}
#if _mem_d_fileno_dirent
		if (dp->d_fileno == dirp->viewp->opaque) continue;
#endif
		intercepted--;
		return (struct dirent*)dp;
	}
	/*NOTREACHED*/
}

#undef	seekdir

#define OFFBITS		(CHAR_BIT*sizeof(long)-TABBITS)
#define SETPOS(l,o)	((((long)(l))<<OFFBITS)|GETOFF(o))
#define GETOFF(p)	((p)&((((long)1)<<OFFBITS)-1))
#define GETLEV(p)	(((p)>>OFFBITS)&((((long)1)<<TABBITS)-1))

void
seekdir3d(register DIR* dirp, long pos)
{
	register int	n;
	int		lev;

	if (intercepted++)
	{
		intercepted--;
		SEEKDIR(dirp, pos);
		return;
	}
	if (pos)
	{
		lev = GETLEV(pos);
		pos = GETOFF(pos);
		for (n = 0; dirp->view[n].dirp; n++)
			if (lev == dirp->view[n].level)
				break;
	}
	else n = 0;
	if (dirp->view[n].dirp)
	{
		SEEKDIR(dirp->view[n].dirp, pos);
		dirp->viewp = &dirp->view[n];
		while (dirp->view[++n].dirp)
			SEEKDIR(dirp->view[n].dirp, 0L);
	}
}

#undef	telldir

long
telldir3d(DIR* dirp)
{
	if (intercepted++)
	{
		intercepted--;
		return TELLDIR(dirp);
	}
	return SETPOS(dirp->viewp->level, TELLDIR(dirp->viewp->dirp));
}

#undef	rewinddir

void
rewinddir3d(register DIR* dirp)
{
	seekdir(dirp, 0L);
}

#if !_nosys_readdir64

struct dirent64*
readdir643d(register DIR* dirp)
{
	register struct dirent64*	dp;
	register Dir_physical_t*	covered;

	if (intercepted++)
	{
		intercepted--;
		return READDIR64(dirp);
	}
	for (;;)
	{
		while (!(dp = READDIR64(dirp->viewp->dirp)))
		{
			if (!(++dirp->viewp)->dirp)
			{
				intercepted--;
				return 0;
			}
			CHEATDIR(dirp);
		}
#if _mem_d_fileno_dirent
		if (!state.in_2d)
		{
			register char*	s = dp->d_name;

			/*
			 * skip opaque and hidden entries
			 */

			if (*s++ == '.' && *s++ == '.' && *s++ == '.')
			{
				/*
				 * the following for old opaque
				 */

				if (!*s && !dirp->viewp->opaque)
					dirp->viewp->opaque = dp->d_fileno;
				continue;
			}
		}
#endif

		/*
		 * NOTE: this (slimey) method assumes READDIR() returns .
		 */

		if (dirp->overlay && (!dirp->boundary || dp->d_name[0] != '.' || dp->d_name[1]))
		{
			if ((covered = (Dir_physical_t*)hashget(dirp->overlay, dp->d_name)) && covered < dirp->viewp)
				continue;
			if ((dirp->viewp + 1)->dirp)
				hashput(dirp->overlay, 0, (char*)dirp->viewp);
		}
#if _mem_d_fileno_dirent
		if (dp->d_fileno == dirp->viewp->opaque)
			continue;
#endif
		intercepted--;
		return dp;
	}
	/*NOTREACHED*/
}

#undef	seekdir

#define OFFBITS64	(CHAR_BIT*sizeof(off64_t)-TABBITS)
#define SETPOS64(l,o)	((((off64_t)(l))<<OFFBITS)|GETOFF(o))
#define GETOFF64(p)	((p)&((((off64_t)1)<<OFFBITS)-1))
#define GETLEV64(p)	(((p)>>OFFBITS)&((((off64_t)1)<<TABBITS)-1))

void
seekdir643d(register DIR* dirp, off64_t pos)
{
	register int	n;
	int		lev;

	if (intercepted)
	{
		SEEKDIR64(dirp, pos);
		return;
	}
	if (pos)
	{
		lev = GETLEV(pos);
		pos = GETOFF(pos);
		for (n = 0; dirp->view[n].dirp; n++)
			if (lev == dirp->view[n].level)
				break;
	}
	else
		n = 0;
	if (dirp->view[n].dirp)
	{
		SEEKDIR64(dirp->view[n].dirp, pos);
		dirp->viewp = &dirp->view[n];
		while (dirp->view[++n].dirp)
			SEEKDIR64(dirp->view[n].dirp, (off64_t)0);
	}
}

#undef	telldir

off64_t
telldir643d(DIR* dirp)
{
	if (intercepted)
		return TELLDIR64(dirp);
	return SETPOS(dirp->viewp->level, TELLDIR64(dirp->viewp->dirp));
}

#undef	rewinddir

void
rewinddir643d(register DIR* dirp)
{
	if (intercepted)
	{
		seekdir(dirp, (off64_t)0);
		return;
	}
	seekdir((DIR*)dirp->viewp->dirp, (off64_t)0);
}

#endif

#if !_mangle_syscall && (!_lib_opendir || !_hdr_dirent)

#include "dirlib.h"

#undef	_dir_ok

#define DIR		DIRDIR
#undef	DIRDIR
#define opendir		OPENDIR
#if defined(SYS3D_opendir)
#undef	OPENDIR
#endif
#define readdir		READDIR
#if defined(SYS3D_readdir)
#undef	READDIR
#endif
#define seekdir		SEEKDIR
#if defined(SYS3D_seekdir)
#undef	SEEKDIR
#endif
#define telldir		TELLDIR
#if defined(SYS3D_telldir)
#undef	TELLDIR
#endif
#define closedir	CLOSEDIR
#if defined(SYS3D_closedir)
#undef	CLOSEDIR
#endif

#define id		sys_id
#define freedirp	sys_freedirp

#ifndef	DIRBLKSIZ
#ifdef	DIRBLK
#define DIRBLKSIZ	DIRBLK
#else
#ifdef	DIRBUF
#define DIRBLKSIZ	DIRBUF
#else
#define DIRBLKSIZ	8192
#endif
#endif
#endif

#include "getdents.c"
#include "opendir.c"
#include "readdir.c"
#include "telldir.c"
#include "seekdir.c"

#if !_nosys_readdir64

#undef	dirent
#define dirent		dirent64
#undef	getdents
#define getdents	getdents64
#define DIR		DIRDIR
#undef	DIRDIR
#undef	readdir
#define readdir		READDIR64
#if defined(SYS3D_readdir64)
#undef	READDIR64
#endif

#if !_nosys_seekdir64
#undef	seekdir
#define seekdir		SEEKDIR64
#if defined(SYS3D_seekdir64)
#undef	SEEKDIR64
#endif
#endif

#if !_nosys_telldir64
#undef	telldir
#define telldir		TELLDIR64
#if defined(SYS3D_telldir64)
#undef	TELLDIR64
#endif
#endif

#include "readdir.c"

#if !_nosys_telldir64
#include "telldir.c"
#endif

#if !_nosys_seekdir64
#include "seekdir.c"
#endif

#endif

#endif
