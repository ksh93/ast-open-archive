/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1984-2004 AT&T Corp.                *
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
 * make target proof algorithm
 */

#include "make.h"

#define PREVIEW(r,x)	do{if(r->preview>x->preview&&(r->scan&&r->scan==x->scan||!r->scan&&!((r->property|x->property)&P_virtual)))r->preview=x->preview;}while(0)

/*
 * make scan prereqs
 */

static int
makescan(register struct rule* r, unsigned long* tm)
{
	register struct rule*	s;
	register struct rule*	u;
	register struct list*	p;
	struct list*		q;
	int			errors = 0;
	unsigned long		tevent;
	unsigned long		t;

	message((-2, "check scan prerequisites"));
	q = scan(r, &tevent);
	for (p = q; p; p = p->next)
	{
		s = p->rule;
		if (!s->scan && (u = staterule(RULE, bind(s), NiL, 0)) && u->scan)
			s->scan = u->scan;
		make(s, &t, NiL, P_implicit);
	}
	errors += complete(r, q, &t, P_implicit);
	if (tevent < t)
		tevent = t;
	*tm = tevent;
	return errors;
}

/*
 * insert|append global g prereqs to r
 */

static void
globalprereqs(register struct rule* r, struct rule* g)
{
	register struct list*	p;
	register struct list*	q;
	register struct list*	t;
	register struct list*	u;
	struct rule*		x;
	struct list*		pos = 0;

	while (x = associate(g, r, NiL, &pos))
		if (p = x->prereqs)
		{
			for (q = t = 0; p; p = p->next)
				if (p->rule != r)
				{
					newlist(u);
					if (t)
						t->next = u;
					else
						q = u;
					(t = u)->rule = p->rule;
				}
			if (q)
			{
				t->next = 0;
				r->prereqs = g == internal.insert_p ? append(q, r->prereqs) : append(r->prereqs, q);
				if (x->dynamic & D_dynamic)
					dynamic(r);
				remdup(r->prereqs);
			}
		}
}

/*
 * undo alias of a -> r in preparation for update()
 * return unaliased a
 */

static struct rule*
unalias(register struct rule* r, register struct rule* a, char* name)
{
	struct frame*	oframe;

	message((-3, "unalias(%s) -> %s", a->name, name));
	if (a == r)
	{
		a->uname = name;
		a->dynamic |= D_alias;
	}
	else
	{
		oframe = a->active;
		a->active = r->active;
		r->active = r->active->previous;
		a->active->previous = oframe;
		a->status = r->status;
		r->status = NOTYET;
		a->action = r->action;
		if (r == state.frame->target)
			state.frame->target = a;
	}
	if (r->view && a->active && !state.mam.statix)
		a->active->original = a->name;
	oldname(a);
	return a;
}

/*
 * prepare for update and execute action
 * r is the target (possibly with explicit action)
 * a is the implicit action rule
 * arg for P_functional
 */

static int
update(register struct rule* r, register struct rule* a, char* arg)
{
	register struct list*	p;
	register struct rule*	u;
	int			errors;
	char*			s;

	errors = 0;
	if (state.override)
	{
		/*
		 * save explicit target generation for non-override runs
		 */

		if (r == a && r->time && (!(r->property & P_target) || !(r->property & (P_archive|P_command|P_make))))
		{
			r->time = CURTIME;
			r->dynamic |= D_triggered;
			if (r->dynamic & D_member)
			{
				r->dynamic &= ~D_member;
				r->dynamic |= D_membertoo;
			}
			return errors;
		}
	}
	for (p = joint(r); p; p = p->next)
	{
		u = p->rule;
		if (u->dynamic & D_alias)
			u = makerule(u->name);
		if (u->dynamic & D_member)
		{
			u->dynamic &= ~D_member;
			u->dynamic |= D_membertoo;
		}
		if (!state.override && u->action)
			a = u;
		if (!(u->property & P_state))
		{
			if (u->uname)
			{
				if (u->view)
				{
					u->view = 0;
					if (u->active && !state.mam.statix)
						u->active->original = u->name;
				}
				oldname(u);
			}
			else if (u->view)
			{
				if (r == a)
					u->view = 0;
				else if (state.fsview)
				{
					u->view = 0;
					if (u->active && !state.mam.statix)
					{
						if (state.expandview)
						{
							mount(u->name, sfstrbase(internal.tmp), FS3D_GET|FS3D_VIEW|FS3D_SIZE(sfstrsize(internal.tmp)), NiL);
							u->active->original = makerule(sfstrbase(internal.tmp))->name;
						}
						else
							u->active->original = u->name;
					}
				}
				else
				{
					int	n;

					/*
					 * this condition is definitely a bug
					 * this fix will have to do until the
					 * place that set u->uname=0 is found
					 */

					n = 0;
					s = u->name;
					while (s = strchr(s, '/'))
					{
						n++;
						if (getrule(++s) == u)
						{
							message((-1, "local binding %s recovered from %s", s, u->name));
							u->view = 0;
							if (u->active && !state.mam.statix)
								u->active->original = u->name;
							u->name = putrule(s, u);
							break;
						}
					}
					if (!s)
					{
						if (!n)
							u->view = 0;
						else if (!(u->property & P_dontcare))
							error(1, "%s: modifying lower view=%d file", u->name, u->view);
					}
				}
			}
		}
	}
	if ((r->property & P_attribute) || (r->property & P_functional) && !(r->active->stem = arg))
	{
		if (!state.accept && !(r->property & P_accept))
			r->dynamic |= D_triggered;
		statetime(r, 1);
	}
	else
	{
		r->action = a->action;
		trigger(r, a, a->action, CO_LOCALSTACK);
	}
	return errors;
}

