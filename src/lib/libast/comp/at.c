/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1985-2013 AT&T Intellectual Property          *
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
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * *at() emulation -- if this code is active for your time critical app then upgrade your system
 */

#ifndef _AST_INTERCEPT
#define _AST_INTERCEPT	0
#endif

#include "astlib.h"

#include <aso.h>
#include <error.h>

#ifndef ENOSYS
#define ENOSYS	EINVAL
#endif

#if defined(_fd_self_dir_fmt) || defined(_fd_pid_dir_fmt)

#if defined(_fd_self_dir_fmt)

#define ATBEG(cwd,path) \
	{ \
		char	_at_##path[PATH_MAX]; \
		if (cwd >= 0) \
			sfsprintf(_at_##path, sizeof(_at_##path), _fd_self_dir_fmt, cwd, "/", path);

#define ATBEGL(lwd,link) \
	{ \
		char	_at_##link[PATH_MAX]; \
		if (lwd >= 0) \
			sfsprintf(_at_##link, sizeof(_at_##link), _fd_self_dir_fmt, lwd, "/", link);

#else

#define ATBEG(cwd,path) \
	{ \
		char	_at_##path[PATH_MAX]; \
		if (cwd >= 0) \
			sfsprintf(_at_##path, sizeof(_at_##path), _fd_pid_dir_fmt, getpid(), cwd, "/", path);

#define ATBEGL(lwd,path) \
	{ \
		char	_at_##link[PATH_MAX]; \
		if (lwd >= 0) \
			sfsprintf(_at_##link, sizeof(_at_##link), _fd_pid_dir_fmt, getpid(), lwd, "/", link);

#endif

#define ATPATH(cwd,path) \
		((cwd>=0)?(const char*)_at_##path:path)

#define ATEND() \
	}

#define ATENDL() \
	}

#else

static unsigned int	_at_lock;

#define ATBEG(cwd,path) \
	{ \
		int		_at_dot = -1; \
		int		_at_ret; \
		unsigned int	_at_tid; \
		if (cwd >= 0) \
		{ \
			sigcrit(SIG_REG_ALL); \
			_at_tid = asothreadid(); \
			if (_at_lock == _at_tid) \
				_at_tid = 0; \
			else \
				asolock(&_at_lock, _at_tid, ASO_LOCK); \
			if ((_at_dot = open(".", O_SEARCH|O_CLOEXEC)) < 0 || fchdir(cwd)) \
			{ \
				if (_at_tid) \
					asolock(&_at_lock, _at_tid, ASO_UNLOCK); \
				sigcrit(SIG_REG_POP); \
				return -1; \
			} \
		}

#define ATPATH(cwd,path) \
		path

#define ATEND() \
		if (_at_dot >= 0) \
		{ \
			_at_ret = fchdir(_at_dot_); \
			close(_at_dot_); \
			if (_at_tid) \
				asolock(&_at_lock, _at_tid, ASO_UNLOCK); \
			sigcrit(SIG_REG_POP); \
			if (_at_ret) \
				return -1; \
		} \
	}

#endif

#define STUB	1

#if !_lib_faccessat

#undef	STUB

int
faccessat(int cwd, const char* path, mode_t mode, int flags)
{
	int	r;

	ATBEG(cwd, path);
	r = (flags & AT_EACCESS) ? eaccess(ATPATH(cwd, path), mode) : access(ATPATH(cwd, path), mode);
	ATEND();
	return r;
}

#endif

#if !_lib_fchmodat

int
fchmodat(int cwd, const char* path, mode_t mode, int flags)
{
	int	r;

	ATBEG(cwd, path);
	if (flags & AT_SYMLINK_NOFOLLOW)
	{
#if _lib_lchmod
		r = lchmod(ATPATH(cwd, path), mode);
#else
		errno = ENOSYS;
		r = -1;
#endif
	}
	else
		r = chmod(ATPATH(cwd, path), mode);
	ATEND();
	return r;
}

#endif

#if !_lib_fchownat

#undef	STUB

int
fchownat(int cwd, const char* path, uid_t owner, gid_t group, int flags)
{
	int	r;

	ATBEG(cwd, path);
	if (flags & AT_SYMLINK_NOFOLLOW)
	{
#if _lib_lchown
		r = lchown(ATPATH(cwd, path), owner, group);
#else
		errno = ENOSYS;
		r = -1;
#endif
	}
	else
		r = chown(ATPATH(cwd, path), owner, group);
	ATEND();
	return r;
}

#endif

#if !_lib_fstatat

#undef	STUB

int
fstatat(int cwd, const char* path, struct stat* st, int flags)
{
	int	r;

	ATBEG(cwd, path);
	r = (flags & AT_SYMLINK_NOFOLLOW) ? lstat(ATPATH(cwd, path), st, flags) : stat(ATPATH(cwd, path), st, flags);
	ATEND();
	return r;
}

#endif

#if !_lib_linkat

#undef	STUB

int
linkat(int pwd, const char* path, int lwd, const char* linkpath, int flags)
{
	int	r;

	if (pwd < 0 || *path == '/')
	{
		if (lwd < 0 || *linkpath == '/')
			r = link(path, linkpath);
		else
		{
			ATBEG(lwd, linkpath);
			r = link(path, ATPATH(lwd, linkpath));
			ATEND();
		}
	}
	else if (lwd < 0 || *linkpath == '/')
	{
		ATBEG(pwd, path);
		r = link(ATPATH(pwd, path), linkpath);
		ATEND();
	}
	else
	{
#ifdef BEGL
		ATBEG(pwd, path);
		ATBEGL(lwd, linkpath);
		r = link(ATPATH(pwd, path), ATPATH(lwd, linkpath));
		ATEND();
		ATEND();
#else
		errno = EINVAL;
		r = -1;
#endif
	}
	return r;
}

#endif

#if !_lib_mkdirat

#undef	STUB

int
mkdirat(int cwd, const char* path, mode_t mode)
{
	int	r;

	ATBEG(cwd, path);
	r = mkdir(ATPATH(cwd, path), mode);
	ATEND();
	return r;
}

#endif

#if !_lib_mkfifoat

#undef	STUB

int
mkfifoat(int cwd, const char* path, mode_t mode)
{
	int	r;

	ATBEG(cwd, path);
	r = mkfifo(ATPATH(cwd, path), mode);
	ATEND();
	return r;
}

#endif

#if !_lib_mknodat

#undef	STUB

int
mknodat(int cwd, const char* path, mode_t mode, dev_t dev)
{
	int	r;

	ATBEG(cwd, path);
	r = mknod(ATPATH(cwd, path), mode, dev);
	ATEND();
	return r;
}

#endif

#if !_lib_openat

#undef	STUB

int
openat(int cwd, const char* path, int flags, ...)
{
	int	r;
	mode_t	mode;
	va_list	ap;

	va_start(ap, flags);
	mode = (flags & O_CREAT) ? va_arg(ap, mode_t) : (mode_t)0;
	va_end(ap);
	ATBEG(cwd, path);
	r = open(ATPATH(cwd, path), flags, mode);
	ATEND();
	return r;
}

#endif

#if !_lib_readlinkat

#undef	STUB

ssize_t
readlinkat(int cwd, const char* path, void* buf, size_t size)
{
	ssize_t	r;

	ATBEG(cwd, path);
	r = readlink(ATPATH(cwd, path), buf, size);
	ATEND();
	return r;
}

#endif

#if !_lib_symlinkat

#undef	STUB

int
symlinkat(const char* path, int cwd, const char* linkpath)
{
	int	r;

	ATBEG(cwd, linkpath);
	r = symlink(path, ATPATH(cwd, linkpath));
	ATEND();
	return r;
}

#endif

#if !_lib_unlinkat

#undef	STUB

int
unlinkat(int cwd, const char* path, int flags)
{
	int	r;

	ATBEG(cwd, path);
	r = unlink(ATPATH(cwd, path), flags);
	ATEND();
	return r;
}

#endif

#if STUB

void _STUB_at(){}

#endif
