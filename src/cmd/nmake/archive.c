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

#define arpointer(x)	((x)>=0&&(x)<elementsof(artab)?artab[x]:0)

static Ardir_t*		artab[2];	/* open archive table		*/

static int		ntouched;	/* count of touched members	*/

#define UPDATE_rand	"$(RANLIB) $(<)"

/*
 * open an archive and determine its type
 */

int
openar(char* name, char* mode)
{
	int	arfd;

	for (arfd = 0; arfd < elementsof(artab); arfd++)
		if (!artab[arfd])
			break;
	if (arfd >= elementsof(artab))
		return -1;
	if (!(artab[arfd] = ardiropen(name, NiL, ARDIR_LOCAL|(strchr(mode, '+') ? ARDIR_UPDATE : 0))))
		return -1;
	internal.arupdate = streq(artab[arfd]->meth->name, "local") ? "($(RANLIB|\":\") $(<)) >/dev/null 2>&1 || true" : (artab[arfd]->flags & ARDIR_RANLIB) ? "$(RANLIB) $(<)" : (char*)0;
	return arfd;
}

/*
 * close an archive opened by openar()
 */

int
closear(int arfd)
{
	register Ardir_t*	ar;

	if (!(ar = arpointer(arfd)))
		return -1;
	artab[arfd] = 0;
	return ardirclose(ar);
}

/*
 * walk through an archive
 * d==0 updates the modify time of preselected members (see touchar())
 * else each member is recorded using addfile()
 */

int
walkar(struct dir* d, int arfd, char* name)
{
	register Ardir_t*	ar;
	register Ardirent_t*	ent;
	register struct rule*	r;

	if (!(ar = arpointer(arfd)))
		return 0;
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
			{
				error(1, "member %s is newer than archive %s", ent->name, name);
				ent->mtime = ar->st.st_mtime;
			}
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
touchar(char* name, register char* member)
{
	register struct rule*	r;
	int			arfd;

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
	else if ((arfd = openar(name, "br+")) >= 0)
	{
		walkar(NiL, arfd, name);
		if (closear(arfd))
			error(1, "error touching archive %s", name);
		if (ntouched > 0)
		{
			message((-2, "checking %d untouched members in %s", ntouched, name));
			hashwalk(table.rule, 0, chktouch, NiL);
		}
		ntouched = 0;
	}
}
