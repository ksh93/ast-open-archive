#pragma prototyped noticed

/*
 * cabinet lzx decoder
 * based on GPL code from cabextract
 *
 * decoder adapted from the LZX method by Jonathan Forbes and Tomi Poutanen
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
#include <ctype.h>

/* some constants defined by the LZX specification */
#define LZX_MIN_MATCH                (2)
#define LZX_MAX_MATCH                (257)
#define LZX_NUM_CHARS                (256)
#define LZX_PRETREE_NUM_ELEMENTS     (20)
#define LZX_ALIGNED_NUM_ELEMENTS     (8)   /* aligned offset tree #elements */
#define LZX_NUM_PRIMARY_LENGTHS      (7)   /* this one missing from spec! */
#define LZX_NUM_SECONDARY_LENGTHS    (249) /* length tree #elements */

/* LZX huffman defines: tweak tablebits as desired */
#define LZX_PRETREE_MAXSYMBOLS  (LZX_PRETREE_NUM_ELEMENTS)
#define LZX_PRETREE_TABLEBITS   (6)
#define LZX_MAINTREE_MAXSYMBOLS (LZX_NUM_CHARS + 50*8)
#define LZX_MAINTREE_TABLEBITS  (12)
#define LZX_LENGTH_MAXSYMBOLS   (LZX_NUM_SECONDARY_LENGTHS+1)
#define LZX_LENGTH_TABLEBITS    (12)
#define LZX_ALIGNED_MAXSYMBOLS  (LZX_ALIGNED_NUM_ELEMENTS)
#define LZX_ALIGNED_TABLEBITS   (7)

#define LZX_LENTABLE_SAFETY (64) /* we allow length table decoding overruns */

#define LZX_DECLARE_TABLE(tbl) \
  UWORD tbl##_table[(1<<LZX_##tbl##_TABLEBITS) + (LZX_##tbl##_MAXSYMBOLS<<1)];\
  UBYTE tbl##_len  [LZX_##tbl##_MAXSYMBOLS + LZX_LENTABLE_SAFETY]

typedef unsigned _ast_int1_t  UBYTE; /* 8 bits exactly    */
typedef unsigned _ast_int2_t  UWORD; /* 16 bits (or more) */
typedef unsigned _ast_int4_t  ULONG; /* 32 bits exactly   */
typedef          _ast_int4_t   LONG; /* 32 bits (or more) */

typedef struct State_s
{
    Codex_t*	codex;

    UBYTE	buf[SF_BUFSIZE];
    UBYTE*	ip;
    UBYTE*	ie;

    ULONG	bitbuf;
    UWORD	bitcount;

    ULONG	align;
    int		eof;

    ULONG window_size;     /* window size (32Kb through 2Mb)          */
    ULONG R0, R1, R2;      /* for the LRU offset system               */
    UWORD main_elements;   /* number of main tree elements            */
    UWORD block_type;      /* type of this block                      */
    ULONG block_length;    /* uncompressed length of this block       */
    UBYTE*block_end;       /* block_end in window                     */
    ULONG frames;          /* the number of relocate blocks processed */
    ULONG filesize;  	   /* magic header value used for transform   */
    ULONG offset;          /* current relocation offset               */
    int   relocate;        /* apply relocation transform              */
    int   slots;
    UBYTE*rel;

    LZX_DECLARE_TABLE(PRETREE);
    LZX_DECLARE_TABLE(MAINTREE);
    LZX_DECLARE_TABLE(LENGTH);
    LZX_DECLARE_TABLE(ALIGNED);

    ULONG position_base[51];
    UBYTE extra_bits[51];

    UBYTE* op;
    UBYTE* oe;
    UBYTE  window[1];         /* the actual decoding window              */
} State_t;

