#pragma prototyped noticed

/*
 * cabinet quantum decoder
 * based on GPL code from cabextract
 *
 * decoder based on the Quantum archiver by David Stafford
 * researched and implemented by Matthew Russoto
 * rewrapped for cabextract by Stuart Caie
 * and finally rewrapped for <codex.h>
 *
 * cabextract 0.6 - a program to extract Microsoft Cabinet files
 * Copyright (c) 2000-2002 Stuart Caie <kyzer@4u.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <codex.h>

typedef unsigned _ast_int1_t  UBYTE; /* 8 bits exactly    */
typedef unsigned _ast_int2_t  UWORD; /* 16 bits (or more) */
typedef unsigned _ast_int4_t  ULONG; /* 32 bits exactly   */
typedef          _ast_int4_t   LONG; /* 32 bits (or more) */

typedef struct Symbol_s
{
	UWORD		sym;
	UWORD		freq;
} Symbol_t;

typedef struct Model_s
{
	int		shiftsleft;
	int		entries; 
	Symbol_t*	syms;
	UWORD		tabloc[256];
} Model_t;

typedef struct State_s
{
	Codex_t*	codex;

	UBYTE		buf[SF_BUFSIZE];
	UBYTE*		ip;
	UBYTE*		ie;

	ULONG		bitbuf;
	UWORD		bitcount;

	UWORD		H;
	UWORD		L;
	UWORD		C;

	ULONG		align;
	int		eof;

	ULONG		window_size;
	int		slots;

	Model_t		mod00;
	Model_t		mod04;
	Model_t		mod05;
	Model_t		mod06l;
	Model_t		mod06p;
	Model_t		mod07;
	Model_t		mod40;
	Model_t		mod80;
	Model_t		modC0;

	Symbol_t	sym00[0x40+1];
	Symbol_t	sym04[0x18+1];
	Symbol_t	sym05[0x24+1];
	Symbol_t	sym06l[0x1b+1];
	Symbol_t	sym06p[0x2a+1];
	Symbol_t	sym07[7+1];
	Symbol_t	sym40[0x40+1];
	Symbol_t	sym80[0x40+1];
	Symbol_t	symC0[0x40+1];

	UBYTE*		op;
	UBYTE*		oe;
	UBYTE		window[1];
} State_t;

static const ULONG	slot_position[] =
{
  0x00000, 0x00001, 0x00002, 0x00003, 0x00004, 0x00006, 0x00008, 0x0000c,
  0x00010, 0x00018, 0x00020, 0x00030, 0x00040, 0x00060, 0x00080, 0x000c0,
  0x00100, 0x00180, 0x00200, 0x00300, 0x00400, 0x00600, 0x00800, 0x00c00,
  0x01000, 0x01800, 0x02000, 0x03000, 0x04000, 0x06000, 0x08000, 0x0c000,
  0x10000, 0x18000, 0x20000, 0x30000, 0x40000, 0x60000, 0x80000, 0xc0000,
  0x100000,
  0x180000
};

static const UBYTE	slot_size[] =
{
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x08,
  0x0a, 0x0c, 0x0e, 0x12, 0x16, 0x1a, 0x1e, 0x26,
  0x2e, 0x36, 0x3e, 0x4e, 0x5e, 0x6e, 0x7e, 0x9e,
  0xbe, 0xde, 0xfe
};

static const UBYTE	extra_bits[] =
{
   0,  0,  0,  0,  1,  1,  2,  2,
   3,  3,  4,  4,  5,  5,  6,  6,
   7,  7,  8,  8,  9,  9, 10, 10,
  11, 11, 12, 12, 13, 13, 14, 14,
  15, 15, 16, 16, 17, 17, 18, 18,
  19, 19
};

static const UBYTE	extra_size[] =
{
   0,  0,  0,  0,  0,  0,  1,  1,
   1,  1,  2,  2,  2,  2,  3,  3,
   3,  3,  4,  4,  4,  4,  5,  5,
   5,  5,  0
};

#define GETCHAR(p)	((p)->ip < (p)->ie ? (int)*(p)->ip++ : fill(p))

