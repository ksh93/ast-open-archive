/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1999-2000 AT&T Corp.                *
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
*                                                                  *
*******************************************************************/
#include	"sftest.h"


/*	Test multiple processes reading/writing from same file
**	descriptor.
*/
MAIN()
{
	char*	s;

	if(argc > 1)
	{	if(strcmp(argv[1],"-r") == 0)	/* doing sfgetr */
		{	if(!(s = sfgetr(sfstdin,'\n',1)) || strcmp(s,"Line2") != 0)
				terror("Coprocess getr did not get Line2\n");
			if(!(s = sfgetr(sfstdin,'\n',1)) || strcmp(s,"Line3") != 0)
				terror("Coprocess getr did not get Line3\n");
		}
		else	/* doing sfmove */
		{	Sfio_t*	f = sfopen(NIL(Sfio_t*),NIL(char*),"swr");
			if(!f)
				terror("Can't open string stream\n");
			if(sfmove(sfstdin,f,(Sfoff_t)2,'\n') != 2)
				terror("Coprocess sfmove failed\n");
			sfseek(f,(Sfoff_t)0,0);
			if(!(s = sfgetr(f,'\n',1)) || strcmp(s,"Line2") != 0)
				terror("Coprocess move did not get Line2\n");
			if(!(s = sfgetr(f,'\n',1)) || strcmp(s,"Line3") != 0)
				terror("Coprocess move did not get Line3\n");
		}
		exit(0);
	}

	if(sfopen(sfstdout, tstfile(0), "w") != sfstdout )
		terror("Opening file\n");
	if(sfputr(sfstdout,"Line1",'\n') < 0 ||
	   sfputr(sfstdout,"Line2",'\n') < 0 ||
	   sfputr(sfstdout,"Line3",'\n') < 0 ||
	   sfputr(sfstdout,"Line4",'\n') < 0)
		terror("Writing data\n");
	sfopen(sfstdout,"/dev/null","w");

	/* testing coprocess calling sfgetr */
	if(sfopen(sfstdin, tstfile(0),"r") != sfstdin)
		terror("Opening to read\n");
	if(!(s = sfgetr(sfstdin,'\n',1)) || strcmp(s,"Line1") != 0)
		terror("Did not get Line1 for sfgetr\n");
	sfsync(sfstdin);
	system(sfprints("%s -r",argv[0]));
	sfseek(sfstdin,(Sfoff_t)lseek(sffileno(sfstdin),0L,1),0);
	if(!(s = sfgetr(sfstdin,'\n',1)) || strcmp(s,"Line4") != 0)
		terror("Did not get Line4 for sfgetr\n");

	/* testing coprocess calling sfmove */
	if(sfopen(sfstdin, tstfile(0), "r") != sfstdin)
		terror("Opening to read\n");
	if(!(s = sfgetr(sfstdin,'\n',1)) || strcmp(s,"Line1") != 0)
		terror("Did not get Line1 for sfmove\n");
	sfsync(sfstdin);
	system(sfprints("%s -m",argv[0]));
	sfseek(sfstdin,(Sfoff_t)lseek(sffileno(sfstdin),0L,1),0);
	if(!(s = sfgetr(sfstdin,'\n',1)) || strcmp(s,"Line4") != 0)
		terror("Did not get Line4 for sfmove\n");

	/* testing the head program */
#ifdef HEAD
	if(sfopen(sfstdin, tstfile(0), "r") != sfstdin)
		terror("Opening to read\n");
	if(!(s = sfgetr(sfstdin,'\n',1)) || strcmp(s,"Line1") != 0)
		terror("Did not get Line1 for head\n");
	sfsync(sfstdin);
	system("head -2 > /dev/null"); /* for testing the head program */
	sfseek(sfstdin,(Sfoff_t)lseek(sffileno(sfstdin),0L,1),0);
	if(!(s = sfgetr(sfstdin,'\n',1)) || strcmp(s,"Line4") != 0)
		terror("Did not get Line4 for head\n");
#endif

	TSTRETURN(0);
}
