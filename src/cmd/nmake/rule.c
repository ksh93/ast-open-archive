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
 * make rule support
 *
 * the attribute names defined in this file must agree with ATTRNAME
 */

#include "make.h"
#include "options.h"

#define ASOC(field,name,flags)	internal.field=rassociate(name,flags)
#define ATTR(field,name,flags)	internal.field=rinternal(name,P_attribute|(flags))
#define FUNC(field,name,func)	((internal.field=setvar(name,NiL,V_builtin|V_functional))->builtin=(func))
#define INIT(field,name,flags)	internal.field=rinternal(name,flags)

/*
 * return non-0 if rule name s requires dynamic expand()
 */

int
isdynamic(register const char* s)
{
	register int	g;

	g = 0;
	for (;;)
		switch (*s++)
		{
		case 0:
			return 0;
		case '*':
		case '?':
		case '[':
		case ']':
		case '|':
		case '&':
			return 1;
		case '@':
		case '!':
		case '}':
		case '$':
			if (*s == '(')
				g = 1;
			break;
		case ')':
			if (g)
				return 1;
			break;
		}
	/*NOTREACHED*/
}

/*
 * return non-0 if rule name s requires dynamic glob()
 */

int
isglob(register const char* s)
{
	register int	g;

	g = 0;
	for (;;)
		switch (*s++)
		{
		case 0:
			return 0;
		case '*':
		case '?':
		case '[':
		case ']':
		case '|':
		case '&':
			return 1;
		case '@':
		case '!':
		case '}':
			if (*s == '(')
				g = 1;
			break;
		case ')':
			if (g)
				return 1;
			break;
		}
	/*NOTREACHED*/
}

/*
 * return non-0 if s is a state variable name
 */

int
isstatevar(register const char* s)
{
	if (*s++ == '(')
		for (;;)
			switch (*s++)
			{
			case 0:
			case '(':
				return 0;
			case ')':
				return !*s;
			}
	return 0;
}

/*
 * return a pointer to a rule given its name,
 * creating the rule if necessary
 */

struct rule*
makerule(register char* name)
{
	register struct rule*	r;
	int			n;

	if (name)
	{
		if (r = getrule(name))
			return r;
		if (!(n = isstate(name)) && !isdynamic(name) && !iscontext(name) && (table.rule->flags & HASH_ALLOCATE))
		{
			pathcanon(name, 0);
			if (r = getrule(name))
				return r;
		}
	}
	newrule(r);
	r->name = putrule(0, r);
	if (!name)
		n = isstate(r->name);
	if (n)
	{
		r->dynamic |= D_compiled;
		if (isstatevar(r->name)) r->property |= P_state|P_statevar;
		else r->property |= P_state|P_staterule;
	}
	else if (state.init || state.readonly) r->dynamic |= D_compiled;
	else r->dynamic &= ~D_compiled;
	r->status = NOTYET;
	r->preview = state.maxview + 1;
#if DEBUG
	message((-13, "adding %s %s", (r->property & P_state) ? "state variable" : "atom", r->name));
#endif
	return r;
}

/*
 * check if a rule name is special
 * special names are ignored in determining the default main target(s)
 */

int
special(register struct rule* r)
{
	register char*	s;

	if (r->property & (P_functional|P_metarule|P_operator|P_repeat|P_state))
		return 1;
	if ((s = r->name) && !istype(*s, C_ID2))
		for (;;)
			switch (*s++)
			{
			case 0:
				return 1;
			case '/':
			case '$':
				return 0;
			}
	return 0;
}

/*
 * return joint list pointer for r
 * static single element list returned for non-joint target
 */

struct list*
joint(register struct rule* r)
{
	static struct list	tmp;

	if ((r->property & (P_joint|P_target)) == (P_joint|P_target))
		return r->prereqs->rule->prereqs;
	tmp.rule = r;
	return &tmp;
}

/*
 * add a single non-duplicated prerequisite x to rule r
 * op = {PREREQ_APPEND,PREREQ_DELETE,PREREQ_INSERT,PREREQ_LENGTH}
 */

void
addprereq(register struct rule* r, register struct rule* x, int op)
{
	register struct list*	p;
	register struct list*	q;

	if (x != r)
	{
		if (p = r->prereqs)
		{
			q = 0;
			while (p)
			{
				if (x == p->rule)
				{
					if (op == PREREQ_DELETE)
					{
						if (q) q->next = p->next;
						else r->prereqs = p->next;
					}
					if (!(x->property & P_multiple)) break;
				}
				if (!p->next)
				{
					if (op != PREREQ_DELETE)
					{
						if (r->property & P_state) state.savestate = 1;
						if (!state.init && !state.readonly) r->dynamic &= ~D_compiled;
						if (op == PREREQ_LENGTH)
						{
							register int	n;

							n = strlen(x->name);
							p = r->prereqs;
							q = 0;
							for (;;)
							{
								if (!p || strlen(p->rule->name) < n)
								{
									if (q) q->next = cons(x, p);
									else r->prereqs = cons(x, r->prereqs);
									break;
								}
								q = p;
								p = p->next;
							}
						}
						else if (op == PREREQ_INSERT) r->prereqs = cons(x, r->prereqs);
						else p->next = cons(x, NiL);
					}
					break;
				}
				q = p;
				p = p->next;
			}
		}
		else if (op != PREREQ_DELETE)
		{
			if (r->property & P_state) state.savestate = 1;
			if (!state.init && !state.readonly) r->dynamic &= ~D_compiled;
			r->prereqs = cons(x, NiL);
		}
	}
}

