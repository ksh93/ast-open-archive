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
 * make state variable routines
 * this file and the is*() macros specify the state variable name format
 *
 *	(<var>)		the state of <var>
 *	(<var>)<rule>	the state of <var> qualified by <rule>
 *	()<rule>	the state of <rule>
 *
 * NOTE: update VERSION in compile.c if the format changes
 */

#include "make.h"
#include "options.h"

/*
 * return a pointer to the rule <s1><s2><s3>
 * force causes the rule to be created
 * force<0 canon's name first
 */

struct rule*
catrule(register char* s1, register char* s2, register char* s3, int force)
{
	struct rule*	r;

	sfputr(internal.nam, s1, *s2++);
	if (*s2) sfputr(internal.nam, s2, s3 ? *s3++ : -1);
	if (s3 && *s3) sfputr(internal.nam, s3, -1);
	s1 = sfstruse(internal.nam);
	if (!(r = getrule(s1)) && force)
	{
		if (force < 0)
		{
			pathcanon(s1, 0);
			if (r = getrule(s1))
				return r;
		}
		r = makerule(NiL);
	}
	return r;
}

/*
 * reconcile s with state view from r
 *
 * NOTE: requires state.maxview>0 && 0<=view<=state.maxview
 */

static struct rule*
stateview(int op, char* name, register struct rule* s, register struct rule* r, int view, int accept, struct rule** pv)
{
	register struct rule*	v;
	register struct list*	p;
	Sfio_t*			fp;

	if (pv) *pv = 0;
	if (state.compile < COMPILED)
	{
#if DEBUG
		if (state.test & 0x00000200) error(2, "STATEVIEW %d %s state file load delayed until after makefile read", view, name);
#endif
		return 0;
	}
	if (!(state.view[view].flags & BIND_LOADED))
	{
		if (view < state.readstate)
		{
			char*	file;
			Sfio_t*	tmp;
			long	n;

			tmp = sfstropen();
			if (name && !s)
			{
				sfputr(tmp, name, 0);
				n = sfstrtell(tmp);
			}
			else n = 0;
			edit(tmp, state.statefile, state.view[view].path, KEEP, KEEP);
			sfputc(tmp, 0);
			file = sfstrset(tmp, n);
			if (fp = sfopen(NiL, file, "r"))
			{
				/*
				 * NOTE: this load should not be a problem for
				 *	 internal rule pointers since all
				 *	 non-state rules in state files
				 *	 are just references
				 */

				state.stateview = view;
				message((-2, "loading state view %d file %s", view, file));
				if (load(fp, file, 0) > 0) state.view[view].flags |= BIND_EXISTS;
				else if (state.corrupt && *state.corrupt == 'a' && !(state.view[0].flags & BIND_EXISTS)) state.accept = 1;
				state.stateview = -1;
				sfclose(fp);
			}
			if (n) strcpy(name, sfstrbase(tmp));
			sfstrclose(tmp);
		}
		state.view[view].flags |= BIND_LOADED;
	}
	if (name)
	{
		viewname(name, view);
		v = getrule(name);
		unviewname(name);
#if DEBUG
		if (state.test & 0x00000200) error(2, "STATEVIEW %s [%s] test %d [%s] -> %s [%s]", name, s ? strtime(s->time) : "no rule", view, strtime(r->time), v ? v->name : null, v ? strtime(v->time) : "no rule");
#endif
		if (v)
		{
			if (pv) *pv = v;
			if (s && (op == RULE && (s->event >= v->event && s->event || !v->time && (v->property & P_force)) || op == PREREQS && s->time >= v->time)) return s;
			if (v->time == r->time || accept || r->view == view || (r->property & (P_state|P_use|P_virtual)) || state.believe && view >= (state.believe - 1))
			{
				if (r->property & P_state)
				{
					if (r->property & P_statevar)
					{
						if (r->statedata && (!v->statedata && *r->statedata || v->statedata && !streq(r->statedata, v->statedata) || r->time > v->time)) return 0;
						s = r;
						s->statedata = v->statedata;
						if (v->property & P_parameter) s->property |= P_parameter;
					}
				}
				else
				{
					if (r->property & P_use)
					{
						if (r->action && (!v->action || !streq(r->action, v->action)) || !r->action && v->action) return 0;
						r->time = v->time;
					}
					if (!s) s = makerule(name);
				}
				s->time = v->time;
				s->attribute = v->attribute;
				s->event = v->event;
				s->action = v->action;
				if (v->property & P_force) s->property |= P_force;
				else s->property &= ~P_force;
				if (v->dynamic & D_built) s->dynamic |= D_built;
				else s->dynamic &= ~D_built;
				s->prereqs = listcopy(v->prereqs);
				s->scan = v->scan;
				for (p = s->prereqs; p; p = p->next)
				{
					v = p->rule;
					if (v->dynamic & D_lower)
					{
						unviewname(v->name);
						p->rule = makerule(v->name);
						viewname(v->name, v->view);
					}
				}
#if DEBUG
				if (state.test & 0x00000200)
				{
					error(2, "STATEVIEW %s accept %d", s->name, view);
					if (state.test & 0x00000400) dumprule(sfstderr, s);
				}
#endif
			}
		}
	}
	return s;
}

