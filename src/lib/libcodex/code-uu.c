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
 * quoted printable coder
 */

#include <codex.h>
#include <ctype.h>

#define UUIN		3
#define UUOUT		4
#define UUCHUNK		15

#define UU_END		(UCHAR_MAX)
#define UU_IGN		(UCHAR_MAX-1)
#define UU_PAD		(UCHAR_MAX-2)

typedef struct
{
	int		pad;
	int		fill;
	int		length;
	const char	map[65];
} Data_t;

typedef struct State_s
{
	Codex_t*	codex;

	const Data_t*	data;

	unsigned char*	bb;
	unsigned char*	bp;
	unsigned char*	bl;
	unsigned char*	be;
	unsigned char*	map;

	int		c1;
	int		c2;
	int		nl;
	int		text;

	unsigned char	mapbuf[UCHAR_MAX + 2];
	unsigned char	buf[SF_BUFSIZE];
} State_t;

static const Data_t	uu_base64 =
{
	'=',
	0,
	0,
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
};

static const Data_t	uu_bsd =
{
	0,
	0156,
	1,
	"`!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
};

static const Data_t	uu_posix =
{
	0,
	0,
	1,
	" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
};

#define GETCHAR(p)	((p)->bp < (p)->be ? (int)*(p)->bp++ : fill(p))

static int
fill(State_t* state)
{
	ssize_t	r;

	state->bp = state->buf;
	if ((r = sfrd(state->codex->sp, state->bp, sizeof(state->buf), &state->codex->sfdisc)) <= 0)
	{
		state->be = state->bp;
		return EOF;
	}
	state->be = state->bp + r;
	return *state->bp++;
}

static int
flush(register State_t* state)
{
	unsigned _ast_int4_t	b;
	int			c3;
	int			x;

	x = 1;
	if (state->c1 >= 0)
	{
		c3 = state->data->fill;
		x++;
		if (state->c2 < 0)
		{
			state->c2 = c3;
			x++;
		}
		b = (state->c1 << 16) | (state->c2 << 8) | c3;
		*state->bp++ = state->data->map[b >> 18];
		*state->bp++ = state->data->map[(b >> 12) & 077];
		*state->bp++ = state->data->pad ? state->data->pad : state->data->map[(b >> 6) & 077];
		*state->bp++ = state->data->pad ? state->data->pad : state->data->map[b & 077];
		state->c1 = state->c2 = -1;
	}
	if ((state->bl - state->bp) < UUOUT * UUCHUNK)
	{
		if (state->bb)
			*state->bb = state->data->map[((state->bp - state->bb - x) / UUOUT) * UUIN + 1];
		*state->bp++ = '\n';
		state->bl = state->bp + UUOUT * UUCHUNK;
		if (sfwr(state->codex->sp, state->buf, state->bp - state->buf, &state->codex->sfdisc) != (state->bp - state->buf))
			return EOF;
	}
	return 0;
}

static int
uu_open(Codex_t* p, char* const args[], Codexnum_t flags)
{
	register State_t*	state;
	register char*		s;
	register char**		a;
	unsigned char*		m;
	unsigned char*		q;
	const Data_t*		data;
	int			c;
	int			n;
	int			text;

	data = &uu_posix;
	text = 0;
	a = (char**)args + 1;
	while (s = *++a)
		if (streq(s, "base64") || streq(s, "mime"))
			data = &uu_base64;
		else if (streq(s, "bsd") || streq(s, "ucb"))
			data = &uu_bsd;
		else if (streq(s, "posix"))
			data = &uu_posix;
		else if (streq(s, "text"))
			text = 1;
		else
		{
			if (p->disc->errorf)
				(*p->disc->errorf)(NiL, p->disc, 2, "%s: %s: unknown option", p->meth->name, s);
			return -1;
		}
	if (!(state = newof(0, State_t, 1, 0)))
	{
		if (p->disc->errorf)
			(*p->disc->errorf)(NiL, p->disc, 2, "out of space");
		return -1;
	}
	state->data = data;
	state->text = text;
	if (p->flags & CODEX_DECODE)
	{
		n = data->length ? 0 : UU_IGN;
		q = state->mapbuf;
		memset(q, n, sizeof(state->mapbuf));
		state->map = ++q;
		q[EOF] = UU_END;
		if (n)
			q[data->pad] = UU_PAD;
		for (m = (unsigned char*)data->map; c = *m; m++)
			q[c] =  m - (unsigned char*)data->map;
	}
	p->data = state;
	state->codex = p;
	return 0;
}

