/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1984-2002 AT&T Corp.                *
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
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * make language dependent scan support
 */

#include "make.h"

#include <tm.h>

#define SCAN_define		(1<<0)	/* enable D actions		*/
#define SCAN_macro		(1<<1)	/* macro (default line) style	*/
#define SCAN_nopropagate	(1<<2)	/* don't propagate scan index	*/
#define SCAN_state		(1<<3)	/* scan for state vars		*/

#define QUOTE_blank		(1<<0)	/* blank out quote		*/
#define QUOTE_comment		(1<<1)	/* comment quote type		*/
#define QUOTE_nested		(1<<2)	/* nesting quote		*/
#define QUOTE_newline		(1<<3)	/* newline terminates quote	*/
#define QUOTE_quote		(1<<4)	/* normal quote			*/
#define QUOTE_single		(1<<5)	/* quote next char		*/
#define QUOTE_space		(1<<6)	/* quote preceded by space	*/

#define ANY			'\t'	/* 0 or more internal code	*/
#define ARG			'\n'	/* grab arg internal code	*/
#define REP			'\r'	/* repeat group internal code	*/
#define SPC			' '	/* 0 or more space internal code*/

#define SCANARGS		2	/* max number % arg matches	*/
#define SCANBUFFER		4096	/* scan buffer size		*/

typedef unsigned short scanstate;

struct action				/* state action			*/
{
	short		type;		/* action type id		*/
	short		flags;		/* SCAN_* flags			*/
	char*		pattern;	/* match expression		*/
	char*		map;		/* path name map		*/
	char*		script;		/* script			*/
	struct
	{
	unsigned long	clear;		/* clear these			*/
	unsigned long	set;		/* set these			*/
	}		attribute;	/* attributes			*/
	struct
	{
	unsigned long	clear;		/* clear these			*/
	unsigned long	set;		/* set these			*/
	}		property;	/* properties			*/
	int		attrprop;	/* attribute|property mods	*/
	int		scan;		/* scan index			*/
};

struct quote				/* quote/comment match info	*/
{
	struct quote*	next;		/* next in list			*/
	char*		begin;		/* begin pattern		*/
	char*		end;		/* end pattern			*/
	int		escape;		/* end escape char		*/
	int		flags;		/* QUOTE_* flags		*/
};

struct scan				/* scan info			*/
{
	unsigned char	type[UCHAR_MAX];/* SCAN_* types indexed by char	*/
	int		flags;		/* SCAN_* flags			*/
	char*		external;	/* external scan		*/
	struct action*	before;		/* do this before scanexec	*/
	struct action*	after;		/* do this after scanexec	*/
	struct quote*	quote;		/* quote patterns		*/
	struct action*	action;		/* action table			*/
	scanstate*	state;		/* state transition table	*/
};

/*
 * default scan strategies
 */

static Namval_t		scantab[] =
{
	".IGNORE",	SCAN_IGNORE,
	".NULL",	SCAN_NULL,
	".STATE",	SCAN_STATE,
};

static struct scan*	strategy[SCAN_MAX+1];

/*
 * initialize the scan atoms
 */

void
initscan(int repeat)
{
	register int		i;
	register struct rule*	r;

	if (!repeat)
		for (i = 0; i < elementsof(scantab); i++)
		{
			r = catrule(internal.scan->name, scantab[i].name, NiL, 1);
			r->property |= P_attribute;
			r->scan = scantab[i].value;
			addprereq(internal.scan, r, PREREQ_APPEND);
		}
}

/*
 * scan pattern sort -- strcmp(3) convention
 */

static int
scansort(register const char* a, register const char* b)
{
	register const char*	s;

	while (*++a == *++b)
		if (!*a)
			return 0;
	if (*a == '*')
		return 1;
	else if (*b == '*')
		return -1;
	for (s = " *%@"; *s; s++)
		if (*a == *s)
			return 1;
		else if (*b == *s)
			return -1;
	return *a - *b;
}

/*
 * compile a branch of the expression tree
 * next state pointer returned
 */

static scanstate*
scanbranch(scanstate* u, struct action* action, struct action* first, struct action* last, register int i)
{
	register struct action*	a;
	struct action*		b;
	struct action*		m;
	scanstate*		v;

	for (a = first; a <= last; a++)
	{
		if (a->pattern)
		{
			while (a->pattern[i])
			{
				v = u + 1;
				m = a;
				for (b = a + 1; b <= last; b++)
				{
					if (b->pattern && b->pattern[i] != m->pattern[i])
					{
						*u++ = m->pattern[i];
						*u++ = 0;
						m = b;
					}
				}
				if (m != a)
				{
					*u++ = m->pattern[i];
					*u++ = 0;
					*u++ = 0;
					*u++ = 0;
					m = a;
					for (b = a + 1; b <= last; b++)
					{
						if (b->pattern && b->pattern[i] != m->pattern[i])
						{
							if ((*v = (u - v)) != (u - v))
								error(3, "pattern transition offset %d too large", (u - v));
							v += 2;
							u = scanbranch(u, action, m, b - 1, i + 1);
							m = b;
						}
					}
					if ((*v = (u - v)) != (u - v))
						error(3, "pattern transition offset %d too large", (u - v));
					return scanbranch(u, action, m, last, i + 1);
				}
				switch (*u++ = a->pattern[i++])
				{
				case REP:
				case SPC:
					*u++ = 1;
					break;
				default:
					*u++ = 3;
					*u++ = 0;
					*u++ = 0;
					break;
				}
			}
			*u++ = 0;
			*u++ = a - action;
			a->pattern = 0;
		}
	}
	return u;
}