/*
 * return a pointer to the state rule of var qualified by r
 * force causes the state rule to be created
 */

struct rule*
staterule(int op, register struct rule* r, char* var, int force)
{
	register struct rule*	s;
	register int		i;
	char*			rul;
	char*			nam;
	struct rule*		v;
	int			j;
	int			k;
	int			m;
	int			nobind = force < 0;
	viewvector*		b;

	switch (op)
	{
	case PREREQS:
		var = "+";
		break;
	case RULE:
		if (!r)
		{
			r = makerule(var);
			nobind = 1;
		}
		var = null;
		break;
	case VAR:
		if (!r)
		{
			sfprintf(internal.nam, "(%s)", var);
			if (!(r = getrule(sfstruse(internal.nam))))
			{
				if (!force)
					return 0;
				r = makerule(sfstrbase(internal.nam));
			}
		}
		if (force < 0)
			return r;
		break;
#if DEBUG
	default:
		error(PANIC, "invalid op=%d to staterule()", op);
		break;
#endif
	}
 again:
	if (r->property & P_statevar)
	{
		if (op == PREREQS)
			return 0;
		op = RULE;
		nam = r->name;
		s = r;
	}
	else if (r->property & P_state)
		return 0;
	else
	{
		if ((r->dynamic & (D_member|D_membertoo)) && (rul = strrchr(r->name, '/')))
			rul++;
		else if (r->uname)
			rul = r->uname;
		else
			rul = r->name;
		sfprintf(internal.nam, "(%s)%s", var, rul);
		nam = sfstruse(internal.nam);
		if (s = getrule(nam))
			nam = s->name;
	}
	if (state.maxview && state.readstate && state.statefile && !nobind)
	{
		b = &r->checked[op];
		k = 0;
		if (!tstbit(*b, i = (r->property & (P_statevar|P_use|P_virtual)) && state.targetview > 0 ? state.targetview : r->view) && (!s || !s->time || (r->property & P_statevar) || !(k = timeq(r->time, s->time))))
		{
			if (!(r->property & (P_statevar|P_use|P_virtual)) && !(r->dynamic & D_bound) && !(r->mark & M_bind) && (s && s->time || !s && state.compile >= COMPILED))
			{
				/*
				 * M_bind guards staterule() recursion
				 */

				s = r;
				s->mark |= M_bind;
				r = bind(r);
				s->mark &= ~M_bind;
				goto again;
			}
			if ((r->property & P_statevar) && r->time == OLDTIME)
			{
				for (i = 0; i <= state.maxview; i++)
				{
					setbit(*b, i);
					setbit(r->checked[CONSISTENT], i);
				}
			}
			else
			{
				if (!(r->property & (P_statevar|P_use|P_virtual)) || state.targetview < 0)
				{
					m = 1;
					j = 0;
					if (!(k = r->view) && !(state.view[0].flags & BIND_EXISTS))
						k = state.maxview;
				}
				else
				{
					m = 0;
					j = k = state.targetview;
				}
#if DEBUG
				if (state.test & 0x00000200)
					error(2, "STATERULE %s search %d..%d targetview=%d", nam, j, k, state.targetview);
#endif
				for (i = j; i <= k; i++)
				{
					setbit(*b, i);
					if (i || !(r->property & P_statevar))
					{
						s = stateview(op, nam, s, r, i, i == k, &v);
						if (v)
						{
							if (m)
							{
								m = i;
								while (++m <= k)
									setbit(*b, m);
							}
							if (op != PREREQS || state.accept)
								do setbit(r->checked[CONSISTENT], i); while (--i > j);
							break;
						}
					}
					if (state.accept)
						setbit(r->checked[CONSISTENT], i);
					if (!i)
					{
						if (op != PREREQS)
							setbit(r->checked[CONSISTENT], i);
						k = state.maxview;
					}
				}
			}
		}
		else if (k)
			setbit(*b, i);
	}
	if (!s && force > 0)
		s = makerule(nam);
	if (s && op == RULE)
	{
		r->attribute |= s->attribute & internal.retain->attribute;
		r->property |= s->property & (P_dontcare|P_terminal);
	}
	return s;
}