static int
fill(State_t* state)
{
	ssize_t	r;

	if (state->eof)
		return 0;
	if ((r = sfrd(state->codex->sp, state->buf, sizeof(state->buf), &state->codex->sfdisc)) <= 0)
	{
		state->eof = 1;
		return 0;
	}
	state->ie = (state->ip = state->buf) + r;
	return *state->ip++;
}

#define GETBITS(p,n)	((n)<=(p)->bitcount ? ((((p)->bitbuf)>>((p)->bitcount-=(n)))&((1L<<(n))-1)) : getbits(p,n))
#define SKIPBITS(p,n)	((p)->bitcount-=(n))

static int
getbits(register State_t* state, int n)
{
	while (state->bitcount < n)
	{
		state->bitbuf <<= 8;
		state->bitbuf |= GETCHAR(state);
		state->bitcount += 8;
	}
	return GETBITS(state, n);
}

/*
 * initialize a model that decodes symbols from [s] to [s]+[n]-1
 */

static void
init(State_t* state, Model_t* m, Symbol_t* sym, int n, int s)
{
	int	i;

	m->shiftsleft = 4;
	m->entries = n;
	m->syms	= sym;

	/* clear out look-up table */

	memset(m->tabloc, 0xFF, sizeof(m->tabloc));
	for (i = 0; i < n; i++)
	{
		m->tabloc[i+s] = i; /* set up a look-up entry for symbol */
		m->syms[i].sym = i+s; /* actual symbol */
		m->syms[i].freq = n-i; /* current frequency of that symbol */
	}
	m->syms[n].freq = 0;
}

static void
update(State_t* state, Model_t* model, int sym)
{
	register int	i;
	register int	j;
	Symbol_t	tmp;

	for (i = 0; i < sym; i++)
		model->syms[i].freq += 8;
	if (model->syms[0].freq > 3800)
	{
		if (--model->shiftsleft)
		{
			for (i = model->entries - 1; i >= 0; i--)
			{
				/* -1, not -2; the 0 entry saves this */
				model->syms[i].freq >>= 1;
				if (model->syms[i].freq <= model->syms[i+1].freq)
					model->syms[i].freq = model->syms[i+1].freq + 1;
			}
		}
		else
		{
			model->shiftsleft = 50;
			for (i = 0; i < model->entries ; i++)
			{
				/* no -1, want to include the 0 entry */
				/* this converts freqs into frequencies, then shifts right */

				model->syms[i].freq -= model->syms[i+1].freq;
				model->syms[i].freq++; /* avoid losing things entirely */
				model->syms[i].freq >>= 1;
			}

			/* now sort by frequencies, decreasing order -- this must be an
		 	 * inplace selection sort, or a sort with the same (in)stability
		 	 * characteristics
		 	 */

			for (i = 0; i < model->entries - 1; i++)
			{
				for (j = i + 1; j < model->entries; j++)
				{
					if (model->syms[i].freq < model->syms[j].freq)
					{
						tmp = model->syms[i];
						model->syms[i] = model->syms[j];
						model->syms[j] = tmp;
					}
				}
			}

			/* then convert frequencies back to freq */

			for (i = model->entries - 1; i >= 0; i--)
				model->syms[i].freq += model->syms[i+1].freq;

			/* then update the other part of the table */

			for (i = 0; i < model->entries; i++)
				model->tabloc[model->syms[i].sym] = i;
		}
	}
}

static int
quantum_open(Codex_t* p, char* const args[], Codexnum_t flags)
{
	register State_t*	state;
	char*			s;
	char*			e;
	ULONG			window;
	ULONG			align;
	int			i;

	if (!(s = args[2]))
	{
		if (p->disc->errorf)
			(*p->disc->errorf)(NiL, p->disc, 2, "%s: window size parameter expected", args[0]);
		return -1;
	}
	window = strton(s, &e, NiL, 0);
	if (*e || (window & (window - 1)) || window < (1L<<10) || window > (1L<<21))
	{
		if (p->disc->errorf)
			(*p->disc->errorf)(NiL, p->disc, 2, "%s: %s: window size must be a power of 2 between 1k and 2m inclusive", args[0], s);
		return -1;
	}
	if (s = args[3])
	{
		align = strton(s, &e, NiL, 0);
		if (*e)
		{
			if (p->disc->errorf)
				(*p->disc->errorf)(NiL, p->disc, 2, "%s: %s: invalid block alignment", args[0], s);
			return -1;
		}
	}
	else
		align = window;
	if (!(state = newof(0, State_t, 1, window - 1)))
	{
		if (p->disc->errorf)
			(*p->disc->errorf)(NiL, p->disc, 2, "out of space");
		return -1;
	}
	state->window_size = window;
	state->align = align;
	for (i = 0; window >>= 1; i++);
	state->slots = i * 2;
	state->codex = p;
	p->data = state;
	return 0;
}

