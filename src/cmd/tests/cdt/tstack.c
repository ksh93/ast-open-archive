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

	/* testing Dtstack */
	if(!(dt = dtopen(&Disc,Dtstack)) )
		terror("dtopen stack");
	if((int)dtinsert(dt,1) != 1)
		terror("Dtstack insert 1");
	if((int)dtinsert(dt,3) != 3)
		terror("Dtstack insert 3.1");
	if((int)dtinsert(dt,2) != 2)
		terror("Dtstack insert 2.1");
	if((int)dtinsert(dt,3) != 3)
		terror("Dtstack insert 3.2");
	if((int)dtinsert(dt,2) != 2)
		terror("Dtstack insert 2.2");
	if((int)dtinsert(dt,3) != 3)
		terror("Dtstack insert 3.3");

	if((int)dtlast(dt) != 1)
		terror("Dtstack dtlast");
	if((int)dtprev(dt,1) != 3)
		terror("Dtstack dtprev 1");
	if((int)dtprev(dt,3) != 2)
		terror("Dtstack dtprev 3.1");
	if((int)dtprev(dt,2) != 3)
		terror("Dtstack dtprev 2.1");
	if((int)dtprev(dt,3) != 2)
		terror("Dtstack dtprev 3.2");
	if((int)dtprev(dt,2) != 3)
		terror("Dtstack dtprev 2.2");
	if((int)dtprev(dt,3) != 0)
		terror("Dtstack dtprev 3.2");

	if((int)dtdelete(dt,NIL(Void_t*)) != 3)
		terror("Dtstack pop 3.3");

	/* search to one of the 3 */
	if((int)dtsearch(dt,3) != 3)
		terror("Dtstack search 3.2");
	if((int)dtdelete(dt,3) != 3)
		terror("Dtstack delete 3.2");

	if((int)dtdelete(dt,NIL(Void_t*)) != 2)
		terror("Dtstack pop 2.2");
	if((int)dtdelete(dt,NIL(Void_t*)) != 2)
		terror("Dtstack pop 2.1");
	if((int)dtdelete(dt,NIL(Void_t*)) != 3)
		terror("Dtstack pop 3.1");
	if((int)dtdelete(dt,NIL(Void_t*)) != 1)
		terror("Dtstack pop 1");

	if(dtsize(dt) != 0)
		terror("Dtstack size");

	return 0;
}
