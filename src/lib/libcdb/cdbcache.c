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
 * cdb record cache support
 */

#include "cdblib.h"

/*
 * cache the given record and return a pointer to the cached version
 * preallocate a new current record for the next read
 */

Cdbrecord_t*
cdbcache(Cdb_t* cdb, Cdbrecord_t* op)
{
	register Cdbrecord_t*	rp;
	register Cdbdata_t*	dp;
	register Cdbdata_t*	ep;
	register char*		s;
	register int		n;

	rp = op;
	if (rp->cache.next || rp->cache.prev)
	{
		if (cdb->disc->errorf)
			(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: (%08llo) record already cached", cdb->path, rp->offset);
		return 0;
	}
	if (rp->cache.next = cdb->cache)
		cdb->cache->cache.prev = rp;
	cdb->cache = rp;
	do
	{
		for (ep = (dp = rp->data) + rp->schema->strings; dp < ep; dp++)
			if ((dp->flags & (CDB_STRING|CDB_CACHED)) == CDB_STRING)
			{
				n = dp->string.length;
				if (!(s = vmoldof(rp->vm, 0, char, n, 1)))
					goto nospace;
				dp->string.base = (char*)memcpy(s, dp->string.base, n);
				s[n] = 0;
			}
	} while (rp = rp->next);
	if (rp = cdb->free)
	{
		cdb->free = cdb->free->cache.next;
		cdb->record = rp;
		rp->cache.next = rp->cache.prev = 0;
	}
	else if (cdbdata(cdb) < 0)
		goto nospace;
	return op;
 nospace:
	cdbnospace(cdb);
	return 0;
}
