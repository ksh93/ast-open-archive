/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1999-2001 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*                 This software was created by the                 *
*                 Network Services Research Center                 *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*******************************************************************/
#include	"sftest.h"

/* test to see if files created in atexit functions work ok */

void ae()
{
	Sfio_t*	f = sfopen(NIL(Sfio_t*), tstfile(0), "w");

	if(!f)
		terror("Can't create file");

	if(sfwrite(f,"1234\n",5) != 5)
		terror("Can't write to file");
}

#if __STD_C
main(int argc, char** argv)
#else
main(argc, argv)
int	argc;
char**	argv;
#endif
{
	Sfio_t* f;

	if(argc <= 1) /* atexit function registered after some sfio access */
	{	if(!(f = sfopen(NIL(Sfio_t*), tstfile(1), "w")) )
			terror("Can't create file");
		if(sfwrite(f,"1234\n",5) != 5)
			terror("Can't write to file");

		atexit(ae);

		system(sfprints("%s 1",argv[0]));
	}
	else /* atexit function registered before some sfio access */
	{	atexit(ae);

		if(!(f = sfopen(NIL(Sfio_t*), tstfile(1), "w")) )
			terror("Can't create file");
		if(sfwrite(f,"1234\n",5) != 5)
			terror("Can't write to file");
	}

	exit(0);
}
