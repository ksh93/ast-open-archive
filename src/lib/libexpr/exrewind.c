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

int
exrewind(Expr_t* ex)
{
	register int	n;

	if (ex->linewrap)
	{
		exerror("too much pushback");
		return -1;
	}
	if (!ex->input->pushback && !(ex->input->pushback = oldof(0, char, sizeof(ex->line), 3)))
	{
		exerror("out of space [rewind]");
		return -1;
	}
	if (n = ex->linep - ex->line)
		memcpy(ex->input->pushback, ex->line, n);
	if (ex->input->peek)
	{
		ex->input->pushback[n++] = ex->input->peek;
		ex->input->peek = 0;
	}
	ex->input->pushback[n++] = ' ';
	ex->input->pushback[n] = 0;
	ex->input->pp = ex->input->pushback;
	ex->input->nesting = ex->nesting;
	setcontext(ex);
	return 0;
}

void
exstatement(Expr_t* ex)
{
	ex->nesting = ex->input->nesting;
	setcontext(ex);
}