/*
 * return the pattern association rule with prefix a that matches r
 * if r is 0 then s is the rule name
 * if pos is not 0 then it is the last match position
 * *pos must be 0 on first call
 * *pos undefined when 0 returned
 */

struct rule*
associate(register struct rule* a, register struct rule* r, register char* s, struct list** pos)
{
	register struct list*	p;
	register struct rule*	x;
	struct list*		u;

	if (r)
	{
		if (r->property & (P_attribute|P_readonly))
			return 0;
		s = r->name;
	}
	do
	{
		u = 0;
		for (p = pos && *pos ? (*pos)->next : a->prereqs; p; p = p->next)
			if ((x = p->rule) != r)
			{
				if (x->property & P_attribute)
				{
					if (r && hasattribute(r, x, NiL))
						break;
				}
				else if (x->name[0] == '%' && !x->name[1])
					u = p;
				else if (metamatch(NiL, s, x->name) || r && r->uname && !(r->property & P_state) && metamatch(NiL, r->uname, x->name))
					break;
			}
		if (p || (p = u))
		{
			if (pos)
				*pos = p;
			return (p->rule->property & P_attribute) ?
				catrule(a->name, "%", p->rule->name, 0) :
				catrule(a->name, p->rule->name, NiL, 0);
		}
	} while (r && s == r->name && r->uname && !(r->property & P_state) && (s = r->uname) != r->name);
	return 0;
}

/*
 * check if r's prerequisite list or named attributes have changed
 *
 * NOTE: IGNORECHANGE(r) prerequisites are ignored in the comparison
 */

#define IGNORECHANGE(r)	((r)->property & (P_joint|P_ignore))

int
prereqchange(register struct rule* r, register struct list* newprereqs, struct rule* o, register struct list* oldprereqs)
{
	if ((r->property & P_accept) || state.accept)
		return 0;
	if ((r->attribute ^ o->attribute) & ~internal.accept->attribute)
	{
		reason((1, "%s named attributes changed", r->name));
		return 1;
	}
	for (;;)
	{
		if (newprereqs)
		{
			if (IGNORECHANGE(newprereqs->rule))
				newprereqs = newprereqs->next;
			else if (oldprereqs)
			{
				if (IGNORECHANGE(oldprereqs->rule))
					oldprereqs = oldprereqs->next;
				else if (newprereqs->rule == oldprereqs->rule || ((newprereqs->rule->dynamic ^ oldprereqs->rule->dynamic) & (D_alias|D_bound)) && getrule(newprereqs->rule) == getrule(oldprereqs->rule))
				{
					newprereqs = newprereqs->next;
					oldprereqs = oldprereqs->next;
				}
				else
					break;
			}
			else
				break;
		}
		else if (oldprereqs && IGNORECHANGE(oldprereqs->rule))
			oldprereqs = oldprereqs->next;
		else
			break;
	}
	if (newprereqs)
	{
		reason((1, "%s prerequisite %s added or re-ordered", r->name, newprereqs->rule->name));
		return 1;
	}
	if (oldprereqs)
	{
		reason((1, "%s prerequisite %s deleted or re-ordered", r->name, oldprereqs->rule->name));
		return 1;
	}
	return 0;
}

/*
 * initialize immediate rule info
 */

static void
getimmediate(register struct rule* r, struct list** prereqs, char** action)
{
	if (r->dynamic & D_dynamic)
		dynamic(r);
	if (*action = r->action)
		r->action = 0;
	if (*prereqs = r->prereqs)
		r->prereqs = 0;
}

/*
 * reset all rule info as if we just started
 */

static int
reset(const char* s, char* v, void* h)
{
	register struct rule*	r = (struct rule*)v;
	struct stat		st;

	if (!(r->property & P_state))
	{
		if ((r->status == EXISTS || r->status == FAILED) && !(r->property & P_virtual))
		{
			r->status = NOTYET;
			if (!stat(r->name, &st))
				r->time = st.st_mtime;
			r->dynamic &= ~(D_entries|D_hasafter|D_hasbefore|D_hasmake|D_hasscope|D_hassemaphore|D_scanned|D_triggered);
		}
		else
			r->dynamic &= ~(D_entries|D_scanned);
	}
	return 0;
}

/*
 * check rule r for immediate action
 * should only be called if r->immediate==1 and r->target==1
 */

