#pragma prototyped

/*
 * crypt-rar-20 decoder/encoder
 */

#include "rar.h"

#define encrypt		rar_encrypt
#define decrypt		rar_decrypt

#define BLK		16
#define REP		32

#define SUB(p,x)	((ui4)(p)->sub[ (x)        & 0xff]        | \
			((ui4)(p)->sub[((x) >>  8) & 0xff] <<  8) | \
			((ui4)(p)->sub[((x) >> 16) & 0xff] << 16) | \
			((ui4)(p)->sub[((x) >> 24) & 0xff] << 24) )

static const ui1	sub[256] =
{
  215, 19,149, 35, 73,197,192,205,249, 28, 16,119, 48,221,  2, 42,
  232,  1,177,233, 14, 88,219, 25,223,195,244, 90, 87,239,153,137,
  255,199,147, 70, 92, 66,246, 13,216, 40, 62, 29,217,230, 86,  6,
   71, 24,171,196,101,113,218,123, 93, 91,163,178,202, 67, 44,235,
  107,250, 75,234, 49,167,125,211, 83,114,157,144, 32,193,143, 36,
  158,124,247,187, 89,214,141, 47,121,228, 61,130,213,194,174,251,
   97,110, 54,229,115, 57,152, 94,105,243,212, 55,209,245, 63, 11,
  164,200, 31,156, 81,176,227, 21, 76, 99,139,188,127, 17,248, 51,
  207,120,189,210,  8,226, 41, 72,183,203,135,165,166, 60, 98,  7,
  122, 38,155,170, 69,172,252,238, 39,134, 59,128,236, 27,240, 80,
  131,  3, 85,206,145, 79,154,142,159,220,201,133, 74, 64, 20,129,
  224,185,138,103,173,182, 43, 34,254, 82,198,151,231,180, 58, 10,
  118, 26,102, 12, 50,132, 22,191,136,111,162,179, 45,  4,148,108,
  161, 56, 78,126,242,222, 15,175,146, 23, 33,241,181,190, 77,225,
    0, 46,169,186, 68, 95,237, 65, 53,208,253,168,  9, 18,100, 52,
  116,184,160, 96,109, 37, 30,106,140,104,150,  5,204,117,112, 84
};

typedef struct State_s
{
	int		eof;

	ui1		sub[256];

	ui4		ini[4];
	ui4		key[4];

	ui1*		cpy;
	ui1*		cpyend;
	ui1		cpybuf[2*BLK];

	ui1*		prt;
	ui1*		buf;
	size_t		bufsiz;
} State_t;

static void
update(register ui4* k, register const ui1* x)
{
	register const ui1*	e = x + BLK;

	while (x < e)
	{
		k[0] ^= crc_tab[*x++];
		k[1] ^= crc_tab[*x++];
		k[2] ^= crc_tab[*x++];
		k[3] ^= crc_tab[*x++];
	}
}

static void
encrypt(register State_t* state, register ui1* f, register ui1* t)
{
	register ui4*	k = state->key;
	int		i;
	ui4		a;
	ui4		b;
	ui4		c;
	ui4		d;
	ui4		ta;
	ui4		tb;

	a=k[0]^((ui4)f[ 0]|((ui4)f[ 1]<<8)|((ui4)f[ 2]<<16)|((ui4)f[ 3]<<24));
	b=k[1]^((ui4)f[ 4]|((ui4)f[ 5]<<8)|((ui4)f[ 6]<<16)|((ui4)f[ 7]<<24));
	c=k[2]^((ui4)f[ 8]|((ui4)f[ 9]<<8)|((ui4)f[10]<<16)|((ui4)f[11]<<24));
	d=k[3]^((ui4)f[12]|((ui4)f[13]<<8)|((ui4)f[14]<<16)|((ui4)f[15]<<24));
	for (i = 0; i < REP; i++)
	{
		ta = k[i&3] ^ (c + ROL(d,11));
		ta = a ^ SUB(state, ta);
		tb = k[i&3] ^ (d ^ ROL(c,17));
		tb = b ^ SUB(state, tb);
		a = c;
		b = d;
		c = ta;
		d = tb;
	}
	c ^= k[0];
	t[0] = c;
	t[1] = c>>8;
	t[2] = c>>16;
	t[3] = c>>24;
	d ^= k[1];
	t[4] = d;
	t[5] = d>>8;
	t[6] = d>>16;
	t[7] = d>>24;
	a ^= k[2];
	t[8] = a;
	t[9] = a>>8;
	t[10] = a>>16;
	t[11] = a>>24;
	b ^= k[3];
	t[12] = b;
	t[13] = b>>8;
	t[14] = b>>16;
	t[15] = b>>24;
	update(k, t);
}