/* Microsoft's LZX document and their implementation of the
 * com.ms.util.cab Java package do not concur.
 *
 * In the LZX document, there is a table showing the correlation between
 * window size and the number of position slots. It states that the 1MB
 * window = 40 slots and the 2MB window = 42 slots. In the implementation,
 * 1MB = 42 slots, 2MB = 50 slots. The actual calculation is 'find the
 * first slot whose position base is equal to or more than the required
 * window size'. This would explain why other tables in the document refer
 * to 50 slots rather than 42.
 *
 * The constant NUM_PRIMARY_LENGTHS used in the decompression pseudocode
 * is not defined in the specification.
 *
 * The LZX document does not state the uncompressed block has an
 * uncompressed length field. Where does this length field come from, so
 * we can know how large the block is? The implementation has it as the 24
 * bits following after the 3 blocktype bits, before the alignment
 * padding.
 *
 * The LZX document states that aligned offset blocks have their aligned
 * offset huffman tree AFTER the main and length trees. The implementation
 * suggests that the aligned offset tree is BEFORE the main and length
 * trees.
 *
 * The LZX document decoding algorithm states that, in an aligned offset
 * block, if an extra_bits value is 1, 2 or 3, then that number of bits
 * should be read and the result added to the match offset. This is
 * correct for 1 and 2, but not 3, where just a huffman symbol (using the
 * aligned tree) should be read.
 *
 * Regarding the E8 preprocessing, the LZX document states 'No translation
 * may be performed on the last 6 bytes of the input block'. This is
 * correct.  However, the pseudocode provided checks for the *E8 leader*
 * up to the last 6 bytes. If the leader appears between -10 and -7 bytes
 * from the end, this would cause the next four bytes to be modified, at
 * least one of which would be in the last 6 bytes, which is not allowed
 * according to the spec.
 *
 * The specification states that the huffman trees must always contain at
 * least one element. However, many CAB files contain blocks where the
 * length tree is completely empty (because there are no matches), and
 * this is expected to succeed.
 */

/* LZX uses what it calls 'position slots' to represent match offsets.
 * What this means is that a small 'position slot' number and a small
 * offset from that slot are encoded instead of one large offset for
 * every match.
 * - state->position_base is an index to the position slot bases
 * - lzx_extra_bits states how many bits of offset-from-base data is needed.
 */

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

#define GETBITS(p,n)	((n)<=(p)->bitcount ? ((((p)->bitbuf)>>((p)->bitcount-=(n)))&((1L<<(n))-1)) : getbits(p,n,0))
#define PEEKBITS(p,n)	((n)<=(p)->bitcount ? ((((p)->bitbuf)>>((p)->bitcount-(n)))&((1L<<(n))-1)) : getbits(p,n,1))
#define SKIPBITS(p,n)	((p)->bitcount-=(n))

/*
 * microsoft does this different from all the other (n-1) lz variants
 */

static int
getbits(register State_t* state, int n, int peek)
{
	while (state->bitcount < n)
	{
		state->bitbuf <<= 16;
		state->bitbuf |= GETCHAR(state);
		state->bitbuf |= GETCHAR(state) << 8;
		state->bitcount += 16;
	}
	if (peek)
	{
		state->eof = 0;
		return PEEKBITS(state, n);
	}
	return GETBITS(state, n);
}

/* Huffman macros */

#define TABLEBITS(tbl)   (LZX_##tbl##_TABLEBITS)
#define MAXSYMBOLS(tbl)  (LZX_##tbl##_MAXSYMBOLS)
#define SYMTABLE(tbl)    (state->tbl##_table)
#define LENTABLE(tbl)    (state->tbl##_len)

/* BUILD_TABLE(tablename) builds a huffman lookup table from code lengths.
 * In reality, it just calls make_decode_table() with the appropriate
 * values - they're all fixed by some #defines anyway, so there's no point
 * writing each call out in full by hand.
 */
#define BUILD_TABLE(state,tbl)						\
  make_decode_table(state,						\
    MAXSYMBOLS(tbl), TABLEBITS(tbl), LENTABLE(tbl), SYMTABLE(tbl)	\
  )


/* READ_HUFFSYM(tablename, var) decodes one huffman symbol from the
 * bitstream using the stated table and puts it in var.
 */
#define READ_HUFFSYM(p,tbl,var) do {					\
  UWORD* hufftbl = SYMTABLE(tbl);					\
  ULONG k = PEEKBITS(p,16);						\
  if ((i = hufftbl[k >> (16 - TABLEBITS(tbl))]) >= MAXSYMBOLS(tbl)) {	\
    j = 1 << (16 - TABLEBITS(tbl));					\
    do {								\
      i <<= 1;								\
      if (k & (j >>= 1))						\
      	i |= 1;								\
    } while ((i = hufftbl[i]) >= MAXSYMBOLS(tbl));			\
  }									\
  j = LENTABLE(tbl)[(var) = i];						\
  SKIPBITS(p,j);							\
} while (0)


/* READ_LENGTHS(tablename, first, last) reads in code lengths for symbols
 * first to last in the given table. The code lengths are stored in their
 * own special LZX way.
 */