void
immediate(register struct rule* r)
{
	register struct list*	p;
	register struct rule*	x;
	struct list*		prereqs;
	char*			action;
	char*			e;
	struct var*		v;
	int			i;
	int			g;
	int			u;
	unsigned long		a;

	if (r == internal.retain || r == internal.state)
	{
		getimmediate(r, &prereqs, &action);
		a = r == internal.retain ? V_retain : V_scan;
		for (p = prereqs; p; p = p->next)
			if (v = varstate(p->rule, 1))
			{
				if (a == V_scan) setvar(v->name, v->value, V_scan);
				else v->property |= a;
			}
	}
	else if (r == internal.rebind || r == internal.accept)
	{
		getimmediate(r, &prereqs, &action);
		i = r == internal.accept;
		for (p = prereqs; p; p = p->next)
			rebind(p->rule, i);
	}
	else if (r == internal.unbind)
	{
		getimmediate(r, &prereqs, &action);
		for (p = prereqs; p; p = p->next)
			if (p->rule->dynamic & (D_bound|D_scanned))
				p->rule->mark |= M_mark;
		hashwalk(table.rule, 0, unbind, r);
		hashwalk(table.rule, 0, unbind, NiL);
	}
	else if (r == internal.bind || r == internal.force)
	{
		getimmediate(r, &prereqs, &action);
		for (p = prereqs; p; p = p->next)
		{
			x = p->rule;
			message((-2, "bind(%s)", x->name));
			x = bind(x);
			if (r == internal.force)
			{
				if (x->time || !(x->property & P_dontcare))
					x->time = CURTIME;
				x->property |= P_force;
			}
		}
	}
	else if (r == internal.always || r == internal.local || r == internal.make || r == internal.run)
	{
		int		errors;
		unsigned long	now;
		unsigned long	tm = 0;

		getimmediate(r, &prereqs, &action);
		i = !prereqs;
		now = CURTIME;
		errors = 0;
		for (p = prereqs; p; p = p->next)
		{
			if ((p->rule->status == UPDATE || p->rule->status == MAKING) && !(p->rule->property & P_repeat))
				p->rule = internal.empty;
			else
			{
				errors += make(p->rule, &tm, NiL, 0);
				if (tm >= now) i = 1;
			}
		}
		if (r != internal.run)
		{
			if (prereqs) errors += complete(NiL, prereqs, &tm, 0);
			if (tm >= now) i = 1;
			if (action)
			{
				if (!errors && i)
				{
					r->status = UPDATE;
					trigger(r, NiL, action, 0);
					complete(r, NiL, NiL, 0);
				}
			}
			if (r == internal.make) r->property &= ~(P_always|P_local);
		}
		r->property &= ~(P_foreground|P_make|P_read);
	}
	else if (r == internal.include)
	{
		getimmediate(r, &prereqs, &action);
		i = COMP_INCLUDE;
		g = state.global;
		u = state.user;
		for (p = prereqs; p; p = p->next)
		{
			x = p->rule;
			if (streq(x->name, "-"))
				i ^= COMP_DONTCARE;
			else if (streq(x->name, "+"))
			{
				state.global = 1;
				state.user = 0;
			}
			else if (x->property & P_use)
				action = x->action;
			else
				readfile(x->name, i, action);
		}
		state.global = g;
		state.user = u;
	}
	else if (r == internal.alarm)
	{
		getimmediate(r, &prereqs, &action);
		if (p = prereqs)
		{
			a = strelapsed(p->rule->name, &e, 1);
			if (*e)
				a = 0;
			else
				p = p->next;
		}
		else
			a = 0;
		wakeup(a, p);
	}
	else if (r == internal.sync)
	{
		getimmediate(r, &prereqs, &action);
		if (!prereqs) savestate();
		else if (state.compile < COMPILED) error(2, "%s: cannot sync until make object compiled", prereqs->rule->name);
		else compile(prereqs->rule->name, prereqs->next ? prereqs->next->rule->name : (char*)0);
	}
	else if (r == internal.reset)
	{
		getimmediate(r, &prereqs, &action);
		hashwalk(table.rule, 0, reset, NiL);
	}
	else if (r == internal.wait)
	{
		getimmediate(r, &prereqs, &action);
		complete(NiL, prereqs, NiL, 0);
	}
	else if ((r->property & P_attribute) && !r->attribute) return;
	else if (!state.op && state.reading && state.compileonly) return;
	else
	{
		maketop(r, 0, NiL);
		getimmediate(r, &prereqs, &action);
	}
	if (prereqs) freelist(prereqs);
	if (r->prereqs)
	{
		freelist(r->prereqs);
		r->prereqs = 0;
	}
	r->action = 0;
	if (r = staterule(RULE, r, NiL, 0))
	{
		r->prereqs = 0;
		r->action = 0;
	}
}

/*
 * remove duplicate prerequisites from prerequisite list p
 */

void
remdup(register struct list* p)
{
	register struct list*	q;
	register struct list*	x;

	for (x = p, q = 0; p; p = p->next)
	{
		if (p->rule->mark & M_mark)
		{
			if (q) q->next = p->next;
#if DEBUG
			else
			{
				dumprule(sfstderr, p->rule);
				error(PANIC, "stray mark on %s", p->rule->name);
			}
#endif
		}
		else
		{
			if (!(p->rule->property & P_multiple))
				p->rule->mark |= M_mark;
			q = p;
		}
	}
	while (x)
	{
		x->rule->mark &= ~M_mark;
		x = x->next;
	}
}

/*
 * do dynamic expansion of rule r's prerequisites
 * the prerequisite list is permanently modified
 */

void
dynamic(register struct rule* r)
{
	register char*		s;
	register struct list*	p;
	register struct list*	q;
	register struct list*	t;
	char**			v;
	char*			buf;
	int			added;
	int			flags;
	struct rule*		x;
	struct rule*		u;
	struct frame*		oframe;
	struct frame		frame;
	Sfio_t*			tmp;
	char*			vec[2];

	tmp = sfstropen();
	oframe = state.frame;
	if ((r->property & P_use) && !(state.frame = r->active))
	{
		zero(frame);
		frame.target = r;
		state.frame = frame.parent = &frame;
	}
	vec[1] = 0;
	added = 0;
	for (p = r->prereqs, q = 0; p; p = p->next)
	{
		if (isdynamic(p->rule->name))
		{
			if (q)
				q->next = p->next;
			else
				r->prereqs = p->next;
			u = p->rule;
			expand(tmp, u->name);
			buf = sfstruse(tmp);
			while (s = getarg(&buf, &flags))
			{
				added = 1;
				if (isglob(s))
					v = globv(s);
				else
					*(v = vec) = s;
				while (s = *v++)
				{
					x = makerule(s);
					if (x->dynamic & D_alias)
						x = makerule(x->name);
					if (flags & A_scope)
						x->dynamic |= D_scope;

					/*
					 * merge u into x
					 */

					merge(u, x, MERGE_ALL|MERGE_SCANNED);

					/*
					 * replace u with x
					 */

					t = cons(x, p->next);
					if (q)
						q->next = t;
					else
						r->prereqs = t;
					q = t;
				}
			}
		}
		else q = p;
	}
	if (added) remdup(r->prereqs);
	r->dynamic &= ~D_dynamic;
	state.frame = oframe;
	sfstrclose(tmp);
}