static void
decrypt(register State_t* state, register ui1* x)
{
	register ui4*	k = state->key;
	int		i;
	ui4		a;
	ui4		b;
	ui4		c;
	ui4		d;
	ui4		ta;
	ui4		tb;
	ui1		tmp[BLK];

	a=k[0]^((ui4)x[ 0]|((ui4)x[ 1]<<8)|((ui4)x[ 2]<<16)|((ui4)x[ 3]<<24));
	b=k[1]^((ui4)x[ 4]|((ui4)x[ 5]<<8)|((ui4)x[ 6]<<16)|((ui4)x[ 7]<<24));
	c=k[2]^((ui4)x[ 8]|((ui4)x[ 9]<<8)|((ui4)x[10]<<16)|((ui4)x[11]<<24));
	d=k[3]^((ui4)x[12]|((ui4)x[13]<<8)|((ui4)x[14]<<16)|((ui4)x[15]<<24));
	for (i = REP-1; i >= 0; i--)
	{
		ta = k[i&3] ^ (c + ROL(d,11));
		ta = a ^ SUB(state, ta);
		tb = k[i&3] ^ (d ^ ROL(c,17));
		tb = b ^ SUB(state, tb);
		a = c;
		b = d;
		c = ta;
		d = tb;
	}
	memcpy(tmp, x, BLK);
	c ^= k[0];
	x[0] = c;
	x[1] = c>>8;
	x[2] = c>>16;
	x[3] = c>>24;
	d ^= k[1];
	x[4] = d;
	x[5] = d>>8;
	x[6] = d>>16;
	x[7] = d>>24;
	a ^= k[2];
	x[8] = a;
	x[9] = a>>8;
	x[10] = a>>16;
	x[11] = a>>24;
	b ^= k[3];
	x[12] = b;
	x[13] = b>>8;
	x[14] = b>>16;
	x[15] = b>>24;
	update(k, tmp);
}

static void
swap(ui1* a, ui1* b)
{
	int	c;

	c = *a;
	*a = *b;
	*b = c;
}

static int
rar_20_open(Codex_t* p, char* const args[], Codexnum_t flags)
{
	register State_t*	state;
	register int		i;
	register int		j;
	register int		k;
	register int		m;
	register int		n;
	ui1			n1;
	ui1			n2;
	char			pwd[256];

	if ((n = codexpass(pwd, sizeof(pwd), p->disc, p->meth)) < 0)
		return -1;
	if (!(state = newof(0, State_t, 1, 0)))
	{
		if (p->disc->errorf)
			(*p->disc->errorf)(NiL, p->disc, 2, "out of space");
		return -1;
	}
	memset(pwd + n, 0, sizeof(pwd) - n);
	memcpy(state->sub, sub, sizeof(sub));
	state->key[0] = 0xd3a3b879L;
	state->key[1] = 0x3f6d12f7L;
	state->key[2] = 0x7515a235L;
	state->key[3] = 0xa4e7f123L;
	m = n < (sizeof(pwd) - 1) ? n : (sizeof(pwd) - 1);
	for (j = 0; j < 256; j++)
		for (i = 0; i < m; i += 2)
		{
			n2 = crc_tab[(pwd[i+1] + j) & 0xff];
			for (k = 1, n1 = crc_tab[(pwd[i] - j) & 0xff]; n1 != n2; n1++, k++)
				swap(&state->sub[n1], &state->sub[(n1 + i + k) & 0xff]);
		}
	m = n < (sizeof(pwd) - BLK - 1) ? n : (sizeof(pwd) - BLK - 1);
	for (i = 0; i < m; i += BLK)
		encrypt(state, (ui1*)pwd + i, (ui1*)pwd + i);
	memset(pwd, 0, n);
	memcpy(state->ini, state->key, sizeof(state->key));
	p->data = state;
	return 0;
}

static int
rar_20_close(Codex_t* p)
{
	register State_t*	state = (State_t*)p->data;

	if (!state)
		return -1;
	if (state->buf)
	{
		memset(state->buf, 0, state->bufsiz);
		free(state->buf);
	}
	memset(state, 0, sizeof(*state));
	free(state);
	return 0;
}