/*
 * top level make
 * p temporarily or'd into r->property
 * arg for P_functional
 *
 * r->status determines the status of each rule:
 *
 *	NOTYET	nothing done yet
 *	UPDATE	rule in process of being updated
 *	MAKING	rule action being executed
 *	TOUCH	archive member to be touched
 *	EXISTS	rule has already been made
 *	IGNORE	rule make failed but ignore errors
 *	FAILED	rule make failed
 */

void
maketop(register struct rule* r, int p, char* arg)
{
	unsigned long	t;
	unsigned long	o;

	if ((p & (P_force|P_repeat)) == (P_force|P_repeat) && (r->property & (P_functional|P_make)) == P_make)
	{
		register struct rule*	a;

		a = catrule(internal.internal->name, ".%%", r->name, 1);
		a->property |= P_internal|P_virtual;
		if (a->prereqs)
			freelist(a->prereqs);
		a->prereqs = cons(r, NiL);
		r = a;
	}
	o = r->property & p;
	r->property |= p;
	if (p & P_ignore)
		state.keepgoing |= 2;
	make(r, &t, arg, 0);
	complete(r, r->prereqs, NiL, 0);
	state.keepgoing &= 1;
	r->property &= ~p;
	r->property |= o;
}

/*
 * intermediate level make
 */

