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