static int
quantum_init(Codex_t* p)
{
	register State_t*	state = (State_t*)p->data;

	state->bitcount = 0;
	state->eof = 0;
	state->ip = state->ie = 0;
	state->op = state->oe = 0;

	/* initialize arithmetic coding models */

	init(state, &state->mod07, &state->sym07[0], 7, 0);
	init(state, &state->mod00, &state->sym00[0], 0x40, 0x00);
	init(state, &state->mod40, &state->sym40[0], 0x40, 0x40);
	init(state, &state->mod80, &state->sym80[0], 0x40, 0x80);
	init(state, &state->modC0, &state->symC0[0], 0x40, 0xC0);
	init(state, &state->mod04, &state->sym04[0], (state->slots < 24) ? state->slots : 24, 0);
	init(state, &state->mod05, &state->sym05[0], (state->slots < 36) ? state->slots : 36, 0);
	init(state, &state->mod06p, &state->sym06p[0], state->slots, 0);
	init(state, &state->mod06l, &state->sym06l[0], 27, 0);

#if 0
sfprintf(sfstderr, "AHA INIT %d\n", 0);
#endif
	state->H = 0xFFFF;
	state->L = 0;
	state->C = GETBITS(state, 16);
#if 0
sfprintf(sfstderr, "AHA C %08x\n", state->C);
#endif

	return 0;
}

#define Z(p,m)		((p)->m.entries)
#define V(p,m,i)	((p)->m.syms[i].sym)
#define Q(p,m,i)	((p)->m.syms[i].freq)

/*
 * GETCODE(state,model,var) fetches the next symbol from the stated model
 * and puts it in var
 */

#define GETCODE(p,m,v)							\
	do								\
	{								\
		range=((p->H-p->L)&0xFFFF)+1;				\
		symf=((((p->C-p->L+1)*Q(p,m,0))-1)/range)&0xFFFF;	\
		for(i=1;i<Z(p,m)&&Q(p,m,i)>symf;i++);			\
		v=V(p,m,i-1);						\
		range=(p->H-p->L)+1;					\
		p->H=p->L+((Q(p,m,i-1)*range)/Q(p,m,0))-1;		\
		p->L=p->L+((Q(p,m,i)*range)/Q(p,m,0));			\
		for(;;)							\
		{							\
			if((p->L&0x8000)!=(p->H&0x8000))		\
			{						\
				if((p->L&0x4000)&&!(p->H&0x4000))	\
				{					\
					p->C^=0x4000;			\
					p->L&=0x3FFF;			\
					p->H|=0x4000;			\
				}					\
				else					\
					break;				\
			}						\
			p->L<<=1;p->H=(p->H<<1)|1;			\
			p->C=(p->C<<1)|GETBITS(p,1);			\
		}							\
		update(p,&(p->m),i);					\
	} while (0)

