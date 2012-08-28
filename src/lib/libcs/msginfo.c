/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1990-2005 AT&T Corp.                  *
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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * system call message data initialization
 */

#include "msglib.h"

Msg_info_t	msg_info =
{
	MSG_TIMEOUT,
	0,
	{
	"NOP",
	"break",	"chmod",	"chown",	"close",
	"control",	"dup",		"exec",		"exit",
	"fork",		"getdents",	"kill",		"link",
	"lock",		"mkdir",	"mknod",	"mount",
	"open",		"pathconf",	"pipe",		"read",
	"remove",	"rename",	"rmdir",	"seek",
	"stat",		"statfs",	"sync",		"truncate",
	"utime",	"wait",		"write",
	}
};