/*
 * return non-zero if r has builtin attribute a
 * if x!=0 then check if merge(x,r,MERGE_ATTR) would have attribute
 */

int
hasattribute(register struct rule* r, register struct rule* a, register struct rule* x)
{
	register long	n;
	register int	attrname;
	struct list*	p;

	attrname = *a->name == ATTRNAME;
	if (a->property & P_attribute)
	{
		n = r->property;
		if (x && !(n & P_readonly))
		{
			n |= x->property & ~(P_attribute|P_immediate|P_implicit|P_internal|P_operator|P_readonly|P_state|P_staterule|P_statevar|P_target);
			if ((x->property & P_implicit) || (x->property & (P_metarule|P_terminal)) == P_terminal)
				n &= ~P_implicit;
		}

		/*
		 * the first group may conflict with a->attribute
		 */

		if (attrname)
		{
			if (a == internal.accept) return n & P_accept;
			if (a == internal.attribute) return n & P_attribute;
			if (a == internal.ignore) return n & P_ignore;
		}
		if (a != internal.retain)
		{
			if (a->attribute & r->attribute) return 1;
			if (x && (a->attribute & x->attribute & ~internal.ignore->attribute) && ((r->property & (P_attribute|P_use)) != P_attribute || r == internal.accept || r == internal.ignore || r == internal.retain)) return 1;
		}
		if (attrname)
		{
			/*
			 * the rest have no a->attribute conflicts
			 */

			if (a == internal.make) return n & P_make;
			if (a == internal.readonly) return n & P_readonly;
			if (a == internal.scan) return r->scan || x && x->scan;
			if (a == internal.semaphore) return r->semaphore;
			if (a == internal.target) return n & P_target;

			/*
			 * the rest have a corresponding P_*
			 */

			if (a->property & n & ~(P_accept|P_attribute|P_ignore|P_internal|P_metarule|P_readonly|P_staterule|P_statevar|P_target))
			{
				if (a == internal.after) return n & P_after;
				if (a == internal.always) return n & P_always;
				if (a == internal.archive) return n & P_archive;
				if (a == internal.before) return n & P_before;
				if (a == internal.command) return n & P_command;
				if (a == internal.dontcare) return n & P_dontcare;
				if (a == internal.force) return n & P_force;
				if (a == internal.foreground) return n & P_foreground;
				if (a == internal.functional) return n & P_functional;
				if (a == internal.immediate) return n & P_immediate;
				if (a == internal.implicit) return n & P_implicit;
				if (a == internal.joint) return n & P_joint;
				if (a == internal.local) return n & P_local;
				if (a == internal.multiple) return n & P_multiple;
				if (a == internal.op) return n & P_operator;
				if (a == internal.parameter) return n & P_parameter;
				if (a == internal.read) return n & P_read;
				if (a == internal.repeat) return n & P_repeat;
				if (a == internal.state) return n & P_state;
				if (a == internal.terminal) return n & P_terminal;
				if (a == internal.use) return n & P_use;
				if (a == internal.virt) return n & P_virtual;
			}
		}
		if (a->scan) return a->scan == r->scan || !r->scan && x && x->scan == a->scan;
	}
	else
	{
		if (attrname)
		{
			/*
			 * r->dynamic readonly attributes
			 */

			n = r->dynamic;
			if (a == internal.bound) return n & D_bound;
			if (a == internal.built) return n & D_built;
			if (a == internal.entries) return n & D_entries;
			if (a == internal.member) return n & (D_member|D_membertoo);
			if (a == internal.regular) return n & D_regular;
			if (a == internal.scanned) return n & D_scanned;
			if (a == internal.triggered) return n & D_triggered;
			if (a == internal.file) return (n & D_bound) && !(r->property & (P_state|P_virtual)) && r->time;

			/*
			 * r->property readonly attributes
			 */

			n = r->property;
			if (a == internal.internal) return n & P_internal;
			if (a == internal.metarule) return n & P_metarule;
			if (a == internal.staterule) return n & P_staterule;
			if (a == internal.statevar) return n & P_statevar;

			/*
			 * r->status readonly attributes
			 */

			n = r->status;
			if (a == internal.exists) return n == EXISTS;
			if (a == internal.failed) return n == FAILED;
			if (a == internal.making) return n == MAKING;
			if (a == internal.notyet) return n == NOTYET;

			/*
			 * other readonly attributes
			 */

			if (a == internal.active) return r->active != 0;
		}

		/*
		 * r prerequisites as pseudo attributes
		 */

		for (p = r->prereqs; p; p = p->next)
			if (a == p->rule)
				return 1;
	}
	return 0;
}

/*
 * merge <from> into <to> according to op
 */

