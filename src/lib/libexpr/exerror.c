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
