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
