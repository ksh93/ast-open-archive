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
 * make signal traps
 */

#include "make.h"

#include <sig.h>
#include <vmalloc.h>

#undef	trap

struct alarms
{
	struct alarms*	next;
	struct rule*	rule;
	unsigned long	time;
};

static int	signals[] =		/* signals to catch		*/
{
	SIGHUP,
	SIGINT,
	SIGALRM,
	SIGTERM,
#ifdef SIGILL
	SIGILL,
#endif
#ifdef SIGIOT
	SIGIOT,
#endif
#ifdef SIGEMT
	SIGEMT,
#endif
#if !DEBUG
	SIGQUIT,
#ifdef SIGBUS
	SIGBUS,
#endif
#ifdef SIGSEGV
	SIGSEGV,
#endif
#endif
#ifdef SIGUSR1
	SIGUSR1,
#endif
#ifdef SIGUSR2
	SIGUSR2,
#endif
};

static struct
{
	int*		caught;		/* caught signals		*/
	struct alarms*	alarms;		/* sorted alarm list		*/
	struct alarms*	freealarms;	/* free alarm list		*/
	int		newalarms;	/* setwakeup() check		*/
} trap;

/*
 * catch and mark interrupts
 */

static void
interrupt(register int sig)
{
	signal(sig, interrupt);
	trap.caught[sig] = 1;
	state.caught = 1;
}

/*
 * (re)initialize wakeup rule pointers
 */

void
initwakeup(int repeat)
{
	register struct alarms*	a;

	NoP(repeat);
	for (a = trap.alarms; a; a = a->next)
		a->rule = makerule(a->rule->name);
}

/*
 * set next wakeup time
 */

static void
setwakeup(void)
{
	register unsigned long	t;
	register unsigned long	now;
	int			level;

	if (!trap.newalarms)
		return;
	trap.newalarms = 0;
	now = CURTIME;
	if (!trap.alarms)
		t = 0;
	else if (trap.alarms->time <= now)
		t = 1;
	else
		t = trap.alarms->time - now;
	alarm(t);
	sfsprintf(tmpname, MAXNAME, "%lu", t ? (now + t) : t);
	setvar(internal.alarm->name, fmtelapsed(t, 1), 0);
	if (error_info.trace <= (level = (state.test & 0x00010000) ? 2 : CMDTRACE))
	{
		register struct alarms*	a;

		if (a = trap.alarms)
		{
			error(level, "ALARM  TIME                 RULE");
			do
			{
				error(level, "%6s %s %s", fmtelapsed((a->time >= now) ? (a->time - now) : 0, 1), strtime(a->time), a->rule->name);
			} while (a = a->next);
		}
		else
			error(level, "ALARM -- NONE");
	}
}

/*
 * wakeup in t seconds and make rules in p
 * t==0 to drop alarms for list p, all for p==0
 */

void
wakeup(unsigned long t, register struct list* p)
{
	register struct alarms*	a;
	register struct alarms*	z;
	register struct alarms*	x;
	unsigned long		now;

	now = CURTIME;
	if (t)
	{
		t += now;
		if (!p)
			p = cons(catrule(external.interrupt, ".", fmtsignal(-SIGALRM), 1), NiL);
	}
	if (p)
	{
		do
		{
			x = 0;
			for (z = 0, a = trap.alarms; a; z = a, a = a->next)
				if (a->rule == p->rule)
				{
					x = a;
					if (z)
						z->next = a->next;
					else
						trap.alarms = a->next;
					trap.newalarms = 1;
					break;
				}
			if (t)
			{
				if (!x)
				{
					if (x = trap.freealarms)
						trap.freealarms = trap.freealarms->next;
					else
						x = newof(0, struct alarms, 1, 0);
					x->rule = p->rule;
				}
				x->time = t;
				x->next = 0;
				for (z = 0, a = trap.alarms; a; z = a, a = a->next)
					if (t <= a->time)
					{
						x->next = a;
						break;
					}
				if (z)
					z->next = x;
				else
					trap.alarms = x;
				trap.newalarms = 1;
			}
		} while (p = p->next);
	}
	else if (a = trap.alarms)
	{
		trap.alarms = 0;
		while (x = a->next)
			a = x;
		a->next = trap.freealarms;
		trap.freealarms = a;
		trap.newalarms = 1;
	}
	setwakeup();
}

