#pragma prototyped

/*
 * crypt-zip decoder/encoder
 */

#include "cryptlib.h"

#define CRC(x,c)		(crc_tab[((x)^(c))&0xff]^((x)>>8))

typedef struct State_s
{
	ui4		ini[3];
	ui4		key[3];

	ui1*		buf;
	size_t		bufsiz;
} State_t;

static void
update(register ui4* k, int c)
{
	k[0] = CRC(k[0], c);
	k[1] += k[0] & 0xff;
	k[1] = k[1] * 0x08088405 + 1;
	k[2] = CRC(k[2], k[1] >> 24);
}

static int
crank(register State_t* state)
{
	int	x;

	x = state->key[2] | 2;
	return ((x * (x ^ 1)) >> 8) & 0xff;
}

static int
zip_open(Codex_t* p, char* const args[], Codexnum_t flags)
{
	register State_t*	state;
	register ui1*		s;
	register int		c;
	char			pwd[256];

	if (!args[2] || args[3] || strcmp(args[2], "zip"))
		return -1;
	if (codexpass(pwd, sizeof(pwd), p->disc, p->meth) < 0)
		return -1;
	if (!(state = newof(0, State_t, 1, 0)))
	{
		if (p->disc->errorf)
			(*p->disc->errorf)(NiL, p->disc, 2, "out of space");
		return -1;
	}
	state->key[0] = 0x12345678;
	state->key[1] = 0x23456789;
	state->key[2] = 0x34567890;
	s = (ui1*)pwd;
	while (c = *s)
	{
		*s++ = 0;
		update(state->key, c);
	}
	memcpy(state->ini, state->key, sizeof(state->key));
	p->data = state;
	return 0;
}

static int
zip_close(Codex_t* p)
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
zip_read(Sfio_t* sp, void* buf, size_t n, Sfdisc_t* disc)
{
	register State_t*	state = (State_t*)CODEX(disc)->data;
	register ui4*		k = state->key;
	register ui1*		s;
	register ui1*		e;
	register int		x;
	ssize_t			r;

	if ((r = sfrd(sp, buf, n, disc)) > 0)
		for (e = (s = (ui1*)buf) + r; s < e; s++)
		{
			x = k[2] | 2;
			update(k, *s ^= ((x * (x ^ 1)) >> 8) & 0xff);
		}
	return r;
}

static ssize_t
zip_write(Sfio_t* sp, const void* buf, size_t n, Sfdisc_t* disc)
{
	register State_t*	state = (State_t*)CODEX(disc)->data;
	register ui4*		k = state->key;
	register ui1*		s = (ui1*)buf;
	register int		x;
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
	{
		x = k[2] | 2;
		state->buf[i] = s[i] ^ ((x * (x ^ 1)) >> 8) & 0xff;
		update(k, s[i]);
	}
	return sfwr(sp, state->buf, n, disc);
}

static int
zip_init(Codex_t* p)
{
	register State_t*	state = (State_t*)p->data;

	memcpy(state->key, state->ini, sizeof(state->ini));
	return 0;
}

Codexmeth_t	crypt_zip =
{
	"crypt-zip",
	"zip encryption. The encryption is asymmetric.",
	0,
	CODEX_DECODE|CODEX_ENCODE|CODEX_CRYPT,
	0,
	0,
	zip_open,
	zip_close,
	zip_init,
	0,
	zip_read,
	zip_write,
	zip_init,
	0,
	0,
	0,
	0,
	crypt_zip_next
};
