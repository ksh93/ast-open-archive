/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1993-2000 AT&T Corp.              *
*      and it may only be used by you under license from       *
*                     AT&T Corp. ("AT&T")                      *
*       A copy of the Source Code Agreement is available       *
*              at the AT&T Internet web site URL               *
*                                                              *
*     http://www.research.att.com/sw/license/ast-open.html     *
*                                                              *
*     If you received this software without first entering     *
*       into a license with AT&T, you have an infringing       *
*           copy and cannot use it without violating           *
*             AT&T's intellectual property rights.             *
*                                                              *
*               This software was created by the               *
*               Network Services Research Center               *
*                      AT&T Labs Research                      *
*                       Florham Park NJ                        *
*                                                              *
*              David Korn <dgk@research.att.com>               *
*                                                              *
***************************************************************/
#pragma prototyped
/*
 * Header file for Huffman coding
 * The coding is the same as that used with the System V pack program
 *
 *   David Korn
 *   AT&T Bell Laboratories
 *   Room 3C-526B
 *   Murray Hill, N. J. 07974
 *   Tel. x7975
 *   ulysses!dgk
 */

#ifndef _HUFFMAN_H_
#define _HUFFMAN_H_	1

#include	<ast.h>

#define HUFFLEV		24	/* maximum number of bits per code */
#define HUFFMAG1	037	/* ascii <US> */
#define HUFFMAG2	036	/* ascii <RS> */

typedef struct
{
	char		length[(1<<CHAR_BIT)+1];
	unsigned char	levcount[HUFFLEV+1];
	long		insize;
	long		outsize;
	long		buffer;
	long		id;
	int		left;
	int		maxlev;
	int		nchars;
	int		excess;
} Huff_t;

Huff_t*		huffinit(Sfio_t*,long);
Huff_t*		huffgethdr(Sfio_t*);
int	 	huffputhdr(Huff_t*,Sfio_t*);
int		huffencode(Huff_t*,Sfio_t*,Sfio_t*,int);
int		huffdecode(Huff_t*,Sfio_t*,Sfio_t*,int);
Sfio_t*		huffdisc(Sfio_t*);

#define huffend(hp)	free((void*)(hp))
#define huffisize(hp)	((hp)->insize)
#define huffosize(hp)	((hp)->outsize)
#define huffhsize(hp)	((hp)->maxlev+(hp)->nchars+7)

#endif /* !_HUFFMAN_H_ */
