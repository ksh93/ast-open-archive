/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1986-2003 AT&T Corp.                *
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
 * common preprocessor macro reference handler
 */

#include "pplib.h"

void
ppmacref(struct ppsymbol* sym, char* file, int line, int type, unsigned long sum)
{
	register char*	p;

	NoP(file);
	NoP(line);
	p = (pp.state & (DIRECTIVE|JOINING)) == DIRECTIVE ? pp.outp : pp.addp;
	p += sfsprintf(p, MAXTOKEN, "\n#%s %d", pp.lineid, error_info.line);
	p += sfsprintf(p, MAXTOKEN, "\n#%s %s:%s %s %d", dirname(PRAGMA), pp.pass, keyname(X_MACREF), sym->name, type);
	if (type > 0)
	{
		if (sym->macro && sym->macro->value)
			sum = strsum(sym->macro->value, (long)sym->macro->arity);
		p += sfsprintf(p, MAXTOKEN, " %lu", sum);
	}
	if ((pp.state & (DIRECTIVE|JOINING)) == DIRECTIVE)
	{
		pp.outp = p;
		ppcheckout();
	}
	else
	{
		*p++ = '\n';
		pp.addp = p;
		pp.state |= ADD;
	}
	pp.pending = pppendout();
}
