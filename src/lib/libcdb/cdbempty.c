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
