/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1999-2003 AT&T Corp.                *
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
*                                                                  *
*******************************************************************/
#include	"sftest.h"


MAIN()
{	Sfio_t	*f;
	int	i, c;

	if(!(f = sftmp(8)))
		terror("Can't open temp file\n");

	for(i = 0; i < 10000; ++i)
		if(sfputc(f,(i%26)+'a') < 0)
			terror("Writing %c\n",(i%26)+'a');

	sfseek(f,(Sfoff_t)0,0);

	for(i = 0; i < 10000; ++i)
		if((c = sfgetc(f)) != ((i%26)+'a'))
			terror("Input=%#o, Expect=%c\n",c,(i%26)+'a');

	TSTEXIT(0);
}
