/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1984-2000 AT&T Corp.              *
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
 * mam support routines
 */

#include "make.h"

/*
 * mam special pathcanon()
 */

char*
mamcanon(char* path)
{
	register char*	s;

	s = path + strlen(path);
	while (s > path + 1 && *(s - 1) == '.' && *(s - 2) == '/')
	{
		for (s -= 2; s > path + 1 && *(s - 1) == '/'; s--);
		*s = 0;
	}
	return s;
}

/*
 * output dynamic mam error message
 */

ssize_t
mamerror(int fd, const void* b, size_t n)
{
	register char*	s = (char*)b;
	register char*	e;
	char*		t;

	if (state.mam.level > 0)
	{
		if (e = strchr(s, ':'))
			for (s = e; *++s && *s == ' ';);
		switch (state.mam.level)
		{
		case ERROR_WARNING:
			t = "warning";
			break;
		case ERROR_PANIC:
			t = "panic";
			break;
		default:
			t = 0;
			break;
		}
		if (!t)
			t = "error";
		else if (e = strchr(s, ':'))
			for (s = e; *++s && *s == ' ';);
		sfprintf(state.mam.out, "%sinfo %s %s", state.mam.label, t, s);
		if (state.mam.regress)
			return 0;
	}
	return (fd == sffileno(sfstderr) && error_info.write == write) ? sfwrite(sfstderr, b, n) : write(fd, b, n);
}

/*
 * translate mam target name
 */

char*
mamname(register struct rule* r)
{
	char*	a;
	char*	s;

	if (r->property & P_state)
		return r->name;
	if (state.mam.dynamic && (r->dynamic & D_alias))
		r = makerule(r->name);
	a = (r->property & P_target) ? unbound(r) : (state.mam.regress || state.expandview) ? r->name : localview(r);
	if (state.mam.statix && (s = call(makerule(external.mamname), a)) && !streq(a, s))
		a = s;
	if (state.mam.root && (*a == '/' || (r->dynamic & (D_entries|D_member|D_membertoo|D_regular)) || access(r->name, R_OK)))
	{
		if (*a != '/')
			sfprintf(internal.nam, "%s/", internal.pwd);
		sfprintf(internal.nam, "%s", a);
		a = sfstruse(internal.nam);
		pathcanon(a, 0);
		if (!strncmp(a, state.mam.root, state.mam.rootlen) && *(a + state.mam.rootlen) == '/')
			a += state.mam.rootlen + 1;
	}
	if (state.mam.regress && *a == '/')
		a = strrchr(a, '/') + 1;
	return a;
}

/*
 * push make|prev op
 * non-zero returned if matching done required
 */

int
mampush(Sfio_t* sp, register struct rule* r, long flags)
{
	int	pop;

	if (strmatch(r->name, "${mam_*}"))
		return 0;
	pop = !(r->dynamic & D_built) || (flags & P_force);
	if (pop && (r->property & (P_joint|P_target)) == (P_joint|P_target) && r->prereqs->rule->prereqs->rule == r)
		mampush(sp, r->prereqs->rule, flags|P_joint|P_virtual);
	sfprintf(sp, "%s%s %s%s%s%s%s\n"
		, state.mam.label
		, pop ? "make" : "prev"
		, mamname(r)
		, (r->property & P_archive) ? " archive" : null
		, (flags & P_implicit) ? " implicit" : null
		, (flags & P_joint) ? " joint" : null
		, (r->property & P_state) ? " state" : null
		);
	return pop;
}

/*
 * pop done op
 */

void
mampop(Sfio_t* sp, register struct rule* r, long flags)
{
	struct rule*	s;
	struct list*	p;

	s = staterule(RULE, r, NiL, 0);
	sfprintf(sp, "%sdone %s%s%s%s\n"
		, state.mam.label
		, mamname(r)
		, (r->property & P_dontcare) ? " dontcare" : null
		, (flags & P_joint) ? " generated" : null
		, (flags & P_virtual) && !(r->property & P_state) && ((r->property & P_virtual) || !(r->dynamic & (D_entries|D_member|D_membertoo|D_regular)) && (!s || !s->time)) ? " virtual" : s && (s->dynamic & D_built) ? " generated" : null
		);
	if ((r->property & (P_joint|P_target)) == (P_joint|P_target) && r->prereqs->rule->prereqs->rule == r)
	{
		for (p = r->prereqs->rule->prereqs; p = p->next;)
			if (mampush(sp, p->rule, flags))
				mampop(sp, p->rule, flags|P_joint);
		mampop(sp, r->prereqs->rule, flags|P_virtual);
	}
}

/*
 * return mam output stream pointer for r
 */

Sfio_t*
mamout(register struct rule* r)
{
	register struct rule*	r0;
	register char*		s;

	if (!state.mam.out || !state.user || r == internal.empty)
		return 0;
	if (state.mam.regress)
		return (r->property & P_dontcare) && !state.mam.dontcare ? (Sfio_t*)0 : state.mam.out;
	if (r->property & (P_make|P_state|P_virtual))
		return 0;
	if (state.mam.dynamic)
		return state.mam.out;
	if ((r->property & (P_after|P_before)) || (r->property & P_dontcare) && !(state.mam.dontcare || r->prereqs || r->action && *r->action || (r0 = staterule(RULE, r, NiL, 0)) && (r0->dynamic & D_built) && r0->action && *r0->action || !(r->dynamic & D_global)))
		return 0;
	if (*(s = mamname(r)) == '/' || !r->time && *s != '$' && strchr(s, '/'))
		return 0;
	return state.mam.out;
}
