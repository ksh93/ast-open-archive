/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1987-2004 AT&T Corp.                *
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
*                                                                  *
*******************************************************************/
#pragma prototyped

/*
 * <tv_sec,tv_usec> interface implementation
 */

#include <tv.h>
#include <tm.h>

#include "FEATURE/tv"

int
tvgettime(Tv_t* tv)
{
#if _lib_clock_gettime && defined(CLOCK_REALTIME)
	struct timespec	s;

	clock_gettime(CLOCK_REALTIME, &s);
	tv->tv_sec = s.tv_sec;
	tv->tv_nsec = s.tv_nsec;
	return 0;
#else
	struct timeval	v;

	tmtimeofday(&v);
	tv->tv_sec = v.tv_sec;
	tv->tv_nsec = v.tv_usec * 1000;
#endif
	return 0;
}

int
tvcmp(register const Tv_t* a, register const Tv_t* b)
{
	if ((unsigned long)a->tv_sec < (unsigned long)b->tv_sec)
		return 1;
	if ((unsigned long)a->tv_sec > (unsigned long)b->tv_sec)
		return -1;
	if ((unsigned long)a->tv_nsec < (unsigned long)b->tv_nsec)
		return 1;
	if ((unsigned long)a->tv_nsec > (unsigned long)b->tv_nsec)
		return -1;
	return 0;
}

int
tvgetstat(register const struct stat* st, register Tv_t* av, register Tv_t* mv, register Tv_t* cv)
{
	if (av)
	{
		av->tv_sec = st->st_atime;
		av->tv_nsec = ST_ATIME_NSEC_GET(st);
	}
	if (mv)
	{
		mv->tv_sec = st->st_mtime;
		mv->tv_nsec = ST_MTIME_NSEC_GET(st);
	}
	if (cv)
	{
		cv->tv_sec = st->st_ctime;
		cv->tv_nsec = ST_CTIME_NSEC_GET(st);
	}
	return 0;
}

int
tvsetstat(register struct stat* st, register const Tv_t* av, register const Tv_t* mv, register const Tv_t* cv)
{
	if (av)
	{
		st->st_atime = av->tv_sec;
		ST_ATIME_NSEC_SET(st, av->tv_nsec);
	}
	if (mv)
	{
		st->st_mtime = mv->tv_sec;
		ST_MTIME_NSEC_SET(st, mv->tv_nsec);
	}
	if (cv)
	{
		st->st_ctime = cv->tv_sec;
		ST_CTIME_NSEC_SET(st, cv->tv_nsec);
	}
	return 0;
}

int
tvtouch(const char* path, register const Tv_t* av, register const Tv_t* mv, const Tv_t* cv, int copy)
{
#if _lib_utimes
	struct stat	st;
	struct timeval	now;
	struct timeval	am[2];

	if (!av || !mv)
	{
		if (!copy)
			tmtimeofday(&now);
		else if (stat(path, &st))
			return -1;
	}
	if (av)
	{
		am[0].tv_sec = av->tv_sec;
		am[0].tv_usec = av->tv_nsec / 1000;
	}
	else if (!copy)
		am[0] = now;
	else
	{
		am[0].tv_sec = st.st_atime;
		am[0].tv_usec = ST_ATIME_NSEC_GET(&st) / 1000;
	}
	if (mv)
	{
		am[1].tv_sec = mv->tv_sec;
		am[1].tv_usec = mv->tv_nsec / 1000;
	}
	else if (!copy)
		am[1] = now;
	else
	{
		am[1].tv_sec = st.st_mtime;
		am[1].tv_usec = ST_MTIME_NSEC_GET(&st) / 1000;
	}
	return utimes(path, am);
#else
	return touch(path, av ? av->tv_sec : 0, mv ? mv->tv_sec : 0, 0);
#endif
}
