/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1986-2000 AT&T Corp.              *
*      and it may only be used by you under license from       *
*                     AT&T Corp. ("AT&T")                      *
*       A copy of the Source Code Agreement is available       *
*              at the AT&T Internet web site URL               *
*                                                              *
*     http://www.research.att.com/sw/license/ast-open.html     *
*                                                              *
*     If you received this software without first entering     *
*       into a license with AT&T, you have an infringing       *
*           copy and cannot use it without violating           *
*             AT&T's intellectual property rights.             *
*                                                              *
*               This software was created by the               *
*               Network Services Research Center               *
*                      AT&T Labs Research                      *
*                       Florham Park NJ                        *
*                                                              *
*             Glenn Fowler <gsf@research.att.com>              *
*                                                              *
***************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * preprocessor builtin macro support
 */

#include "pplib.h"

#include <times.h>

/*
 * process a #(...) builtin macro call
 * `#(' has already been seen
 */

void
ppbuiltin(void)
{
	register int		c;
	register char*		p;
	register char*		a;

	int			n;
	int			op;
	char*			token;
	long			number;
	struct ppinstk*		in;
	struct pplist*		list;
	struct ppsymbol*	sym;

	number = pp.state;
	pp.state |= DISABLE|FILEPOP|NOSPACE;
	token = pp.token;
	p = pp.token = pp.tmpbuf;
	*(a = pp.args) = 0;
	if ((c = pplex()) != T_ID)
	{
		error(2, "%s: #(<identifier>...) expected", p);
		*p = 0;
	}
	switch (op = (int)hashget(pp.strtab, p))
	{
	case V_DEFAULT:
		n = 0;
		p = pp.token = pp.valbuf;
		if ((c = pplex()) == ',')
		{
			op = -1;
			c = pplex();
		}
		pp.state &= ~NOSPACE;
		for (;;)
		{
			if (!c)
			{
				error(2, "%s in #(...) argument", pptokchr(c));
				break;
			}
			if (c == '(') n++;
			else if (c == ')' && !n--) break;
			else if (c == ',' && !n && op > 0) op = 0;
			if (op) pp.token = pp.toknxt;
			c = pplex();
		}
		*pp.token = 0;
		pp.token = token;
		pp.state = number;
		break;
	case V_EMPTY:
		p = pp.valbuf;
		if ((c = pplex()) == ')') *p = '1';
		else
		{
			*p = '0';
			n = 0;
			for (;;)
			{
				if (!c)
				{
					error(2, "%s in #(...) argument", pptokchr(c));
					break;
				}
				if (c == '(') n++;
				else if (c == ')' && !n--) break;
				c = pplex();
			}
		}
		*(p + 1) = 0;
		pp.token = token;
		pp.state = number;
		break;
	case V_ITERATE:
		n = 0;
		pp.token = pp.valbuf;
		if ((c = pplex()) != T_ID || !(sym = ppsymref(pp.symtab, pp.token)) || !sym->macro || sym->macro->arity != 1 || (c = pplex()) != ',')
		{
			error(2, "#(%s <macro(x)>, ...) expected", p);
			for (;;)
			{
				if (!c)
				{
					error(2, "%s in #(...) argument", pptokchr(c));
					break;
				}
				if (c == '(') n++;
				else if (c == ')' && !n--) break;
				c = pplex();
			}
			*pp.valbuf = 0;
		}
		else while (c != ')')
		{
			p = pp.token;
			if (pp.token > pp.valbuf) *pp.token++ = ' ';
			STRCOPY(pp.token, sym->name, a);
			*pp.token++ = '(';
			if (!c || !(c = pplex()))
			{
				pp.token = p;
				error(2, "%s in #(...) argument", pptokchr(c));
				break;
			}
			pp.state &= ~NOSPACE;
			while (c)
			{
				if (c == '(') n++;
				else if (c == ')' && !n--) break;
				else if (c == ',' && !n) break;
				pp.token = pp.toknxt;
				c = pplex();
			}
			*pp.token++ = ')';
			pp.state |= NOSPACE;
		}
		p = pp.valbuf;
		pp.token = token;
		pp.state = number;
		break;
	default:
		pp.token = token;
		while (c != ')')
		{
			if (!c)
			{
				error(2, "%s in #(...) argument", pptokchr(c));
				break;
			}
			if ((c = pplex()) == T_ID && !*a)
				strcpy(a, pp.token);
		}
		pp.state = number;
		switch (op)
		{
		case V_ARGC:
			c = -1;
			for (in = pp.in; in; in = in->prev)
				if ((in->type == IN_MACRO || in->type == IN_MULTILINE) && (in->symbol->flags & SYM_FUNCTION))
				{
					c = *((unsigned char*)(pp.macp->arg[0] - 2));
					break;
				}
			sfsprintf(p = pp.valbuf, MAXTOKEN, "%d", c);
			break;
		case V_BASE:
			p = (a = strrchr(error_info.file, '/')) ? a + 1 : error_info.file;
			break;
		case V_DATE:
			if (!(p = pp.date))
			{
				time_t	tm;

				time(&tm);
				a = p = ctime(&tm) + 4;
				*(p + 20) = 0;
				for (p += 7; *p = *(p + 9); p++);
				pp.date = p = strdup(a);
			}
			break;
		case V_FILE:
			p = error_info.file;
			break;
		case V_LINE:
			sfsprintf(p = pp.valbuf, MAXTOKEN, "%d", error_info.line);
			break;
		case V_PATH:
			p = pp.path;
			break;
		case V_STDC:
			p = pp.valbuf;
#if __sun__ || __sun || sun
			p[0] = ((pp.state & (COMPATIBILITY|TRANSITION)) || (pp.mode & HOSTED)) ? '0' : '1';
#else
			p[0] = (pp.state & (COMPATIBILITY|TRANSITION)) ? '0' : '1';
#endif
			p[1] = 0;
			break;
		case V_TIME:
			if (!(p = pp.time))
			{
				time_t	tm;

				time(&tm);
				p = ctime(&tm) + 11;
				*(p + 8) = 0;
				pp.time = p = strdup(p);
			}
			break;
		case V_VERSION:
			p = (char*)pp.version;
			break;
		case V_DIRECTIVE:
			pp.state |= NEWLINE;
			pp.mode |= RELAX;
			strcpy(p = pp.valbuf, "#");
			break;
		case V_GETENV:
			if (!(p = getenv(a))) p = "";
			break;
		case V_GETMAC:
			p = (sym = pprefmac(a, REF_NORMAL)) ? sym->macro->value : "";
			break;
		case V_GETOPT:
			sfsprintf(p = pp.valbuf, MAXTOKEN, "%ld", ppoption(a));
			break;
		case V_GETPRD:
			p = (list = (struct pplist*)hashget(pp.prdtab, a)) ? list->value : "";
			break;
		default:
			if (pp.builtin && (a = (*pp.builtin)(pp.valbuf, p, a))) p = a;
			break;
		}
		break;
	}
	if (strchr(p, MARK))
	{
		a = pp.tmpbuf;
		strcpy(a, p);
		c = p != pp.valbuf;
		p = pp.valbuf + c;
		for (;;)
		{
			if (p < pp.valbuf + MAXTOKEN - 2) switch (*p++ = *a++)
			{
			case 0:
				break;
			case MARK:
				*p++ = MARK;
				/*FALLTHROUGH*/
			default:
				continue;
			}
			break;
		}
		p = pp.valbuf + c;
	}
	if (p == pp.valbuf) PUSH_STRING(p);
	else
	{
		if (p == pp.valbuf + 1) *pp.valbuf = '"';
		else
		{
			if (strlen(p) > MAXTOKEN - 2) error(1, "%-.16s: builtin value truncated", p);
			sfsprintf(pp.valbuf, MAXTOKEN, "\"%-.*s", MAXTOKEN - 2, p);
		}
		PUSH_QUOTE(pp.valbuf, 1);
	}
}
