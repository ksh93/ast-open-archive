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

Dtdisc_t Disc =
	{ 0, sizeof(int), -1,
	  newint, NIL(Dtfree_f), compare, hashint,
	  NIL(Dtmemory_f), NIL(Dtevent_f)
	};

main()
{
	Dt_t*		dt;

	/* testing Dtqueue */
	if(!(dt = dtopen(&Disc,Dtqueue)) )
		terror("dtopen queue");
	if((int)dtinsert(dt,1) != 1)
		terror("Dtqueue insert 1");
	if((int)dtinsert(dt,3) != 3)
		terror("Dtqueue insert 3.1");
	if((int)dtinsert(dt,2) != 2)
		terror("Dtqueue insert 2.1");
	if((int)dtinsert(dt,3) != 3)
		terror("Dtqueue insert 3.2");
	if((int)dtinsert(dt,2) != 2)
		terror("Dtqueue insert 2.2");
	if((int)dtinsert(dt,3) != 3)
		terror("Dtqueue insert 3.3");

	if((int)dtlast(dt) != 3)
		terror("Dtqueue dtlast");
	if((int)dtprev(dt,3) != 2)
		terror("Dtqueue dtprev 3.3");
	if((int)dtprev(dt,2) != 3)
		terror("Dtqueue dtprev 2.2");
	if((int)dtprev(dt,3) != 2)
		terror("Dtqueue dtprev 3.2");
	if((int)dtprev(dt,2) != 3)
		terror("Dtqueue dtprev 2.1");
	if((int)dtprev(dt,3) != 1)
		terror("Dtqueue dtprev 3.1");
	if((int)dtprev(dt,1) != 0)
		terror("Dtqueue dtprev 1");

	if((int)dtdelete(dt,NIL(Void_t*)) != 1)
		terror("Dtqueue pop 1");
	if((int)dtdelete(dt,NIL(Void_t*)) != 3)
		terror("Dtqueue delete 3.1");
	if((int)dtdelete(dt,NIL(Void_t*)) != 2)
		terror("Dtqueue delete 2");
	if((int)dtdelete(dt,NIL(Void_t*)) != 3)
		terror("Dtqueue delete 3.2");
	if((int)dtdelete(dt,NIL(Void_t*)) != 2)
		terror("Dtqueue delete 2.1");
	if((int)dtdelete(dt,NIL(Void_t*)) != 3)
		terror("Dtqueue delete 3.3");

	if(dtsize(dt) != 0)
		terror("Dtqueue size");

	return 0;
}
