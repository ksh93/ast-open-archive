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
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*******************************************************************/
#include	"sftest.h"

static int	Type;

#if __STD_C
static void notify(Sfio_t* f, int type, int fd)
#else
static void notify(f, type, fd)
Sfio_t*	f;
int		type;
int		fd;
#endif
{
	switch(Type = type)
	{
	case SF_NEW:
	case SF_CLOSING:
	case SF_SETFD:
	case SF_READ:
	case SF_WRITE:
		return;
	default:
		terror("Unexpected nofity-type: %d\n",type);
	}
}

MAIN()
{
	Sfio_t*	f;
	int	fd;

	sfnotify(notify);

	if(!(f = sfopen(NIL(Sfio_t*), tstfile(0), "w")) && Type != SF_NEW)
		terror("Notify did not announce SF_NEW event\n");
	fd = sffileno(f);
	close(fd+5);
	if(sfsetfd(f,fd+5) != fd+5 || Type != SF_SETFD)
		terror("Notify did not announce SF_SETFD event\n");
	if(sfclose(f) < 0 || Type != SF_CLOSING)
		terror("Notify did not announce SF_CLOSING event\n");
	
	if(sfputc(sfstdin,'a') >= 0 || Type != SF_WRITE)
		terror("Notify did not announce SF_WRITE event\n");

	if(sfgetc(sfstdout) >= 0 || Type != SF_READ)
		terror("Notify did not announce SF_READ event\n");

	TSTEXIT(0);
}