static int
uu_init(Codex_t* p)
{
	register State_t*	state = (State_t*)p->data;
	int			n;

	if (p->flags & CODEX_ENCODE)
	{
		state->bp = state->buf;
		n = UUOUT * UUCHUNK + state->data->length + 1;
		state->be = state->bp + (sizeof(state->buf) / n) * n;
		if (state->data->length)
		{
			state->bb = state->bp;
			*state->bp++ = state->data->map[UUIN * UUCHUNK];
		}
		state->bl = state->bp + UUOUT * UUCHUNK;
		state->c1 = state->c2 = -1;
	}
	else if (state->data->length)
		state->nl = -1;
	return 0;
}

static ssize_t
uu_read(Sfio_t* sp, void* buf, size_t n, Sfdisc_t* disc)
{
	register State_t*		state = (State_t*)CODEX(disc)->data;
	register char*			s = (char*)buf;
	register char*			e = s + n - UUIN + 1;
	register unsigned _ast_int4_t	b;
	register int			c;
	register int			x;

	if (state->data->length)
		while (s < e)
		{
			switch (c = GETCHAR(state))
			{
			case EOF:
				goto done;
			case '\n':
				state->nl = -1;
				continue;
			}
			if (state->nl < 0)
				state->nl = state->map[c];
			else if (state->nl > 0)
			{
				b = state->map[c];
				if ((c = GETCHAR(state)) == EOF)
					c = 0;
				else
					c = state->map[c];
				b = (b << 6) | c;
				if ((c = GETCHAR(state)) == EOF)
					c = 0;
				else
					c = state->map[c];
				b = (b << 6) | c;
				if ((c = GETCHAR(state)) == EOF)
					c = 0;
				else
					c = state->map[c];
				b = (b << 6) | c;
				if (state->text)
				{
					if ((c = (b >> 16) & 0xFF) != '\r')
						*s++ = c;
					if ((c = (b >> 8) & 0xFF) != '\r')
						*s++ = c;
					if ((c = b & 0xFF) != '\r')
						*s++ = c;
				}
				else
				{
					*s++ = (b >> 16);
					*s++ = (b >> 8);
					*s++ = b;
				}
				if ((state->nl -= 3) < 0)
					while (state->nl++ < 0)
						s--;
			}
		}
	else
		while (s < e)
		{
			while ((c = state->map[GETCHAR(state)]) >= 64)
				if (c != UU_IGN)
					goto done;
			b = c;
			while ((c = state->map[GETCHAR(state)]) >= 64)
				if (c != UU_IGN)
				{
					if (state->codex->disc->errorf)
						(*state->codex->disc->errorf)(NiL, state->codex->disc, 1, "%c: extra input character ignored", c);
					goto done;
				}
			b = (b << 6) | c;
			while ((c = state->map[GETCHAR(state)]) >= 64)
				if (c != UU_IGN)
				{
					if (state->text)
					{
						if ((x = (b >> 4) & 0xFF) != '\r')
							*s++ = x;
					}
					else
						*s++ = b >> 4;
					goto done;
				}
			b = (b << 6) | c;
			while ((c = state->map[GETCHAR(state)]) >= 64)
				if (c != UU_IGN)
				{
					if (state->text)
					{
						if ((x = (b >> 10) & 0xFF) != '\r')
							*s++ = x;
						if ((x = (b >> 2) & 0xFF) != '\r')
							*s++ = x;
					}
					else
					{
						*s++ = n >> 10;
						*s++ = n >> 2;
					}
					goto done;
				}
			b = (b << 6) | c;
			if (state->text)
			{
				if ((x = (b >> 16) & 0xFF) != '\r')
					*s++ = x;
				if ((x = (b >> 8) & 0xFF) != '\r')
					*s++ = x;
				if ((x = b & 0xFF) != '\r')
					*s++ = x;
			}
			else
			{
				*s++ = b >> 16;
				*s++ = b >> 8;
				*s++ = b;
			}
		}
 done:
	return s - (char*)buf;
}

