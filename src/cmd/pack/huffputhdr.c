/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1993-2005 AT&T Corp.                  *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                            by AT&T Corp.                             *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
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
 * huffman coding routine to write pack format file header
 *
 *   David Korn
 *   AT&T Laboratories
 */

#include	"huffman.h"

/*
 * This routine outputs a pack format header to <outfile> and returns
 * the number of bytes in the header
 */

int huffputhdr(register Huff_t *hp,Sfio_t *outfile)
{
	register int		i, c;
	register Sfio_t	*fp = outfile;
	/* output magic number */
	sfputc(fp,HUFFMAG1);
	sfputc(fp,HUFFMAG2);
	if(sizeof(Sfoff_t)>4 && hp->insize >= ((Sfoff_t)1)<<(4*CHAR_BIT))
	{
		sfputc(fp,hp->insize>>(7*CHAR_BIT));
		sfputc(fp,hp->insize>>(6*CHAR_BIT));
		sfputc(fp,hp->insize>>(5*CHAR_BIT));
		sfputc(fp,hp->insize>>(4*CHAR_BIT));
		sfputc(fp,0);
	}
	/* output the length and the dictionary */
	sfputc(fp,hp->insize>>(3*CHAR_BIT));
	sfputc(fp,hp->insize>>(2*CHAR_BIT));
	sfputc(fp,hp->insize>>(CHAR_BIT));
	sfputc(fp,hp->insize);
	/* output number of levels and count for each level */
	sfputc(fp,hp->maxlev);
	for (i=1; i<hp->maxlev; i++)
		sfputc(fp,hp->levcount[i]);
	sfputc(fp,hp->levcount[hp->maxlev]-2);
	/* output the characters */
	for (i=1; i<=hp->maxlev; i++)
		for (c=0; c < (1<<CHAR_BIT); c++)
			if (hp->length[c] == i)
				sfputc(fp,c);
	return(huffhsize(hp));
}
