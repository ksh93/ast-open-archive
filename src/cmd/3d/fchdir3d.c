/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2012 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                 Eclipse Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
*                                                                      *
*                A copy of the License is available at                 *
*          http://www.eclipse.org/org/documents/epl-v10.html           *
*         (with md5 checksum b35adb5213ca9657e911e9befb180842)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                   Eduardo Krell <ekrell@adexus.cl>                   *
*                                                                      *
***********************************************************************/
#pragma prototyped

#include "3d.h"

#if defined(fchdir3d)

#if defined(AT_FDCWD)

char*
fdirpath(int fd, const char* path, char* buf, size_t size, const char* call)
{
	int		r;
	int		k;
	size_t		n;
	Dir_t*		dp;
	struct stat	st;

	if (fd == AT_FDCWD || path && *path == '/')
		return (char*)path;
	if (fd < 0 || fd >= elementsof(state.file))
		return 0;

	/*
	 * technically the second STAT() should always happen but
	 * for efficiency we limit it to the fchdir() case (path==0)
	 */

	k = 0;
	if ((dp = state.file[fd].dir) && !FSTAT(fd, &st) && dp->dev == st.st_dev && dp->ino == st.st_ino && (path || !STAT(dp->path, &st) && (k = 1) && dp->dev == st.st_dev && dp->ino == st.st_ino))
	{
		if (path)
		{
			sfsprintf(buf, size, "%s/%s", dp->path, path);
			message((-2, "fdirpath: %s(%d %s) => %s state.file [%d]", call, fd, path, buf, state.level));
			return buf;
		}
		message((-2, "fdirpath: %s(%d %s) => %s state.file [%d]", call, fd, path, dp->path, state.level));
		return dp->path;
	}
	if (state.pwd && !FCHDIR(fd))
	{
		r = !getcwd(buf, size);
		CHDIR(state.pwd);
		if (!r)
		{
			if (dp)
			{
				state.file[fd].dir = 0;
				free(dp);
			}
			n = strlen(buf);
			if (dp = newof(0, Dir_t, 1, n))
			{
				strcpy(dp->path, buf);
				if (!k)
					STAT(buf, &st);
				dp->dev = st.st_dev;
				dp->ino = st.st_ino;
				state.file[fd].dir = dp;
			}
			if (path)
				sfsprintf(buf + n, size - n, "/%s", path);
			message((-2, "fdirpath: %s(%d %s) => %s getcwd [%d]", call, fd, path, buf, state.level));
			return buf;
		}
	}
	message((-2, "fdirpath: %s(%d %s) => 0 [%d]", call, fd, path, state.level));
	return 0;
}

#endif

int
fchdir3d(int fd)
{
	char*		dir;
	char		buf[PATH_MAX];

	if (dir = fdirpath(fd, NiL, buf, sizeof(buf), "fchdir"))
		return chdir(dir);
	return FCHDIR(fd);
}

#else

NoP(fchdir)

#endif

#if defined(faccessat3d) && defined(fchdir3d)

int
faccessat3d(int fd, const char* path, int mode, int flags)
{
	int		pflags;
	char*		dir;
	char		buf[PATH_MAX];

	if (!state.in_2d && (dir = fdirpath(fd, path, buf, sizeof(buf), "faccessat")))
	{
		pflags = P_TOP;
		if (state.safe)
			pflags |= P_SAFE;
		if (flags & AT_SYMLINK_NOFOLLOW)
			pflags |= P_LSTAT;
		if (!(dir = pathreal(dir, pflags, NiL)))
			return -1;
		path = (const char*)dir;
	}
	return FACCESSAT(fd, path, mode, flags);
}

#else

NoP(faccessat)

#endif

#if defined(fstatat3d) && defined(fchdir3d)