/*
 * return a non-state rule pointer corresponding to the staterule r
 * force causes the non-state rule to be created
 */

struct rule*
rulestate(register struct rule* r, int force)
{
	register char*		s;

	if (r->property & P_staterule)
	{
		s = r->name;
		while (*s && *s++ != ')');
		if (!(r = getrule(s)) && force) r = makerule(s);
	}
	return r;
}

/*
 * return a variable pointer corresponding to the state variable r
 * force causes the variable to be created
 */

struct var*
varstate(register struct rule* r, int force)
{
	register char*		s;
	register char*		t;
	register struct var*	v;

	s = r->name;
	if (r->property & P_state)
	{
		if (r->property & P_statevar)
		{
			s++;
			*(t = s + strlen(s) - 1) = 0;
		}
		else return 0;
	}
	else t = 0;
	if (!(v = getvar(s)) && force) v = setvar(s, null, 0);
	if (t) *t = ')';
	return v;
}

/*
 * return the auxiliary variable pointer for s
 * force causes the variable to be created
 */

struct var*
auxiliary(char* s, int force)
{
	struct var*	v;

	sfprintf(internal.nam, "(&)%s", s);
	if (!(v = getvar(sfstruse(internal.nam))) && force)
		v = setvar(sfstrbase(internal.nam), null, 0);
	return v;
}

/*
 * force r->scan == (*(unsigned char*)h) or all files to be re-scanned
 */

int
forcescan(const char* s, char* v, void* h)
{
	register struct rule*	r = (struct rule*)v;
	register int		n = h ? *((unsigned char*)h) : r->scan;

	NoP(s);
	if ((r->property & P_staterule) && r->scan == n && !(r->dynamic & D_scanned))
		r->property |= P_force;
	return 0;
}

/*
 * report state file lock fatal error
 */

static void
badlock(char* file, int view, unsigned long date)
{
	long	d;

	/*
	 * probably a bad lock if too old
	 */

	if ((d = (CURTIME - date)) > 24 * 60 * 60)
		error(1, "%s is probably an invalid lock file", file);
	else
		error(1, "another make has been running on %s in %s for the past %s", state.makefile, state.view[view].path, fmtelapsed(d, 1));
	error(3, "use -%c to override", OPT(OPT_ignorelock));
}