void
merge(register struct rule* from, register struct rule* to, int op)
{
	register struct list*	p;

	if (from->name)
	{
		if (from == to || to->status != NOTYET && (to->status != UPDATE || !(from->property & P_use))) return;
#if DEBUG
		if (to->name) message((-4, "merging %s%s into %s", (op & MERGE_ATTR) ? "attributes of " : null, from->name, to->name));
#endif
	}
	to->property |= from->property & (P_accept|P_after|P_always|P_archive|P_before|P_command|P_dontcare|P_force|P_foreground|P_functional|P_ignore|P_implicit|P_joint|P_local|P_make|P_multiple|P_parameter|P_read|P_repeat|P_terminal|P_virtual);
	if (from->property & P_implicit) to->property &= ~P_terminal;
	if ((from->property & (P_metarule|P_terminal)) == P_terminal) to->property &= ~P_implicit;
	if (op & MERGE_ALL)
	{
		if (!to->action) to->action = from->action;
		to->attribute |= from->attribute;
		to->property |= from->property & (P_accept|P_immediate|P_target|P_use);
		to->dynamic |= from->dynamic & (D_dynamic|D_global|D_regular);
		if (!(op & MERGE_SCANNED))
			to->dynamic |= from->dynamic & (D_entries|D_scanned);
		if (from->scan && from->scan != SCAN_NULL) to->scan = from->scan;
		if (to->status == NOTYET) to->status = from->status;
		for (p = from->prereqs; p; p = p->next)
			addprereq(to, p->rule, PREREQ_APPEND);
		if (!(to->property & P_state))
		{
			if (op & MERGE_BOUND)
			{
				from->mark |= M_bind;
				to->mark |= M_bind;
			}
			mergestate(from, to);
			if (op & MERGE_BOUND)
			{
				from->mark &= ~M_bind;
				to->mark &= ~M_bind;
			}
		}
	}
	else if (op & MERGE_FORCE)
	{
		if (from->attribute && from != internal.accept && from != internal.ignore && from != internal.retain && ((to->property & (P_attribute|P_use)) != P_attribute || to == internal.accept || to == internal.ignore || to == internal.retain))
			to->attribute |= from->attribute;
		if (from->scan) to->scan = from->scan;
	}
	else
	{
		if (from->attribute && from != internal.accept && from != internal.ignore && from != internal.retain && ((to->property & (P_attribute|P_use)) != P_attribute || to == internal.accept || to == internal.ignore || to == internal.retain))
			to->attribute |= from->attribute & ~internal.ignore->attribute;
		if (!to->scan) to->scan = from->scan;
	}
}

/*
 * merge <from> state rules into <to>
 */

void
mergestate(struct rule* from, struct rule* to)
{
	register int		i;
	register struct rule*	fromstate;
	register struct rule*	tostate;
	struct rule*		t;
	char*			s;

	/*
	 * if RULE merges then RULE+1..STATERULES also merge
	 */

	if (fromstate = staterule(RULE, from, NiL, 0)) tostate = staterule(RULE, to, NiL, 1);
	else if ((from->dynamic & D_alias) && (tostate = staterule(RULE, to, NiL, 0))) fromstate = staterule(RULE, from, NiL, 1);
	else return;
#if DEBUG
	if (state.test & 0x00000800)
	{
		error(2, "MERGESTATE from: %s: %s time=[%s] event=[%s]", from->name, fromstate->name, strtime(fromstate->time), strtime(fromstate->event));
		error(2, "MERGESTATE   to: %s: %s time=[%s] event=[%s]", to->name, tostate->name, strtime(tostate->time), strtime(tostate->event));
	}
#endif
	if ((from->dynamic & D_alias) && fromstate->time && fromstate->time != tostate->time)
	{
		/*
		 * the solution is conservative but ok
		 * since aliases probably don't change
		 * very often -- every target depending
		 * on <to> will be forced out of date rather
		 * than those that just depend on <from>
		 */

		reason((1, "%s alias has changed to %s", unbound(from), unbound(to)));
		to->dynamic |= D_aliaschanged;
	}
	if (fromstate->event != tostate->event)
	{
		if (fromstate->event < tostate->event)
		{
			/*
			 * merge in the other direction
			 */

			t = from;
			from = to;
			to = t;
			t = fromstate;
			fromstate = tostate;
			tostate = t;
		}
		s = tostate->name;
		*tostate = *fromstate;
		tostate->prereqs = listcopy(fromstate->prereqs);
		tostate->name = s;
		for (i = RULE + 1; i <= STATERULES; i++)
			if ((fromstate = staterule(i, from, NiL, 0)) && !staterule(i, to, NiL, 0))
			{
				tostate = staterule(i, to, NiL, 1);
				s = tostate->name;
				*tostate = *fromstate;
				tostate->prereqs = listcopy(fromstate->prereqs);
				tostate->name = s;
			}
	}
}

/*
 * negate <from> attributes in <to>
 */

void
negate(register struct rule* from, register struct rule* to)
{
	to->attribute &= ~from->attribute;
	to->property &= ~(from->property & (P_accept|P_after|P_always|P_archive|P_before|P_command|P_dontcare|P_force|P_functional|P_ignore|P_immediate|P_implicit|P_local|P_make|P_multiple|P_parameter|P_repeat|P_target|P_terminal|P_use|P_virtual));
	to->dynamic &= ~(from->dynamic & (D_dynamic|D_entries|D_regular));
	if (from->scan) to->scan = 0;
	if (from->semaphore) to->semaphore = 0;
}

