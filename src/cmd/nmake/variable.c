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
 * make variable routines
 */

#include "make.h"
#include "options.h"

/*
 * generator for genprereqs()
 * sp!=0 is the first pass that sets M_generate
 * sp==0 is the second pass that clears M_generate
 */

static int
scanprereqs(register Sfio_t* sp, struct rule* r, int dostate, int all, int top, int sep)
{
	register int		i;
	register struct list*	p;
	register struct rule*	x;
	struct rule*		z;
	struct list*		prereqs[4];
	struct list		t;

	i = 0;
	if (r->scan == SCAN_IGNORE && !(state.questionable & 0x02000000))
		top = -1;
	else if ((x = staterule(PREREQS, r, NiL, 0)) && (x->property & P_implicit))
		prereqs[i++] = x->prereqs;
	else
		top = 1;
	if (top)
	{
		if ((x = staterule(RULE, r, NiL, 0)) && x->prereqs != r->prereqs)
			prereqs[i++] = x->prereqs;
		prereqs[i++] = r->prereqs;
	}
	if (r->active && r->active->primary)
	{
		t.rule = makerule(r->active->primary);
		t.next = 0;
		prereqs[i++] = &t;
	}
	while (--i >= 0)
		for (p = prereqs[i]; p; p = p->next)
		{
			x = p->rule;
			do
			{
				if (!(x->dynamic & D_alias))
					z = 0;
				else if (!(z = getrule(x->name)))
					break;
				if (x->mark & M_generate)
				{
					if (!sp)
					{
						x->mark &= ~M_generate;
						if (top >= 0)
							scanprereqs(sp, x, dostate, all, 0, sep);
					}
				}
				else if (sp && (all || ((x->property & P_state) || x->scan || !r->scan) && !(x->property & (P_ignore|P_use|P_virtual)) && (!(x->property & P_dontcare) || x->time)))
				{
					x->mark |= M_generate;
					if (all || ((x->property & P_state) != 0) == dostate)
					{
						if (sep)
							sfputc(sp, ' ');
						else
							sep = 1;
						sfputr(sp, state.localview ? localview(x) : ((x->dynamic & D_alias) ? x->uname : x->name), -1);
					}
					if (top >= 0)
						sep = scanprereqs(sp, x, dostate, all, 0, sep);
				}
			} while (x = z);
		}
	return sep;
}

/*
 * generate the list of source|explicit prerequisites in sp
 * if sep!=0 then ' ' separation needed
 * new value of sep is returned
 */

static int
genprereqs(Sfio_t* sp, struct rule* r, int dostate, int all, int sep)
{
	sep = scanprereqs(sp, r, dostate, all, 1, sep);
	scanprereqs(NiL, r, dostate, all, 1, sep);
	return sep;
}

/*
 * return the value of a variable given its name
 * the internal automatic variables are (lazily) expanded here
 * op: <0:auxiliary 0:primary+auxiliary >0:primary
 */

