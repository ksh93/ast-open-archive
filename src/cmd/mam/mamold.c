/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1989-2002 AT&T Corp.                *
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
 * AT&T Bell Laboratories
 *
 * convert a make abstract machine stream on stdin
 * to an oldmake makefile on stdout
 */

static const char id[] = "\n@(#)$Id: mamold (gsf@research.att.com) 1989-03-22 $\0\n";

#include <ast.h>
#include <mam.h>
#include <ctype.h>
#include <error.h>
#include <stdio.h>

#define LONGLINE	72		/* too long output line length	*/

#define A_listprereq	(A_LAST<<1)	/* prereqs listed		*/
#define A_listtarg	(A_LAST<<2)	/* listed as target		*/

struct state				/* program state		*/
{
	int		graph;		/* output dependency graph info	*/
	int		header;		/* header supplied		*/
	int		heredoc;	/* last value had <<		*/
	struct mam*	mam;		/* make abstract machine info	*/
	struct block*	omit;		/* dir prefixes to omit		*/
};

static struct state	state;

/*
 * clear listprereq for all prerequisites
 */

static void
clrprereqs(register struct rule* r)
{
	register struct list*	p;

	r->attributes &= ~A_listprereq;
	for (p = r->prereqs; p; p = p->next)
		if (p->rule->attributes & A_listprereq)
			clrprereqs(p->rule);
	for (p = r->implicit; p; p = p->next)
		if (p->rule->attributes & A_listprereq)
			clrprereqs(p->rule);
}

/*
 * dump a value that may be expanded by oldmake
 */

static int
dumpvalue(register int col, register char* s, int sep)
{
	register int	c;
	register char*	v;
	int		dollar;
	int		escape = 0;
	int		quote = 0;

	if (sep)
	{
		sfputc(sfstdout, sep);
		col++;
	}
	for (;;) switch (c = *s++)
	{
	case 0:
		if (sep && sep != '\t')
		{
			col = 1;
			sfputc(sfstdout, '\n');
		}
		return(col);
	case ' ':
	case '\t':
		if (sep == '\t' && state.heredoc || col < LONGLINE - 8) goto emit;
		while (isspace(*s)) s++;
		if (*s)
		{
			sfputr(sfstdout, " \\\n\t", -1);
			col = 8;
		}
		break;
	case '#':
		sfputr(sfstdout, "$(sharp)", -1);
		col += 8;
		break;
	case '<':
		if (sep == '\t' && *s == c) state.heredoc = 1;
		goto emit;
	case '\'':
		quote = !quote;
		goto emit;
	case '\\':
		if (*s != '$' && *s != '\'') goto emit;
		s++;
		escape = -1;
		/*FALLTHROUGH*/
	case '$':
		escape++;
		dollar = 1;
		if (isalpha(*s) || *s == '_')
		{
			for (v = s; isalnum(*v) || *v == '_'; v++);
			c = *v;
			*v = 0;
			if (getvar(state.mam->main, s))
			{
				sfprintf(sfstdout, "$(%s)", s);
				col += (v - s) + 3;
				*(s = v) = c;
				escape = 0;
				break;
			}
			*v = c;
		}
		if (escape)
		{
			escape = 0;
			if (!quote) switch (*s)
			{
			case '{':
				if (*(v = s + 1))
					v++;
				while (isalnum(*v) || *v == '_')
					v++;
				switch (*v)
				{
				case ':':
				case '-':
				case '+':
					break;
				default:
					dollar = 0;
					break;
				}
				break;
			case '$':
				s--;
				do
				{
					sfputc(sfstdout, '$');
					col++;
				} while (*++s == '$');
				break;
			default:
				sfputc(sfstdout, '\\');
				col++;
				break;
			}
		}
		else dollar = 0;
		c = '$';
		if (dollar)
		{
			sfputc(sfstdout, c);
			col++;
		}
		/*FALLTHROUGH*/
	default:
	emit:
		sfputc(sfstdout, c);
		col++;
		break;
	}
}

/*
 * dump a name keeping track of the right margin
 */

static int
dumpname(int col, char* s)
{
	register int	n;

	if (!state.graph)
	{
		n = strlen(s);
		if (col + n >= LONGLINE)
		{
			sfputr(sfstdout, " \\\n\t\t", -1);
			col = 16;
		}
		else if (col <= 1) col = 1;
		else
		{
			sfputc(sfstdout, ' ');
			col++;
		}
		col += n;
	}
	else if (col++ > 1) sfputc(sfstdout, ' ');
	dumpvalue(0, s, 0);
	return(col);
}

/*
 * dump an action
 */