static ssize_t
decode(State_t* state)
{
	UBYTE*		window = state->window;
	register UBYTE*	p = window;
	register UBYTE*	e = p + state->window_size;
	register UBYTE*	a = p + state->align;
	register int	c;
	LONG		match_offset;
	int		match_length;
#if 0
int code;
#endif

	/* used by GETCODE */

	ULONG		range;
	UWORD		symf;
	int		i;

	for (;;)
	{
		if (p >= a)
		{
#if 0
sfprintf(sfstderr, "AHA INIT %d\n", state->bitcount & 7);
#endif
			a = p + state->align;
			if (i = state->bitcount & 7)
				SKIPBITS(state, i);
			while (!(i = GETBITS(state, 8)));
			state->C = (i << 8) | GETBITS(state, 8);
			state->H = 0xFFFF;
			state->L = 0;
#if 0
sfprintf(sfstderr, "AHA C %08x\n", state->C);
#endif
			continue;
		}
		if (p >= e || state->eof)
			break;
		GETCODE(state, mod07, c);
#if 0
sfprintf(sfstderr, "AHA CODE %u H=%04x L=%04x C=%04x\n", c, state->H, state->L, state->C);
code = c;
#endif
		switch (c)
		{
		case 0:
			GETCODE(state, mod00, c);
#if 0
sfprintf(sfstderr, "AHA code %4d %4d\n", code, c);
#endif
			*p++ = c;
			continue;
		case 1:
			GETCODE(state, mod40, c);
#if 0
sfprintf(sfstderr, "AHA code %4d %4d\n", code, c);
#endif
			*p++ = c;
			continue;
		case 2:
			GETCODE(state, mod80, c);
#if 0
sfprintf(sfstderr, "AHA code %4d %4d\n", code, c);
#endif
			*p++ = c;
			continue;
		case 3:
			GETCODE(state, modC0, c);
#if 0
sfprintf(sfstderr, "AHA code %4d %4d\n", code, c);
#endif
			*p++ = c;
			continue;
		case 4:
			/* fixed length 3 */
			GETCODE(state, mod04, c);
			match_offset = slot_position[c] + GETBITS(state, extra_bits[c]) + 1;
			match_length = 3;
			break;
		case 5:
			/* fixed length 4 */
			GETCODE(state, mod05, c);
			match_offset = slot_position[c] + GETBITS(state, extra_bits[c]) + 1;
			match_length = 4;
			break;
		case 6:
			/* variable length */
			GETCODE(state, mod06l, c);
			match_length = slot_size[c] + GETBITS(state, extra_size[c]) + 5;
			GETCODE(state, mod06p, c);
			match_offset = slot_position[c] + GETBITS(state, extra_bits[c]) + 1;
			break;
		default:
			if (state->codex->disc->errorf)
				(*state->codex->disc->errorf)(NiL, state->codex->disc, 2, "%d: corrupt encoding", c);
			return -1;
		}
#if 0
sfprintf(sfstderr, "AHA code %4d %4d %4d\n", code, match_length, match_offset);
#endif
		match_offset = (LONG)(p - window) - match_offset;
		if (match_offset < 0)
		{
			match_offset += state->window_size;
			while (match_length--)
			{
				*p++ = window[match_offset++];
				match_offset &= (state->window_size - 1);
			}
		}
		else
			while (match_length--)
				*p++ = window[match_offset++];
	}
	return (state->oe = p) - (state->op = window);
}

static ssize_t
quantum_read(Sfio_t* sp, void* buf, size_t n, Sfdisc_t* disc)
{
	register State_t*	state = (State_t*)CODEX(disc)->data;
	register UBYTE*		p = (UBYTE*)buf;
	register UBYTE*		e = p + n;
	register ssize_t	m;

	while (n = e - p)
	{
		if (!(m = state->oe - state->op) && (state->eof || (m = decode(state)) <= 0))
			break;
		if (m > n)
			m = n;
		memcpy(p, state->op, m);
		p += m;
		state->op += m;
	}
	return p - (UBYTE*)buf;
}

Codexmeth_t	codex_quantum =
{
	"quantum",
	"Microsoft cabinet archive quantum compression from cabextract 0.6."
	" The first parameter is the window size which must be a power"
	" of 2 between 1k and 2m inclusive. The second parameter is the"
	" optional block aligment; 32k is required for cabinet file"
	" decompression.",
	0,
	CODEX_DECODE|CODEX_COMPRESS,
	0,
	0,
	quantum_open,
	0,
	quantum_init,
	0,
	quantum_read,
	0,
	0,
	0,
	0,
	0,
	0,
	CODEXNEXT(codex_quantum_next)
};

CODEXLIB(&codex_quantum)
