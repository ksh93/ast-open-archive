/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1993-2004 AT&T Corp.                  *
*                      and is licensed under the                       *
*          Common Public License, Version 1.0 (the "License")          *
*                        by AT&T Corp. ("AT&T")                        *
*      Any use, downloading, reproduction or distribution of this      *
*      software constitutes acceptance of the License.  A copy of      *
*                     the License is available at                      *
*                                                                      *
*         http://www.research.att.com/sw/license/cpl-1.0.html          *
*         (with md5 checksum 8a5e0081c856944e76c69a1cf29c2e8b)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                  David Korn <dgk@research.att.com>                   *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Header file for Huffman coding
 * The coding is the same as that used with the System V pack program
 *
 *   David Korn
 *   AT&T Laboratories
 */

#ifndef _HUFFMAN_H_
#define _HUFFMAN_H_	1

#include	<ast.h>

#define HUFFLEV		32	/* maximum number of bits per code */
#define HUFFMAG1	037	/* ascii <US> */
#define HUFFMAG2	036	/* ascii <RS> */

typedef struct
{
	char		length[(1<<CHAR_BIT)+1];
	unsigned char	levcount[HUFFLEV+1];
	Sfoff_t		insize;
	Sfoff_t		outsize;
	long		buffer;
	long		id;
	int		left;
	int		maxlev;
	int		nchars;
	int		excess;
} Huff_t;

Huff_t*		huffinit(Sfio_t*,Sfoff_t);
Huff_t*		huffgethdr(Sfio_t*);
int	 	huffputhdr(Huff_t*,Sfio_t*);
Sfoff_t		huffencode(Huff_t*,Sfio_t*,Sfio_t*,int);
Sfoff_t		huffdecode(Huff_t*,Sfio_t*,Sfio_t*,int);
Sfio_t*		huffdisc(Sfio_t*);

#define huffend(hp)	free((void*)(hp))
#define huffisize(hp)	((hp)->insize)
#define huffosize(hp)	((hp)->outsize)
#define huffhsize(hp)	((hp)->maxlev+(hp)->nchars+7)

#endif /* !_HUFFMAN_H_ */
