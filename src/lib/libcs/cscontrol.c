/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1990-2004 AT&T Corp.                  *
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
