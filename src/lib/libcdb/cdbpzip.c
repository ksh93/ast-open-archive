/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1997-2004 AT&T Corp.                  *
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
 * cdb pzip stack method
 */

#include "cdblib.h"

#include <pzip.h>

/*
 * return -1 if cdb->io is not a pzip file
 * io position is not advanced
 */

static int
pziprecognize(register Cdb_t* cdb)
{
	return sfdcpzip(cdb->io, NiL, PZ_STAT, NiL) > 0 ? 0 : -1;
}

/*
 * stack methods push an sfio discipline
 * and then get out of the way
 */

static int
pzipevent(register Cdb_t* cdb, int op)
{
	Pzdisc_t*	disc;

	switch (op)
	{
	case CDB_PUSH:
		if (cdb->disc->details)
		{
			if (!(disc = vmnewof(cdb->vm, 0, Pzdisc_t, 1, 0)))
			{
				cdbnospace(cdb);
				return -1;
			}
			disc->version = PZ_VERSION;
			disc->options = cdb->disc->details;
			disc->errorf = cdb->disc->errorf;
		}
		else
			disc = 0;
		return sfdcpzip(cdb->io, NiL, 0, disc);
	case CDB_CLOSE:
		break;
	}
	return 0;
}

Cdbmeth_t	_Cdbpzip =
{
	"pzip",
	"pzip compression",
	".pz",
	CDB_STACK,
	pzipevent,
	pziprecognize,
	0,
	0,
	0,
	0,
	0,
	0,
	&_Cdbgzip
};

__DEFINE__(Cdbmeth_t*, Cdbpzip, &_Cdbpzip);