#define READ_LENGTHS(state,tbl,first,last) \
  lzx_read_lens(state,LENTABLE(tbl),(first),(last))

/* make_decode_table(nsyms, nbits, length[], table[])
 *
 * This function was coded by David Tritscher. It builds a fast huffman
 * decoding table out of just a canonical huffman code lengths table.
 *
 * nsyms  = total number of symbols in this huffman tree.
 * nbits  = any symbols with a code length of nbits or less can be decoded
 *          in one lookup of the table.
 * length = A table to get code lengths from [0 to syms-1]
 * table  = The table to fill up with decoded symbols and pointers.
 *
 * Returns 0 for OK or 1 for error
 */

static int make_decode_table(State_t* state, ULONG nsyms, ULONG nbits, UBYTE *length, UWORD *table) {
  register UWORD sym;
  register ULONG leaf;
  register UBYTE bit_num = 1;
  ULONG fill;
  ULONG pos         = 0; /* the current position in the decode table */
  ULONG table_mask  = 1 << nbits;
  ULONG bit_mask    = table_mask >> 1; /* don't do 0 length codes */
  ULONG next_symbol = bit_mask; /* base of allocation for long codes */

  /* fill entries for codes short enough for a direct mapping */
  while (bit_num <= nbits) {
    for (sym = 0; sym < nsyms; sym++) {
      if (length[sym] == bit_num) {
        leaf = pos;

        if((pos += bit_mask) > table_mask)
	{
	  if (state->codex->disc->errorf)
	    (*state->codex->disc->errorf)(NiL, state->codex->disc, 2, "decode table overflow");
	  return -1;
	}

        /* fill all possible lookups of this symbol with the symbol itself */
        fill = bit_mask;
        while (fill-- > 0) table[leaf++] = sym;
      }
    }
    bit_mask >>= 1;
    bit_num++;
  }

  /* if there are any codes longer than nbits */
  if (pos != table_mask) {
    /* clear the remainder of the table */
    for (sym = pos; sym < table_mask; sym++) table[sym] = 0;

    /* give ourselves room for codes to grow by up to 16 more bits */
    pos <<= 16;
    table_mask <<= 16;
    bit_mask = 1 << 15;

    while (bit_num <= 16) {
      for (sym = 0; sym < nsyms; sym++) {
        if (length[sym] == bit_num) {
          leaf = pos >> 16;
          for (fill = 0; fill < bit_num - nbits; fill++) {
            /* if this path hasn't been taken yet, 'allocate' two entries */
            if (table[leaf] == 0) {
              table[(next_symbol << 1)] = 0;
              table[(next_symbol << 1) + 1] = 0;
              table[leaf] = next_symbol++;
            }
            /* follow the path and select either left or right for next bit */
            leaf = table[leaf] << 1;
            if ((pos >> (15-fill)) & 1) leaf++;
          }
          table[leaf] = sym;

          if ((pos += bit_mask) > table_mask)
	  {
	    if (state->codex->disc->errorf)
	      (*state->codex->disc->errorf)(NiL, state->codex->disc, 2, "decode table overflow #2");
	    return -1;
	  }
        }
      }
      bit_mask >>= 1;
      bit_num++;
    }
  }
  if (pos != table_mask)
    for (sym = 0; sym < nsyms; sym++)
      if (length[sym])
      {
	if (state->codex->disc->errorf)
	  (*state->codex->disc->errorf)(NiL, state->codex->disc, 2, "decode table corrupt");
	return -1;
      }
  return 0;
}

static int
lzx_read_lens(State_t* state, UBYTE *lens, ULONG first, ULONG last)
{
  ULONG i,j, x,y;
  int z;

  for (x = 0; x < 20; x++)
    LENTABLE(PRETREE)[x] = GETBITS(state, 4);
  if (BUILD_TABLE(state, PRETREE))
    return -1;
  for (x = first; x < last; ) {
    READ_HUFFSYM(state, PRETREE, z);
    if (z == 17) {
      y = GETBITS(state, 4) + 4;
      while (y--) lens[x++] = 0;
    }
    else if (z == 18) {
      y = GETBITS(state, 5) + 20;
      while (y--) lens[x++] = 0;
    }
    else if (z == 19) {
      y = GETBITS(state, 1) + 4;
      READ_HUFFSYM(state, PRETREE, z);
      z = lens[x] - z; if (z < 0) z += 17;
      while (y--) lens[x++] = z;
    }
    else {
      z = lens[x] - z; if (z < 0) z += 17;
      lens[x++] = z;
    }
  }
  return 0;
}