/*
 * parse scan action for a in s
 * s points to the first delimiter
 */

static void
scanaction(struct action* a, register char* s)
{
	register int	c;
	register int	t;
	register char*	v;
	register char*	n;
	struct rule*	u;
	unsigned long	m;

	if (c = *s++)
		do
		{
			v = n = s;
			while (t = *s++)
			{
				if (t == c)
					break;
				if (t == '\\' && !(t = *s++))
					break;
				*n++ = t;
			}
			*n = 0;
			if (!*v)
				break;
			switch (*v++)
			{
			case 'A':
				n = tokopen(v, 1);
				while (v = tokread(n))
				{
					if (((t = *v) == ATTRSET || t == ATTRCLEAR) && *(v + 1))
					{
						*v = ATTRNAME;
						u = getrule(v);
						*v = t;
						if (!u)
							u = getrule(v + 1);
					}
					else
						u = getrule(v);
					if (!u || !(u->property & P_attribute))
						error(3, "%s: must be an attribute", v);
					if (u->scan)
					{
						if (!a->scan)
							a->scan = u->scan;
					}
					else if (u->attribute)
					{
						a->attrprop = 1;
						if (t == '-')
							a->attribute.clear |= u->attribute;
						else
							a->attribute.set |= u->attribute;
					}
					else if (m = u->property & (P_accept|P_after|P_always|P_archive|P_before|P_command|P_dontcare|P_force|P_foreground|P_functional|P_ignore|P_implicit|P_local|P_make|P_multiple|P_parameter|P_repeat|P_terminal|P_virtual))
					{
						a->attrprop = 1;
						if (t == '-')
							a->property.clear |= m;
						else
							a->property.set |= m;
					}
				}
				tokclose(n);
				break;
			case 'M':
				if (a->map)
				{
					error(1, "%c: multiply defined", *(v - 1));
					free(a->map);
				}
				a->map = strdup(v);
				break;
			case 'O':
				for (;;)
				{
					switch (*v++)
					{
					case 0:
						break;
					case 'X':
						a->flags |= SCAN_nopropagate;
						continue;
					default:
						error(1, "%c: invalid match option", *(v - 1));
						continue;
					}
					break;
				}
				break;
			case 'R':
				if (a->script)
				{
					error(1, "%c: multiply defined", *(v - 1));
					free(a->script);
				}
				a->script = strdup(v);
				break;
			default:
				error(1, "%c: invalid match operation", *(v - 1));
				break;
			}
		} while (t);
}

/*
 * compile scan description in r and return scan pointer
 */

