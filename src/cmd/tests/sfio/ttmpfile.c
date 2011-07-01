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

	if(!(f = sftmp(1025)))
		terror("Can't open temp file");
	if(sffileno(f) >= 0)
		terror("Attempt to create file detected");
	if(sfputc(f,0) < 0)
		terror("Can't write to temp file");
	if(sffileno(f) >= 0)
		terror("Attempt to create file detected");
	if(sfclose(f) < 0)
		terror("Can't close temp file");
	if(sffileno(f) >= 0)
		terror("Attempt to create file detected");

	if(!(f = sftmp(8)))
		terror("Can't open temp file");
	if(sffileno(f) >= 0)
		terror("Attempt to create file detected");
	sfdisc(f,NIL(Sfdisc_t*));
	if(sffileno(f) < 0)
		terror("Real file wasn't created");
	if(sfclose(f) < 0)
		terror("Can't close temp file");

	if(!(f = sftmp(8)))
		terror("Can't open temp file");
	if(sffileno(f) >= 0)
		terror("Attempt to create file detected");
	if(sfseek(f, (Sfoff_t)8, SEEK_SET) < 0)
		terror("Can't seek on temp file");
	if(sffileno(f) >= 0)
		terror("Attempt to create file detected");
	if(sfputc(f,0) < 0)
		terror("Can't write to temp file");
	if(sffileno(f) < 0)
		terror("Real file wasn't created");
	if(sfclose(f) < 0)
		terror("Can't close temp file");

	TSTEXIT(0);
}
