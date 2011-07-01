/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1999-2011 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#include	"sftest.h"

MAIN()
{
	Sfio_t*	f;
	Sfio_t	sf;

	if(sfopen(sfstdout,"abc","s") != sfstdout)
		terror("Bad reopening of sfstdout");
	if(sfopen(sfstdin,"123","s") != sfstdin)
		terror("Bad reopening of sfstdin");
	sfclose(sfstdin);

	if(!(f = sfopen(NIL(Sfio_t*),"123","s")) )
		terror("Opening a stream");
	sfclose(f);
	if(sfopen(f,"123","s") != NIL(Sfio_t*))
		terror("can't reopen a closed stream!");

	if(sfnew(&sf,NIL(char*),(size_t)SF_UNBOUND,0,SF_EOF|SF_READ) != &sf)
		terror("Did not open sf");
	sfset(&sf,SF_STATIC,1);
	if(!sfclose(&sf) || errno != EBADF)
		terror("sfclose(sf) should fail with EBADF");
	if(!(sfset(&sf,0,0)&SF_STATIC))
		terror("Did not close sf");

	/* test for exclusive opens */
	unlink(tstfile(0));
	if(!(f = sfopen(NIL(Sfio_t*),tstfile(0),"wx") ) )
		terror("sfopen failed");
	if((f = sfopen(f,tstfile(0),"wx") ) )
		terror("sfopen should not succeed here");

	TSTEXIT(0);
}
