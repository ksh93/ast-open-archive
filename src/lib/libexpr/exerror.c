/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1989-2005 AT&T Corp.                  *
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
 * expression library
 */

#include <exlib.h>

/*
 * library error handler
 */

void
exerror(const char* format, ...)
{
	Sfio_t*	sp;

	if (expr.program->disc->errorf && !expr.program->errors && (sp = sfstropen()))
	{
		va_list	ap;
		char*	s;
		char	buf[64];

		expr.program->errors = 1;
		excontext(expr.program, buf, sizeof(buf));
		sfputr(sp, buf, -1);
		va_start(ap, format);
		sfvprintf(sp, format, ap);
		va_end(ap);
		s = sfstruse(sp);
		(*expr.program->disc->errorf)(expr.program, expr.program->disc, (expr.program->disc->flags & EX_FATAL) ? 3 : 2, "%s", s);
		sfclose(sp);
	}
	else if (expr.program->disc->flags & EX_FATAL)
		exit(1);
}