char*
getval(register char* s, int op)
{
	register struct list*	p;
	register struct rule*	r;
	struct rule*		x;
	struct rule*		z;
	struct var*		v;
	struct var*		a;
	char*			t;
	char*			val;
	char*			next;
	int			c;
	int			n;
	int			var;
	int			tokens;
	int			pop;
	int			sep;
	char**			ap;
	char*			arg[16];

	if (!*s)
		return null;
	else if (isstatevar(s))
	{
		if (!(r = getrule(s)))
			return null;
		if ((r->property & (P_functional|P_virtual)) && r->status != UPDATE)
			maketop(r, 0, NiL);
		return r->statedata ? r->statedata : null;
	}
	else if (!istype(*s, C_VARIABLE1|C_ID1|C_ID2))
	{
		sep = 0;

		/*
		 * some internal vars have no associated rule
		 */

		switch (var = *s)
		{

		case '-':	/* option settings suitable for command line */
		case '+':	/* option settings suitable for set */
			if (*++s)
			{
				getop(internal.val, s, var != '-');
				return sfstruse(internal.val);
			}
			if (state.mam.statix && (state.never || state.frame->target && !(state.frame->target->property & P_always)))
				return "$MAKEFLAGS";
			if (var == '-')
			{
				listops(internal.val, 0);
				for (p = internal.preprocess->prereqs; p; p = p->next)
					sfprintf(internal.val, " %s", p->rule->name);
				return sfstruse(internal.val);
			}
			listops(internal.val, 0);
			return sfstruse(internal.val);

		case '=':	/* command line script args and export vars */
			for (n = 1; n < state.argc; n++)
				if (state.argf[n] & (ARG_ASSIGN|ARG_SCRIPT))
				{
					if (sep)
						sfputc(internal.val, ' ');
					else
						sep = 1;
					shquote(internal.val, state.argv[n]);
				}
			for (p = internal.exported->prereqs; p; p = p->next)
				if (v = getvar(p->rule->name))
				{
					if (sep)
						sfputc(internal.val, ' ');
					else
						sep = 1;
					sfprintf(internal.val, "%s=%s", v->name, (v->property & V_scan) ? "=" : null);
					shquote(internal.val, v->value);
				}
				else if (strchr(p->rule->name, '='))
				{
					if (sep)
						sfputc(internal.val, ' ');
					else
						sep = 1;
					shquote(internal.val, p->rule->name);
				}
			return sfstruse(internal.val);
		}
		for (pop = -1; *s == var; s++, pop++);
		next = 0;
		tokens = 0;
		for (;;)
		{
			val = null;
			while (*s == ' ')
				s++;
			if (!*s)
			{
				if (tokens)
					goto done;
				r = state.frame->target;
			}
			else
			{
				if (next = strchr(s, ' '))
				{
					*next = 0;
					if (!tokens++)
						state.val++;
				}
				if (*s == MARK_CONTEXT)
				{
					if (!(t = next))
						t = s + strlen(s);
					if (*--t == MARK_CONTEXT)
					{
						*t = 0;
						s++;
					}
					else
						t = 0;
				}
				else
					t = 0;
				if (!(r = getrule(s)))
					switch (var)
					{
					case '!':
					case '&':
					case '?':
						if (staterule(RULE, NiL, s, -1) || staterule(PREREQS, NiL, s, -1))
							r = makerule(s);
						break;
					}
				if (t)
					*t = MARK_CONTEXT;
				if (!r)
					goto done;
				s = r->name;
				if (*s == ATTRNAME && !r->active && !(r->dynamic & D_cached) && strmatch(s, internal.issource))
					r = source(r);
			}
			for (n = pop; n > 0; n--)
			{
				if (!r->active)
					goto done;
				r = r->active->parent->target;
			}
			switch (c = var)
			{

			case '#': /* local arg count */
				val = 0;
				argcount();
				break;

			case ';': /* target data */
				if (r->property & P_statevar)
					r = bind(r);
				if (r->statedata && !(r->property & P_staterule))
					val = r->statedata;
				break;

			case '<': /* target name */
				if ((r->property & (P_joint|P_target)) != (P_joint|P_target))
				{
					val = state.localview ? localview(r) : r->name;
					break;
				}
				r = r->prereqs->rule;
				c = '~';
				/*FALLTHROUGH*/

			case '>': /* updated target file prerequisites */
			case '*': /* all target file prerequisites */
			case '~': /* all target prerequisites */
				n = 0;
				if (r->active && (t = r->active->primary))
				{
					x = makerule(t);
					t = state.localview ? localview(x) : x->name;
					if (c == '>')
					{
						val = t;
						break;
					}
					sfputr(internal.val, t, -1);
					n = 1;
				}
				else
				{
					x = 0;
					n = 0;
				}
				val = 0;
				for (p = r->prereqs; p; p = p->next)
				{
					if (p->rule != x && (c == '~' || !notfile(p->rule) &&
					    (c != '>' || !(p->rule->dynamic & D_same) &&
					     (!(r->property & P_archive) && (p->rule->time >= state.start || p->rule->time > r->time || !(z = staterule(RULE, p->rule, NiL, 0)) || !z->time) ||
					      (r->property & P_archive) && !(p->rule->dynamic & D_member) && p->rule->time))))
					{
						t = state.localview ? localview(p->rule) : p->rule->name;
						if (n)
							sfputc(internal.val, ' ');
						else
						{
							if (!p->next)
							{
								val = t;
								break;
							}
							else
								n = 1;
							if (sep)
								sfputc(internal.val, ' ');
							else
								sep = 1;
						}
						sfputr(internal.val, t, -1);
					}
				}
				break;

			case '@':	/* target action */
				if (r->action)
					val = r->action;
				break;

			case '%':	/* target stem or functional args */
				if (r->active && r->active->stem)
				{
					val = r->active->stem;
					if (state.context && (t = strrchr(val, '/')))
						val = t + 1;
				}
				else
					val = unbound(r);
				break;

			case '!':	/* explicit and generated file prerequisites  */
			case '&':	/* explicit and generated state prerequisites */
			case '?':	/* all explicit and generated prerequisites   */
				sep = genprereqs(internal.val, r, c == '&', c == '?', sep);
				val = 0;
				break;

			case '^':	/* original bound name */
				if (!r->active)
					break;
				if ((r->property & (P_joint|P_target)) != (P_joint|P_target))
				{
					if (r->active->original && !streq(r->active->original, r->name))
						val = state.localview ? localview(makerule(r->active->original)) : r->active->original;
					break;
				}
				for (p = r->prereqs->rule->prereqs; p; p = p->next)
					if (p->rule->active && (t = p->rule->active->original))
					{
						if (sep)
							sfputc(internal.val, ' ');
						else
							sep = 1;
						sfputr(internal.val, state.localview ? localview(makerule(t)) : t, -1);
					}
				val = 0;
				break;

			default:
#if DEBUG
				error(1, "%c: invalid internal variable name", c);
#endif
				return null;
			}
		done:
			if (tokens)
			{
				if (val && *val)
				{
					if (sep)
						sfputc(internal.val, ' ');
					else
						sep = 1;
					sfputr(internal.val, val, -1);
				}
				if (!next)
				{
					state.val--;
					return sfstruse(internal.val);
				}
				*next++ = ' ';
				s = next;
			}
			else if (val)
				return val;
			else
				return sfstruse(internal.val);
		}
	}
	else if ((v = getvar(s)) || (t = strchr(s, ' ')))
	{
		if (v)
		{
			t = 0;
			if (!(v->property & V_functional) || !(r = getrule(v->name)) || !(r->property & P_functional))
				r = 0;
		}
		else
		{
			/*
			 * functional var with args
			 */

			*t = 0;
			if (!(v = getvar(s)) || !v->builtin)
			{
				if (!(r = getrule(s)) || !(r->property & P_functional))
					r = catrule(".", s, ".", 0);
				if (!r || !(r->property & P_functional))
				{
					*t++ = ' ';
					return null;
				}
				if (v = getvar(r->name))
					v->property |= V_functional;
				else
					v = setvar(r->name, NiL, V_functional);
			}
			*t++ = ' ';
		}
		if (v->builtin)
		{
			ap = arg;
			if (t)
				for (;;)
				{
					while (isspace(*t))
						t++;
					if (!*t)
						break;
					*ap++ = t;
					if (ap >= &arg[elementsof(arg) - 1])
						break;
					while (*t && !isspace(*t))
						t++;
					if (!*t)
						break;
					*t++ = 0;
				}
			*ap = 0;
			return (t = (*v->builtin)(arg)) ? t : null;
		}
		if (r)
			maketop(r, 0, t ? t : null);
		if (state.reading && !state.global)
		{
			v->property &= ~V_compiled;
			if (istype(*s, C_ID1))
				v->property |= V_frozen;
		}
		t = v->value;
		if (state.mam.regress && state.user > 1 && (v->property & (V_import|V_local_E)) == V_import)
		{
			v->property |= V_local_E;
			dumpregress(state.mam.out, "setv", v->name, v->value);
		}
		if (v->property & V_auxiliary)
		{
			if (op <= 0)
			{
				if ((a = auxiliary(v->name, 0)) && *a->value)
				{
					if (op < 0)
						return a->value;
					if (*v->value)
						sfprintf(internal.val, "%s ", v->value);
					sfputr(internal.val, a->value, -1);
					t = sfstruse(internal.val);
				}
				else if (op < 0)
					t = null;
			}
		}
		else if (op < 0)
			t = null;
		return t;
	}
	if (state.reading && !state.global && istype(*s, C_ID1) && (v = setvar(s, null, 0)))
		v->property |= V_frozen;
	return null;
}

