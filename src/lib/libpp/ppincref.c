/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1986-2000 AT&T Corp.              *
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
/*
 * Glenn Fowler
 * AT&T Research
 *
 * common include reference handler
 * the type arg is inclusive or of PP_SYNC_*
 */

#include "pplib.h"

void
ppincref(char* parent, char* file, int line, int type)
{
	NoP(parent);
	NoP(line);
	if (type & PP_SYNC_PUSH) error(0, "%s", file);
}
