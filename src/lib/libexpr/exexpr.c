/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1989-2004 AT&T Corp.                  *
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
 * expression library
 */

#include "exlib.h"

/*
 * return the expression for name or sym coerced to type
 */

Exnode_t*
exexpr(Expr_t* ex, const char* name, Exid_t* sym, int type)
{
	if (ex)
	{
		if (!sym)
			sym = name ? (Exid_t*)dtmatch(ex->symbols, name) : &ex->main;
		if (sym && sym->lex == PROCEDURE && sym->value)
		{
			if (type != DELETE)
				return excast(ex, sym->value->data.procedure.body, type, NiL, 0);
			exfreenode(ex, sym->value);
			sym->lex = NAME;
			sym->value = 0;
		}
	}
	return 0;
}