/*
 * reset variable p value to v
 * append!=0 if v is from append
 */

static void
resetvar(register struct var* p, char* v, int append)
{
	register int	n;

	n = strlen(v);
	if (!p->value || (p->property & V_import) || n > p->length)
	{
		if (append)
			n = (n + 1023) & ~1023;
		if (n < MINVALUE)
			n = MINVALUE;
		if (!(p->property & V_free))
		{
			p->property |= V_free;
			p->value = 0;
		}
		p->value = newof(p->value, char, n + 1, 0);
		p->length = n;
	}
	strcpy(p->value, v);
}

/*
 * set the value of a variable
 */

struct var*
setvar(char* s, char* v, int flags)
{
	register char*		t;
	register struct var*	p;
	register int		n;
	int			isid;

	if (!v)
		v = null;

	/*
	 * the name determines the variable type
	 */

	t = s;
	if (istype(*s, C_ID1))
		while (istype(*++t, C_ID1|C_ID2));
	else if (isstatevar(s))
	{
		bindstate(makerule(s), v);
		return 0;
	}
	else if (!istype(*s, C_VARIABLE1|C_ID1|C_ID2) && *s != '(')
		error(2, "%s: invalid variable name", s);
	isid = (t > s && *t == 0);

	/*
	 * check for a previous definition
	 */

	if (!(p = getvar(s)))
	{
		newvar(p);
		if (p->property & V_import)
		{
			p->property &= ~V_import;
			p->value = 0;
		}
		else if (p->value)
			*p->value = 0;
		p->name = putvar(0, p);
		p->builtin = 0;
	}

	/*
	 * check the variable attributes for precedence
	 */

	if (flags & V_auxiliary)
	{
		if (!p->value)
		{
			p->value = null;
			p->property |= V_import;
		}
		p->property |= V_auxiliary;
		p = auxiliary(s, 1);
	}
	p->property |= flags & (V_builtin|V_functional);
	if (state.user || state.readonly || !(p->property & V_readonly) && (!(p->property & V_import) || state.global != 1 || (flags & V_import) || state.base && !state.init))
	{
		if (flags & V_import)
		{
			if (p->property & V_free)
			{
				p->property &= ~V_free;
				free(p->value);
			}
			p->value = v;
		}
		else
		{
			t = v;
			if (state.user)
				p->property &= ~V_append;
			if (n = (flags & V_append))
			{
				if (state.reading && !state.global && isid)
					p->property |= V_frozen;
				if (p->value && *p->value)
				{
					sfprintf(internal.nam, "%s %s", p->value, v);
					t = sfstruse(internal.nam);
				}
			}
			resetvar(p, t, n);
		}
		if (flags & V_import)
			p->property |= V_import;
		else
			p->property &= ~V_import;
		if (state.readonly)
		{
			p->property |= V_readonly;
			if (flags & V_append)
				p->property |= V_append;
		}
		else if (state.init)
			p->property |= V_compiled;
		else
			p->property &= ~V_compiled;
		if ((flags & V_scan) && !(p->property & V_scan))
		{
			if (isid && state.user <= 1)
			{
				p->property |= V_scan;
				staterule(VAR, NiL, p->name, -1);
			}
			else
				error(1, "%s: not marked as candidate state variable", p->name);
		}
		if (state.vardump && !(p->property & V_import))
			dumpvar(sfstdout, p);
	}
	else if (state.reading)
	{
		if (p->property & V_readonly)
		{
			/*
			 * save old value for makefile compiler
			 */

			s = (p->property & V_oldvalue) ? getold(p->name) : (char*)0;
			t = v;
			if (flags & V_append)
			{
				if (state.reading && !state.global && isid)
					p->property |= V_frozen;
				if (s)
				{
					sfprintf(internal.nam, "%s %s", s, v);
					t = sfstruse(internal.nam);
				}
			}
			putold(p->name, strdup(t));
			p->property |= V_oldvalue;
			if ((p->property & V_append) && p->value && *p->value)
			{
				sfprintf(internal.nam, "%s %s", t, p->value + (s ? strlen(s) + 1 : 0));
				resetvar(p, sfstruse(internal.nam), 1);
			}
		}
		if (isid && (flags & V_scan) && state.makefile)
		{
			p->property |= V_scan;
			staterule(VAR, NiL, p->name, -1);
		}
	}
	if (!p->value)
	{
		p->value = null;
		p->property |= V_import;
	}
	return p;
}

