/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1984-2001 AT&T Corp.                *
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
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * make file read routines
 */

#include "make.h"

/*
 * read the base and global rules
 */

static void
readrules(void)
{
	register char*		s;
	register Sfio_t*	tmp;
	register struct list*	p;

	state.global = 1;

	/*
	 * read the base rules
	 */

	if (!(s = state.rules)) state.rules = null;
	else if (*s)
	{
		tmp = sfstropen();
		edit(tmp, s, KEEP, KEEP, external.object);
		readfile(sfstruse(tmp), COMP_BASE, NiL);
		edit(tmp, s, DELETE, KEEP, DELETE);
		state.rules = strdup(sfstruse(tmp));
		sfstrclose(tmp);
	}
	setvar(external.rules, state.rules, 0)->property |= V_compiled;

	/*
	 * read the explicit global makefiles
	 *
	 * NOTE: internal.tmplist is used to handle the effects
	 *	 of load() on internal list pointers
	 */

	if (p = internal.globalfiles->prereqs)
	{
		for (p = internal.tmplist->prereqs = listcopy(p); p; p = p->next)
			readfile(p->rule->name, COMP_GLOBAL, NiL);
		freelist(internal.tmplist->prereqs);
		internal.tmplist->prereqs = 0;
	}
	state.global = 0;
}

/*
 * read a file given an open file pointer
 */

static void
readfp(Sfio_t* sp, register struct rule* r, int type)
{
	register char*	s;
	register char*	t;
	int		n;
	int		needrules;
	int		preprocess;
	int		splice;
	char*		name;
	char*		b;
	char*		e;
	struct rule*	x;
	Sfio_t*		fp;

	name = r->name;
	if (!state.makefile)
	{
		/*
		 * set up the related file names
		 */

		fp = sfstropen();
		setvar(external.file, r->name, 0)->property |= V_compiled;
		edit(fp, r->name, DELETE, KEEP, KEEP);
		state.makefile = strdup(sfstruse(fp));
		edit(fp, r->name, DELETE, KEEP, external.object);
		state.objectfile = strdup(sfstruse(fp));
		edit(fp, r->name, DELETE, KEEP, external.state);
		state.statefile = strdup(sfstruse(fp));
		sfstrclose(fp);
	}
	needrules = !state.base && !state.rules;

	/*
	 * load if object file
	 */

	if (loadable(sp, r, 0))
	{
		if (!state.base && !state.global && !state.list)
			error(3, "%s: explicit make object files must be global", r->name);
		if (!state.rules)
			readrules();
		message((-2, "loading %sobject file %s", state.global ? "global " : null, r->name));
		if (load(sp, r->name, 0) > 0)
		{
			sfclose(sp);
			return;
		}
		error(3, "%s: must be recompiled", name);
	}

	/*
	 * check object corresponding to file
	 */

	if (state.global || !state.forceread && (!(type & COMP_FILE) || needrules))
	{
		fp = sfstropen();
		edit(fp, r->name, DELETE, KEEP, external.object);
		state.init++;
		x = bindfile(NiL, sfstruse(fp), 0);
		state.init--;
		sfstrclose(fp);
		if (!x || !x->time)
			/* ignore */;
		else if (x == r)
			error(3, "%s must be recompiled", r->name);
		else if (fp = sfopen(NiL, s = x->name, "br"))
		{
			if (needrules)
				x->dynamic |= D_built;
			if (loadable(fp, x, 1))
			{
				if (needrules)
				{
					needrules = 0;
					if (state.rules)
					{
						Sfio_t*	op;
						Sfio_t*	np;

						op = sfstropen();
						np = sfstropen();
						edit(op, state.rules, DELETE, KEEP, DELETE);
						edit(np, b = getval(external.rules, 1), DELETE, KEEP, DELETE);
						if (strcmp(sfstruse(op), sfstruse(np)))
						{
							message((-2, "%s: base rules changed to %s", state.rules, b));
							state.rules = b;
							state.forceread = 1;
							needrules = 1;
						}
						sfstrclose(np);
						sfstrclose(op);
					}
					if (!needrules)
						readrules();
				}
				if (!state.forceread)
				{
					message((-2, "loading %s file %s", state.global ? "global" : "object", s));
					n = load(fp, s, 0);
					if (n > 0)
					{
						sfclose(fp);
						sfclose(sp);
						return;
					}
				}
				r = getrule(name);
			}
			sfclose(fp);
			if (state.global)
				error(1, "%s: reading%s", r->name, state.forceread ? " -- should be compiled before local makefiles" : null);
			else if (state.writeobject)
				error(state.exec || state.mam.out ? -1 : 1, "%s: recompiling", s);
		}
	}

	/*
	 * at this point we have to read it
	 * if its the first makefile then the
	 * base rules must be determined and loaded
	 * along with the global rules before the parse
	 */

	preprocess = state.preprocess;
	if (!state.global)
	{
		/*
		 * first check for and apply makefile converter
		 */

		s = 0;
		if (*(t = getval(external.convert, 1)))
		{
			if (e = strchr(r->name, '/'))
				e++;
			else
				e = r->name;
			b = tokopen(t, 1);
			while (t = tokread(b))
			{
				n = strmatch(e, t);
				if (!(s = tokread(b)))
				{
					error(2, "%s: %s: no action for pattern", external.convert, t);
					break;
				}
				if (n)
				{
					s = getarg((e = t = strdup(s), &e), NiL);
					break;
				}
				s = 0;
			}
			tokclose(b);
		}
		if (s)
		{
			message((-2, "converting %s using \"%s\"", r->name, s));
			sfclose(sp);
			if (!(sp = fapply(internal.internal, null, r->name, s, CO_ALWAYS|CO_LOCAL|CO_URGENT)))
				error(3, "%s: error in makefile converter \"%s\"", r->name, s);
			free(t);
			preprocess = -1;
		}
		if (needrules)
		{
			if ((s = sfreserve(sp, 0, 0)) && (n = sfvalue(sp)) >= 0)
			{
				int	c;
				int	d;

				if (n > 0)
				{
					if (n > MAXNAME)
						n = MAXNAME;
					else
						n--;
				}

				/*
				 * quick makefile type check while
				 * checking for base rules
				 */

				splice = 0;
				b = s;
				c = *(s + n);
				*(s + n) = 0;
				for (;;)
				{
					if (e = strchr(s, '\n'))
						*e = 0;
					else if (c != '\n')
						break;
					if (splice)
						/* skip */;
					else if (*s == SALT)
					{
						while (isspace(*++s));
						for (t = s; isalnum(*t); t++);
						d = *t;
						*t = 0;
						if (strneq(s, "rules", 5))
						{
							if (*t = d)
								t++;
							while (*t == ' ' || *t == '\t')
								t++;
							rules(*t == '/' && *(t + 1) == '*' ? null : t);
							break;
						}
						else if (!strmatch(s, "assert|comment|define|elif|else|endif|endmac|error|ident|if|ifdef|ifndef|include|line|macdef|pragma|unassert|undef|warning"))
							punt(1);
						else if (!preprocess)
							preprocess = 1;
						*t = d;
					}
					else
					{
						while (isspace(*s))
							s++;
						if (strneq(s, "rules", 5))
						{
							for (s += 5; *s == ' ' || *s == '\t'; s++);
							rules(*s == '/' && *(s + 1) == '*' ? null : s);
							break;
						}
						else if (*s == '/' && *++s == '*')
						{
							if (!(t = e))
								break;
							while (t > s && isspace(*--t));
							if (*t-- != '/' || t <= s || *t != '*')
								break;
						}
					}
					if (!(s = e))
						break;
					splice = e > b && *(e - 1) == '\\';
					*s++ = '\n';
				}
				if (e)
					*e = '\n';
				*(b + n) = c;
			}
			if (!state.rules)
				state.rules = getval(external.rules, 1);
			readrules();
			r = getrule(name);
		}
	}

	/*
	 * check for obsolete makefile preprocessor
	 */

	if (preprocess > 0)
	{
		s = "$(MAKEPP) $(MAKEPPFLAGS) $(>)";
		message((-2, "preprocessing %s using \"%s\"", r->name, s));
		sfclose(sp);
		if (!(sp = fapply(internal.internal, null, r->name, s, CO_ALWAYS|CO_LOCAL|CO_URGENT)))
			error(3, "%s: error in makefile preprocessor \"%s\"", r->name, s);
	}

	/*
	 * parse the file
	 */

	if (state.base)
	{
		if (!state.compile)
			state.compile = RECOMPILE;
		state.global = 1;
	}
	n = state.reading;
	state.reading = 1;
	parse(sp, NiL, r->name, 0);
	sfclose(sp);
	state.reading = n;
	if (!state.compile && !state.global)
		state.compile = RECOMPILE;
	if ((state.questionable & 0x00000400) || !state.global)
		state.forceread = 1;
}

