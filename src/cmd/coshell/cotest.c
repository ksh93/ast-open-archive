/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1990-2000 AT&T Corp.                *
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
*                                                                  *
*******************************************************************/
#pragma prototyped
/*
 * coshell attribute expression test
 */

#include <service.h>

State_t		state;

void
shellclose(Coshell_t* a, int b)
{
	NoP(a);
	NoP(b);
}

void
jobcheck(Coshell_t* a)
{
	NoP(a);
}

static void
init(void)
{
	register int	n;
	register char*	s;

	message((-1, "init"));
	state.toss = state.start = cs.time;
	for (n = 0; n < 10; n++) TOSS;
	state.fdtotal = sysconf(_SC_OPEN_MAX);
	if (!(state.con = newof(0, Connection_t, state.fdtotal, 0)))
		error(3, "out of space [con]");
	state.con[0].type = POLL;
	if (!(state.job = state.jobnext = newof(0, Cojob_t, state.fdtotal / 2, 0)))
		error(3, "out of space [job]");
	state.jobmax = state.jobnext += state.fdtotal / 2 - 1;

	/*
	 * initialze the shell table
	 */

	state.busy = BUSY;
	state.grace = GRACE;
	state.pool = ((s = getenv(CO_ENV_PROC)) && *s) ? (int)strtol(s, NiL, 0) : POOL;
	if (!(state.home = search(DEF|NEW, csname(0), NiL, NiL)))
		error(3, "cannot get local host address");
	state.shell = state.shellnext = state.home;
	message((-1, "local name is %s", state.home->name));

	/*
	 * load the local net configuration
	 */

	info(DEF|NEW, NiL);

	/*
	 * bias the local host so it can generate more work
	 */

	if (state.home->idle)
	{
		state.home->idle = 0;
		if (!(state.home->flags & SETBIAS)) state.home->bias *= 4;
	}
}

int
main(int argc, char** argv)
{
	register Coshell_t*	sp;
	register char*		s;
	register int		op;
	Coattr_t		attr;

	NoP(argc);
	NoP(argv);
	error(-1, "debug");
	init();
	while ((s = sfgetr(sfstdin, '\n', 0)) && sfvalue(sfstdin) > 1) switch (s[sfvalue(sfstdin) - 1] = 0, op = *s == ':' ? (s++, *s++) : '?')
	{
	case '#':
		break;
	case '?':
	case ':':
		attributes(s, &attr, NiL);
		sp = state.shell;
		do
		{
			if (match(sp, &attr, 0))
			{
				if (op == '?') sfputr(sfstdout, sp->name, '\n');
				else
				{
					sfputr(sfstdout, sp->name, '\t');
					sfputr(sfstdout, sp->misc, '\n');
				}
			}
		} while ((sp = sp->next) != state.shell);
		break;
	case '=':
		if (!search(SET, s, NiL, NiL))
			error(2, "%s: invalid host name", s);
		break;
	default:
		error(2, "`%s': invalid command", s - 2);
		break;
	}
	exit(0);
}
