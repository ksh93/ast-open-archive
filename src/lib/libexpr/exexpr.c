/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1989-2003 AT&T Corp.                *
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
*                 Phong Vo <kpv@research.att.com>                  *
*                                                                  *
*******************************************************************/
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