static struct scan*
scancompile(struct rule* r, int flags)
{
	register char*		s;
	register struct scan*	ss;
	register struct quote*	q;
	register int		c;
	int			i;
	int			t;
	int			z;
	int			line;
	char*			file;
	char*			n;
	char*			v;
	char*			y;
	char*			x[128];
	char**			p;
	struct action*		a;
	Sfio_t*			tmp;

	z = 0;
	p = x;
	ss = newof(0, struct scan, 1, 0);
	ss->flags = flags;
	tmp = sfstropen();
	expand(tmp, r->action);
	s = sfstruse(tmp);
	file = error_info.file;
	error_info.file = r->name;
	line = error_info.line;
	error_info.line = 0;
	do
	{
		error_info.line++;
		if (n = strchr(s, '\n'))
			*n++ = 0;
		switch (*s++)
		{
		case 0:
			break;
		case 'D':
			if (!(ss->flags & SCAN_define))
				break;
			/*FALLTHROUGH*/
		case 'A':
		case 'B':
		case 'E':
		case 'I':
		case 'T':
			if (*s)
			{
				if (p >= &x[elementsof(x) - 1])
					error(3, "too many patterns");
				*p++ = s - 1;
				z += n ? (n - s) : strlen(s);
			}
			break;
		case 'F':
			ss->after = newof(0, struct action, 1, 0);
			scanaction(ss->after, s);
			break;
		case 'O':
			c = isalnum(*s) ? 0 : *s++;
			for (;;)
			{
				switch (*s++)
				{
				case 0:
					break;
				case 'M':
					ss->flags |= SCAN_macro;
					continue;
				case 'S':
					ss->flags |= SCAN_state;
					continue;
				default:
					if (*(s - 1) == c)
						break;
					error(1, "%c: invalid scan option", *(s - 1));
					continue;
				}
				break;
			}
			break;
		case 'Q':
			if (c = *s++)
			{
				q = newof(0, struct quote, 1, 0);
				q->next = ss->quote;
				ss->quote = q;
				q->end = null;
				z = 0;
				do
				{
					v = y = s;
					while (t = *s++)
					{
						if (t == c || t == '\\' && !(t = *s++))
							break;
						*y++ = t;
					}
					*y = 0;
					switch (z++)
					{
					case 0:
						q->begin = strdup(v);
						break;
					case 1:
						if (!*(q->end = strdup(v)))
							q->flags |= QUOTE_newline;
						break;
					case 2:
						q->escape = *v;
						break;
					case 3:
						for (;;)
						{
							switch (*v++)
							{
							case 0:
								break;
							case 'C':
								q->flags |= QUOTE_comment;
								continue;
							case 'L':
								q->flags |= QUOTE_newline;
								continue;
							case 'N':
								q->flags |= QUOTE_nested;
								continue;
							case 'Q':
								q->flags |= QUOTE_quote;
								continue;
							case 'S':
								q->flags |= QUOTE_single;
								continue;
							case 'W':
								q->flags |= QUOTE_space;
								continue;
							default:
								error(1, "%c: invalid quote option", *(v - 1));
								continue;
							}
							break;
						}
						break;
					case 4:
						break;
					default:
						error(1, "%s: too many quote fields", v - 1);
						break;
					}
				} while (t);
				ss->type[*q->begin] |= q->flags;
			}
			break;
		case 'S':
			ss->before = newof(0, struct action, 1, 0);
			scanaction(ss->before, s);
			break;
		case 'X':
			if (*s)
				s++;
			if (ss->external)
			{
				error(1, "%s: external scan multiply defined", s - 1);
				free(ss->external);
			}
			ss->external = strdup(s);
			break;
		default:
			error(1, "%c: invalid scan directive", *(s - 1));
			break;
		}
	} while (s = n);
	if (p > x)
	{
		ss->action = a = newof(0, struct action, p - x + 1, 0);
		ss->state = newof(0, scanstate, z * 4, 0);
		z = p - x;
		strsort(x, z, scansort);
		*p = 0;
		p = x;
		while (s = *p++)
		{
			a++;
			a->type = *s++;
			c = *s++;
			i = 0;
			a->pattern = n = s;
			while (t = *s++)
			{
				if (t == c)
					break;
				if (t == '\\')
				{
					if (!(t = *s++))
						break;
				}
				else if (isspace(t))
				{
					if (n > a->pattern && *(n - 1) == SPC)
						continue;
					t = SPC;
				}
				else if (t == '%')
				{
					i++;
					t = ARG;
				}
				else if (t == '*')
					t = ANY;
				else if (t == '@')
					t = REP;
				*n++ = t;
			}
			scanaction(a, s - 1);
			*n = 0;
			switch (a->type)
			{
			case 'A':
			case 'I':
				if (!i)
					error(3, "%% arg match missing from pattern [%s]", a->pattern);
				if (i > SCANARGS)
					error(3, "too many %% arg matches in pattern [%s]", a->pattern);
				break;
			}
		}
		scanbranch(ss->state, ss->action, ss->action + 1, ss->action + z, 0);
	}
	sfstrclose(tmp);
	error_info.file = file;
	error_info.line = line;
	maketop(r, P_virtual, NiL);
	if (r->dynamic & D_triggered)
	{
		message((-2, "%s forces re-scan", r->name));
		hashwalk(table.rule, 0, forcescan, &r->scan);
	}
	return ss;
}

/*
 * skip quotes in q
 */

static unsigned char*
scanquote(int fd, unsigned char* buf, unsigned char** p, register unsigned char* g, struct quote* q, int flags)
{
	register int		c;
	register unsigned char*	t;
	unsigned char*		x;
	unsigned char*		y;
	unsigned char*		z;
	int			e;

	for (x = g; q; q = q->next)
		if ((flags & q->flags) && (t = (unsigned char*)q->begin) && *(g - 1) == *t++ && (!(q->flags & QUOTE_space) || (g - 1) == *p || *(g - 2) == ' ' || *(g - 2) == '\t'))
		{
			for (;;)
			{
				while (!(c = *g++))
				{
					if (*p >= buf + SCANBUFFER)
					{
						c = g - *p - 1;
						memcpy(buf + SCANBUFFER - c, *p, c);
						*p = buf + SCANBUFFER - c;
					}
					x = (buf + SCANBUFFER) - (g - x - 1);
					if ((c = read(fd, g = buf + SCANBUFFER, SCANBUFFER)) <= 0)
					{
						g[0] = 0;
						return g;
					}
					g[c] = 0;
				}
				if (!*t)
				{
					if (q->flags & QUOTE_single)
						goto accept;
					e = *q->end;
					y = g;
					for (;;)
					{
						if (c == '\n' && (q->flags & QUOTE_newline) && *(g - 2) != q->escape)
						{
							g--;
							goto accept;
						}
						if (c == e && *(g - 2) != q->escape)
						{
							z = g;
							t = (unsigned char*)q->end + 1;
							/*UNDENT*/
	for (;;)
	{
		if (!*t)
		{
			if (c == '\n')
				g--;
			goto accept;
		}
		while (!(c = *g++))
		{
			if (q->flags & QUOTE_comment)
			{
				c = g - z;
				memcpy(buf + SCANBUFFER - c, z - 1, c);
				z = buf + SCANBUFFER - c;
				y = (buf + SCANBUFFER) - (g - y - 1);
				x = (buf + SCANBUFFER) - (g - x - 1);
			}
			else
			{
				c = g - *p - 1;
				memcpy(buf + SCANBUFFER - c, *p, c);
			}
			*p = buf + SCANBUFFER - c;
			if ((c = read(fd, g = buf + SCANBUFFER, SCANBUFFER)) <= 0)
			{
				g[0] = 0;
				return g;
			}
			g[c] = 0;
		}
		if (c != *t++)
		{
			g = z;
			break;
		}
	}
							/*INDENT*/
						}
						while (!(c = *g++))
						{
							if (*p >= buf + SCANBUFFER)
							{
								c = g - *p - 1;
								memcpy(buf + SCANBUFFER - c, *p, c);
								*p = buf + SCANBUFFER - c;
							}
							x = (buf + SCANBUFFER) - (g - x - 1);
							y = (buf + SCANBUFFER) - (g - y - 1);
							if ((c = read(fd, g = buf + SCANBUFFER, SCANBUFFER)) <= 0)
							{
								g[0] = 0;
								return g;
							}
							g[c] = 0;
						}
					}
				}
				if (c != *t++)
				{
					g = x;
					break;
				}
			}
		}
	return g;
 accept:
	if ((flags & (QUOTE_blank|q->flags)) == (QUOTE_blank|QUOTE_comment))
		for (x--; x < g; *x++ = ' ');
	return g;
}

