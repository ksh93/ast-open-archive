/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1986-2000 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*                 This software was created by the                 *
*                 Network Services Research Center                 *
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
 * miscellaneous preprocessor support
 */

#include "pplib.h"

/*
 * macro symbol def|ref
 */

struct ppsymbol*
pprefmac(char* name, int ref)
{
	register struct ppsymbol*	sym;

	if (!(sym = ppsymget(pp.symtab, name)) && (ref <= REF_NORMAL && pp.macref || ref == REF_CREATE || ref == REF_DELETE && (pp.mode & (INIT|READONLY))))
	{
		if ((pp.state & COMPILE) && pp.truncate && strlen(name) > pp.truncate)
			name[pp.truncate] = 0;
		sym = ppsymset(pp.symtab, NiL);
	}
	if (sym && ref <= REF_NORMAL)
	{
		if (pp.macref) (*pp.macref)(sym, error_info.file, error_info.line, ref == REF_NORMAL && (pp.state & CONDITIONAL) ? REF_IF : ref, 0L);
		if (!sym->macro) sym = 0;
	}
#if COMPATIBLE
	if (!(pp.state & COMPATIBILITY))
#endif
	if (ref == REF_IF && sym && (sym->flags & SYM_PREDEFINED) && *name != '_' && !(pp.mode & (HOSTED|INACTIVE)))
	{
		if (pp.state & STRICT)
		{
			error(1, "%s: obsolete predefined symbol reference disabled", name);
			return(0);
		}
		error(1, "%s: obsolete predefined symbol referenced", name);
	}
	return(sym);
}

/*
 * common predicate assertion operations
 * op is DEFINE or UNDEF
 */

void
ppassert(int op, char* pred, char* args)
{
	register struct pplist*		a;
	register struct ppsymbol*	sym;
	register struct pplist*		p;
	register struct pplist*		q;

	if (!args) switch (op)
	{
	case DEFINE:
		goto mark;
	case UNDEF:
		a = 0;
		goto unmark;
	}
	if (a = (struct pplist*)hashget(pp.prdtab, pred))
	{
		p = 0;
		q = a;
		while (q)
		{
			if (streq(q->value, args))
			{
				if (op == DEFINE) return;
				q = q->next;
				if (p) p->next = q;
				else a = q;
			}
			else
			{
				p = q;
				q = q->next;
			}
		}
		if (op == UNDEF)
		{
		unmark:
			hashput(pp.prdtab, pred, a);
			if (sym = ppsymref(pp.symtab, pred))
				sym->flags &= ~SYM_PREDICATE;
			return;
		}
	}
	if (op == DEFINE)
	{
		p = newof(0, struct pplist, 1, 0);
		p->next = a;
		p->value = strdup(args);
		hashput(pp.prdtab, NiL, p);
	mark:
		if ((pp.state & COMPILE) && pp.truncate) return;
		if (sym = ppsymset(pp.symtab, pred))
			sym->flags |= SYM_PREDICATE;
	}
}

/*
 * parse a predicate argument list
 * the args are placed in pp.args
 * the first non-space/paren argument token type is returned
 * forms:
 *
 *	predicate <identifier>			type=T_ID
 *	predicate ( <identifier> )		type=T_ID
 *	predicate ( )				type=0
 *	predicate ( <balanced-paren-list> )	type=T_STRING
 *	otherwise				type=<other>
 */

int
pppredargs(void)
{
	register int	c;
	register int	n;
	register int	type;
	char*		pptoken;

	pptoken = pp.token;
	pp.token = pp.args;
	switch (type = pplex())
	{
	case '(':
		type = 0;
		n = 1;
		pp.state |= HEADER;
		pp.state &= ~STRIP;
		c = pplex();
		pp.state &= ~NOSPACE;
		for (;;)
		{
			switch (c)
			{
			case '(':
				n++;
				break;
			case '\n':
				ungetchr(c);
				error(2, "missing %d )%s in predicate argument list", n, n == 1 ? "" : "'s");
				type = 0;
				goto done;
			case ')':
				if (!--n) goto done;
				break;
			}
			pp.token = pp.toknxt;
			if (c != ' ')
			{
				if (type) type = T_STRING;
				else type = (c == T_ID) ? T_ID : T_STRING;
			}
			c = pplex();
		}
	done:
		pp.state &= ~HEADER;
		pp.state |= NOSPACE|STRIP;
		if (pp.token > pp.args && *(pp.token - 1) == ' ') pp.token--;
		*pp.token = 0;
		break;
	case '\n':
		ungetchr('\n');
		type = 0;
		break;
	}
	pp.token = pptoken;
	return(type);
}
