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
 * cdb vdelta stack method
 */

#include "cdblib.h"

#include <vdelta.h>

/*
 * stub for now
 */

static int
sfvdelta(Sfio_t* sp)
{
	return -1;
}

/*
 * return -1 if cdb->io is not a vdelta file
 * io position is not advanced
 */

static int
vdeltarecognize(register Cdb_t* cdb)
{
	register unsigned char*	s;
	register unsigned char*	m;
	register int		n;
	register int		z;

	z = sizeof(VD_MAGIC) - 1;
	if (!(s = (unsigned char*)sfreserve(cdb->io, z, SF_LOCKR)))
		return -1;
	m = (unsigned char*)VD_MAGIC;
	for (n = 0; n < z; n++)
		if (s[n] != m[n])
		{
			m = (unsigned char*)VD_MAGIC_OLD;
			for (n = 0; n < z; n++)
				if (s[n] != m[n])
					break;
			if (n == (z - 1) && s[n] == '1')
				n++;
			break;
		}
	sfread(cdb->io, s, 0);
	return (n < z) ? -1 : 0;
}

/*
 * stack methods push an sfio discipline
 * and then get out of the way
 */

static int
vdeltaevent(register Cdb_t* cdb, int op)
{
	switch (op)
	{
	case CDB_PUSH:
		return sfvdelta(cdb->io);
	case CDB_CLOSE:
		break;
	}
	return 0;
}

Cdbmeth_t	_Cdbvdelta =
{
	"vdelta",
	"vdelta compression",
	".vz",
	CDB_STACK,
	vdeltaevent,
	vdeltarecognize,
	0,
	0,
	0,
	0,
	0,
	0,
	&_Cdbpzip
};

__DEFINE__(Cdbmeth_t*, Cdbvdelta, &_Cdbvdelta);