/*
 * parameter definition
 */

static struct list*
scandefine(register char* s, struct list* p)
{
	register char*	t;
	char*		b;
	char*		z;
	int		c;
	struct rule*	u;
	struct var*	v;
	Sfio_t*		tmp;

	while (isspace(*s))
		s++;
	t = s;
	while (*s && istype(*s, C_ID1|C_ID2))
		s++;
	if (s > t)
	{
		c = *s;
		*s = 0;
		u = staterule(VAR, NiL, t, 1);
		if (!(v = getvar(t)))
			v = setvar(t, null, 0);
		*s = c;
		b = t = s;
		for (;;)
		{
			while (isspace(*s))
				s++;
			while (*s && !isspace(*s))
				*t++ = *s++;
			if (!*s)
				break;
			*t++ = ' ';
		}
		for (z = t; z < s; *z++ = ' ');
		*t = 0;
		if (*v->value)
		{
			tmp = sfstropen();
			sfprintf(tmp, "%s#%s", v->value, b);
			setvar(v->name, sfstruse(tmp), 0);
			sfstrclose(tmp);
		}
		else
			setvar(v->name, b, 0);
		if (!(u->mark & M_scan))
		{
			u->mark |= M_scan;
			u->property |= P_parameter;
			v->property |= V_scan;
			p = cons(u, p);
		}
		if (t < s)
			*t = ' ';
	}
	return p;
}

/*
 * macro scan -- only partial parameterization
 *
 *	include(<SP>*<FILE><SP>*)
 *	sinclude(<SP>*<FILE><SP>*)
 *	INCLUDE(<SP>*<FILE>[,<FILE>]*<SP>*)
 *	define(<SP>*<VARIABLE><SP>*,<DEFINITION><SP>*)
 *	ifelse(c,t,f)
 *	# comment
 *
 * attribute and property from first I pattern propagated to prereq files
 */