/*
 * get|release exclusive (advisory) access to state file
 *
 * this is a general solution that should work on all systems
 * the following problems need to be addressed
 *
 *	o  flock() or lockf() need to work for distributed
 *	   as well as local file systems
 *
 *	o  the file system clock may be different than the
 *	   local system clock
 *
 *	o  creating a specific lock file opens the door for
 *	   lock files laying around after program and system
 *	   crashes -- placing the pid of the locking process
 *	   as file data may not work on distributed process
 *	   systems
 */

#define LOCKTIME(p,m)	((unsigned long)((m)?(p)->st_mtime:(p)->st_ctime))

void
lockstate(register char* file)
{
	register int		fd;
	struct stat		st;

	static char*		lockfile;
	static unsigned long	locktime;
	static int		lockmtime;

	if (file)
	{
		if (!state.exec || state.virtualdot || !state.writestate)
			return;
		edit(internal.nam, file, DELETE, KEEP, external.lock);
		file = strdup(sfstruse(internal.nam));
		if (!state.ignorelock)
		{
			int	uid = geteuid();

			for (fd = 1; fd <= state.maxview; fd++)
			{
				edit(internal.nam, file, state.view[fd].path, KEEP, KEEP);
				if (!stat(sfstruse(internal.nam), &st) && st.st_uid != uid)
					badlock(sfstrbase(internal.nam), fd, LOCKTIME(&st, lockmtime));
			}
		}
		locktime = 0;
		for (;;)
		{
			lockfile = file;
			if ((fd = open(file, O_WRONLY|O_CREAT|O_TRUNC|O_EXCL, 0)) >= 0)
				break;
			lockfile = 0;
			if (stat(file, &st) < 0)
				error(3, "cannot create lock file %s", file);
			if (!state.ignorelock)
				badlock(file, 0, LOCKTIME(&st, lockmtime));
			if (remove(file) < 0)
				error(3, "cannot remove lock file %s", file);
		}
		if (fstat(fd, &st) < 0)
			error(3, "cannot stat lock file %s", file);
		close(fd);
		lockmtime = st.st_atime < st.st_mtime || st.st_ctime < st.st_mtime;
		locktime = LOCKTIME(&st, lockmtime);
	}
	else if (lockfile)
	{
		if (locktime)
		{
			if (stat(lockfile, &st) < 0 || LOCKTIME(&st, lockmtime) != locktime)
			{
				if (state.writestate)
					error(1, "the state file lock on %s has been overridden", state.makefile);
			}
			else if (remove(lockfile) < 0)
				error(1, "cannot remove lock file %s", lockfile);
		}
		else
			remove(lockfile);
		lockfile = 0;
	}
}

/*
 * read state from a previous make
 */

void
readstate(char* file)
{
	register Sfio_t*	fp;

	if (file)
	{
		lockstate(file);
		if (state.readstate && (fp = sfopen(NiL, file, "r")))
		{
			state.stateview = 0;
			message((-2, "loading state file %s", file));
			makerule(file)->dynamic |= D_built;
			if (load(fp, file, 10) > 0) state.view[0].flags |= BIND_EXISTS;
			else if (!state.corrupt) error(3, "use -%c%c to accept current state or -%c to remake", OPT(OPT_accept), OPT(OPT_readstate), OPT(OPT_readstate));
			else if (*state.corrupt == 'a') state.accept = 1;
			state.stateview = -1;
			sfclose(fp);
		}
	}
	state.view[0].flags |= BIND_LOADED;
}

/*
 * update the superimposed code
 */

static void
code(register const char* s)
{
	message((-6, "enter candidate state variable %s", s));
	settype(*s++, C_VARPOS1);
	if (*s)
	{
	 settype(*s++, C_VARPOS2);
	 if (*s)
	 {
	  settype(*s++, C_VARPOS3);
	  if (*s)
	  {
	   settype(*s++, C_VARPOS4);
	   if (*s)
	   {
	    settype(*s++, C_VARPOS5);
	    if (*s)
	    {
	     settype(*s++, C_VARPOS6);
	     if (*s)
	     {
	      settype(*s++, C_VARPOS7);
	      if (*s) settype(*s, C_VARPOS8);
	     }
	    }
	   }
	  }
	 }
	}
}

