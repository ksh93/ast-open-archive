/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1984-2003 AT&T Corp.                *
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
 * make archive access routines
 */

#include "make.h"

#include <ardir.h>

static int		ntouched;	/* count of touched members	*/

/*
 * return the update command for the named archive
 */

char*
arupdate(char* name)
{
	Ardir_t*	ar;
	char*		update;

	if (!(ar = ardiropen(name, NiL, ARDIR_LOCAL)))
		return 0;
	update = streq(ar->meth->name, "local") ? "($(RANLIB|\":\") $(<)) >/dev/null 2>&1 || true" : (ar->flags & ARDIR_RANLIB) ? "$(RANLIB) $(<)" : (char*)0;
	ardirclose(ar);
	return update;
}

/*
 * walk through an archive
 * d==0 updates the modify time of preselected members (see artouch())
 * else each member is recorded using addfile()
 */

static int
walkar(register Ardir_t* ar, struct dir* d, char* name)
{
	register Ardirent_t*	ent;
	register struct rule*	r;

	if (d)
	{
		putar(d->name, d);
		d->truncate = ar->truncate;
	}
	while (ent = ardirnext(ar))
	{
		if (d)
		{
			if (ent->mtime > ar->st.st_mtime)
				message((-1, "member %s is newer than archive %s", ent->name, name));
			addfile(d, ent->name, ent->mtime);
		}
		else if ((r = getrule(ent->name)) && r->status == TOUCH)
		{
			ent->mtime = CURTIME;
			ardirchange(ar, ent);
			r->status = EXISTS;
			staterule(RULE, r, NiL, 1)->time = r->time = ent->mtime;
			state.savestate = 1;
			if (!state.silent)
				error(0, "touch %s/%s", name, ent->name);
			ntouched--;
		}
	}
	return 0;
}

/*
 * check for any untouched r->status==TOUCH members
 */

static int
chktouch(const char* s, char* v, void* h)
{
	struct rule*	r = (struct rule*)v;

	NoP(s);
	NoP(h);
	if (r->status == TOUCH)
	{
		r->status = FAILED;
		error(1, "archive member %s not touched", r->name);
	}
	return 0;
}

/*
 * touch the modify time of an archive member (and the archive itself!)
 */

void
artouch(char* name, register char* member)
{
	register struct rule*	r;
	Ardir_t*		ar;

	if (member)
	{
		if (!(r = getrule(member)))
			error(PANIC, "%s[%s] not scanned", name, member);
		else
		{
			r->status = TOUCH;
			ntouched++;
		}
	}
	else if (ar = ardiropen(name, NiL, ARDIR_LOCAL|ARDIR_UPDATE))
	{
		walkar(ar, NiL, name);
		if (ardirclose(ar))
			error(1, "error touching archive %s", name);
		if (ntouched > 0)
		{
			message((-2, "checking %d untouched members in %s", ntouched, name));
			hashwalk(table.rule, 0, chktouch, NiL);
		}
		ntouched = 0;
	}
}

/*
 * scan archive r and record all its entries
 */

void
arscan(struct rule* r)
{
	Ardir_t*	ar;
	struct dir*	d;

	if (r->dynamic & D_scanned)
		return;
	r->dynamic |= D_scanned;
	if (r->property & P_state)
		r->dynamic &= ~D_entries;
	else if (!(d = unique(r)))
		r->dynamic |= D_entries;
	else if (r->scan >= SCAN_USER)
	{
#if DEBUG
		message((-5, "scan aggregate %s", r->name));
#endif
		d->archive = 1;
		state.archive = d;
		scan(r, NiL);
		state.archive = 0;
		r->dynamic |= D_entries;
	}
	else if (ar = ardiropen(r->name, NiL, ARDIR_LOCAL))
	{
#if DEBUG
		message((-5, "scan archive %s", r->name));
#endif
		d->archive = 1;
		if (walkar(ar, d, r->name))
			r->dynamic |= D_entries;
		else
			r->dynamic &= ~D_entries;
		if (ardirclose(ar))
			error(1, "%s: archive scan error", r->name);
	}
#if DEBUG
	else
		message((-5, "arscan(%s) failed", r->name));
#endif
}
