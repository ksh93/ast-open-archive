/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1997-2005 AT&T Corp.                  *
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
