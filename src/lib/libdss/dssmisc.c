/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 2002-2006 AT&T Corp.                  *
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
 * dss misc support
 */

#include "dsshdr.h"

/*
 * compile an expression
 * input is from string s or stream sp if s==0
 */

Dssexpr_t*
dsscomp(Dss_t* dss, const char* s, Sfio_t* sp)
{
	Cxexpr_t*	expr;

	if (cxpush(dss->cx, NiL, sp, s, -1))
		return 0;
	expr = cxcomp(dss->cx);
	while (!dss->cx->eof && !cxpop(dss->cx));
	return expr;
}

/*
 * called once before the first dsseval(expr)
 */

int	
dssbeg(Dss_t* dss, Dssexpr_t* expr)
{
	return cxbeg(dss->cx, expr, dss->meth->name);
}

/*
 * evaluate expr
 */

int	
dsseval(Dss_t* dss, Dssexpr_t* expr, Dssrecord_t* record)
{
	Cxoperand_t	cv;

	if (!expr->begun && dssbeg(dss, expr))
		return -1;
	return cxeval(dss->cx, expr, record, &cv);
}

/*
 * called once after the last dsseval(expr)
 * there can be multiple dssbeg ... dsseval ... dssend sequences
 */

int	
dssend(Dss_t* dss, Dssexpr_t* expr)
{
	return cxend(dss->cx, expr);
}

/*
 * list expr
 */

int	
dsslist(Dss_t* dss, Dssexpr_t* expr, Sfio_t* sp)
{
	return cxlist(dss->cx, expr, sp);
}

/*
 * free expr
 */

int	
dssfree(Dss_t* dss, Dssexpr_t* expr)
{
	return cxfree(dss->cx, expr);
}

/*
 * return variable value in value
 * type!=0 casts to type, variable->type by default
 * details!=0 is optional type format details
 */

int
dssget(Dssrecord_t* record, Dssvariable_t* variable, Dsstype_t* type, const char* details, Dssvalue_t* value)
{
	Cxoperand_t	ret;

	if (cxcast(record->file->dss->cx, &ret, variable, type, record, details))
		return -1;
	*value = ret.value;
	return 0;
}

/*
 * return type pointer given name
 */

Dsstype_t*
dsstype(Dss_t* dss, const char* name)
{
	return cxtype(dss->cx, name, dss->disc);
}

/*
 * return variable pointer given name
 */

Dssvariable_t*
dssvariable(Dss_t* dss, const char* name)
{
	return cxvariable(dss->cx, name, NiL, dss->disc);
}