/*
 * bind() and scan() r->parameter file prerequisites
 * this catches all implicit state variables before the
 * parent files are scanned
 */

static int
checkparam(const char* s, char* v, void* h)
{
	register struct rule*	r = (struct rule*)v;
	register struct list*	p;
	register char*		t;
	unsigned long		tm;

	NoP(s);
	NoP(h);
	if ((r->property & (P_attribute|P_parameter|P_state)) == P_parameter)
	{
		maketop(r, 0L, NiL);
		for (p = scan(r, &tm); p; p = p->next)
			if (((r = p->rule)->property & (P_parameter|P_statevar)) == (P_parameter|P_statevar))
			{
				r->dynamic |= D_scanned;
				if (t = strchr(r->name, ')'))
				{
					*t = 0;
					code(r->name + 1);
					*t = ')';
				}
			}
	}
	return 0;
}

/*
 * check implicit state variable vars
 */

static int
checkvar1(register const char* s, char* u, void* h)
{
	register struct var*	v = (struct var*)u;
	register struct rule*	r;

	NoP(h);
	if (v->property & V_scan)
	{
		state.fullscan = 1;
		r = staterule(VAR, NiL, (char*)s, 1);
		if (!r->scan)
		{
#if DEBUG
			message((-5, "%s and %s force re-scan", v->name, r->name));
#endif
			r->scan = SCAN_STATE;
			state.forcescan = 1;
		}
		code(s);
	}
	return 0;
}

/*
 * check implicit state variable rules
 */

static int
checkvar2(const char* s, char* u, void* h)
{
	register struct rule*	r = (struct rule*)u;
	struct var*		v;

	NoP(s);
	NoP(h);
	if ((r->property & P_statevar) && r->scan && !r->view && (!(v = varstate(r, 0)) || !(v->property & V_scan)) && (!(r->property & P_parameter) || !(r->dynamic & D_scanned)))
	{
#if DEBUG
		message((-5, "%s forces re-scan", r->name));
#endif
		r->scan = 0;
		state.forcescan = 1;
	}
	return 0;
}

/*
 * freeze the parameter files and candidate state variables
 */

void
candidates(void)
{
	int	view;

	if (state.scan)
	{
		message((-2, "freeze candidate state variables and parameter files"));
		hashwalk(table.rule, 0, checkparam, NiL);
		hashwalk(table.var, 0, checkvar1, NiL);
		hashwalk(table.rule, 0, checkvar2, NiL);
		if (state.forcescan)
		{
			for (view = 1; view <= state.maxview; view++)
				stateview(0, NiL, NiL, NiL, view, 0, NiL);
			hashwalk(table.rule, 0, forcescan, NiL);
		}
	}
}

/*
 * save state for the next make
 */

void
savestate(void)
{
	if (state.statefile && state.user && state.compile == COMPILED)
	{
		if (state.writestate)
		{
			if (state.finish) state.compile = SAVED;
			if (state.exec && state.savestate)
			{
				message((-2, "saving state in %s", state.statefile));
				state.stateview = 0;
				compile(state.statefile, NiL);
				state.stateview = -1;
				state.savestate = 0;
			}
		}
		if (state.finish) lockstate(NiL);
	}
}

/*
 * bind statevar r to a variable
 */