/*
 * translate ':' in s's expanded value to del in sp for list generators
 * 0 returned if s empty
 */

char*
colonlist(register Sfio_t* sp, register char* s, register int del)
{
	register char*	p;
	struct var*	v;

	if (!(v = getvar(s)))
		return 0;
	s = v->value;
	expand(sp, s);
	for (p = sfstruse(sp); isspace(*p); p++);
	if (!*(s = p))
		return 0;
	for (;;)
		switch (*p++)
		{
		case ':':
			if (*(p - 1) = del)
				break;
			/*FALLTHROUGH*/
		case 0:
			return s;
		}
}

/*
 * copy variable reference into sp
 */

void
localvar(Sfio_t* sp, register struct var* v, char* value, int property)
{
	register char*	s;
	register int	c;
	char*		prefix;
	char*		t;
	struct var*	x;

	prefix = (property & V_local_D) ? null : "_";
	if (!(v->property & property) || !sp)
	{
		v->property |= property;
		sfprintf(state.mam.out, "%ssetv %s%s ", state.mam.label, prefix, v->name);
		if (*(s = value))
		{
			sfprintf(state.mam.out, "%s%s", (property & V_local_D) ? "-D" : null, v->name);
			if (!(property & V_local_D) || *s != '1' || *(s + 1))
			{
				sfputc(state.mam.out, '=');
				sfputc(state.mam.out, '"');

				/*
				 * this quoting allows simple parameterization
				 */

				while (c = *s++)
				{
					switch (c)
					{
					case '$':
						if (istype(*s, C_ID1))
						{
							for (t = s; istype(*t, C_ID1|C_ID2); t++);
							c = *t;
							*t = 0;
							x = getvar(s);
							*t = c;
							c = '$';
							if (x)
								break;
						}
						else if (*s == '{')
							break;
						/*FALLTHROUGH*/
					case '\\':
					case '"':
					case '`':
						sfputc(state.mam.out, '\\');
						break;
					}
					sfputc(state.mam.out, c);
				}
				sfputc(state.mam.out, '"');
			}
		}
		else if (property & V_local_D)
			sfprintf(state.mam.out, "-U%s%s", prefix, v->name);
		sfputc(state.mam.out, '\n');
	}
	if (sp)
		sfprintf(sp, "\"${%s%s}\"", prefix, v->name);
}

/*
 * read the environment and set internal variables with setvar()
 * complicated by those who pollute the environment
 */

void
readenv(void)
{
	register char**	e;
	register char*	t;

	for (e = environ; t = *e; e++)
		if (istype(*t, C_ID1))
		{
			while (istype(*t, C_ID2))
				sfputc(internal.nam, *t++);
			if (*t++ == '=')
				setvar(sfstruse(internal.nam), t, V_import);
			else
				sfstrset(internal.nam, 0);
		}
}
