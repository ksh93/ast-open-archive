/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1996-2005 AT&T Corp.                  *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                            by AT&T Corp.                             *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                   Phong Vo <kpv@research.att.com>                    *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#include "rshdr.h"

/*
 * create temp stream ready for write
 */

#if __STD_C
Sfio_t* rstempwrite(Rs_t* rs, Sfio_t* sp)
#else
Sfio_t* rstempwrite(rs, sp)
Rs_t*	rs;
Sfio_t*	sp;
#endif
{
	Sfio_t*		op = sp;

	if ((sp || (sp = sftmp(0))) && (rs->events & RS_TEMP_WRITE) && rsnotify(rs, RS_TEMP_WRITE, sp, (Void_t*)0, rs->disc) < 0)
	{
		if (!op)
			sfclose(sp);
		sp = 0;
	}
	return sp;
}

/*
 * rewind temp stream and prepare for read
 */

#if __STD_C
int rstempread(Rs_t* rs, Sfio_t* sp)
#else
int rstempread(rs, sp)
Rs_t*	rs;
Sfio_t*	sp;
#endif
{
	int	n;

	if (rs->events & RS_TEMP_READ)
	{
		if ((n = rsnotify(rs, RS_TEMP_READ, sp, (Void_t*)0, rs->disc)) < 0)
			return -1;
		if (n)
			return 0;
	}
	return sfseek(sp, (Sfoff_t)0, SEEK_SET) ? -1 : 0;
}

/*
 * close temp stream
 */

#if __STD_C
int rstempclose(Rs_t* rs, Sfio_t* sp)
#else
int rstempclose(rs, sp)
Rs_t*	rs;
Sfio_t*	sp;
#endif
{
	int	n;

	if (rs->events & RS_TEMP_CLOSE)
	{
		if ((n = rsnotify(rs, RS_TEMP_CLOSE, sp, (Void_t*)0, rs->disc)) < 0)
			return -1;
		if (n)
			return 0;
	}
	return sfclose(sp);
}