struct rule*
bindstate(register struct rule* r, register char* val)
{
	struct rule*	s;

#if DEBUG
	if (!(r->property & P_state))
		error(PANIC, "bindstate(%s) called for non-state rule", r->name);
#endif
	if (state.maxview && (r->property & P_statevar) && (s = staterule(VAR, r, NiL, 0)))
		r = s;
	if ((r->dynamic & D_bound) && !val)
		return r;
	if (r->property & P_statevar)
	{
		register struct var*	v;
		Sfio_t*			tmp = 0;

		/*
		 * determine the current state variable value
		 */

		if (val)
			r->property |= P_virtual;
		else if (r->property & P_virtual)
			val = null;
		else if (v = varstate(r, 0))
		{
			tmp = sfstropen();
			r->dynamic |= D_bound;
			expand(tmp, getval(v->name, 1));
			r->dynamic &= ~D_bound;
			val = sfstruse(tmp);
		}
		else if ((r->property & P_parameter) && r->statedata)
			val = r->statedata;
		else
			val = null;
		if (!r->time && state.maxview && (state.view[0].flags & BIND_LOADED))
		{
			/*
			 * see if some other view has an initial value
			 */

			r->statedata = strdup(val);
			staterule(RULE, r, NiL, 0);
		}

		/*
		 * check if the state variable value changed
		 * the previous value, if defined, has already
		 * been placed in r->statedata by readstate()
		 */

		message((-2, "checking state variable %s value `%s'", r->name, r->statedata ? r->statedata : null));
		if (!r->time || r->statedata && !streq(r->statedata, val) || !r->statedata && *val)
		{
			/*
			 * state variable changed
			 */

			if (!r->view && !(r->property & P_accept))
			{
				if (r->time)
					reason((1, "state variable %s changed to `%s' from `%s'", r->name, val, r->statedata));
				else
					reason((1, "state variable %s initialized to `%s'", r->name, val));
			}
			state.savestate = 1;
			if (r->statedata != val)
				r->statedata = strdup(val);
			r->time = CURTIME - 1;	/* hack around 1 sec granularity */
		}
		if ((r->property & P_accept) || state.accept)
			r->time = OLDTIME;
		if (tmp)
			sfstrclose(tmp);
	}
	bindattribute(r);
	return r;
}

/*
 * check and stat built target r
 * otherwise check for motion from . to dir of r
 *
 * NOTE: this needs clarification
 */

static int
checkcurrent(register struct rule* r, struct stat* st)
{
	register int	n;
	register char*	s;
	long		pos;

	if (r->uname && !(n = rstat(r->uname, st, 1))) oldname(r);
	else if ((n = rstat(r->name, st, 1)) && (state.exec || state.mam.out && !state.mam.port))
	{
		rebind(r, -1);
		n = rstat(r->name, st, 1);
	}
	if (!(r->dynamic & D_entries) && !n && S_ISREG(st->st_mode)) r->dynamic |= D_regular;
	if (!(r->dynamic & D_triggered)) return n;
	edit(internal.nam, r->name, KEEP, DELETE, DELETE);
	if (!(pos = sfstrtell(internal.nam))) return n;
	sfputc(internal.nam, 0);
	sfputr(internal.nam, r->name, 0);
	s = sfstrset(internal.nam, pos + 1);
	pathcanon(s, 0);
	if (!streq(r->name, s))
	{
		if (!r->uname) r->uname = r->name;
		r->name = strdup(s);
	}
	s = sfstrset(internal.nam, 0);
#if DEBUG
	if (state.test & 0x00000100) error(2, "statetime(%s): dir=%s n=%d time=[%s]", r->name, s, n, strtime(n ? NOTIME : st->st_mtime));
#endif
	newfile(r, s, n ? NOTIME : st->st_mtime);
	return n;
}

/*
 * update internal time of r after its action has completed
 * sync>0 syncs the state rule prereqs and action
 * sync<0 resolves r but does not update state
 */

