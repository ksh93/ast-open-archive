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
*                  David Korn <dgk@research.att.com>                   *
*                   Eduardo Krell <ekrell@adexus.cl>                   *
*                                                                      *
***********************************************************************/
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

#if _lib_mmap

#define COPYMAP		(32*COPYBUF)

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
