/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1986-2004 AT&T Corp.                *
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
 * crypt-rar-{13,15} decoder/encoder
 */

#include "rar.h"

typedef struct State_s
{
	ui2		ini[7];
	ui2		key[7];

	ui1*		buf;
	size_t		bufsiz;
} State_t;

static int
rar_13_open(Codex_t* p, char* const args[], Codexnum_t flags)
{
	register State_t*	state;
	register ui1*		s;
	register int		c;
	ui4			key;
	char			pwd[256];

	if (codexpass(pwd, sizeof(pwd), p->disc, p->meth) < 0)
		return -1;
	if (!(state = newof(0, State_t, 1, 0)))
	{
		if (p->disc->errorf)
			(*p->disc->errorf)(NiL, p->disc, 2, "out of space");
		return -1;
	}
	key = 0xffffffff;
	s = (ui1*)pwd;
	while (c = *s++)
		key = crc_tab[(key ^ c) & 0xff] ^ (key >> 8);
	state->ini[0] = key;
	state->ini[1] = key >> 16;
	s = (ui1*)pwd;
	while (c = *s)
	{
		*s++ = 0;
		state->ini[4] += c;
		state->ini[5] &= c;
		state->ini[6] += c;
		state->ini[6] = ROL(state->ini[6], 1);
		state->ini[2] ^= c ^ crc_tab[c];
		state->ini[3] += c + (crc_tab[c] >> 16);
	}
	p->data = state;
	return 0;
}

static int
rar_13_close(Codex_t* p)
{
	register State_t*	state = (State_t*)p->data;

	if (!state)
		return -1;
	if (state->buf)
		free(state->buf);
	free(state);
	return 0;
}

static ssize_t
rar_13_read(Sfio_t* sp, void* buf, size_t n, Sfdisc_t* disc)
{
	register State_t*	state = (State_t*)CODEX(disc)->data;
	register ui1*		s;
	register ui1*		e;
	ssize_t			r;

	if ((r = sfrd(sp, buf, n, disc)) > 0)
		for (e = (s = (ui1*)buf) + r; s < e; s++)
			*s -= (state->key[4] += (state->key[5] += state->key[6]));
	return r;
}

static ssize_t
rar_13_write(Sfio_t* sp, const void* buf, size_t n, Sfdisc_t* disc)
{
	register State_t*	state = (State_t*)CODEX(disc)->data;
	register ui1*		s = (ui1*)buf;
	register size_t		i;

	if (n > state->bufsiz)
	{
		state->bufsiz = roundof(n, 256);
		if (!(state->buf = newof(state->buf, ui1, state->bufsiz, 0)))
		{
			if (CODEX(disc)->disc->errorf)
				(*CODEX(disc)->disc->errorf)(NiL, CODEX(disc)->disc, 2, "out of space");
			return -1;
		}
	}
	for (i = 0; i < n; i++)
		state->buf[i] = s[i] + (state->key[4] += (state->key[5] += state->key[6]));
	return sfwr(sp, state->buf, n, disc);
}

static int
rar_13_init(Codex_t* p)
{
	register State_t*	state = (State_t*)p->data;

	memcpy(state->key, state->ini, sizeof(state->ini));
	return 0;
}

#define CRYPT_15(k,c)	( \
			k[0] += 0x1234, \
			k[1] ^= crc_tab[(k[0] >> 1) & 0xff], \
			k[2] -= crc_tab[(k[0] >> 1) & 0xff] >> 16, \
			k[0] ^= k[2], \
			k[3] = ROR(k[3], 1) ^ k[1], \
			k[3] = ROR(k[3], 1), \
			k[0] ^= k[3], \
			c ^ k[0] >> 8 \
			)

static ssize_t
rar_15_read(Sfio_t* sp, void* buf, size_t n, Sfdisc_t* disc)
{
	register State_t*	state = (State_t*)CODEX(disc)->data;
	register ui2*		key = state->key;
	register ui1*		s;
	register ui1*		e;
	ssize_t			r;

	if ((r = sfrd(sp, buf, n, disc)) > 0)
		for (e = (s = (ui1*)buf) + r; s < e; s++)
			*s = CRYPT_15(key, *s);
	return r;
}

static ssize_t
rar_15_write(Sfio_t* sp, const void* buf, size_t n, Sfdisc_t* disc)
{
	register State_t*	state = (State_t*)CODEX(disc)->data;
	register ui2*		key = state->key;
	register ui1*		s = (ui1*)buf;
	register size_t		i;

	if (n > state->bufsiz)
	{
		state->bufsiz = roundof(n, 256);
		if (!(state->buf = newof(state->buf, ui1, state->bufsiz, 0)))
		{
			if (CODEX(disc)->disc->errorf)
				(*CODEX(disc)->disc->errorf)(NiL, CODEX(disc)->disc, 2, "out of space");
			return -1;
		}
	}
	for (i = 0; i < n; i++)
		state->buf[i] = CRYPT_15(key, s[i]);
	return sfwr(sp, state->buf, n, disc);
}

Codexmeth_t	crypt_rar_13 =
{
	"crypt-rar-13",
	"rar version 13 and older encryption. The encryption is asymmetric.",
	0,
	CODEX_DECODE|CODEX_ENCODE|CODEX_CRYPT,
	0,
	0,
	rar_13_open,
	rar_13_close,
	rar_13_init,
	0,
	rar_13_read,
	rar_13_write,
	rar_13_init,
	0,
	0,
	0,
	0,
	0
};

Codexmeth_t	crypt_rar_15 =
{
	"crypt-rar-15",
	"rar version 15 and older encryption.",
	0,
	CODEX_DECODE|CODEX_ENCODE|CODEX_CRYPT,
	0,
	0,
	rar_13_open,
	rar_13_close,
	rar_13_init,
	0,
	rar_15_read,
	rar_15_write,
	rar_13_init,
	0,
	0,
	0,
	0,
	0
};