int
make(register struct rule* r, unsigned long* ttarget, char* arg, long flags)
{
	register struct list*	p;
	register struct rule*	r1;
	unsigned long		t;
	unsigned long		tevent;
	unsigned long		otime;
	char*			s;
	char*			v;
	char*			r3name;
	int			errors;
	int			explicit;
	int			must;
	int			otargetview;
	int			pop;
	struct rule*		r0;
	struct rule*		r2;
	struct rule*		r3;
	struct rule*		r4;
	struct rule*		parent;
	struct list*		q;
	struct frame*		fp;
	struct frame*		oframe;
	struct frame		frame;
	Sfio_t*			mam;
	char			stem[MAXNAME];

	trap();
	errors = 0;
	*ttarget = 0;
	if (r == internal.query)
	{
		interpreter(NiL);
		return errors;
	}
	if (r->property & P_use)
	{
		/*
		 * r->use rules modify the parent
		 */

		r1 = state.frame->target;
		if (!(r->property & P_metarule) && r1->action && !state.override)
			return errors;
		if (r->status == NOTYET)
		{
			/*
			 * check if the action changed
			 */

			r0 = staterule(RULE, r, NiL, 1);
			if (r->action && (!r0->action || !streq(r->action, r0->action)) || !r->action && r0->action)
			{
				reason((1, "%s action changed [#1]", r->name));
				r0->action = r->action;
				r0->time = ((r1->property & P_accept) || state.accept) ? OLDTIME : CURTIME;
				state.savestate = 1;
			}
			else if ((r1->property & P_accept) || state.accept)
				r0->time = OLDTIME;
			r->time = r0->time;
			r->status = EXISTS;
		}
#if DEBUG
		else if (r->status != EXISTS)
			error(PANIC, "%s rule %s has invalid status=%d", internal.use->name, r->name, r->status);
#endif
		tevent = ((r->property & P_metarule) && (r1->property & P_implicit) && r1->action) ? 0 : r->time;

		/*
		 * append the prerequisites
		 */

		q = listcopy(r->prereqs);
		if (r->dynamic & D_dynamic)
		{
			fp = r->active;
			r->active = state.frame;
			dynamic(r);
			r->active = fp;
			r->dynamic |= D_dynamic;
		}
		p = r->prereqs;
		r->prereqs = q;
		r1->prereqs = append(r1->prereqs, p);
		if (r->property & P_metarule)
			for (; p; p = p->next)
			{
				r2 = p->rule;
				if (r2->dynamic & D_scope)
					r1->dynamic |= D_hasscope;
				else if ((r2->property & (P_make|P_local|P_use)) == (P_make|P_local))
				{
					r2->property |= P_virtual;
					r1->dynamic |= D_hasscope;
					errors += make(r2, &t, NiL, 0);
					if (tevent < t)
						tevent = t;
				}
				else if (r2->property & P_after)
				{
					r1->dynamic |= D_hasafter;
					if ((r2->property & (P_make|P_foreground)) == (P_make|P_foreground))
						r1->dynamic |= D_hasmake;
				}
				else if (r2->property & P_before)
					r1->dynamic |= D_hasbefore;
				else if (r2->semaphore)
					r1->dynamic |= D_hassemaphore;
				else if ((r2->property & (P_joint|P_target)) != P_joint)
				{
					errors += make(r2, &t, NiL, 0);
					if (tevent < t)
						tevent = t;
				}
			}

		/*
		 * propagate the attributes
		 */

		merge(r, r1, MERGE_ATTR);
		if (!(r->property & P_metarule))
			r1->action = r->action;
		if ((r->property & P_dontcare) && !state.unwind)
			state.unwind = error_info.indent;
		if (!(r->property & P_ignore))
		{
			*ttarget = tevent;
			PREVIEW(r1, r);
		}
		return errors;
	}
	else if (r->semaphore)
		return complete(r, NiL, ttarget, 0);

	/*
	 * check if rule has been or is in the process of being made
	 */

	message((-1, "make(%s%s%s)", r->name, arg ? " " : null, arg ? arg : null));
	error_info.indent++;
	r3 = r;
	r3name = unbound(r3);
	zero(frame);
	frame.target = r;
	frame.parent = state.frame;
	parent = frame.parent->target;
	frame.previous = r->active;
	r->active = &frame;
	for (;;)
	{
		if (r->status != NOTYET)
		{
			if (!(r->property & P_repeat))
			{
				tevent = r->time;
				r0 = 0;
				if (!(r->property & P_state))
				{
					if (mam = mamout(r))
						pop = mampush(mam, r, flags);
					if (r->scan && !(r->dynamic & D_scanned) && state.scan)
					{
						if (r->status == MAKING)
							complete(r, NiL, NiL, 0);
						if (r->status == EXISTS)
						{
							otime = r->time;
							r0 = staterule(RULE, r, NiL, 1);
							if (r0->time)
								r->time = r0->time;
							makescan(r, &t);
							if (tevent < t)
								r->time = r0->event = tevent = t;
							else
								r->time = otime;
						}
					}
					if (mam && pop)
					{
						if (state.mam.statix)
							r->dynamic |= D_built;
						mampop(mam, r, P_virtual);
					}
				}
				if (!(r->property & P_ignore))
				{
					if (r->dynamic & D_aliaschanged)
						tevent = CURTIME;
					else if (!(r->dynamic & D_triggered) && parent->scan == SCAN_IGNORE && (r0 || (r0 = staterule(RULE, r, NiL, 0))) && tevent < r0->time)
						tevent = r0->time;
					*ttarget = tevent;
					PREVIEW(parent, r);
				}
				message((-1, "time(%s) = %s", r->name, strtime(tevent)));
				r->active = frame.previous;
				if (state.unwind == error_info.indent)
				{
					state.unwind = 0;
					errors = 0;
				}
				error_info.indent--;
				return r->status == FAILED;
			}
			if (r->status == MAKING)
				complete(r, NiL, NiL, 0);
		}
		if ((r->property & P_dontcare) && !state.unwind)
			state.unwind = error_info.indent;
		if ((r1 = bind(r)) == r)
			break;
		if ((r->property & P_target) && !(r1->property & P_target))
		{
			r->dynamic &= ~D_alias;
			break;
		}
		fp = r1->active;
		r1->active = r->active;
		r->active = r->active->previous;
		r = r1;
		r->active->target = r;
		r->active->previous = fp;
	}
	if ((r3 == r || (r->property & P_target) && !(r3->property & P_target)) && (!(r->dynamic & D_alias) || r3name == unbound(r)))
		r3 = 0;
	otargetview = state.targetview;
	if (state.maxview && !(r->property & P_state) && !r->scan && r->view <= state.maxview)
		state.targetview = r->view;
	frame.action = r->action;
	frame.prereqs = r->prereqs;
	if ((r->property & (P_joint|P_target)) == (P_joint|P_target))
		for (p = r->prereqs->rule->prereqs; p; p = p->next)
			if ((r1 = p->rule) != r)
			{
				bind(r1);
				if (r1->status == NOTYET)
				{
					r0 = staterule(RULE, r1, NiL, 1);
					if (!timeq(r1->time, r0->time) || !r0->event)
					{
						reason((1, "%s joint sibling %s is out of date", r->name, r1->name));
						staterule(RULE, r, NiL, 1)->time = 0;
					}
				}
				fp = newof(0, struct frame, 1, 0);
				fp->target = r1;
				fp->parent = state.frame;
				fp->previous = r1->active;
				fp->action = r1->action;
				fp->prereqs = r1->prereqs;
				r1->active = fp;
			}
	oframe = state.frame;
	state.frame = &frame;
	if (r->dynamic & D_dynamic)
		dynamic(r);
	otime = r->time;
	if (r->property & P_state)
		while (r->active->prereqs && r->active->prereqs->next)
			r->active->prereqs = r->active->prereqs->next;
	r->dynamic &= ~(D_hasafter|D_hasbefore|D_hasmake|D_hasscope|D_hassemaphore|D_triggered);
	r->status = UPDATE;
	message((-1, "time(%s) = %s", r->name, strtime(r->time)));
	if (mam = mamout(r))
	{
		pop = mampush(mam, r, flags);
		if (state.mam.statix)
			r->dynamic |= D_built;
	}

	/*
	 * check for file changes since the last make
	 * the changes take a few forms:
	 *
	 *	o  the file modify time has changed since the
	 *	   last make
	 *
	 *	o  the file is from a different directory than
	 *	   the last make -- this is checked by using
	 *	   the modify time of the file which may fail
	 *	   (low probability) if the two source files
	 *	   in question have both the same (base) name
	 *	   and the same modify time -- this avoids
	 *	   saving the full bound names in the state file
	 *
	 * notice that this technique detects when an old source
	 * file is reinstated and forces updating just as if
	 * the file had been touched
	 */

	must = 0;
	tevent = 0;
	if (r->property & P_state)
		r0 = 0;
	else
	{
		r0 = staterule(RULE, r, NiL, 1);
		if (!(r->property & P_virtual))
		{
			if (!state.intermediate && !r->time && r0->time && r0->event && (r0->dynamic & D_built) && !(parent->property & P_state) && (r2 = staterule(RULE, parent, NiL, 0)) && (r2->dynamic & D_built))
			{
				message((-1, "pretending intermediate target %s exists", r->name));
				must = 1;
				otime = r->time = r0->time;
				r->dynamic |= D_intermediate;
				r->must++;
			}
			else if (!timeq(r->time, r0->time) || !r0->event || (r->dynamic & D_aliaschanged))
			{
				if (!(r->property & P_accept) && !state.accept)
				{
					if (r->dynamic & D_aliaschanged)
						reason((1, "%s [%s] binds to a different file", r->name, strtime(r->time)));
					else
					{
						if (r->time && r->time < r0->time && (r->dynamic & D_regular)) error(1, "%s has been replaced by an older version", unbound(r));
						if (r0->event && r0->time)
							reason((1, "%s [%s] has changed [%s]", r->name, strtime(r->time), strtime(r0->time)));
						else if (r0->event && r->view)
							reason((1, "%s [%s] has changed in view %s", r->name, strtime(r->time), state.view[r->view].path));
						else
							reason((1, "%s [%s] has no previous state", r->name, strtime(r->time)));
					}
					must = 1;
					r0->event = CURTIME;
					r->must++;
					state.savestate = 1;
				}
				if ((r->dynamic & (D_alias|D_bound)) == (D_alias|D_bound))
				{
					s = r->uname;
					r->uname = 0;
					r1 = staterule(RULE, r, NiL, 0);
					r->uname = s;
					if (r1 && r1 != r0 && r1->time)
					{
						r0->time = r1->time;
						r0->event = r1->event;
					}
					else
					{
						r0->time = r->time;
						r0->property |= P_force;
					}
				}
				else
				{
					r0->time = r->time;
					r0->property |= P_force;
				}
			}
			if ((r->property & P_accept) || state.accept)
			{
				if (r->time < r0->event && (r1 = staterule(PREREQS, r, NiL, 0)) && r1->time >= r0->event)
					r->time = r1->time;
				else
					r0->event = r->time ? r->time : OLDTIME;
				state.savestate = 1;
			}
			else
				r->time = r0->event;
		}
	}

	/*
	 * check global insert|append pattern prerequisites
	 */

	globalprereqs(r, internal.insert_p);
	globalprereqs(r, internal.append_p);

	/*
	 * check explicit prerequisites
	 */

	message((-2, "check explicit prerequisites"));
	explicit = 0;
	p = r->prereqs;
	while (p)
	{
		r1 = p->rule;
		if (r1->mark & M_bind)
			r1->mark &= ~M_bind;
		else if (r1->dynamic & D_scope)
			r->dynamic |= D_hasscope;
		else if ((r1->property & (P_make|P_local|P_use)) == (P_make|P_local))
		{
			r1->property |= P_virtual;
			r->dynamic |= D_hasscope;
			errors += make(r1, &t, NiL, 0);
			if (tevent < t)
				tevent = t;
		}
		else if (r1->property & P_after)
		{
			r->dynamic |= D_hasafter;
			if ((r1->property & (P_make|P_foreground)) == (P_make|P_foreground))
				r->dynamic |= D_hasmake;
		}
		else if (r1->property & P_before)
			r->dynamic |= D_hasbefore;
		else if (r1->semaphore)
			r->dynamic |= D_hassemaphore;
		else if ((r1->property & (P_joint|P_target)) != P_joint)
		{
			explicit = 1;
			errors += make(r1, &t, NiL, 0);
			if (tevent < t)
				tevent = t;
			if (r->time < t || (r1->dynamic & D_same))
				r->must++;
			if ((r2 = associate(internal.require_p, r1, NiL, NiL)) && (v = call(r2, unbound(r1))))
			{
				if (r->prereqs == r->active->prereqs)
					for (p = r->prereqs = listcopy(r->prereqs); p->rule != r1; p = p->next);
				q = 0;
				while (s = getarg(&v, NiL))
				{
					if ((r2 = makerule(s)) == r1)
						r1->mark |= M_bind;
					if (streq(r2->name, "-"))
						r1->property |= P_virtual;
					else if (q)
						q = q->next = cons(r2, q->next);
					else
						(q = p)->rule = r2;
				}
				if (q && !(state.questionable & 0x00020000))
				{
					int	u = 0;
					int	n = 0;

					/*
					 * mutually dependent requirements can
					 * get us into a loop -- this limits
					 * the total number to half the square
					 * of the number of unique elements
					 */

					for (q = r->prereqs; q; q = q->next)
					{
						n++;
						if (!(q->rule->mark & M_mark))
						{
							q->rule->mark |= M_mark;
							u++;
						}
					}
					for (q = r->prereqs; q; q = q->next)
						q->rule->mark &= ~M_mark;
					if (n < (u * u) / 2)
						continue;
				}
			}
		}
		p = p->next;
	}

	/*
	 * check metarule prerequisites
	 */

	if (!errors && !(r->property & (P_attribute|P_functional|P_operator|P_state|P_virtual)) && ((r->property & P_implicit) || !r->action && !explicit))
	{
		message((-2, "check metarule prerequisites"));
#if DEBUG
		if (!r->active)
		{
			dumprule(sfstderr, r);
			error(PANIC, "%s: active=0", r->name);
		}
#endif
		if ((r2 = metaget(r, r->active->prereqs, stem, &r4)) && !(state.questionable & 0x00100000) && ((state.questionable & 0x00200000) || !(r->property & P_implicit)) && strchr(unbound(r), '/') && !strchr(r4->name, '/'))
			r2 = 0;
		if (r2)
		{
			r1 = r4;
			if (state.mam.out && !mam)
			{
				mam = state.mam.out;
				pop = mampush(mam, r, flags);
			}
#if DEBUG
			message((-9, "metarule=%s source=%s stem=%s", r4->name, r2->name, stem));
#endif
			frame.stem = stem;

			/*
			 * primary metarule match
			 */

			if ((r1->property & P_accept) && r->must == 1 && r0 && r0->time)
			{
				tevent = 0;
				r->must = 0;
				must = 0;
			}
			else if (!r0 || !r0->time)
			{
				tevent = CURTIME;
				r->must++;
				must = 1;
			}

			/*
			 * check the implicit source prerequisite
			 */

			if (!(errors += make(r2, &t, NiL, 0)))
			{
				if (tevent < t)
					tevent = t;
				if (r->time < t || (r2->dynamic & D_same))
					r->must++;

				/*
				 * check joint metarule targets
				 */

				if (r4 = metainfo('S', r1->name, NiL, 0))
				{
					struct rule*	joint;
					struct rule*	x;
					Sfio_t*		tmp;

					message((-2, "check joint metarule targets"));
					tmp = sfstropen();
					sfprintf(tmp, "%s.%s", internal.joint->name, unbound(r2));
					joint = makerule(sfstruse(tmp));
					joint->property |= P_joint|P_readonly|P_virtual;
					for (p = r4->prereqs; p; p = p->next)
					{
						metaexpand(tmp, stem, p->rule->name);
						s = sfstruse(tmp);
						x = makerule(s);
						if (x->property & P_joint)
							x->prereqs->rule = joint;
						else
						{
							x->property |= P_joint|P_target;
							x->prereqs = cons(joint, x->prereqs);
						}
						joint->prereqs = cons(x, joint->prereqs);
						if (x != r)
						{
							message((-1, "make(%s)", s));
							error_info.indent++;

							/*
							 * NOTE: some joint targets may not be generated
							 */

							if (!(r4 = bindfile(NiL, s, 0)) || !state.accept && (((t = r4->time), !(r4 = staterule(RULE, r4, NiL, 0))) || (r4->dynamic |= D_built, r4->time != t) || !t && !r4->event))
							{
								tevent = CURTIME;
								reason((1, "joint metarule target %s changed", s));
							}
							x->status = r->status;
							message((-1, "time(%s) = %s", s, strtime(t)));
							if (state.unwind == error_info.indent)
							{
								state.unwind = 0;
								errors = 0;
							}
							error_info.indent--;
							if (!x->active || x->active->parent != oframe)
							{
								fp = newof(0, struct frame, 1, 0);
								fp->target = x;
								fp->parent = oframe;
								fp->previous = x->active;
								fp->action = x->action;
								fp->prereqs = x->prereqs;
								x->active = fp;
							}
						}
					}
					sfstrclose(tmp);
				}

				/*
				 * check the metarule
				 */

				errors += make(r1, &t, NiL, 0);
				if (!errors)
				{
					/*UNDENT*/
	if (tevent < t)
		tevent = t;
	if (r->time < t || (r1->dynamic & D_same))
		r->must++;

	/*
	 * check the explicit action
	 */

	if ((r->property & P_implicit) && r->action && r->action == r->active->action && (!r0->action || !streq(r0->action, r->action)))
	{
		reason((1, "%s action changed [#2]", r->name));
		r0->action = r->action;
		state.savestate = 1;
		if (!(r->property & P_accept) && !state.accept)
		{
			tevent = CURTIME;
			r->must++;
		}
	}
	if ((state.questionable & 0x00000010) && r->view > r->preview && !(r->property & P_accept) && (!(r4 = staterule(PREREQS, r, NiL, 0)) || !r4->time))
	{
		reason((1, "%s view %d must be in view %d", r->name, r->view, r->preview));
		tevent = CURTIME;
		r->must++;
	}

	/*
	 * check for update
	 */

	timefix(tevent);
	if (must || r->time < tevent && (!(r4 = staterule(PREREQS, r, NiL, 0)) || r4->time < tevent) || (r->property & P_force) || prereqchange(r, r->prereqs, r0, r0->prereqs) || state.force)
	{
		if (state.touch)
			r->time = CURTIME;

		/*
		 * trigger the action
		 */

		if (r3)
		{
			r = unalias(r, r3, r3name);
			if (r0)
				r0 = staterule(RULE, r, NiL, 1);
		}
		r->active->primary = r2->name;
		errors += update(r, r1, arg);
	}
					/*INDENT*/
				}
			}
		}
		else if (!errors && (r->property & P_implicit) && !state.override)
		{
			errors++;
			r->status = FAILED;
			parentage(internal.tmp, r, " : ");
			error(state.keepgoing || state.unwind ? 1 : 3, "can't find source for %s", sfstruse(internal.tmp));
			state.errors++;
		}
	}
	else
		r2 = 0;

	/*
	 * determine the update rule if no metarule applied
	 */

	if (r2)
	{
		r1 = 0;
		r0->dynamic |= D_built;
	}
	else if (r->action)
	{
		r1 = r;
		if (r0)
		{
			if (r->action == r->active->action && (!r0->action || !streq(r0->action, r->action)))
			{
				reason((1, "%s action changed [#3]", r->name));
				r0->action = r->action;
				state.savestate = 1;
				if (!(r->property & P_accept) && !state.accept)
				{
					tevent = CURTIME;
					r->must++;
				}
			}
			r0->dynamic |= D_built;
		}
	}
	else
	{
		r1 = 0;
		if (r0)
		{
			if (r0->action)
			{
				reason((1, "%s action changed [#4]", r->name));
				r0->action = 0;
				state.savestate = 1;
			}

			/*
			 * at this point we accept the target as terminal source
			 */

			r0->dynamic &= ~D_built;

			/*
			 * ... unless it doesn't exist
			 */

			if (!r0->time)
				r->time = 0;
		}
	}
	if (r1)
	{
		if ((state.questionable & 0x00000010) && r->view > r->preview && !(r->property & P_accept) && (!(r4 = staterule(PREREQS, r, NiL, 0)) || !r4->time))
		{
			reason((1, "%s view %d must be in view %d", r->name, r->view, r->preview));
			must = 1;
			tevent = CURTIME;
			r->must++;
		}
		else if ((r->property & (P_attribute|P_functional|P_target)) == P_target && (state.force || r0 && !r0->time))
		{
			/*
			 * this takes care of non-file targets
			 */

			must = 1;
			tevent = CURTIME;
			r->must++;
		}
	}

	/*
	 * do the explicit action
	 */

	timefix(tevent);
	message((-2, "[%s] : [%s]%s%s%s", strtime(r->time), strtime(tevent), errors ? " ERRORS" : null, errors && state.unwind >= error_info.indent ? " ignored" : null, must ? " must" : null));
	if (errors && !(state.questionable & 0x00800000))
		r->status = FAILED;
	else
	{
		if (!errors && !(r->dynamic & D_triggered) && r->status == UPDATE && (r1 && must || r->time < tevent && (!(r4 = staterule(PREREQS, r, NiL, 0)) || r4->time < tevent) || !r->time || !r2 && ((r->property & P_force) || r0 && (r->prereqs || r->action) && prereqchange(r, r->prereqs, r0, r0->prereqs))))
		{
			if (r1)
			{
				if (r3)
				{
					r = unalias(r, r3, r3name);
					if (r0)
						r0 = staterule(RULE, r, NiL, 1);
				}
				errors += update(r, r1, arg);
			}
			else if (r->property & P_dontcare)
			{
				statetime(r, 0);
				tevent = 0;
			}
			else if (!(r->property & (P_state|P_virtual)))
			{
				if (!(r->property & (P_target|P_terminal)) || r2 || (r->property & P_terminal) && !r->time)
				{
					if (r->status == UPDATE)
					{
						/*
						 * the attribute test handles rules in
						 * make object files that have since
						 * become attributes, e.g., .READONLY
						 */

						if ((r->property & P_attribute) || (r1 = associate(internal.dontcare_p, r, NiL, NiL)) && call(r1, r->name))
						{
							r->status = IGNORE;
							statetime(r, 0);
							tevent = 0;
						}
						else
						{
							errors++;
							r->status = FAILED;
							parentage(internal.tmp, r, " : ");
							error(state.keepgoing || state.unwind ? 1 : 3, "don't know how to make %s", sfstruse(internal.tmp));
							state.errors++;
						}
					}
				}
				else if (state.exec || state.mam.statix)
				{
					statetime(r, 0);
					if (!(r->property & P_terminal))
						tevent = 0;
				}
			}
		}
		if (!(r->dynamic & D_triggered))
			trigger(r, NiL, NiL, 0);
	}
	if (r->property & P_statevar)
	{
		if (state.targetview >= 0)
		{
			if (!tstbit(r->checked[RULE], state.targetview))
				staterule(RULE, r, NiL, 1);
			if (!tstbit(r->checked[CONSISTENT], state.targetview))
			{
				reason((1, "%s inconsistent with view %s", r->name, state.view[state.targetview].path));
				tevent = CURTIME;
			}
		}
		if (!state.accept && !(r->property & P_accept) && r->time < tevent)
			r->time = tevent;
	}
	else if (!(r->property & P_state))
	{
		if (!(r->property & P_archive) && r->scan && state.scan)
		{
			if (r->status == MAKING)
				complete(r, NiL, NiL, 0);
			if (r->status == UPDATE || r->status == EXISTS)
			{
				tevent = r->time;
				errors += makescan(r, &t);
				if (tevent < t)
					tevent = t;
			}
		}
		else
		{
			if (r->property & P_parameter)
				tevent = OLDTIME;
			if (r0 && ((r0->dynamic & D_built) || !r0->scan))
			{
				if (r0->prereqs != r->prereqs)
				{
#if _HUH_1992_09_30 /* this test is insufficient */
					if ((r->property & (P_joint|P_target)) != (P_joint|P_target))
						freelist(r0->prereqs);
#endif
					r0->prereqs = r->prereqs;
				}
				r0->attribute = r->attribute;
				r0->scan = 0;
			}
		}
		if (r0 && (r->property & (P_joint|P_target)) == (P_joint|P_target))
			for (p = r->prereqs->rule->prereqs; p; p = p->next)
				if (p->rule != r)
				{
					r1 = staterule(RULE, p->rule, NiL, 1);
					r1->prereqs = r0->prereqs;
					r1->attribute = r0->attribute;
					r1->scan = r0->scan;
					r1->action = r0->action;
				}
		if (r->time < tevent || (r->property & (P_attribute|P_parameter)))
		{
			r->time = tevent;
			if (!r0)
				r0 = staterule(RULE, r, NiL, 1);
			r0->event = tevent;
		}
	}
	if (r->status == FAILED)
		errors++;
	if (r->property & P_state)
	{
		if (r->time != otime)
			state.savestate = 1;
	}
	else if (state.force && (!(r->property & P_dontcare) || r->action || r2))
		r->time = CURTIME;
	if (r0 && (r->dynamic & D_triggered) && (r->property & P_make))
	{
		r0->time = r0->event = r->time;
		state.savestate = 1;
	}

	/*
	 * restore and return
	 */

	for (p = joint(r); p; p = p->next)
	{
		r1 = p->rule;
		if (fp = r1->active)
		{
			if ((r1->property & P_state) && fp->prereqs)
				fp->prereqs->next = 0;
			r1->action = fp->action;
			r1->active = fp->previous;
			if (fp != &frame)
				free(fp);
		}
		if (r1 != r)
		{
			r1->time = r->time;
			r1->status = r->status;
		}
	}
	state.targetview = otargetview;
	state.frame = oframe;
	if (state.unwind == error_info.indent)
	{
		state.unwind = 0;
		errors = 0;
	}
	error_info.indent--;
	if (!(r->property & P_ignore))
	{
		t = r->time;
		if (parent->scan == SCAN_IGNORE)
		{
			if (!(r->dynamic & D_triggered) && !r->must)
				t = r0 ? r0->time : otime;
		}
		else if ((parent->property & P_archive) && (r->dynamic & D_regular) && (parent->dynamic & D_entries) && !(r->dynamic & D_member) && !(r->property & (P_archive|P_command|P_dontcare|P_ignore|P_state|P_virtual)))
			t = CURTIME;
		*ttarget = t;
		PREVIEW(parent, r);
	}
	message((-1, "time(%s) = %s", r->name, strtime(*ttarget)));
	if (mam && pop)
		mampop(mam, r, P_virtual);
	return errors;
}

/*
 * make explicit before prerequisites if r's action will trigger
 */

int
makebefore(register struct rule* r)
{
	register struct list*	p;
	register int		errors;
	unsigned long		t;

	errors = 0;
	if ((r->dynamic & (D_hasbefore|D_triggered)) == (D_hasbefore|D_triggered))
	{
		r->dynamic &= ~D_hasbefore;
		message((-2, "check explicit before `prerequisites'"));
		for (p = r->prereqs; p; p = p->next)
			if (p->rule->property & P_before)
				errors += make(p->rule, &t, NiL, 0);
	}
	return errors;
}

/*
 * make explicit after prerequisites if r's action triggered
 */

int
makeafter(register struct rule* r)
{
	register struct list*	p;
	register int		errors;
	unsigned long		t;

	errors = 0;
	if ((r->dynamic & (D_hasafter|D_triggered)) == (D_hasafter|D_triggered))
	{
		statetime(r, -1);
		r->dynamic &= ~(D_hasafter|D_hasmake);
		message((-2, "check explicit after `prerequisites'"));
		for (p = r->prereqs; p; p = p->next)
			if (p->rule->property & P_after)
				errors += make(p->rule, &t, NiL, 0);
	}
	return errors;
}
