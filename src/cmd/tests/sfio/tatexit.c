/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1999-2004 AT&T Corp.                  *
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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
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

	TSTEXIT(0);
}