static int
lzx_open(Codex_t* p, char* const args[], Codexnum_t flags)
{
	register State_t*	state;
	char*			s;
	char*			e;
	ULONG			window;
	ULONG			align;
	int			i;
	int			j;

	if (!(s = args[2]))
	{
		if (p->disc->errorf)
			(*p->disc->errorf)(NiL, p->disc, 2, "%s: window size parameter expected", args[0]);
		return -1;
	}
	window = strton(s, &e, NiL, 0);
	if (*e || (window & (window - 1)) || window < (1L<<15) || window > (1L<<21))
	{
		if (p->disc->errorf)
			(*p->disc->errorf)(NiL, p->disc, 2, "%s: %s: window size must be a power of 2 between 32k and 2m inclusive", args[0], s);
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

	/* initialize static tables */

	for (i = j = 0; i <= 50; i += 2)
	{
		/* 0,0,0,0,1,1,2,2,3,3... */
		state->extra_bits[i] = state->extra_bits[i+1] = j;
		/* 0,0,1,2,3,4...15,16,17,17,17,17... */
		if ((i != 0) && (j < 17))
			j++;
	}
	for (i = j = 0; i <= 50; i++)
	{
		/* 0,1,2,3,4,6,8,12,16,24,32,... */
		state->position_base[i] = j;
		/* 1,1,1,1,2,2,4,4,8,8,16,16,32,32,... */
		j += 1 << state->extra_bits[i];
	}
	if (window == (1L<<21))
		i = 50;
	else if (window == (1L<<20))
		i = 42;
	else
	{
		for (i = 15; window > (1L << i); i++);
		i <<= 1;
	}
	state->slots = i;
	state->codex = p;
	p->data = state;
	return 0;
}

static int
lzx_close(Codex_t* p)
{
	register State_t*	state = (State_t*)p->data;

	if (!state)
		return 0;
	if (state->rel)
		free(state->rel);
	free(state);
	return 0;
}

static int
lzx_init(Codex_t* p)
{
	register State_t*	state = (State_t*)p->data;
	register int		i;

	state->bitcount = 0;
	state->eof = 0;
	state->ip = state->ie = 0;
	state->op = state->oe = 0;
	state->block_end = state->window;
	state->R0 = state->R1 = state->R2 = 1;
	state->main_elements = LZX_NUM_CHARS + (state->slots << 3);
	state->frames = 0;
	state->block_type = 0;
	state->offset = 0;
	state->relocate = 0;
	for (i = 0; i < LZX_MAINTREE_MAXSYMBOLS; i++)
		state->MAINTREE_len[i] = 0;
	for (i = 0; i < LZX_LENGTH_MAXSYMBOLS; i++)
		state->LENGTH_len[i] = 0;
	if (GETBITS(state, 1))
	{
		state->filesize = GETBITS(state, 16) << 16;
		state->filesize |= GETBITS(state, 16);
	}
	else
		state->filesize = 0;
	return 0;
}

/*
 * restore relocation offsets munged for better compression
 * is this really worth the effort?
 */

static int
relocate(register State_t* state)
{
	register UBYTE*	p;
	register UBYTE*	e;
	register UBYTE*	x;
	LONG		cur_off = state->offset;
	LONG		filesize = state->filesize;
	LONG		abs_off;
	LONG		rel_off;

	if (!state->rel && !(state->rel = newof(0, UBYTE, state->window_size, 0)))
	{
		if (state->codex->disc->errorf)
			(*state->codex->disc->errorf)(NiL, state->codex->disc, 2, "out of space -- cannot relocate");
		return -1;
	}
	rel_off = state->oe - state->op;
	p = state->rel;
	x = p + rel_off - 6;
	memcpy(p, state->op, rel_off);
	state->op = p;
	state->oe = p + rel_off;
	while (p < x)
	{
		if ((e = p + 32768 - 6) > x)
			e = x;
		while (p < e)
		{
			if (*p++ == 0xE8)
			{
				abs_off = p[0] | (p[1]<<8) | (p[2]<<16) | (p[3]<<24);
				if ((abs_off >= -cur_off) && (abs_off < filesize))
				{
					rel_off = (abs_off >= 0) ? (abs_off - cur_off) : (abs_off + filesize);
					p[0] = (UBYTE) rel_off;
					p[1] = (UBYTE) (rel_off >> 8);
					p[2] = (UBYTE) (rel_off >> 16);
					p[3] = (UBYTE) (rel_off >> 24);
				}
				p += 4;
				cur_off += 5;
			}
			else
				cur_off++;
		}
		p = e + 6;
		cur_off = roundof(cur_off - 1, 32768);
		if (++state->frames >= 32768)
		{
			state->filesize = 0;
			break;
		}
	}
	return 0;
}

static ssize_t
decode(register State_t* state)
{
	UBYTE*			window = state->window;
	register UBYTE*		p = window;
	register UBYTE*		e = p + state->window_size;
	register UBYTE*		b;
	UBYTE*			bb = state->block_end;
	UBYTE*			ab = p + state->align;
	register int		c;

	ULONG R0 = state->R0;
	ULONG R1 = state->R1;
	ULONG R2 = state->R2;
	ULONG i,j; /* ij used in READ_HUFFSYM macro */
	LONG match_offset;
	int aligned_bits;
	int match_length, length_footer, extra, verbatim_bits;

	b = (ab < bb) ? ab : bb;
	for (;;)
	{
		if (p >= b)
		{
			if (p >= ab)
			{
				ab = p + state->align;
				b = (ab < bb) ? ab : bb;
				if (state->bitcount < 16)
					state->bitcount = 0;
				else if (i = state->bitcount - 16)
					SKIPBITS(state, i);
				continue;
			}

			/* new block */

			if (state->block_type == 3)
			{
				/* realign bitstream to word */

				if (state->block_length & 1)
					GETCHAR(state);
				state->bitcount = 0;
			}
			state->block_type = GETBITS(state, 3);
			if (!state->block_type)
				goto done;
			i = GETBITS(state, 16);
			j = GETBITS(state, 8);
			bb = state->block_end = p + (state->block_length = (i << 8) | j);
			b = (ab < bb) ? ab : bb;
			switch (state->block_type)
			{
			case 2:
				for (i = 0; i < 8; i++)
					LENTABLE(ALIGNED)[i] = GETBITS(state, 3);
				if (BUILD_TABLE(state, ALIGNED))
					return -1;
				/*FALLTHROUGH*/
			case 1:
				if (READ_LENGTHS(state, MAINTREE, 0, 256))
					return -1;
				if (READ_LENGTHS(state, MAINTREE, 256, state->main_elements))
					return -1;
				if (BUILD_TABLE(state, MAINTREE))
					return -1;
				if (LENTABLE(MAINTREE)[0xE8] != 0)
					state->relocate = 1;
				if (READ_LENGTHS(state, LENGTH, 0, LZX_NUM_SECONDARY_LENGTHS))
					return -1;
				if (BUILD_TABLE(state, LENGTH))
					return -1;
				break;
			case 3:
				state->relocate = 1; /* because we can't assume otherwzse */
				PEEKBITS(state, 16); /* get up to 16 pad bits into the buffer */
				if (state->bitcount > 16)
					state->ip -= 2; /* and align the bitstream! */
				R0 = GETCHAR(state)|(GETCHAR(state)<<8)|(GETCHAR(state)<<16)|(GETCHAR(state)<<24);
				R1 = GETCHAR(state)|(GETCHAR(state)<<8)|(GETCHAR(state)<<16)|(GETCHAR(state)<<24);
				R2 = GETCHAR(state)|(GETCHAR(state)<<8)|(GETCHAR(state)<<16)|(GETCHAR(state)<<24);
				break;
			case 0:
				if (state->block_length == 0)
					goto done;
				/*FALLTHROUGH*/
			default:
				if (state->codex->disc->errorf)
					(*state->codex->disc->errorf)(NiL, state->codex->disc, 2, "%u: invalid block type", state->block_type);
				return -1;
			}
		}
		if (p >= e)
		{
			state->block_end = window + (state->block_end - p);
			break;
		}
		if (state->eof)
			break;
		switch (state->block_type)
		{
		case 1:
			while (p < b)
			{
				READ_HUFFSYM(state, MAINTREE, c);
				if (c < LZX_NUM_CHARS)
				{
					/* literal: 0 to LZX_NUM_CHARS-1 */

					*p++ = c;
					continue;
				}

				/* match: LZX_NUM_CHARS + ((slot<<3) | length_header (3 bits)) */

				c -= LZX_NUM_CHARS;
				match_length = c & LZX_NUM_PRIMARY_LENGTHS;
				if (match_length == LZX_NUM_PRIMARY_LENGTHS)
				{
					READ_HUFFSYM(state, LENGTH, length_footer);
					match_length += length_footer;
				}
				match_length += LZX_MIN_MATCH;
				match_offset = c >> 3;
				if (match_offset > 2)
				{
					/* not repeated offset */

					if (match_offset != 3)
					{
						extra = state->extra_bits[match_offset];
						verbatim_bits = GETBITS(state, extra);
						match_offset = state->position_base[match_offset] - 2 + verbatim_bits;
					}
					else
						match_offset = 1;

					/* update repeated offset LRU queue */

					R2 = R1;
					R1 = R0;
					R0 = match_offset;
				}
				else if (match_offset == 0)
					match_offset = R0;
				else if (match_offset == 1)
				{
					match_offset = R1;
					R1 = R0;
					R0 = match_offset;
				}
				else
				{
					/* match_offset == 2 */

					match_offset = R2;
					R2 = R0;
					R0 = match_offset;
				}
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
			break;
		case 2:
			while (p < b)
			{
				READ_HUFFSYM(state, MAINTREE, c);
				if (c < LZX_NUM_CHARS)
				{
					/* literal: 0 to LZX_NUM_CHARS-1 */

					*p++ = c;
					continue;
				}

				/* match: LZX_NUM_CHARS + ((slot<<3) | length_header (3 bits)) */

				c -= LZX_NUM_CHARS;
				match_length = c & LZX_NUM_PRIMARY_LENGTHS;
				if (match_length == LZX_NUM_PRIMARY_LENGTHS)
				{
					READ_HUFFSYM(state, LENGTH, length_footer);
					match_length += length_footer;
				}
				match_length += LZX_MIN_MATCH;
				match_offset = c >> 3;
				if (match_offset > 2)
				{
					/* not repeated offset */

					extra = state->extra_bits[match_offset];
					match_offset = state->position_base[match_offset] - 2;
					if (extra > 3)
					{
						/* verbatim and aligned bits */

						extra -= 3;
						match_offset += GETBITS(state, extra) << 3;
						READ_HUFFSYM(state, ALIGNED, aligned_bits);
						match_offset += aligned_bits;
					}
					else if (extra == 3)
					{
						/* aligned bits only */
						READ_HUFFSYM(state, ALIGNED, aligned_bits);
						match_offset += aligned_bits;
					}
					else if (extra > 0)
					{
						/* extra==1, extra==2 */
						/* verbatim bits only */
						match_offset += GETBITS(state, extra);
					}
					else /* extra == 0 */
					{
						/* ??? */
						match_offset = 1;
					}
	
					/* update repeated offset LRU queue */
					R2 = R1;
					R1 = R0;
					R0 = match_offset;
				}
				else if (match_offset == 0)
					match_offset = R0;
				else if (match_offset == 1)
				{
					match_offset = R1;
					R1 = R0;
					R0 = match_offset;
				}
				else /* match_offset == 2 */
				{
					match_offset = R2;
					R2 = R0;
					R0 = match_offset;
				}
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
			break;
		case 3:
			while (p < b)
				*p++ = GETCHAR(state);
			break;
		case 0:
			goto done;
		default:
			if (state->codex->disc->errorf)
				(*state->codex->disc->errorf)(NiL, state->codex->disc, 2, "%u: invalid block type #2", state->block_type);
			return -1;
		}
	}
 done:
	state->R0 = R0;
	state->R1 = R1;
	state->R2 = R2;
	R0 = (state->oe = p) - (state->op = window);
	if (state->filesize)
	{
		if (state->relocate && relocate(state))
			return -1;
		state->offset += R0;
	}
	return R0;
}

static ssize_t
lzx_read(Sfio_t* sp, void* buf, size_t n, Sfdisc_t* disc)
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

Codexmeth_t	codex_lzx =
{
	"lzx",
	"Microsoft cabinet archive lzx compression from cabextract 0.6."
	" The first parameter is the window size which must be a power"
	" of 2 between 32k and 2m inclusive. The second parameter is the"
	" optional block aligment; 32k is required for cabinet file"
	" decompression.",
	0,
	CODEX_DECODE|CODEX_COMPRESS,
	0,
	0,
	lzx_open,
	lzx_close,
	lzx_init,
	0,
	lzx_read,
	0,
	0,
	0,
	0,
	0,
	0,
	CODEXNEXT(codex_lzx_next)
};

CODEXLIB(&codex_lzx)
