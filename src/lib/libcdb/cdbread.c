/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1997-2000 AT&T Corp.              *
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
 * cdb read function
 * in case the macro is bypassed
 */

#include "cdblib.h"

#undef	cdbread

Cdbrecord_t*
cdbread(Cdb_t* cdb, Cdbkey_t* key)
{
	return _cdb_read(cdb, key);
}