/*
 * read a makefile
 */

int
readfile(register char* file, int type, char* filter)
{
	register struct rule*	r;
	Sfio_t*			rfp;

	if (streq(file, "-") && (file = "/dev/null") || isdynamic(file))
	{
		rfp = sfstropen();
		expand(rfp, file);
		state.init++;
		file = makerule(sfstruse(rfp))->name;
		state.init--;
		sfstrclose(rfp);
	}
	state.init++;
	r = bindfile(NiL, file, BIND_FORCE|BIND_MAKEFILE|BIND_RULE);
	state.init--;
	if (r)
	{
		compref(type, unbound(r), r->time);
		r->dynamic |= D_scanned;
		file = r->name;
		if (rfp = filter ? fapply(internal.internal, null, file, filter, CO_ALWAYS|CO_LOCAL|CO_URGENT) : rsfopen(file))
		{
			if (state.mam.dynamic || state.mam.regress)
				mampush(state.mam.out, r, P_force);
			fcntl(sffileno(rfp), F_SETFD, FD_CLOEXEC);
			if (state.user)
			{
				r->status = EXISTS;
				parse(rfp, NiL, file, 0);
				sfclose(rfp);
			}
			else readfp(rfp, r, type);
			if (state.mam.dynamic || state.mam.regress)
				mampop(state.mam.out, r, 0);
			if ((type & COMP_BASE) && r->uname)
			{
				oldname(r);
				r->dynamic &= ~D_bound;
			}
			return(1);
		}
		if ((type & COMP_DONTCARE) || (r->property & P_dontcare))
		{
			r->property |= P_dontcare;
			return(0);
		}
	}
	if (!(type & COMP_DONTCARE))
		error((type & COMP_INCLUDE) ? 2 : 3, "%s: cannot read%s", file, (type & COMP_INCLUDE) ? " include file" : (type & COMP_GLOBAL) ? " global rules" : (type & COMP_BASE) ? " base rules" : null);
	return(0);
}
