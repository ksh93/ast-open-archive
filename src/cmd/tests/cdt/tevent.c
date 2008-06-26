/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1999-2008 AT&T Intellectual Property          *
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
#include	"dttest.h"

static int Pevent;
static int Event;
static int Hinit;

#if __STD_C
static int event(Dt_t* dt, int type, Void_t* obj, Dtdisc_t* disc)
#else
static int event(dt, type, obj, disc)
Dt_t*	dt;
int	type;
Void_t* obj;
Dtdisc_t* disc;
#endif
{
	Pevent = Event;
	Event = type;

	if(type == DT_HASHSIZE)
	{	Hinit += 1;
		if(*(int*)obj == 0)
		{	*(int*)obj = 8;
			return 1;
		}
		else if(*(int*)obj != 8)
			terror("Wrong hash table size");
		else
		{	*(int*)obj = -1;
			return 1;
		}
	}

	return 0;
}

Dtdisc_t Disc =
	{ 0, sizeof(int), -1,
	  newint, NIL(Dtfree_f), compare, hashint,
	  NIL(Dtmemory_f), event
	};

main()
{
	Dt_t		*dt;
	int		k;

	if(!(dt = dtopen(&Disc,Dtset)) )
		terror("Opening Dtset");
	if(Pevent != DT_OPEN && Event != DT_ENDOPEN)
		terror("No open event");

	dtmethod(dt,Dtoset);
	if(Event != DT_METH)
		terror("No meth event");

	dtdisc(dt,&Disc,0);
	if(Event != DT_DISC)
		terror("No disc event");

	dtclose(dt);
	if(Pevent != DT_CLOSE && Event != DT_ENDCLOSE)
		terror("No close event");

	if(!(dt = dtopen(&Disc,Dtset)) )
		terror("Opening Dtset");

	Pevent = Event = 0;
	for(k = 1; k <= 100; ++k)
		dtinsert(dt, k);
	if(Hinit != 2)
		terror("Wrong number of hash table events");

	return 0;
}