static ssize_t
uu_write(Sfio_t* sp, const void* buf, size_t n, Sfdisc_t* disc)
{
	register State_t*		state = (State_t*)CODEX(disc)->data;
	register unsigned char*		s;
	register unsigned char*		e;
	register unsigned _ast_int4_t	b;
	register int			c1;
	register int			c2;
	register int			c3;

	s = (unsigned char*)buf;
	e = s + n;
	if ((c1 = state->c1) >= 0)
	{
		state->c1 = -1;
		if ((c2 = state->c2) >= 0)
		{
			state->c2 = -1;
			goto get_3;
		}
		goto get_2;
	}
	while (s < e)
	{
		do
		{
			if (state->nl)
			{
				state->nl = 0;
				c1 = '\n';
				goto get_2;
			}
			c1 = *s++;
			if (state->text && c1 == '\n')
			{
				c1 = '\r';
				c2 = '\n';
				goto get_3;
			}
 get_2:
			if (s >= e)
			{
				state->c1 = c1;
				return n;
			}
			c2 = *s++;
			if (state->text && c2 == '\n')
			{
				c2 = '\r';
				c3 = '\n';
				goto put_123;
			}
 get_3:
			if (s >= e)
			{
				state->c1 = c1;
				state->c2 = c2;
				return n;
			}
			c3 = *s++;
			if (state->text && c3 == '\n')
			{
				state->nl = 1;
				c3 = '\r';
			}
 put_123:
			b = (c1 << 16) | (c2 << 8) | c3;
			*state->bp++ = state->data->map[b >> 18];
			*state->bp++ = state->data->map[(b >> 12) & 077];
			*state->bp++ = state->data->map[(b >> 6) & 077];
			*state->bp++ = state->data->map[b & 077];
		} while (state->bp < state->bl);
		*state->bp++ = '\n';
		if (state->bp >= state->be)
		{
			if (sfwr(sp, state->buf, state->bp - state->buf, disc) != (state->bp - state->buf))
				return -1;
			state->bp = state->buf;
		}
		if (state->bb)
		{
			state->bb = state->bp;
			*state->bp++ = state->data->map[UUIN * UUCHUNK];
		}
		state->bl = state->bp + UUOUT * UUCHUNK;
	}
	return n;
}

static int
uu_sync(Codex_t* p)
{
	return flush((State_t*)p->data);
}

Codexmeth_t	codex_uu =
{
	"uu",
	"uuencode printable encoding.",
	"[+posix?Posix \buuencode\b(1). This is the default.]"
	"[+base64|mime?MIME base64 encoding.]"
	"[+bsd|ucb?BSD \buuencode\b(1).]"
	"[+text?Encode \\n => \\r\\n, decode \\r\\n => \\n.]"
	"[+(version)?codex-uu (AT&T Labs Research) 1998-11-11]"
	"[+(author)?Glenn Fowler <gsf@research.att.com>]",
	CODEX_DECODE|CODEX_ENCODE|CODEX_UU,
	0,
	0,
	uu_open,
	0,
	uu_init,
	0,
	uu_read,
	uu_write,
	uu_sync,
	0,
	0,
	0,
	0,
	CODEXNEXT(codex_uu_next)
};

CODEXLIB(&codex_uu)