static void
dumpaction(register struct block* p)
{
	if (p)
	{
		state.heredoc = 0;
		for (;;)
		{
			dumpvalue(0, p->data, '\t');
			if (!(p = p->next)) break;
			sfputr(sfstdout, "$(newline)", -1);
			if (!state.heredoc) sfputr(sfstdout, " \\\n", -1);
		}
		sfputc(sfstdout, '\n');
	}
}

/*
 * dump r and its implicit prerequisites
 */

static int
dumpprereqs(register int col, register struct rule* r)
{
	register struct block*	d;
	register struct list*	p;

	if (!(r->attributes & A_listprereq))
	{
		r->attributes |= A_listprereq;
		for (d = state.omit; d; d = d->next)
			if (strmatch(r->name, d->data))
				return(col);
		col = dumpname(col, r->name);
		for (p = r->implicit; p; p = p->next)
			col = dumpprereqs(col, p->rule);
	}
	return(col);
}

/*
 * dump the rules
 */

static void
dump(register struct rule* r)
{
	register int		col;
	register struct list*	p;

	if (!(r->attributes & (A_listtarg|A_metarule)))
	{
		r->attributes |= A_listtarg;
		if (r->action || r->prereqs)
		{
			clrprereqs(r);
			r->attributes |= A_listprereq;
			sfputc(sfstdout, '\n');
			col = dumpname(1, r->name);
			col = dumpname(col, ":");
			for (p = r->prereqs; p; p = p->next)
				if (!(p->rule->attributes & A_listprereq))
				{
					clrprereqs(p->rule);
					col = dumpprereqs(col, p->rule);
				}
			sfputc(sfstdout, '\n');
			if (!state.graph) dumpaction(r->action);
		}
		for (p = r->prereqs; p; p = p->next)
			if (p->rule != r)
				dump(p->rule);
		for (p = r->implicit; p; p = p->next)
			if (p->rule != r)
				dump(p->rule);
	}
}

/*
 * dump var definition
 */

static int
dumpvar(const char* an, char* av, void* handle)
{
	char*		name = (char*)an;
	struct var*	v = (struct var*)av;
	register char*	s;
	register char*	t;
	register int	c;

	NoP(handle);
	if (*v->value)
	{
		s = t = v->value;
		while (c = *t++ = *s++)
		{
			if (c == '\\')
			{
				if (!(*t++ = *s++)) break;
			}
			else if (c == '"') t--;
		}
		dumpvalue(dumpname(0, name), v->value, '=');
	}
	return(0);
}

/*
 * add prefix to list of dir prefixes to omit
 */

static void
omit(char* prefix)
{
	int		n;
	struct block*	p;

	n = strlen(prefix);
	p = newof(0, struct block, 1, n + 1);
	strcpy(p->data = (char*)p + sizeof(struct block), prefix);
	strcpy(p->data + n, "*");
	p->next = state.omit;
	state.omit = p;
}

main(int argc, char** argv)
{
	register int		c;
	register struct list*	p;

	NoP(argc);
	error_info.id = "mamold";
	while (c = optget(argv, "d#[debug]gh:[header]x:[omit-dir]")) switch (c)
	{
	case 'd':
		error_info.trace = -opt_info.num;
		break;
	case 'g':
		state.graph = 1;
		break;
	case 'h':
		state.header = 1;
		sfputr(sfstdout, opt_info.arg, '\n');
		break;
	case 'x':
		omit(opt_info.arg);
		break;
	case '?':
		error(ERROR_USAGE|4, opt_info.arg);
		break;
	case ':':
		error(2, opt_info.arg);
		break;
	}
	if (error_info.errors) error(ERROR_USAGE|4, optusage(NiL));

	/*
	 * initialize
	 */

	omit("/usr/include");
	omit("/");

	/*
	 * scan, collect and dump
	 */

	if (!state.graph && !state.header)
	{
		sfprintf(sfstdout, "# # oldmake makefile generated by mamold # #\n");
		sfprintf(sfstdout, "# oldmake ... null='' sharp='$(null)#' newline='$(null)\n");
		sfprintf(sfstdout, "# '\n");
		sfprintf(sfstdout, "newline=;\n");
	}
	if (!(state.mam = mamalloc()))
		error(3, "cannot initialize");
	mamvar(state.mam->main, "HOME", "");
	mamvar(state.mam->main, "newline", "");
	if (mamscan(state.mam, NiL) < 0)
		error(3, "invalid input");
	if (!state.graph) hashwalk(state.mam->main->vars, 0, dumpvar, NiL);
	for (p = state.mam->main->root->prereqs; p; p = p->next)
		dump(p->rule);
	exit(error_info.errors != 0);
}
