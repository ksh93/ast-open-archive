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
 * make n empty fields starting at dp
 */

#include "cdblib.h"

int
cdbempty(Cdb_t* cdb, register Cdbdata_t* dp, register Cdbformat_t* fp, size_t n)
{
	register Cdbdata_t*	ep;

	for (ep = dp + n; dp < ep; dp++, fp++)
		if (dp->flags & (CDB_FLOATING|CDB_INTEGER|CDB_STRING))
		{
			dp->string.base = "";
			dp->string.length = 0;
			dp->number.floating = 0;
			dp->flags = CDB_CACHED|CDB_TERMINATED;
			if ((fp->flags & (CDB_NEGATIVE|CDB_SPACE|CDB_ZERO)) != (CDB_NEGATIVE|CDB_SPACE|CDB_ZERO))
				switch (fp->type)
				{
				case CDB_FLOATING:
					SETFLOATING(dp, fp, 0);
					break;
				case CDB_INTEGER:
					SETINTEGER(dp, fp, 0);
					break;
				case CDB_STRING:
					if (!(fp->flags & CDB_SPACE))
						fp->flags |= CDB_STRING;
					break;
				}
		}
	return 0;
}
