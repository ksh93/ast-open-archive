/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1999-2004 AT&T Corp.                *
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

	if(!(f = sftmp(1025)))
		terror("Can't open temp file\n");
	if(sffileno(f) >= 0)
		terror("Attempt to create file detected\n");
	if(sfputc(f,0) < 0)
		terror("Can't write to temp file\n");
	if(sffileno(f) >= 0)
		terror("Attempt to create file detected\n");
	if(sfclose(f) < 0)
		terror("Can't close temp file\n");
	if(sffileno(f) >= 0)
		terror("Attempt to create file detected\n");

	if(!(f = sftmp(8)))
		terror("Can't open temp file\n");
	if(sffileno(f) >= 0)
		terror("Attempt to create file detected\n");
	sfdisc(f,NIL(Sfdisc_t*));
	if(sffileno(f) < 0)
		terror("Real file wasn't created\n");
	if(sfclose(f) < 0)
		terror("Can't close temp file\n");

	if(!(f = sftmp(8)))
		terror("Can't open temp file\n");
	if(sffileno(f) >= 0)
		terror("Attempt to create file detected\n");
	if(sfseek(f, (Sfoff_t)8, SEEK_SET) < 0)
		terror("Can't seek on temp file\n");
	if(sffileno(f) >= 0)
		terror("Attempt to create file detected\n");
	if(sfputc(f,0) < 0)
		terror("Can't write to temp file\n");
	if(sffileno(f) < 0)
		terror("Real file wasn't created\n");
	if(sfclose(f) < 0)
		terror("Can't close temp file\n");

	TSTEXIT(0);
}