/* the 5 arg _fxstatat() disrupts our proto game -- every party needs one */
#if defined(__STDC__) || defined(__cplusplus) || defined(_proto) || defined(c_plusplus)
int fstatat3d(int fd, const char* path, struct stat* st, int flags)
#else
#if defined(_FSTATAT)
int _fstatat(fd, path, st, flags) int fd; char* path; struct stat* st; { return fstatat(fd, path, st, flags); }
#endif
int fstatat(fd, path, st, flags) int fd; char* path; struct stat* st;
#endif
{
	char*		sp;
#if FS
	Mount_t*	mp;
	int		op;
#endif
#ifdef _3D_STAT_VER
	struct stat*	so;
	struct stat	sc;
#endif
	char		buf[PATH_MAX];

#ifdef _3D_STAT_VER
	if (_3d_ver != _3D_STAT_VER)
	{
		so = st;
		st = &sc;
	}
#endif
#if FS
	if (!fscall(NiL, MSG_fstatat, 0, fd, path, st, flags))
		return state.ret;
	mp = monitored();
#endif
	if (sp = fdirpath(fd, path, buf, sizeof(buf), "fstatat"))
		path = (const char*)sp;
	if (!(sp = pathreal(path, (flags & AT_SYMLINK_NOFOLLOW) ? P_LSTAT : 0, st)))
		return -1;
#if FS
	op = (flags & AT_SYMLINK_NOFOLLOW) ? MSG_lstat : MSG_stat;
	if (mp)
		fscall(mp, op, 0, state.path.name, st);
	for (mp = state.global; mp; mp = mp->global)
		if (fssys(mp, op))
			fscall(mp, op, 0, state.path.name, st);
#endif
#ifdef _3D_STAT_VER
	if (st == &sc)
	{
		st = so;
		if (FXSTATAT(fd, _3d_ver, sp, st, flags))
			return -1;
#ifdef _3D_STAT64_VER
		if (_3d_ver == _3D_STAT64_VER)
			IVIEW(((struct stat64*)st), state.path.level);
		else
#endif
		IVIEW(st, state.path.level);
	}
#endif
#if _mem_d_type_dirent
	if (S_ISDIR(st->st_mode))
		st->st_nlink = _3D_LINK_MAX;
#endif
	return 0;
}

#if defined(_LARGEFILE64_SOURCE) && defined(STAT643D) && !defined(_3D_STAT64_VER)

int
fstatat643d(int fd, const char* path, struct stat64* st, int flags)
{
	char*		sp;
	struct stat	ss;
	char		buf[PATH_MAX];
#if FS
	int		op;
	Mount_t*	mp;

	if (!fscall(NiL, MSG_fstatat, 0, fd, path, &ss, flags))
		return state.ret;
	mp = monitored();
#endif
	if (sp = fdirpath(fd, path, buf, sizeof(buf), "fstatat64"))
		path = (const char*)sp;
	if (!(sp = pathreal(path, (flags & AT_SYMLINK_NOFOLLOW) ? P_LSTAT : 0, &ss)))
		return -1;
	if ((flags & AT_SYMLINK_NOFOLLOW) ? LSTAT64(sp, st) : STAT64(sp, st))
		return -1;
	IVIEW(st, state.path.level);
#if FS
	op = (flags & AT_SYMLINK_NOFOLLOW) ? MSG_lstat : MSG_stat;
	if (mp)
		fscall(mp, op, 0, state.path.name, &ss);
	for (mp = state.global; mp; mp = mp->global)
		if (fssys(mp, op))
			fscall(mp, op, 0, state.path.name, &ss);
#endif
#if _mem_d_type_dirent
	if (S_ISDIR(st->st_mode))
		st->st_nlink = _3D_LINK_MAX;
#endif
	return 0;
}

#endif

#else

NoP(fstatat)

#endif

#if defined(openat3d) && defined(fchdir3d)

int
openat3d(int fd, const char* path, int oflag, ...)
{
	char*		dir;
	mode_t		mode;
	char		buf[PATH_MAX];
	va_list		ap;

	va_start(ap, oflag);
	mode = (oflag & O_CREAT) ? va_arg(ap, int) : 0;
	va_end(ap);
	if (!state.in_2d && (dir = fdirpath(fd, path, buf, sizeof(buf), "openat")))
		return open(dir, oflag, mode);
	return OPENAT(fd, path, oflag, mode);
}

#else

NoP(openat)

#endif

#if defined(unlinkat3d) && defined(fchdir3d)

int
unlinkat3d(int fd, const char* path, int flags)
{
	char*		dir;
	char		buf[PATH_MAX];

	if (!state.in_2d && (dir = fdirpath(fd, path, buf, sizeof(buf), "unlinkat")))
		return (flags & AT_REMOVEDIR) ? rmdir(dir) : unlink(dir);
	return UNLINKAT(fd, path, flags);
}

#else

NoP(unlinkat)

#endif
