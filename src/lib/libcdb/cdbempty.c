/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1997-2003 AT&T Corp.                *
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
