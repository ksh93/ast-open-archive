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
 * Glenn Fowler
 * AT&T Research
 *
 * cs service test
 */

static const char id[] = "@(#)cs.tst-old (AT&T Research) 1997-11-11\0\n";

#define TST_VERSION	"1.0"

#include <cs.h>
#include <ctype.h>
#include <error.h>
#include <sfstr.h>
#include <tm.h>

typedef struct
{
	Cs_id_t		id;
	int		clone;
	char*		args;
} Connection_t;

typedef struct
{
	int		active;
	int		dormant;
	Connection_t	id[1];
} State_t;

static char	buf[1024];
static char	dat[1024];
static char	tim[1024];

static void*
svc_init(void* handle, int maxfd)
{
	State_t*	state;

	NoP(handle);
	if (!(state = newof(0, State_t, 1, (maxfd - 1) * sizeof(Connection_t)))) exit(1);
	cstimeout(20 * 1000L);
	return (void*)state;
}

static int
svc_connect(void* handle, int fd, Cs_id_t* id, int clone, char** argv)
{
	State_t*	state = (State_t*)handle;
	char*		s;
	Sfio_t*		sp;
	char**		ap;

	NoP(clone);
	state->active++;
	state->dormant = 0;
	state->id[fd].id = *id;
	state->id[fd].clone = clone;
	state->id[fd].args = 0;
	if ((ap = argv) && (sp = sfstropen()))
	{
		while (s = *ap++)
		{
			if (ap > argv + 1)
				sfputc(sp, ' ');
			sfputr(sp, s, -1);
		}
		state->id[fd].args = strdup(sfstruse(sp));
		sfclose(sp);
	}
	return 0;
}

static int
svc_read(void* handle, int fd)
{
	State_t*	state = (State_t*)handle;
	int		n;

	if ((n = csread(fd, dat, sizeof(dat), CS_LINE)) <= 0) goto drop;
	dat[--n] = 0;
	if (isalpha(dat[0]) && (dat[1] == 0 || isdigit(dat[1]))) switch (dat[0])
	{
	case 'd':
		error_info.trace = -(int)strtol(dat + 1, NiL, 0);
		n = sfsprintf(buf, sizeof(buf), "I debug level %d\n", -error_info.trace);
		break;
	case 'Q':
		if (state->active == 1) exit(0);
		/*FALLTHROUGH*/
	case 'q':
		goto drop;
	default:
		n = sfsprintf(buf, sizeof(buf), "E %s: unknown command\n", dat);
		break;
	}
	else
	{
		tmform(tim, *dat ? dat : "%I:%M:%S %p %Z", NiL);
		n = sfsprintf(buf, sizeof(buf), "I [%s] server=%s version=%s %s=%s server.pid=%d pid=%d uid=%d gid=%d clone=%d args=`%s'\n", tim, csname(0), TST_VERSION, CS_HOST_LOCAL, csntoa(state->id[fd].id.hid), getpid(), state->id[fd].id.pid, state->id[fd].id.uid, state->id[fd].id.gid, state->id[fd].clone, state->id[fd].args);
	}
	if (cswrite(fd, buf, n) != n) goto drop;
	return 0;
 drop:
	if (state->id[fd].args) free(state->id[fd].args);
	state->active--;
	return -1;
}

static int
svc_timeout(void* handle)
{
	State_t*	state = (State_t*)handle;

	error(1, "timeout active=%d dormant=%d", state->active, state->dormant);
	if (state->active <= 0)
	{
		if (state->dormant)
			exit(0);
		state->dormant = 1;
	}
	return 0;
}

int
main(int argc, char** argv)
{
	NoP(argc);
	csserve(NiL, argv[1], svc_init, NiL, svc_connect, svc_read, NiL, svc_timeout);
	exit(1);
}
