/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1997-2001 AT&T Corp.                *
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
*                 This software was created by the                 *
*                 Network Services Research Center                 *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*******************************************************************/
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
	return sfdcpzip(cdb->io, NiL, NiL, PZ_STAT) > 0 ? 0 : -1;
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
			disc->errorf = (Pzerror_f)cdb->disc->errorf;
		}
		else
			disc = 0;
		return sfdcpzip(cdb->io, disc, NiL, 0);
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
