/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2011 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * output control mark on fd
 */

#include "cslib.h"

int
cscontrol(register Cs_t* state, int fd)
{
#if CS_LIB_SOCKET
	return send(fd, "", 1, MSG_OOB) == 1 ? 0 : -1;
#else
#if CS_LIB_STREAM
	struct strbuf	buf;

	buf.maxlen = 0;
	return putmsg(fd, NiL, &buf, RS_HIPRI);
#else
	return write(fd, "", 1) == 1 ? 0 : -1;
#endif
#endif
}

int
_cs_control(int fd)
{
	return cscontrol(&cs, fd);
}
