/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1990-2000 AT&T Corp.              *
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
*             Glenn Fowler <gsf@research.att.com>              *
*              David Korn <dgk@research.att.com>               *
*               Eduardo Krell <ekrell@adexus.cl>               *
*                                                              *
***************************************************************/
#pragma prototyped

#include "3d.h"

#if _hdr_utime
#include <utime.h>
#else
struct utimbuf
{
	time_t  actime;
	time_t  modtime;
};
#endif

#define COPYBUF		8096

#if __sun__ || sun

#if _lib_mmap && (_hdr_mman || _sys_mman)

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide mmap munmap
#else
#define mmap		______mmap
#define munmap		______munmap
#endif

#if _hdr_mman
#include <mman.h>
#else
#include <sys/mman.h>
#endif

#define COPYMAP		(32*COPYBUF)

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide mmap munmap
#else
#undef	mmap
#undef	munmap
#endif

extern int		munmap(caddr_t, size_t);
extern void*		mmap(void*, size_t, int, int, int, off_t);

#endif

#endif

/*
 * copy rfd to wfd
 * return 0 if success, otherwise -1.
 */

int
fs3d_copy(int rfd, int wfd, struct stat* st)
{
	register ssize_t	n;
	ssize_t			filesize;
	struct stat		stbuf;
	struct utimbuf		ut;
	char			buf[COPYBUF];

#ifdef COPYMAP
	off_t			offset;
	size_t			munmapsize;
	size_t			mapsize;
	char*			mapbuf;
#endif

	if (!st && FSTAT(rfd, st = &stbuf)) return -1;
	if (!S_ISREG(st->st_mode)) return 0;
	if ((filesize = st->st_size) <= 0)
		filesize = 1;
#ifdef COPYMAP
	mapsize = (filesize < COPYMAP) ? filesize : COPYMAP;
	if ((filesize > COPYBUF) && ((mapbuf = mmap((caddr_t)0, mapsize, PROT_READ, MAP_SHARED, rfd, 0)) != ((caddr_t)-1)))
	{
		offset = 0;
		munmapsize = mapsize;
		for (;;)
		{
			if (write(wfd, mapbuf, mapsize) != mapsize)
			{
				munmap(mapbuf, munmapsize);
				return -1;
			}
			filesize -= mapsize;
			if (filesize <= 0) break;
			offset += mapsize;
			if (filesize < mapsize) mapsize = filesize;
			if (mmap(mapbuf, mapsize, PROT_READ, MAP_SHARED|MAP_FIXED, rfd, offset) == (caddr_t)-1)
			{
				munmap(mapbuf, munmapsize);
				return -1;
			}
		}
		munmap(mapbuf, munmapsize);
	}
#endif
	if (filesize > 0)
	{
		while ((n = read(rfd, buf, COPYBUF)) > 0)
			if (write(wfd, buf, n) != n) 
				return -1;
		if (n < 0 || lseek(wfd, (off_t)0, 0)) return -1;
	}
	ut.actime = st->st_atime;
	ut.modtime = st->st_mtime;
	UTIME(state.path.name, &ut);
	return 0;
}