/*
 * initialize the traps
 */

void
inittrap(void)
{
	register int	sig;

	memfatal();
	trap.caught = newof(0, int, sig_info.sigmax + 1, 0);
	for (sig = 0; sig < elementsof(signals); sig++)
		if (signal(signals[sig], interrupt) == SIG_IGN)
			signal(signals[sig], SIG_IGN);
}

/*
 * handle interrupts
 * called by trap() in safe regions
 * 0 returned if no interrupts to handle
 */

int
handle(void)
{
	register int		sig;
	register struct rule*	r;
	register struct alarms*	a;
	char*			s;
	char*			w;
	struct var*		v;
	unsigned long		t;

	if (!state.caught)
		return 0;
	while (state.caught)
	{
		state.caught = 0;
		for (sig = 1; sig <= sig_info.sigmax; sig++)
			if (trap.caught[sig])
			{
				trap.caught[sig] = 0;

				/*
				 * flush the output streams
				 */

				sfsync(sfstderr);
				sfsync(sfstdout);

				/*
				 * continue if already in finish
				 */

				if (state.finish)
				{
					if (!state.interrupt)
						state.interrupt = sig;
					for (sig = 1; sig <= sig_info.sigmax; sig++)
						trap.caught[sig] = 0;
					return 0;
				}

				/*
				 * check user trap (some cannot be trapped)
				 */

				w = 0;
				if (!state.compileonly) switch (sig)
				{
				case SIGALRM:
					t = CURTIME;
					while ((a = trap.alarms) && a->time <= t)
					{
						trap.alarms = a->next;
						r = a->rule;
						a->next = trap.freealarms;
						trap.freealarms = a;
						trap.newalarms = 1;
						maketop(r, (P_dontcare|P_force|P_ignore|P_repeat)|(r->property & P_make)?0:P_foreground, NiL);
					}
					setwakeup();
					continue;
				default:
					s = fmtsignal(-sig);
					if ((r = catrule(external.interrupt, ".", s, 0)) || (r = getrule(external.interrupt)))
					{
						v = setvar(external.interrupt, s, 0);
						maketop(r, P_dontcare|P_force|P_ignore|P_repeat|(r->property & P_make)?0:P_foreground, s);
						w = (r->property & P_functional) ? getval(r->name, 1) : v->value;
						if (r->status == EXISTS && (!(r->property & P_functional) || streq(w, s) || streq(w, "continue")))
						{
							message((-1, "trap %s handler %s status CONTINUE return %s", s, r->name, w));
							continue;
						}
						message((-1, "trap %s handler %s status TERMINATE return %s", s, r->name, w));
					}
					/*FALLTHROUGH*/
#ifdef SIGILL
				case SIGILL:
#endif
#ifdef SIGIOT
				case SIGIOT:
#endif
#ifdef SIGEMT
				case SIGEMT:
#endif
#ifdef SIGBUS
				case SIGBUS:
#endif
#ifdef SIGSEGV
				case SIGSEGV:
#endif
					break;
				}

				/*
				 * terminate outstanding jobs
				 */

				terminate();

				/*
				 * the interpreter resumes without exit
				 */

				if (state.interpreter)
				{
					if (state.waiting)
						return 1;
					longjmp(state.resume.label, 1);
				}

				/*
				 * if external.interrupt=""|"exit" then exit
				 * otherwise terminate via original signal
				 */

				if (w && (!*w || streq(w, "exit")))
					state.interrupt = 0;
				else if (!state.interrupt)
					state.interrupt = sig;
				finish(3);

				/*
				 * shouldn't get here
				 */

				exit(3);
			}
	}
	return 1;
}
