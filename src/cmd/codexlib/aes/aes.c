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
 * codex aes crypt implementation
 * based on the GPL ccrypt implementation
 */

/* Copyright (C) 2000-2003 Peter Selinger.
   This file is part of ccrypt. It is free software and it is covered
   by the GNU general public license. See the file COPYING for details. */

/* NOTE: this library is copyrighted under the GNU General Public
   License, *not* under the GNU Library General Public License.  This
   means, among other things, that you cannot use this library in
   closed-source software. */

/* ccryptlib implements a stream cipher based on the block cipher
   Rijndael, the AES standard. */

/* derived from original source: rijndael-alg-ref.h   v2.0   August '99
 * Reference ANSI C code for NIST competition
 * authors: Paulo Barreto
 *          Vincent Rijmen
 */

#include <codex.h>
#include <tm.h>

#define encrypt		aes_encrypt
#define decrypt		aes_decrypt

typedef unsigned _ast_int1_t word8;
typedef unsigned _ast_int4_t word32;

#define BLK		32
#define WRD		sizeof(word32)
#define BUF		(BLK/WRD)

typedef word8 buf8[BUF][4];
typedef word8 (*ptr8)[4];

typedef word32 buf32[BUF];
typedef word32 (*ptr32);

#define MAGIC		magic[0]
#define MAGICLEN	4

static const char*	magic[] =
{
	"cx20",				/* codex encrypt/decrypt	*/
	"c051",				/* ccrypt decrypt		*/
};

#define MAXBITS		(BLK*WRD)
#define MAXROUNDS	14

typedef struct Roundkey_s
{
  int BC;
  int KC;
  int ROUNDS;
  int shift[2][4];
  word32 rk[(MAXROUNDS+1)*BUF];
} Roundkey_t;

typedef struct State_s
{
  Roundkey_t	rkk;  /* Roundkey_t = preprocessed key */
  Roundkey_t	ini;
  int		eof;
  int		iv;
  buf32		buf; /* current buffer; partly ciphertext, partly mask */

  word8*	cpy;
  word8*	cpyend;
  word8		cpybuf[2*BLK];

  word8*	prt;
  word8*	out;
  size_t	outsiz;
} State_t;

#include "tables.h"

/* rijndael.c - optimized version of the Rijndeal cipher */
/* $Id: rijndael.c,v 1.9 2003/08/25 18:06:20 selinger Exp $ */

/* derived from original source: rijndael-alg-ref.c   v2.0   August '99
 * Reference ANSI C code for NIST competition
 * authors: Paulo Barreto
 *          Vincent Rijmen
 */

static int xshifts[3][2][4] = {
  { { 0,1,2,3 },
    { 0,3,2,1 } },

  { { 0,1,2,3 },
    { 0,5,4,3 } },

  { { 0,1,3,4 },
    { 0,7,5,4 } },
};

/* Exor corresponding text input and round key input bytes */
/* the result is written to res, which can be the same as a */
static void xKeyAddition(buf8 res, buf8 a, buf32 rk, int BC) {
  int j;

  for(j = 0; j < BC; j++)
    *(word32 *)res[j] = *(word32 *)a[j] ^ rk[j];
}

/* shift rows a, return result in res. This avoids having to copy a
   tmp array back to a. res must not be a. */
static void xShiftRow(buf8 res, buf8 a, int shift[4], int BC)
{
  /* Row 0 remains unchanged
   * The other three rows are shifted a variable amount
   */
  int i, j;
  int s;

  for (j=0; j < BC; j++) {
    res[j][0] = a[j][0];
  }
  for(i = 1; i < 4; i++) {
    s = shift[i];
    for(j = 0; j < BC; j++)
      res[j][i] = a[(j + s) % BC][i];
  }
}

static void xSubstitution(buf8 a, word8 box[256], int BC) {
  /* Replace every byte of the input by the byte at that place
   * in the nonlinear S-box
   */
  int i, j;

  for(i = 0; i < 4; i++)
    for(j = 0; j < BC; j++)
      a[j][i] = box[a[j][i]] ;
}

