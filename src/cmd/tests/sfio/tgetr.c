/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1999-2002 AT&T Corp.                *
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
{
	Sfio_t*	f;
	char*	s;
	char*	string = "111\n222\n333";

	f = sfopen(NIL(Sfio_t*),string,"s");
	if(!(s = sfgetr(f,'\n',SF_STRING|SF_LOCKR)) || strcmp(s,"111") != 0)
		terror("sfgetr failed1\n");

	if(sfgetr(f,'\n',0) != NIL(char*))
		terror("sfgetr should have failed because of locking\n");
	sfread(f,s,1);

	if(!(s = sfgetr(f,'\n',SF_STRING)) || strcmp(s,"222") != 0)
		terror("sfgetr failed2\n");

	if((s = sfgetr(f,'\n',0)) != NIL(char*))
		terror("sfgetr should have failed because of partial record\n");

	if(!(s = sfgetr(f,0,SF_LASTR)) )
		terror("sfgetr should have succeeded getting partial record\n");

	/* test type == -1 and type == 1 modes */
	sfseek(f,(Sfoff_t)0,0);
	if(!(s = sfgetr(f,'\n',1)) || strcmp(s,"111") != 0)
		terror("sfgetr failed in compatible mode\n");

	if(!(s = sfgetr(f,'\n',SF_STRING|SF_LOCKR)) || strcmp(s,"222") != 0)
		terror("sfgetr failed3\n");
	if(sfgetr(f,'\n',1) )
		terror("sfgetr should have failed due to locking\n");
	sfread(f,s,0);

	if(sfgetr(f,'\n',1) )
		terror("sfgetr should have failed because record is incomplete\n");
	
	if(!(s = sfgetr(f,0,-1)) || strcmp(s,"333") != 0)
		terror("sfgetr failed in getting last partial record\n");

	TSTEXIT(0);
}