/*
 * make an internal rule pointer
 */

static struct rule*
rinternal(char* s, register int flags)
{
	register struct rule*	r;

	r = makerule(s);
	r->property |= flags;
	if (!r->prereqs && !r->action)
		r->dynamic |= D_compiled;
	return r;
}

/*
 * make an internal pattern association rule pointer
 * NOTE: this is required to sync pre 2001-05-09 make objects 
 */

static struct rule*
rassociate(char* s, register int flags)
{
	register struct rule*	r;
	register struct rule*	a;
	register struct list*	p;

	r = rinternal(s, flags);
	for (p = r->prereqs; p; p = p->next)
		if (!(p->rule->property & P_attribute) && p->rule->name[0] == '%' && p->rule->name[1] == ATTRNAME && (a = getrule(p->rule->name + 1)) && (a->property & P_attribute))
			p->rule = a;
	return r;
}

/*
 * maintain atomic dir-rule names
 */

static int
diratom(const char* s, char* v, void* h)
{
	struct dir*	d = (struct dir*)v;

	NoP(s);
	NoP(h);
	d->name = getrule(d->name)->name;
	return 0;
}

/*
 * external engine name initialization -- the rest are in initrule()
 *
 * NOTE: version.c may reference some of these names, not to mention
 *	 non-engine source makefiles
 */

struct external external =
{
	/*
	 * variable names
	 */

	"MAKEARGS",
	"MAKECONVERT",
	"MAKEFILE",
	"MAKEFILES",
	"MAKEIMPORT",
	"MAKELIB",
	"MAKE",
	CO_ENV_PROC,
	"OLDMAKE",
	"PWD",
	"MAKERULES",
	"MAKEVERSION",
	"MAKEPATH",
	"VPATH",

	/*
	 * infrequently used engine interface names
	 */

	".COMPDONE",
	".COMPINIT",
	".DONE",
	".INIT",
	".INTERRUPT",
	".JOBDONE",
	".MAKEDONE",
	".MAKEINIT",
	".MAKEPROMPT",
	".MAKERUN",
	".MAMNAME.",
	".MAMACTION.",

	/*
	 * related file suffixes
	 */

	".ml",
	".mo",
	".mk",
	".ms",
};

/*
 * initialize some attribute and internal rule pointers
 */