/* profiling shows that the ccrypt program spends about 50% of its
   time in the function xShiftSubst. Splitting the inner "for"
   statement into two parts - versus using the expensive "%" modulo
   operation, makes this function about 44% faster, thereby making the
   entire program about 28% faster. With -O3 optimization, the time
   savings are even more dramatic - ccrypt runs between 55% and 65%
   faster on most platforms. */

/* do ShiftRow and Substitution together. res must not be a. */
static void xShiftSubst(buf8 res, buf8 a, int shift[4], int BC, const word8 box[256]) {
  int i, j;
  int s;

  for (j=0; j < BC; j++) {
    res[j][0] = box[a[j][0]];
  }
  for(i = 1; i < 4; i++) {
    s = shift[i];
    for(j = 0; j < BC-s; j++)
      res[j][i] = box[a[(j + s)][i]];
    for(j = BC-s; j < BC; j++)
      res[j][i] = box[a[(j + s) - BC][i]];
  }
}

/* Mix the four bytes of every column in a linear way */
/* the result is written to res, which may equal a */
static void xMixColumn(buf8 res, buf8 a, int BC) {
  int j;
  word32 b;

  for(j = 0; j < BC; j++) {
    b =  M0[0][a[j][0]];
    b ^= M0[1][a[j][1]];
    b ^= M0[2][a[j][2]];
    b ^= M0[3][a[j][3]];
    *(word32 *)res[j] = b;
  }
}

/* do MixColumn and KeyAddition together */
static void xMixAdd(buf8 res, buf8 a, buf32 rk, int BC) {
  int j;
  word32 b;

  for(j = 0; j < BC; j++) {
    b =  M0[0][a[j][0]];
    b ^= M0[1][a[j][1]];
    b ^= M0[2][a[j][2]];
    b ^= M0[3][a[j][3]];
    b ^= rk[j];
    *(word32 *)res[j] = b;
  }
}

/* Mix the four bytes of every column in a linear way
 * This is the opposite operation of xMixColumn */
/* the result is written to res, which may equal a */
static void xInvMixColumn(buf8 res, buf8 a, int BC) {
  int j;
  word32 b;

  for(j = 0; j < BC; j++) {
    b =  M1[0][a[j][0]];
    b ^= M1[1][a[j][1]];
    b ^= M1[2][a[j][2]];
    b ^= M1[3][a[j][3]];
    *(word32 *)res[j] = b;
  }
}

/* do KeyAddition and InvMixColumn together */
static void xAddInvMix(buf8 res, buf8 a, buf32 rk, int BC) {
  int j;
  word32 b;

  for(j = 0; j < BC; j++) {
    *(word32 *)a[j] = *(word32 *)a[j] ^ rk[j];
    b =  M1[0][a[j][0]];
    b ^= M1[1][a[j][1]];
    b ^= M1[2][a[j][2]];
    b ^= M1[3][a[j][3]];
    *(word32 *)res[j] = b;
  }
}