/*ARGSUSED*/
static struct list*
scanmacro(int fd, struct rule* r, struct scan* ss, register struct list* p)
{
	register int		c;
	register int		inquote;
	register char*		w;
	int			n;
	int			dontcare;
	int			ifparen;
	int			paren;
	int			h;
	int			t;
	Sfio_t*			fp;
	Sfio_t*			tmp;
	struct rule*		u;
	struct var*		v;

	if (!(fp = sfnew(NiL, NiL, SF_UNBOUND, fd, SF_READ)))
	{
		error(2, "%s: cannot fdopen", r->name);
		return p;
	}
	tmp = sfstropen();
	ifparen = inquote = paren = 0;
	while ((c = sfgetc(fp)) != EOF)
		switch (c)
		{
		case ')':
			if (!inquote)
			{
				sfstrset(tmp, 0);
				if (paren-- == ifparen)
					ifparen = 0;
			}
			break;

		case '"':
		case '#':
		case '\n':
			if (inquote == c)
			{
				inquote = 0;
				sfstrset(tmp, 0);
			}
			else if (!inquote && c != '\n')
				inquote = c == '#' ? '\n' : c;
			break;

		case '\\':
			sfstrset(tmp, 0);
			sfgetc(fp);
			break;

		default:
			if (!inquote)
			{
				if (!sfstrtell(tmp))
				{
					if (c == '(')
						paren++;
					else if (istype(c, C_VARPOS1) && state.fullscan)
					{
						h = 1;
						t = C_VARPOS1;
						sfputc(tmp, c);
					}
					else if (istype(c, C_ID1|C_ID2))
					{
						h = 0;
						t = C_ID1|C_ID2;
						sfputc(tmp, c);
					}
				}
				else
				{
					if (t != (C_ID1|C_ID2))
					{
						if (t == C_VARPOS8)
							t = C_ID1|C_ID2;
						else
							t <<= 1;
					}
					if (istype(c, t))
						sfputc(tmp, c);
					else if (t == (C_ID1|C_ID2) || !istype(c, C_ID1|C_ID2))
					{
						w = sfstruse(tmp);
						if (h && (v = getvar(w)) && (v->property & V_scan))
						{
							u = staterule(VAR, NiL, w, 1);
							if (!(u->mark & M_scan))
							{
								u->mark |= M_scan;
								p = cons(u, p);
							}
						}
						if (c == '(')
						{
							paren++;
							/*UNDENT*/
		if (*w == 's')
		{
			dontcare = 1;
			w++;
		}
		else
			dontcare = ifparen != 0;
		if (*w == 'i')
		{
			if (!strcmp(w, "include"))
			{
				sfstrset(tmp, 0);
				while ((c = sfgetc(fp)) != EOF && c != ')')
					if (!isspace(c))
						sfputc(tmp, c);
				sfungetc(fp, c);
				u = makerule(sfstruse(tmp));
				if (!(u->mark & M_scan))
				{
					u->mark |= M_scan;
					if (dontcare)
						u->property |= P_dontcare;
					else if (!(u->property & P_target))
						u->property &= ~P_dontcare;
					if (ss->action && ss->action[1].attrprop && ss->action[1].type == 'I')
					{
						u->attribute &= ~ss->action[1].attribute.clear;
						u->attribute |= ss->action[1].attribute.set;
						u->property &= ~ss->action[1].property.clear;
						u->property |= ss->action[1].property.set;
					}
					u->scan = r->scan;
					staterule(RULE, u, NiL, 1)->scan = r->scan;
					p = cons(u, p);
				}
			}
			else if (!ifparen && (!strcmp(w, "ifdef") || !strcmp(w, "ifelse")))
				ifparen = paren;
		}
		else if (*w == 'I' && !strcmp(w, "INCLUDE"))
		{
			do
			{
				sfstrset(tmp, 0);
				while ((c = sfgetc(fp)) != EOF && c != ')' && c != ',')
					if (!isspace(c))
						sfputc(tmp, c);
				u = makerule(sfstruse(tmp));
				if (!(u->mark & M_scan))
				{
					u->mark |= M_scan;
					if (dontcare)
						u->property |= P_dontcare;
					else if (!(u->property & P_target))
						u->property &= ~P_dontcare;
					if (ss->action && ss->action[1].attrprop && ss->action[1].type == 'I')
					{
						u->attribute &= ~ss->action[1].attribute.clear;
						u->attribute |= ss->action[1].attribute.set;
						u->property &= ~ss->action[1].property.clear;
						u->property |= ss->action[1].property.set;
					}
					u->scan = r->scan;
					staterule(RULE, u, NiL, 1)->scan = r->scan;
					p = cons(u, p);
				}
			} while (c == ',');
			sfungetc(fp, c);
		}
		else if ((r->property & P_parameter) && *w == 'd' && !strcmp(w, "define"))
		{
			sfstrset(tmp, 0);
			while ((c = sfgetc(fp)) != EOF && c != ',')
				if (!isspace(c))
					sfputc(tmp, c);
			sfputc(tmp, ' ');
			while (isspace(c = sfgetc(fp)));
			if (c == '`')
				inquote = '\'';
			else
				sfungetc(fp, c);
			n = 1;
			while ((c = sfgetc(fp)) != EOF && c != inquote)
			{
				if (!inquote)
				{
					if (c == '(')
						n++;
					else if (c == ')' && !n--)
						break;
				}
				else if (c == '#')
				{
					while ((c = sfgetc(fp)) != EOF && c != '\n');
					continue;
				}
				sfputc(tmp, c);
			}
			inquote = 0;
			p = scandefine(sfstruse(tmp), p);
		}
							/*INDENT*/
						}
					}
					else
					{
						h = 0;
						t = C_ID1|C_ID2;
						sfputc(tmp, c);
					}
				}
			}
			break;
		}
	sfstrclose(tmp);
	sfsetfd(fp, -1);
	sfclose(fp);
	return p;
}

/*
 * do scan match action a on r with matched string s
 * b is beginning of the original line
 */

