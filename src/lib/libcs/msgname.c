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
*                                                              *
***************************************************************/
#pragma prototyped

#include "msglib.h"

/*
 * return name given call
 */

const char*
msgname(register unsigned long call)
{
	register int		n;

	static char		buf[12];

	if ((n = MSG_CALL(call)) > MSG_STD)
	{
		sfsprintf(buf, sizeof(buf), "user_%d", n);
		return (const char*)buf;
	}
	switch (MSG_VAR(call))
	{
	case MSG_VAR_FILE:
		if (MSG_ARG(call, 1) == MSG_ARG_file)
		{
			sfsprintf(buf, sizeof(buf), "f%s", msg_info.name[n]);
			return (const char*)buf;
		}
		break;
	case MSG_VAR_IPC:
		sfsprintf(buf, sizeof(buf), "ipc%s", msg_info.name[n]);
		return (const char*)buf;
	case MSG_VAR_SYM:
		sfsprintf(buf, sizeof(buf), "sym%s", msg_info.name[n]);
		return (const char*)buf;
	}
	return msg_info.name[n];
}