static int schedule(Roundkey_t *rkk, buf32 key, int keyBits, int blockBits) {
  /* Calculate the necessary round keys
   * The number of calculations depends on keyBits and blockBits */
  int KC, BC, ROUNDS;
  int i, j, t, rconpointer = 0;
  ptr8 k = (ptr8)key;

  switch (keyBits) {
  case 128: KC = 4; break;
  case 192: KC = 6; break;
  case 256: KC = 8; break;
  default : return -1;
  }

  switch (blockBits) {
  case 128: BC = 4; break;
  case 192: BC = 6; break;
  case 256: BC = 8; break;
  default : return -2;
  }

  ROUNDS = KC>BC ? KC+6 : BC+6;

  t = 0;
  /* copy values into round key array */
  for(j = 0; (j < KC) && (t < (ROUNDS+1)*BC); j++, t++)
    rkk->rk[t] = key[j];

  while (t < (ROUNDS+1)*BC) { /* while not enough round key material */
    /* calculate new values */
    for(i = 0; i < 4; i++)
      k[0][i] ^= xS[k[KC-1][(i+1)%4]];
    k[0][0] ^= xrcon[rconpointer++];

    if (KC != 8)
      for(j = 1; j < KC; j++)
	key[j] ^= key[j-1];
    else {
      for(j = 1; j < 4; j++)
	key[j] ^= key[j-1];
      for(i = 0; i < 4; i++)
	k[4][i] ^= xS[k[3][i]];
      for(j = 5; j < 8; j++)
	key[j] ^= key[j-1];
    }
    /* copy values into round key array */
    for(j = 0; (j < KC) && (t < (ROUNDS+1)*BC); j++, t++)
      rkk->rk[t] = key[j];
  }

  /* make Roundkey_t structure */
  rkk->BC = BC;
  rkk->KC = KC;
  rkk->ROUNDS = ROUNDS;
  for (i=0; i<2; i++)
    for (j=0; j<4; j++)
      rkk->shift[i][j] = xshifts[(BC-4) >> 1][i][j];

  return 0;
}

/* Encryption of one block. */

static void encrypt(Roundkey_t *rkk, buf32 block)
{
  ptr8 a = (ptr8)block;
  buf8 t; /* hold intermediate result */
  int r;

  int *shift = rkk->shift[0];
  int BC = rkk->BC;
  int ROUNDS = rkk->ROUNDS;
  word32 *rp = rkk->rk;

  /* begin with a key addition */
  xKeyAddition(a,a,rp,BC);
  rp += BC;

  /* ROUNDS-1 ordinary rounds */
  for(r = 1; r < ROUNDS; r++) {
    xShiftSubst(t,a,shift,BC,xS);
    xMixAdd(a,t,rp,BC);
    rp += BC;
  }

  /* Last round is special: there is no xMixColumn */
  xShiftSubst(t,a,shift,BC,xS);
  xKeyAddition(a,t,rp,BC);
}

static void decrypt(Roundkey_t *rkk, buf32 block)
{
  ptr8 a = (ptr8)block;
  buf8 t; /* Hold intermediate result */
  int r;

  int *shift = rkk->shift[1];
  int BC = rkk->BC;
  int ROUNDS = rkk->ROUNDS;
  word32 *rp = rkk->rk + ROUNDS*BC;

  /* To decrypt: apply the inverse operations of the encrypt routine,
   *             in opposite order
   *
   * (xKeyAddition is an involution: it 's equal to its inverse)
   * (the inverse of xSubstitution with table S is xSubstitution with the
   * inverse table of S)
   * (the inverse of xShiftRow is xShiftRow over a suitable distance)
   */

  /* First the special round:
   *   without xInvMixColumn
   *   with extra xKeyAddition
   */
  xKeyAddition(t,a,rp,BC);
  xShiftSubst(a,t,shift,BC,xSi);
  rp -= BC;

  /* ROUNDS-1 ordinary rounds
   */
  for(r = ROUNDS-1; r > 0; r--) {
    xKeyAddition(a,a,rp,BC);
    xInvMixColumn(t,a,BC);
    xShiftSubst(a,t,shift,BC,xSi);
    rp -= BC;
  }

  /* End with the extra key addition
   */

  xKeyAddition(a,a,rp,BC);
}

/* ---------------------------------------------------------------------- */
/* some private functions dealing with hashes, keys, nonces */

/* hash a keystring into a 256-bit cryptographic random value. */
static void hash(char *keystring, buf32 hash) {
  int i;
  Roundkey_t rkk;
  buf32 key;      /* rijndael key */

  for (i=0; i<BUF; i++)
    key[i] = hash[i] = 0;

  do {
    for (i=0; i<32; i++) {
      if (*keystring != 0) {
	((word8 *)key)[i] ^= *keystring;
	*keystring++ = 0;
      }
    }
    schedule(&rkk, key, 256, 256);
    encrypt(&rkk, hash);
  } while (*keystring != 0);
}

