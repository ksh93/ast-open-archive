/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1986-2004 AT&T Corp.                  *
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
