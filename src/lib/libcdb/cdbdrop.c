/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1997-2002 AT&T Corp.                *
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
 * cdb record cache support
 */

#include "cdblib.h"

/*
 * drop the given record from the cache
 * the record must have been previously returned by cdbcache()
 */

int
cdbdrop(Cdb_t* cdb, register Cdbrecord_t* rp)
{
	if (rp->cache.next)
		rp->cache.next->cache.prev = rp->cache.prev;
	if (rp->cache.prev)
		rp->cache.prev->cache.next = rp->cache.next;
	else
		cdb->cache = rp->cache.next;
	rp->cache.next = cdb->free;
	cdb->free = rp;
	return 0;
}
