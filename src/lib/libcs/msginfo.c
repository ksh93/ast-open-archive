/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1990-2002 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*******************************************************************/
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