unsigned long
statetime(register struct rule* r, int sync)
{
	register struct rule*	s;
	int			a;
	int			n;
	int			zerostate = 0;
	unsigned long		t;
	struct rule*		x;
	struct stat		st;
	struct stat		ln;

	if (r->property & P_state)
		return r->time;
	s = 0;
	if (state.interrupt && r->status != EXISTS)
		zerostate = 1;
	else if (r->status == FAILED)
		st.st_mtime = 0;
	else if (r->property & P_virtual)
		r->time = st.st_mtime = CURTIME;
	else if (checkcurrent(r, &st))
	{
		if (r->property & P_dontcare)
			st.st_mtime = 0;
		else
		{
			st.st_mtime = CURTIME;
			zerostate = 1;
		}
	}
	else if (sync < 0)
		return r->time;
	else if ((s = staterule(RULE, r, NiL, 1)) && s->time == st.st_mtime)
	{
		if (state.exec && !state.touch)
		{
			/*
			 * this alternate event time prevents the action from
			 * triggering next time if nothing else changes
			 */

			x = staterule(PREREQS, r, NiL, 1);
			x->time = r->time;
			r->time = s->event;
			if (r->dynamic & D_triggered)
				r->dynamic |= D_same;
		}
		state.savestate = 1;
	}
	else if ((r->dynamic & D_triggered) && state.exec)
	{
		static int	localsync;
		static int	localtest;
		static long	localskew;

		/*
		 * r is built since its time changed after its action triggered
		 */

		s->dynamic |= D_built;
		if (x = staterule(PREREQS, r, NiL, 0))
			x->property |= P_force;

		/*
		 * check for file system and local system time consistency
		 * directories and multi hard link files are not sync'd
		 */

		if (st.st_nlink <= 1 && !S_ISDIR(st.st_mode))
		{

#if DEBUG
			if (state.test & 0x00000100)
				error(2, "%s: r[%s] s[%s] f[%s]", r->name, strtime(r->time), strtime(s->time), strtime(st.st_mtime));
#endif
			if (!localsync && !state.override && r->time && r->time != OLDTIME && !(r->property & P_force) && st.st_mtime == st.st_ctime)
			{
				if (((n = (r->time - (unsigned long)st.st_mtime - 1)) >= 0 || (n = (CURTIME - (unsigned long)st.st_mtime + 2)) <= 0) && (lstat(r->name, &ln) || !S_ISLNK(ln.st_mode)))
				{
					/*
					 * warn if difference not tolerable
					 */

					a = (n > 0) ? n : -n;
					if (a > 1)
						error(1, "%s file system time %s local time by at least %s", r->name, n > 0 ? "lags" : "leads", fmtelapsed(a, 1));
					localsync = a > state.tolerance ? 1 : -1;
				}
			}
			if (localsync > 0)
			{
				/*
				 * NOTE: time stamp syncs work on the assumption that
				 *	 all source files have an mtime that is older
				 *	 than CURTIME -- this isn't too bad since
				 *	 only built files are sync'd
				 */

				n = 0;
				for (;;)
				{
					t = CURTIME + localskew;
					if (touch(r->name, (time_t)0, t, 0))
					{
						error(ERROR_SYSTEM|1, "%s not sync'd to local time", r->name);
						break;
					}
					if (n || localtest || localskew)
					{
						st.st_mtime = t;
						break;
					}

					/*
					 * some systems try to fix up the local
					 * remote skew in the utime() call
					 * >> this never works <<
					 * members of the club include
					 *	netbsd.i386
					 */

					if (stat(r->name, &st))
					{
						error(ERROR_SYSTEM|1, "%s not found", r->name);
						break;
					}
					if (st.st_mtime == t)
					{
						localtest = 1;
						break;
					}
					if (!localtest)
					{
						localtest = 1;
						error(1, "the utime(2) or utimes(2) system call is botched for the filesystem containing %s -- the state may be out of sync", r->name);
					}
					localskew = t - st.st_mtime;
					n = 0;
				}
			}
		}
	}
	if (!s)
		s = staterule(RULE, r, NiL, 1);
	if (sync)
	{
		s->dynamic |= D_built;
		s->attribute = r->attribute;
		s->action = r->action;
		if (s->prereqs != r->prereqs)
		{
			if ((r->property & (P_joint|P_target)) != (P_joint|P_target))
				freelist(s->prereqs);
			s->prereqs = r->prereqs;
		}
	}
	if (s->time != st.st_mtime || zerostate && s->time)
	{
		s->time = zerostate ? 0 : st.st_mtime;
		s->event = CURTIME;
		state.savestate = 1;
	}
	return s->time;
}