void
initrule(void)
{
	struct rule*	r;
	int		i;

	static int	repeat;

	hashclear(table.rule, HASH_ALLOCATE);

	/*
	 * dynamic rule attributes
	 */

	ATTR(accept,		".ACCEPT",	P_accept|P_immediate);
	ATTR(after,		".AFTER",	P_after);
	ATTR(always,		".ALWAYS",	P_always|P_immediate);
	ATTR(archive,		".ARCHIVE",	P_archive);
	ATTR(attribute,		".ATTRIBUTE",	P_readonly);
	ATTR(before,		".BEFORE",	P_before);
	ATTR(command,		".COMMAND",	P_command);
	ATTR(dontcare,		".DONTCARE",	P_dontcare);
	ATTR(force,		".FORCE",	P_force|P_immediate);
	ATTR(foreground,	".FOREGROUND",	P_foreground);
	ATTR(functional,	".FUNCTIONAL",	P_functional);
	ATTR(ignore,		".IGNORE",	P_ignore);
	ATTR(immediate,		".IMMEDIATE",	P_immediate);
	ATTR(implicit,		".IMPLICIT",	P_implicit);
	ATTR(joint,		".JOINT",	P_joint);
	ATTR(local,		".LOCAL",	P_local|P_immediate);
	ATTR(make,		".MAKE",	P_immediate);
	ATTR(multiple,		".MULTIPLE",	P_multiple);
	ATTR(op,		".OPERATOR",	P_operator);
	ATTR(parameter,		".PARAMETER",	P_parameter);
	ATTR(read,		".READ",	P_read);
	ATTR(readonly,		".READONLY",	P_readonly);
	ATTR(repeat,		".REPEAT",	P_repeat);
	ATTR(state,		".STATE",	P_state|P_immediate);
	ATTR(terminal,		".TERMINAL",	P_terminal);
	ATTR(use,		".USE",		P_use);
	ATTR(virt,		".VIRTUAL",	P_virtual);

	/*
	 * readonly rule attributes
	 */

	INIT(active,		".ACTIVE",	0);
	INIT(bound,		".BOUND",	0);
	INIT(built,		".BUILT",	0);
	INIT(entries,		".ENTRIES",	0);
	INIT(exists,		".EXISTS",	0);
	INIT(failed,		".FAILED",	0);
	INIT(file,		".FILE",	0);
	INIT(member,		".MEMBER",	0);
	INIT(notyet,		".NOTYET",	0);
	INIT(regular,		".REGULAR",	0);
	INIT(scanned,		".SCANNED",	0);
	INIT(staterule,		".STATERULE",	0);
	INIT(statevar,		".STATEVAR",	0);
	INIT(target,		".TARGET",	P_attribute|P_target);
	INIT(triggered,		".TRIGGERED",	0);

	/*
	 * special rules and names
	 */

	INIT(alarm,		".ALARM",	P_immediate);
	INIT(args,		".ARGS",	P_internal);
	INIT(assert,		".ASSERT",	0);
	INIT(assign,		".ASSIGN",	0);
	INIT(bind,		".BIND",	P_immediate);
	INIT(clear,		".CLEAR",	P_attribute);
	INIT(copy,		".COPY",	P_attribute);
	INIT(delete,		".DELETE",	P_attribute);
	INIT(dot,		".",		0);
	INIT(empty,		"",		0);
	INIT(error,		".ERROR",	0);
	INIT(exported,		".EXPORT",	P_attribute|P_immediate);
	INIT(globalfiles,	".GLOBALFILES",	P_internal|P_readonly);
	INIT(include,		".INCLUDE",	P_immediate);
	INIT(insert,		".INSERT",	P_attribute);
	INIT(internal,		".INTERNAL",	P_internal|P_readonly);
	INIT(main,		".MAIN",	0);
	INIT(makefiles,		".MAKEFILES",	P_internal|P_readonly);
	INIT(making,		".MAKING",	0);
	INIT(metarule,		".METARULE",	0);
	INIT(null,		".NULL",	P_attribute);
	INIT(options,		".OPTIONS",	P_internal|P_readonly);
	INIT(preprocess,	".PREPROCESS",	P_internal|P_readonly);
	INIT(query,		".QUERY",	P_immediate|P_multiple|P_virtual);
	INIT(rebind,		".REBIND",	P_immediate);
	INIT(reset,		".RESET",	P_immediate);
	INIT(retain,		".RETAIN",	P_immediate);
	INIT(run,		".RUN",		P_immediate);
	INIT(scan,		".SCAN",	P_attribute|P_readonly);
	INIT(semaphore,		".SEMAPHORE",	P_attribute);
	INIT(source,		".SOURCE",	0);
	INIT(special,		".SPECIAL",	P_attribute|P_internal|P_readonly);
	INIT(sync,		".SYNC",	P_immediate);
	INIT(tmplist,		".TMPLIST",	P_internal|P_readonly);
	INIT(unbind,		".UNBIND",	P_immediate);
	INIT(view,		".VIEW",	P_internal|P_readonly);
	INIT(wait,		".WAIT",	P_immediate);

	/*
	 * pattern association rules
	 */

	ASOC(append_p,		".APPEND.",	0);
	ASOC(attribute_p,	".ATTRIBUTE.",	0);
	ASOC(bind_p,		".BIND.",	0);
	ASOC(dontcare_p,	".DONTCARE.",	0);
	ASOC(insert_p,		".INSERT.",	0);
	ASOC(require_p,		".REQUIRE.",	0);
	ASOC(source_p,		".SOURCE.",	0);

	/*
	 * builtin functions
	 */

	FUNC(getconf,		".GETCONF",	b_getconf);
	FUNC(outstanding,	".OUTSTANDING",	b_outstanding);

#if DEBUG
	putrule(".DEBUG", internal.query);
#endif
	hashset(table.rule, HASH_ALLOCATE);

	/*
	 * initialize the builtin attributes
	 */

	if (!repeat)
	{
		static struct frame	frame;

		frame.target = internal.internal;
		state.frame = frame.parent = &frame;
		internal.attribute->attribute = 1;
		internal.empty->dynamic |= D_bound;
		internal.empty->status = IGNORE;
		internal.empty->time = OLDTIME;
		internal.scan->scan = SCAN_USER;
		addprereq(internal.source, internal.dot, PREREQ_APPEND);
		addprereq(internal.special, internal.attribute, PREREQ_APPEND);
		addprereq(internal.special, internal.scan, PREREQ_APPEND);
		sfprintf(internal.tmp, "%s?(.*)", internal.source->name);
		internal.issource = strdup(sfstruse(internal.tmp));
	}
	initscan(repeat);
	initwakeup(repeat);

	/*
	 * maintain atomic dir-rule names
	 */

	hashwalk(table.dir, 0, diratom, NiL);
#if !BINDINDEX
	for (i = 0; i <= state.maxview; i++)
	{
		r = getrule(state.view[i].path);
		r->view = i;
		state.view[i].path = r->name;
	}
#endif

	/*
	 * expand some dynamic values now for efficiency
	 */

	if (internal.metarule->dynamic & D_dynamic) dynamic(internal.metarule);

	/*
	 * some things are only done once
	 */

	repeat++;
}

/*
 * low level for initview()
 */

static struct list*
view(register char* s, register char* d, struct list* p)
{
	register int		i;
	register struct rule*	r;

	if (!d)
		d = s;
	i = pathcanon(d, 0) - d;
	if (i > 2 && d[i - 1] == '/')
		d[--i] = 0;
	r = makerule(d);
	if ((unique(r) || !r->time) && r != internal.dot)
	{
		if (state.maxview < MAXVIEW - 1)
		{
#if BINDINDEX
			r->dynamic |= D_bindindex;
			state.view[++state.maxview].path = r;
#else
			state.view[++state.maxview].path = r->name;
#endif
			state.view[state.maxview].pathlen = i;
			r->view = state.maxview;
			p = p->next = cons(r, NiL);
			if (s != d)
			{
				i = pathcanon(s, 0) - s;
				if (i > 2 && s[i - 1] == '/')
					s[--i] = 0;
				r = makerule(s);
				r->view = state.maxview;
			}
			state.view[state.maxview].root = r->name;
			state.view[state.maxview].rootlen = i;
#if BINDINDEX
			message((-2, "view[%d]: %s %s", state.maxview, state.view[state.maxview].path->name, state.view[state.maxview].root));
#else
			message((-2, "view[%d]: %s %s", state.maxview, state.view[state.maxview].path, state.view[state.maxview].root));
#endif
		}
		else
			error(1, "view level %s ignored -- %d max", r->name, MAXVIEW);
	}
	return p;
}