static struct list*
scanmatch(struct list* p, register struct action* a, struct rule* r, char* b, char* s, int iflev, int split)
{
	int		n;
	char*		t;
	char*		o;
	struct rule*	u = 0;
	struct rule*	x = 0;
	Sfio_t*		tmp = 0;

	static char	label[] = "X-scan-action";

	if (a->script || a->map)
	{
		o = state.frame->original;
		state.frame->original = b;
		t = state.frame->stem;
		state.frame->stem = s;
		if (a->script)
		{
			label[0] = a->type;
			parse(NiL, a->script, label, 0);
		}
		if (a->map)
		{
			tmp = sfstropen();
			expand(tmp, a->map);
		}
		state.frame->original = o;
		state.frame->stem = t;
	}
	if (tmp || *s)
	{
		if (tmp)
			s = sfstruse(tmp);
		if (split)
			t = tokopen(s, 1);
		do
		{
			if (split && !(s = tokread(t)))
				break;
			if (o = strchr(s, ' '))
			{
				do
				{
					*o++ = (state.test & 4) ? '?' : FILE_SPACE;
				} while (o = strchr(o, ' '));
				o = s;
			}
#if _WIN32
			if (isalpha(*s) && *(s + 1) == ':' && (*(s + 2) == '/' || *(s + 2) == '\\'))
			{
				*(s + 1) = *s;
				*s = '/';
				for (o = s; o = strchr(s, '\\'); *o++ = '/');
			}
#endif
			u = makerule(s);
			if (u->status == UPDATE)
			{
				if (u->uname && streq(s, u->name))
				{
					message((-2, "%s split from %s", u->name, u->uname));
					oldname(u);
					u = makerule(s);
				}
				else
				{
					parentage(internal.tmp, r, " : ");
					error(1, "%s : %s: implicit reference before action completed", sfstruse(internal.tmp), unbound(u));
				}
			}
			if (!(u->mark & M_scan))
			{
				u->mark |= M_scan;
				p = cons(u, p);
				if (iflev || o)
					u->property |= P_dontcare;
				else if (!(u->property & P_target))
					u->property &= ~P_dontcare;
				x = (u->dynamic & D_alias) ? getrule(u->name) : (struct rule*)0;
				if (a->attrprop)
				{
					u->attribute &= ~a->attribute.clear;
					u->attribute |= a->attribute.set;
					u->property &= ~a->property.clear;
					u->property |= a->property.set;
					if (x)
					{
						x->attribute &= ~a->attribute.clear;
						x->attribute |= a->attribute.set;
						x->property &= ~a->property.clear;
						x->property |= a->property.set;
					}
				}
				if (!(a->flags & SCAN_nopropagate) && (!(u->dynamic & D_scanned) || !u->scan))
				{
					if (!(n = a->scan))
						n = r->scan;
					else if (n == SCAN_NULL)
						n = 0;
					if (n && u->scan != n && !(u->property & P_state))
					{
						if (u->scan)
						{
							char*		os;
							char*		ns;
							struct list*	q;

							os = ns = internal.scan->name;
							for (q = internal.scan->prereqs; q; q = q->next)
								if (q->rule->scan == u->scan)
									os = q->rule->name;
								else if (q->rule->scan == n)
									ns = q->rule->name;
							error(1, "%s: scan strategy %s overrides %s", u->name, ns, os);
						}
						u->scan = n;
						staterule(RULE, u, NiL, 1)->scan = n;
						if (x)
							x->scan = n;
					}
				}
			}
		} while (split);
		if (split)
			tokclose(t);
		if (tmp)
			sfstrclose(tmp);
	}
	else
	{
		x = (r->dynamic & D_alias) ? getrule(r->name) : (struct rule*)0;
		if (a->attrprop)
		{
			r->attribute &= ~a->attribute.clear;
			r->attribute |= a->attribute.set;
			r->property &= ~a->property.clear;
			r->property |= a->property.set;
			if (x)
			{
				x->attribute &= ~a->attribute.clear;
				x->attribute |= a->attribute.set;
				x->property &= ~a->property.clear;
				x->property |= a->property.set;
			}
		}
	}
	return p;
}

/*
 * scan fd on file r for patterns compiled in ss
 * cons prereqs on p
 */