/* return a 256-bit value that is practically unique */
static void nonce(buf32 buf) {
  char acc[512], host[256];
  struct timeval tv;
  static unsigned int count=0;

  gethostname(host, sizeof(host));
  tmtimeofday(&tv);
  sfsprintf(acc, sizeof(acc), "%-.*s,%lu,%lu,%lu,%p,%u", sizeof(host), host, (unsigned long)tv.tv_sec, (unsigned long)tv.tv_usec, (unsigned long)getpid(), malloc(0), count++);
  hash(acc, buf);
}

/* ---------------------------------------------------------------------- */

static int
aes_open(Codex_t* p, char* const args[], Codexnum_t flags)
{
	register State_t*	state;
	int			bits;
	char*			e;
	buf32			keyblock;
	char			pp[256];

	if (args[2])
	{
		bits = (int)strtol(args[2], &e, 10);
		if (*e || bits != 128 && bits != 192 && bits != 256)
		{
			if (p->disc->errorf)
				(*p->disc->errorf)(NiL, p->disc, 2, "%s: valid cipher bits are { 128 192 256 }", args[0]);
			return -1;
		}
	}
	else
		bits = 256;
	if (codexpass(pp, sizeof(pp), p->disc, p->meth) < 0)
		return -1;
	if (!(state = newof(0, State_t, 1, 0)))
	{
		if (p->disc->errorf)
			(*p->disc->errorf)(NiL, p->disc, 2, "out of space");
		return -1;
	}
  	hash(pp, keyblock);
	schedule(&state->rkk, keyblock, bits, bits);
	state->ini = state->rkk;
	p->data = state;
	return 0;
}

static int
aes_close(Codex_t* p)
{
	register State_t*	state = (State_t*)p->data;

	if (!state)
		return -1;
	if (state->buf)
		free(state->buf);
	free(state);
	return 0;
}

static int
aes_init(Codex_t* p)
{
	register State_t*	state = (State_t*)p->data;
	ssize_t			r;
	int			i;
	buf32			loc;

	state->rkk = state->ini;
	state->cpy = state->cpyend = 0;
	state->prt = 0;
	state->eof = 0;
	state->iv = BLK;
	if (p->flags & CODEX_DECODE)
	{
		if ((r = sfrd(p->sp, state->buf, sizeof(state->buf), &p->sfdisc)) == sizeof(state->buf))
		{
			memcpy(loc, state->buf, sizeof(loc));
			decrypt(&state->rkk, loc);
			for (i = 0; memcmp(loc, magic[i], MAGICLEN);)
				if (++i >= elementsof(magic))
				{
					if (p->disc->errorf)
						(*p->disc->errorf)(NiL, p->disc, 2, "encryption magic mismatch");
					return -1;
				}
		}
		else if (r)
		{
			if (p->disc->errorf)
				(*p->disc->errorf)(NiL, p->disc, 2, "truncated encryption header");
			return -1;
		}
	}
	else
	{
		if (!state->out)
		{
			state->outsiz = roundof(BLK, 1024);
			if (!(state->out = newof(state->out, word8, state->outsiz, BLK)))
			{
				if (p->disc->errorf)
					(*p->disc->errorf)(NiL, p->disc, 2, "out of space");
				return -1;
			}
		}
		nonce(state->buf);
		memcpy(state->buf, MAGIC, MAGICLEN);
		encrypt(&state->rkk, state->buf);
		memcpy(state->out, state->buf, BLK);
	}
	return 0;
}

static int
aes_done(Codex_t* p)
{
	register State_t*	state = (State_t*)p->data;
	register word8*		b;
	register int		i;
	size_t			n;

	if (!state->eof)
	{
		state->eof = 1;
		if (!state->prt)
			return 0;
		encrypt(&state->rkk, state->buf);
		n = state->prt - state->out;
		state->prt = 0;
		b = (word8*)state->buf;
		for (i = 0; i < n; i++)
			state->out[i] ^= b[i];
		if (sfwr(p->sp, state->out, n, &p->sfdisc) != n)
			return -1;
	}
	return 0;
}