/*
 * initialize the views
 */

void
initview(void)
{
	register char*	s;
	register char*	t;
	register int	n;
	int		c;
	int		pwdlen;
	char*		pwd;
	char*		tok;
	char*		u;
	struct list*	p;
	struct rule*	r;
	struct stat	top;
	struct stat	bot;
	Sfio_t*		tmp;

	p = internal.view->prereqs = cons(internal.dot, NiL);
	tmp = sfstropen();
	if (fs3d(FS3D_TEST))
	{
		if ((n = (s = colonlist(tmp, external.viewnode, ' ')) != 0) || (s = colonlist(tmp, external.viewdot, ' ')))
		{
			tok = tokopen(s, 0);
			if (s = n ? tokread(tok) : ".")
				while (t = tokread(tok))
				{
					message((-2, "vpath %s %s", s, t));
					mount(t, s, FS3D_VIEW, NiL);
					s = t;
				}
			tokclose(tok);
		}
		for (n = 1; n <= MAXVIEW; n++)
			sfputr(tmp, "/...", -1);
		sfputc(tmp, 0);
		s = t = sfstrrel(tmp, 0);
		while (!stat(t -= 4, &top))
		{
			if (state.maxview >= MAXVIEW - 1)
			{
				error(1, "view levels past %s ignored -- %d max", t += 4, MAXVIEW);
				break;
			}
			p = p->next = cons(r = makerule(t), NiL);
#if BINDINDEX
			state.view[++state.maxview].path = r;
#else
			state.view[++state.maxview].path = r->name;
#endif
			state.view[state.maxview].pathlen = s - t;
			message((-2, "view[%d]: %s", state.maxview, r->name));
		}
		state.fsview = 1;
		setvar(external.viewnode, NiL, 0);
		if (!stat(".", &top) && !stat("...", &bot) && top.st_ino == bot.st_ino && top.st_dev == bot.st_dev)
			state.virtualdot = 1;
	}
	else
	{
		unique(internal.dot);
		if (s = colonlist(tmp, external.viewnode, ' '))
		{
			tok = tokopen(s, 1);
			while (s = tokread(tok))
			{
				if (*s != '/')
					sfprintf(internal.tmp, "%s/", internal.pwd);
				sfputr(internal.tmp, s, -1);
				t = sfstruse(internal.tmp);
				s = pathcanon(t, 0);
				if (*(s - 1) == '/')
					*--s = 0;
				n = s - t;
				pwd = 0;
				if (strncmp(internal.pwd, t, n) || (c = internal.pwd[n]) && c != '/')
				{
					/*
					 * ksh pwd and ast getcwd() are logical
					 * others are physical
					 * this gets PWD and VPATH in sync
					 * if they're not
					 */

					if (!stat(t, &top))
					{
						sfputr(internal.nam, internal.pwd, -1);
						u = sfstruse(internal.nam);
						c = 0;
						for (;;)
						{
							if (!stat(u, &bot) && bot.st_ino == top.st_ino && bot.st_dev == top.st_dev)
							{
								sfprintf(internal.nam, "%s%s", t, internal.pwd + (s - u));
								pwd = strdup(sfstruse(internal.nam));
								pwdlen = strlen(pwd);
								break;
							}
							if (!(s = strrchr(u, '/')))
								break;
							*s = 0;
							c = 1;
						}
					}
					if (!pwd)
					{
						setvar(external.viewnode, NiL, 0);
						break;
					}
				}
				if (pwd)
				{
					internal.pwd = pwd;
					internal.pwdlen = pwdlen;
					setvar(external.pwd, internal.pwd, V_import);
				}
				state.view[0].root = makerule(t)->name;
				state.view[0].rootlen = n;
#if BINDINDEX
				message((-2, "view[%d]: %s %s", state.maxview, state.view[state.maxview].path->name, state.view[state.maxview].root));
#else
				message((-2, "view[%d]: %s %s", state.maxview, state.view[state.maxview].path, state.view[state.maxview].root));
#endif
				while (s = tokread(tok))
				{
					if (!c)
						p = view(s, NiL, p);
					else
					{
						if (*s != '/')
							sfprintf(internal.tmp, "%s/", internal.pwd);
						sfprintf(internal.tmp, "%s%s", s, internal.pwd + n);
						p = view(s, sfstruse(internal.tmp), p);
					}
				}
				if (!(opentry(OPT_strictview, 0)->status & OPT_READONLY))
					state.strictview = 1;
				break;
			}
			tokclose(tok);
		}
		if (s = colonlist(tmp, external.viewdot, ' '))
		{
			n = state.maxview;
			tok = tokopen(s, 1);
			while (s = tokread(tok))
			{
				pathcanon(s, 0);
				if (!n || *s != '.' || *(s + 1) != '.' || *(s + 2) && *(s + 2) != '/')
					p = view(s, NiL, p);
				else for (c = 0; c <= n; c++)
				{
#if BINDINDEX
					sfprintf(internal.tmp, "%s/%s", c ? state.view[c].path->name : internal.pwd, s);
#else
					sfprintf(internal.tmp, "%s/%s", c ? state.view[c].path : internal.pwd, s);
#endif
					p = view(sfstruse(internal.tmp), NiL, p);
				}
			}
			tokclose(tok);
		}
	}
	if (!state.maxview)
		state.believe = 0;
	sfstrclose(tmp);
}
