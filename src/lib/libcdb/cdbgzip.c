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
 * cdb gzip stack method
 */

#include "cdblib.h"

#include <sfdcgzip.h>

/*
 * return -1 if cdb->io is not a gzip file
 * io position is not advanced
 */

static int
gziprecognize(register Cdb_t* cdb)
{
	return sfdcgzip(cdb->io, SFGZ_VERIFY) > 0 ? 0 : -1;
}

/*
 * stack methods push an sfio discipline
 * and then get out of the way
 */

static int
gzipevent(register Cdb_t* cdb, int op)
{
	switch (op)
	{
	case CDB_PUSH:
		return sfdcgzip(cdb->io, 0);
	case CDB_CLOSE:
		break;
	}
	return 0;
}

Cdbmeth_t	_Cdbgzip =
{
	"gzip",
	"gzip compression",
	".gz",
	CDB_STACK,
	gzipevent,
	gziprecognize,
	0,
	0,
	0,
	0,
	0,
	0,
	0
};

__DEFINE__(Cdbmeth_t*, Cdbgzip, &_Cdbgzip);