static ssize_t
rar_20_read(Sfio_t* sp, void* buf, size_t n, Sfdisc_t* disc)
{
	register State_t*	state = (State_t*)CODEX(disc)->data;
	register ui1*		s = (ui1*)buf;
	register ui1*		e = s + n;
	register ui1*		t = s;
	ssize_t			r;
	size_t			m;

	if (state->eof)
		return 0;
	while (state->cpy < state->cpyend && s < e)
		*s++ = *state->cpy++;
	while (n = e - s)
	{
		if ((r = sfrd(sp, s, n, disc)) <= 0)
		{
			state->eof = 1;
			break;
		}
		s += r;
	}
	while ((s - t) > BLK)
	{
		decrypt(state, t);
		t += BLK;
	}
	r = t - (ui1*)buf;
	if (!state->eof)
	{
		n = s - t;
		memcpy(state->cpy = state->cpybuf, t, n);
		state->cpyend = state->cpy + n;
	}
	else if (s > t)
	{
		decrypt(state, t);
		if (CODEX(disc)->size >= 0)
			r += CODEX(disc)->size & (BLK - 1);
		else
		{
			m = t[BLK-1];
			n = BLK;
			while (n-- && t[n] == m)
				if (n == m)
					return r += m;
			r += BLK;
		}
	}
	return r;
}

static ssize_t
rar_20_write(Sfio_t* sp, const void* buf, size_t n, Sfdisc_t* disc)
{
	register State_t*	state = (State_t*)CODEX(disc)->data;
	register ui1*		s = (ui1*)buf;
	register ui1*		e = s + n;
	register ui1*		t;
	size_t			m;

	if (n > state->bufsiz)
	{
		state->bufsiz = roundof(n, 1024);
		if (!(state->buf = newof(state->buf, ui1, state->bufsiz, BLK)))
		{
			if (CODEX(disc)->disc->errorf)
				(*CODEX(disc)->disc->errorf)(NiL, CODEX(disc)->disc, 2, "out of space");
			return -1;
		}
	}
	if (t = state->prt)
	{
		while ((t - state->buf) < BLK)
		{
			if (s >= e)
			{
				state->prt = t;
				return s - (ui1*)buf;
			}
			*t++ = *s++;
		}
		state->prt = 0;
		encrypt(state, state->buf, state->buf);
		t = state->buf + BLK;
	}
	else
		t = state->buf;
	while ((e - s) > BLK)
	{
		encrypt(state, s, t);
		s += BLK;
		t += BLK;
	}
	if ((m = t - state->buf) && sfwr(sp, state->buf, m, disc) < 0)
		return -1;
	if (m = e - s)
	{
		memcpy(state->buf, s, m);
		state->prt = state->buf + m;
	}
	return n;
}

static int
rar_20_init(Codex_t* p)
{
	register State_t*	state = (State_t*)p->data;

	memcpy(state->key, state->ini, sizeof(state->ini));
	state->cpy = state->cpyend = 0;
	state->eof = 0;
	state->prt = 0;
	return 0;
}

static int
rar_20_done(Codex_t* p)
{
	register State_t*	state = (State_t*)p->data;
	size_t			i;
	size_t			n;
	size_t			m;
	ui1			tmp[BLK];

	if (!state->eof)
	{
		state->eof = 1;
		if (!state->prt)
			return 0;
		n = state->prt - state->buf;
		state->prt = 0;
		if (n < BLK)
			memset(state->buf + n, n, BLK - n);
		else
			memcpy(tmp, state->buf, BLK);
		encrypt(state, state->buf, state->buf);
		if (n != BLK)
			n = BLK;
		else
		{
			m = n;
			for (;;)
			{
				if (!m--)
					goto flush;
				i = BLK;
				while (i-- > m)
					if (tmp[i] != m)
						goto flush;
				break;
			}
			memset(state->buf + BLK, 0, BLK);
			encrypt(state, state->buf + BLK, state->buf + BLK);
			n = 2 * BLK;
		}
	flush:
		if (sfwr(p->sp, state->buf, n, &p->sfdisc) != n)
			return -1;
	}
	return 0;
}

Codexmeth_t	crypt_rar_20 =
{
	"crypt-rar-20",
	"rar version 20 and newer encryption.",
	0,
	CODEX_DECODE|CODEX_ENCODE|CODEX_CRYPT,
	0,
	0,
	rar_20_open,
	rar_20_close,
	rar_20_init,
	rar_20_done,
	rar_20_read,
	rar_20_write,
	rar_20_done,
	0,
	0,
	0,
	0,
	0
};