static struct list*
scanexec(int fd, struct rule* r, struct scan* ss, struct list* p)
{
	register int		c;
	register unsigned char*	g;
	register scanstate*	s;
	scanstate*		m;
	unsigned char*		pb;
	unsigned char*		x;
	unsigned char*		b;
	scanstate*		rep;
	scanstate*		per;
	char*			a;
	int			hit;
	int			n;
	int			collect;
	int			iflev;
	int			h;
	int			t;
	struct frame		frame;
	struct rule*		u;
	struct var*		v;
	struct
	{
	int			arg;
	scanstate*		state;
	unsigned char*		buffer;
	}			any[8], *pop;
	unsigned char		buf[2 * SCANBUFFER + 1];
	struct
	{
		int		begin;
		int		end;
		int		replace;
	}			arg[SCANARGS];

	if (ss->flags & SCAN_macro)
		return scanmacro(fd, r, ss, p);
	if (ss->external)
	{
		Sfio_t*	sp;

		if (!(sp = fapply(r, null, r->name, ss->external, CO_ALWAYS|CO_LOCAL|CO_URGENT)))
			error(3, "%s: external scan error", r->name);
		parse(sp, NiL, "external-scan", 0);
		sfclose(sp);
		p = internal.implicit->prereqs;
		internal.implicit->prereqs = 0;
		return p;
	}
	zero(frame);
	frame.parent = state.frame;
	if (!(frame.previous = r->active))
	{
		frame.target = r;
		state.frame = r->active = &frame;
	}
	else if (state.frame != r->active)
		state.frame = r->active;
	iflev = (r->property & P_dontcare) ? 1 : 0;
	g = buf + 2 * SCANBUFFER;
	g[0] = 0;
	m = 0;
	collect = 0;
	for (;;)
	{
		n = 0;
		pop = any;
		rep = per = 0;
		pb = 0;
		hit = 0;
		b = g;
		if (s = ss->state)
		{
		next:
			while (!(c = *g++))
			{
				if (b >= buf + SCANBUFFER)
				{
					c = g - b - 1;
					memcpy(buf + SCANBUFFER - c, b, c);
					b = buf + SCANBUFFER - c;
				}
				if ((c = read(fd, g = buf + SCANBUFFER, SCANBUFFER)) <= 0)
					goto done;
				g[c] = 0;
			}
			for (;;)
			{
				/*UNDENT*/
	if (*s == SPC)
	{
		h = g == b + 1;
		for (;;)
		{
			if (isspace(c))
			{
				if (m)
				{
					m = 0;
					if (collect)
					{
						collect = 0;
						arg[n++].end = g - b - 1;
					}
				}
#if DEBUG
				if (!(state.questionable & 0x00000020))
					h = 1;
#endif
				if (c == '\n')
					break;
				h = 1;
			}
			else if (!c)
			{
				if (b >= buf + SCANBUFFER)
				{
					c = g - b - 1;
					memcpy(buf + SCANBUFFER - c, b, c);
					b = buf + SCANBUFFER - c;
				}
				if ((c = read(fd, g = buf + SCANBUFFER, SCANBUFFER)) <= 0)
					goto done;
				g[c] = 0;
			}
			else if (ss->type[c] & (t = (m ? (QUOTE_blank|QUOTE_comment|QUOTE_quote) : QUOTE_comment)))
			{
				if ((x = scanquote(fd, buf, &b, g, ss->quote, t)) == g)
					break;
				g = x;
				if (ss->type[c] & QUOTE_comment)
					h = 1;
			}
			else if (!m)
				break;
			c = *g++;
		}
		s = s + *(s + 1) + 1;
		if (!h && !rep && (*s == ANY || *s == ARG || *s == 0))
		{
#if DEBUG
			if ((state.test & 0x00000080) && *s == 0)
				error(2, "scanexec: HIT *s==0");
#endif
			m = 0;
			collect = 0;
			break;
		}
	}
	else if (*s == ANY)
	{
		if (pop < &any[elementsof(any)])
		{
			pop->arg = n;
			pop->state = s;
			pop->buffer = g;
			pop++;
		}
		s += *(s + 1) + 1;
	}
	else if (*s == ARG)
	{
		m = s = s + *(s + 1) + 1;
		if (n < elementsof(arg))
		{
			collect = 1;
			arg[n].begin = g - b - 1;
		}
	}
	else if (*s == REP)
	{
		if (rep)
		{
			per = s + *(s + 1) + 1;
			if (n && arg[0].end > arg[0].begin)
			{
				x = b + arg[0].begin;
				while (x < g)
				{
					if (*per == SPC)
						while (x < g && isspace(*x))
							x++;
					else
					{
						if (*x != *per)
							break;
						x++;
					}
					per += *(per + 1) + 1;
				}
				if (x >= g)
					break;
			}
			for (s = per; *s; s = s + *(s + 1) + 1);
			goto rephit;
		}
		else
			rep = s = s + *(s + 1) + 1;
	}
	else if (*s == 0)
	{
 rephit:
		if (m)
		{
			if (c != '\n')
			{
				s = m;
				goto next;
			}
			m = 0;
			if (collect)
			{
				collect = 0;
				arg[n++].end = g - b - 1;
			}
		}
		if (c == '\n')
		{
			if (per || rep && *rep != ARG && (*rep != SPC || *(rep + *(rep + 1) + 1) != ARG) && (!hit || n))
				g++;
			else
				rep = 0;
		}
		if (*++s && (!n || arg[0].begin < arg[0].end))
		{
			if (rep)
				hit += n;
			if (n < elementsof(arg))
			{
				arg[n].begin = arg[n].end = g - b - 1;
				n++;
			}
			for (c = 0; c < n; c++)
				arg[c].replace = *(b + arg[c].end);
			for (c = 0; c < n; c++)
				*(b + arg[c].end) = 0;
			while (c < elementsof(arg))
				arg[c++].begin = arg[n - 1].begin;
#if DEBUG
			if (state.test & 0x00000080)
				error(2, "scanexec: %s: %c n=%d \"%s\" \"%s\"", r->name, ss->action[*s].type, n - 1, b + arg[0].begin, b + arg[1].begin);
#endif
			a = null;
			switch (ss->action[*s].type)
			{
			case 'A':
				if (state.archive)
				{
					unsigned long	date;

					if (!(date = strtol((char*)b + arg[1].begin, NiL, 10)))
						date = tmdate((char*)b + arg[1].begin, NiL, NiL);
					addfile(state.archive, (char*)b + arg[0].begin, date);
				}
				else
					error(2, "%s: `A' scan pattern for non-%s", r->name, internal.archive->name);
				a = (char*)b + arg[0].begin;
				break;
			case 'B':
				iflev++;
				break;
			case 'D':
				if (r->property & P_parameter)
					p = scandefine((char*)b + arg[0].begin, p);
				break;
			case 'E':
				iflev--;
				break;
			case 'I':
				a = (char*)b + arg[0].begin;
				break;
			}
			p = scanmatch(p, &ss->action[*s], r, (char*)b, a, iflev, a > (char*)b && (c = *(a - 1)) != '"' && c != '\'' && (c != '<' || arg[0].replace != '>'));
			for (c = 0; c < n; c++)
				*(b + arg[c].end) = arg[c].replace;
		}
		else
		{
			if (hit)
				rep = 0;
			if (pop > any)
			{
				pop--;
				m = s = pop->state;
				g = pop->buffer;
				n = pop->arg;
				goto next;
			}
		}
		if (rep)
		{
			s = rep;
			if ((b = --g) == pb)
				goto done;
			pb = b;
			n = 0;
			goto next;
		}
		break;
	}
	else if (c == *s++)
	{
		if (m)
		{
			m = 0;
			if (collect)
			{
				collect = 0;
				arg[n++].end = g - b - 1;
			}
		}
		s = s + *s;
		goto next;
	}
	else
		s++;
				/*INDENT*/
			}
			g = b;
		}
		n = (ss->flags & SCAN_state) && state.fullscan;
		x = ss->type;
		while ((c = *g++) != '\n')
		{
			if (!c)
			{
				if ((c = read(fd, b = g = buf + SCANBUFFER, SCANBUFFER)) <= 0)
					goto done;
				g[c] = 0;
			}
			else if (x[c] & (QUOTE_comment|QUOTE_quote))
			{
				if (g - 2 >= b)
					b = g - 2;
				g = scanquote(fd, buf, &b, g, ss->quote, QUOTE_comment|QUOTE_quote);
			}
			else if (n && istype(c, C_ID1))
			{
				if (istype(c, C_VARPOS1))
				{
					h = 1;
					t = C_VARPOS1;
				}
				else
				{
					h = 0;
					t = C_ID1|C_ID2;
				}
				b = g - 1;
				for (;;)
				{
					while (!(c = *g++))
					{
						if (b >= buf + SCANBUFFER)
						{
							c = g - b - 1;
							memcpy(buf + SCANBUFFER - c, b, c);
							b = buf + SCANBUFFER - c;
						}
						if ((c = read(fd, g = buf + SCANBUFFER, SCANBUFFER)) <= 0)
							goto done;
						g[c] = 0;
					}
					if (t != (C_ID1|C_ID2))
					{
						if (t == C_VARPOS8)
							t = C_ID1|C_ID2;
						else
							t <<= 1;
					}
					if (!istype(c, t))
					{
						if (t == (C_ID1|C_ID2) || !istype(c, C_ID1|C_ID2))
							break;
						t = C_ID1|C_ID2;
						h = 0;
					}
				}
				g--;
				if (h)
				{
					*g = 0;
					if ((v = getvar(b)) && (v->property & V_scan))
					{
						u = staterule(VAR, NiL, (char*)b, 1);
						if (!(u->mark & M_scan))
						{
							u->mark |= M_scan;
							p = cons(u, p);
						}
					}
					*g = c;
				}
			}
		}
	}
 done:
	r->active = frame.previous;
	state.frame = frame.parent;
	return p;
}