static ssize_t
aes_read(Sfio_t* sp, void* buf, size_t n, Sfdisc_t* disc)
{
	register State_t*	state = (State_t*)CODEX(disc)->data;
	register word8*		s = (word8*)buf;
	register word8*		e = s + n;
	register word8*		t = s;
	register int		i;
	ssize_t			r;
	buf32			loc;

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
	while ((s - t) >= BLK)
	{
		encrypt(&state->rkk, state->buf);
		memcpy(loc, state->buf, BLK);
		memcpy(state->buf, t, BLK);
		for (i = 0; i < BUF; i++)
			loc[i] ^= state->buf[i];
		memcpy(t, loc, BLK);
		t += BLK;
	}
	r = t - (word8*)buf;
	if (n = s - t)
	{
		if (state->eof)
		{
			encrypt(&state->rkk, state->buf);
			s = (word8*)state->buf;
			for (i = 0; i < n; i++)
				t[i] ^= s[i];
			r += n;
		}
		else
		{
			memcpy(state->cpy = state->cpybuf, t, n);
			state->cpyend = state->cpy + n;
		}
	}
	return r;
}

static ssize_t
aes_write(Sfio_t* sp, const void* buf, size_t n, Sfdisc_t* disc)
{
	register State_t*	state = (State_t*)CODEX(disc)->data;
	register word8*		s = (word8*)buf;
	register word8*		e = s + n;
	register word8*		t;
	register int		i;
	size_t			m;
	buf32			loc;

	if (n > state->outsiz)
	{
		state->outsiz = roundof(n, 1024);
		if (!(state->out = newof(state->out, word8, state->outsiz, BLK)))
		{
			if (CODEX(disc)->disc->errorf)
				(*CODEX(disc)->disc->errorf)(NiL, CODEX(disc)->disc, 2, "out of space");
			return -1;
		}
	}
	if (t = state->prt)
	{
		while ((t - state->out) < BLK)
		{
			if (s >= e)
			{
				state->prt = t;
				return s - (word8*)buf;
			}
			*t++ = *s++;
		}
		state->prt = 0;
		encrypt(&state->rkk, state->buf);
		memcpy(loc, state->out, BLK);
		for (i = 0; i < BUF; i++)
			state->buf[i] ^= loc[i];
		memcpy(state->out, state->buf, BLK);
		t = state->out + BLK;
	}
	else
	{
		t = state->out + state->iv;
		state->iv = 0;
	}
	while ((e - s) >= BLK)
	{
		encrypt(&state->rkk, state->buf);
		memcpy(loc, s, BLK);
		for (i = 0; i < BUF; i++)
			state->buf[i] ^= loc[i];
		memcpy(t, state->buf, BLK);
		s += BLK;
		t += BLK;
	}
	if ((m = t - state->out) && sfwr(sp, state->out, m, disc) < 0)
		return -1;
	if (m = e - s)
	{
		memcpy(state->out, s, m);
		state->prt = state->out + m;
	}
	return n;
}

Codexmeth_t	codex_aes =
{
	"aes",
	"AES (FIPS-197 rijndael) encryption. The first option is the block"
	" size in bits. Sizes { 128 192 256 } are supported.",
	"[+(author)?Peter Selinger (ccrypt)]"
	"[+(license)?GPL]",
	CODEX_DECODE|CODEX_ENCODE|CODEX_CRYPT,
	0,
	0,
	aes_open,
	aes_close,
	aes_init,
	aes_done,
	aes_read,
	aes_write,
	aes_done,
	0,
	0,
	0,
	0,
	CODEXNEXT(codex_aes_next)
};

CODEXLIB(&codex_aes)
