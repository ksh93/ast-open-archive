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

MAIN()
{
	Sfio_t*	f;
	Sfio_t	sf;

	if(sfopen(sfstdout,"abc","s") != sfstdout)
		terror("Bad reopening of sfstdout\n");
	if(sfopen(sfstdin,"123","s") != sfstdin)
		terror("Bad reopening of sfstdin\n");
	sfclose(sfstdin);

	if(!(f = sfopen(NIL(Sfio_t*),"123","s")) )
		terror("Opening a stream\n");
	sfclose(f);
	if(sfopen(f,"123","s") != NIL(Sfio_t*))
		terror("can't reopen a closed stream!\n");

	if(sfnew(&sf,NIL(char*),SF_UNBOUND,0,SF_EOF|SF_READ) != &sf)
		terror("Did not open sf\n");
	sfset(&sf,SF_STATIC,1);
	if(sfclose(&sf) < 0 || !(sf.flags&SF_STATIC))
		terror("Did not close sf\n");

	/* test for exclusive opens */
	unlink(tstfile(0));
	if(!(f = sfopen(NIL(Sfio_t*),tstfile(0),"wx") ) )
		terror("sfopen failed\n");
	if((f = sfopen(f,tstfile(0),"wx") ) )
		terror("sfopen should not succeed here\n");

	TSTEXIT(0);
}
