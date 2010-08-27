/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1986-2010 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * zip huffman codeing interface
 */

#ifndef _HUFF_H
#define _HUFF_H		1

#include "zip.h"

#include <vmalloc.h>

/* Huffman code lookup table entry--this entry is four bytes for machines
   that have 16-bit pointers (e.g. PC's in the small or medium model).
   Valid extra bits are 0..13.	e == 15 is EOB (end of block), e == 16
   means that v is a literal, 16 < e < 32 means that v is a pointer to
   the next table, which codes e - 16 bits, and lastly e == 99 indicates
   an unused code.  If a code with e == 99 is looked up, this implies an
   error in the data. */

struct Huff_s; typedef struct Huff_s Huff_t;

struct Huff_s
{
    uch			e;	/* number of extra bits or operation */
    uch			b;	/* number of bits in this code or subcode */
    union
    {
	ush		n;	/* literal, length base, or distance base */
	Huff_t*		t;	/* pointer to next level of table */
    } v;
};

extern int	huff(ulg*, ulg, ulg, ush*, ush*, Huff_t**, int*, Vmalloc_t*);

#endif
