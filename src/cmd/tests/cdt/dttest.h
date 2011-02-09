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
#include	"cdt.h"
#include	"terror.h"

#if __STD_C
static compare(Dt_t* dt, Void_t* o1, Void_t* o2, Dtdisc_t* disc)
#else
static compare(dt,o1,o2,disc)
Dt_t*		dt;
Void_t*		o1;
Void_t*		o2;
Dtdisc_t*	disc;
#endif
{
	return (int)o1 - (int)o2;
}

#if __STD_C
static rcompare(Dt_t* dt, Void_t* o1, Void_t* o2, Dtdisc_t* disc)
#else
static rcompare(dt,o1,o2,disc)
Dt_t*		dt;
Void_t*		o1;
Void_t*		o2;
Dtdisc_t*	disc;
#endif
{
	return (int)o2 - (int)o1;
}

#if __STD_C
static Void_t* newint(Dt_t* dt, Void_t* o, Dtdisc_t* disc)
#else
static Void_t* newint(dt,o,disc)
Dt_t*		dt;
Void_t*		o;
Dtdisc_t*	disc;
#endif
{
	return o;
}

#if __STD_C
static unsigned int hashint(Dt_t* dt, Void_t* o, Dtdisc_t* disc)
#else
static unsigned int hashint(dt,o,disc)
Dt_t*		dt;
Void_t*		o;
Dtdisc_t*	disc;
#endif
{
	return (unsigned int)o;
}
