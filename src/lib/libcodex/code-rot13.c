/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 2003-2004 AT&T Corp.                *
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
*                David Korn <dgk@research.att.com>                 *
*                 Phong Vo <kpv@research.att.com>                  *
*                                                                  *
*******************************************************************/
#pragma prototyped

/*
 * rot13 decoder/encoder
 */

#include <codex.h>

typedef struct State_s
{
	unsigned char	rot[256];

	unsigned char*	buf;
	size_t		bufsiz;
} State_t;

static const char	rot[] = "abcdefghijklmnopqrstuvwxyz";
static const char	ROT[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static int
rot13_open(Codex_t* p, char* const args[], Codexnum_t flags)
{
	register State_t*	state;
	register int		i;

	if (!(state = newof(0, State_t, 1, 0)))
	{
		if (p->disc->errorf)
			(*p->disc->errorf)(NiL, p->disc, 2, "out of space");
		return -1;
	}
	for (i = 0; i < sizeof(state->rot); i++)
		state->rot[i] = i;
	for (i = 0; i < sizeof(rot); i++)
		state->rot[rot[i]] = rot[(i + 13) % 26];
	for (i = 0; i < sizeof(ROT); i++)
		state->rot[ROT[i]] = ROT[(i + 13) % 26];
	p->data = state;
	return 0;
}

static int
rot13_close(Codex_t* p)
{
	State_t*	state = (State_t*)p->data;

	if (!state)
		return -1;
	if (state->buf)
		free(state->buf);
	free(state);
	return 0;
}

static ssize_t
rot13_read(Sfio_t* sp, void* buf, size_t n, Sfdisc_t* disc)
{
	register State_t*	state = (State_t*)CODEX(disc)->data;
	register unsigned char*	s;
	register unsigned char*	e;
	ssize_t			r;

	if ((r = sfrd(sp, buf, n, disc)) > 0)
		for (e = (s = (unsigned char*)buf) + r; s < e; s++)
			*s = state->rot[*s];
	return r;
}

static ssize_t
rot13_write(Sfio_t* sp, const void* buf, size_t n, Sfdisc_t* disc)
{
	register State_t*	state = (State_t*)CODEX(disc)->data;
	register unsigned char*	s;
	register unsigned char*	e;
	register unsigned char*	b;

	if (n > state->bufsiz)
	{
		state->bufsiz = roundof(n, 1024);
		if (!(state->buf = newof(state->buf, unsigned char, state->bufsiz, 0)))
		{
			if (CODEX(disc)->disc->errorf)
				(*CODEX(disc)->disc->errorf)(NiL, CODEX(disc)->disc, 2, "out of space");
			return -1;
		}
	}
	for (b = state->buf, e = (s = (unsigned char*)buf) + n; s < e; s++)
		*b++ = state->rot[*s];
	return sfwr(sp, state->buf, n, disc);
}

Codexmeth_t	codex_rot13 =
{
	"rot13",
	"rot13 self-inverting encoding.",
	"[+(version)?codex-rot13 (AT&T Labs Research) 2003-12-11]"
	"[+(author)?Glenn Fowler <gsf@research.att.com>]",
	CODEX_DECODE|CODEX_ENCODE|CODEX_UU,
	0,
	0,
	rot13_open,
	rot13_close,
	0,
	0,
	rot13_read,
	rot13_write,
	0,
	0,
	0,
	0,
	0,
	CODEXNEXT(codex_rot13_next)
};

CODEXLIB(&codex_rot13)