/*
 * return list of r's implicit prerequisites
 * r must be bound to a file which is then scanned
 */

struct list*
scan(register struct rule* r, unsigned long* tm)
{
	register struct rule*	s;
	register struct list*	oprereqs;
	struct rule*		alt;
	struct list*		p;
	struct scan*		ss;
	int			fd;

	if (tm)
		*tm = 0;
	if ((r->property & (P_attribute|P_state|P_virtual)) || !(r->dynamic & D_regular))
		return 0;
	s = staterule(RULE, r, NiL, 1);
	if ((r->property & P_joint) || (s->dynamic & D_built))
	{
		alt = s;
		s = staterule(PREREQS, r, NiL, 1);
		s->property |= P_implicit;
		r->preview = r->view;
	}
	else if (alt = staterule(PREREQS, r, NiL, 0))
	{
		alt->property &= ~P_implicit;
		alt = 0;
	}
	if (r->scan < SCAN_USER)
	{
		ss = 0;
		if (alt)
			s->prereqs = 0;
	}
	else
	{
		if (!strategy[r->scan])
		{
			for (p = internal.scan->prereqs; p; p = p->next)
				if (p->rule->scan == r->scan)
				{
					strategy[r->scan] = scancompile(p->rule, (r->property & P_parameter) ? SCAN_define : 0);
					break;
				}
			if (!p)
				error(3, "%s: invalid scan index %d", r->name, r->scan);
		}
		ss = strategy[r->scan];
	}
	r->dynamic |= D_scanned;
	if (s->dynamic & D_scanned)
		return s->prereqs;
	oprereqs = s->prereqs;
	if (ss)
	{
		if (!(r->property & P_accept) && !state.accept && !((s->property | (alt ? alt->property : 0L)) & P_force) && r->scan == s->scan)
			ss = 0;
		else if ((fd = ropen(r->name, O_RDONLY)) < 0)
		{
			if (state.exec && !(r->property & P_dontcare))
				error(1, "cannot read %s", r->name);
			return 0;
		}
		else
		{
			message((-2, "scanning %s for prerequisites", r->name));
			state.savestate = 1;
			s->prereqs = scanexec(fd, r, ss, ss->before ? scanmatch(NiL, ss->before, r, null, null, 0, 1) : (struct list*)0);
			close(fd);
		}
	}
	s->property &= ~P_force;
	s->dynamic |= D_scanned;
	s->scan = r->scan;
	if (alt)
	{
		s->event = alt->event;
		alt->attribute = r->attribute;
		alt->property &= ~P_force;
		alt->scan = s->scan;
		if (alt->prereqs != r->prereqs)
		{
			if ((r->property & (P_joint|P_target)) != (P_joint|P_target))
				freelist(alt->prereqs);
			alt->prereqs = r->prereqs;
		}
	}
	else
		s->attribute = r->attribute;
	if (ss)
	{
		if (ss->after)
		{
			for (p = s->prereqs; p; p = p->next)
				if (p->rule->mark & M_scan)
				{
					p->rule->mark &= ~M_scan;
#if DEBUG
					message((-5, "%s: implicit prerequisite %s", r->name, p->rule->name));
#endif
				}
			s->prereqs = scanmatch(s->prereqs, ss->after, r, null, null, 0, 1);
		}
		for (p = s->prereqs; p; p = p->next)
			if (p->rule->mark & M_scan)
			{
				p->rule->mark &= ~M_scan;
#if DEBUG
				message((-5, "%s: implicit prerequisite %s", r->name, p->rule->name));
#endif
			}
	}
	if (tm && prereqchange(r, s->prereqs, r, oprereqs))
		*tm = CURTIME;
	if (oprereqs != s->prereqs && (r->property & (P_joint|P_target)) != (P_joint|P_target))
		freelist(oprereqs);
	return s->prereqs;
}
