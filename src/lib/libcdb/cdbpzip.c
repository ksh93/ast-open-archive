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
