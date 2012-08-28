/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1999-2011 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                 Eclipse Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
*                                                                      *
*                A copy of the License is available at                 *
*          http://www.eclipse.org/org/documents/epl-v10.html           *
*         (with md5 checksum b35adb5213ca9657e911e9befb180842)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#include	"dttest.h"

/* test to see if the Dtuser_t structure has the right elements */

Dtdisc_t Disc =
	{ 0, sizeof(long), -1,
	  newint, NIL(Dtfree_f), compare, hashint,
	  NIL(Dtmemory_f), NIL(Dtevent_f)
	};

tmain()
{
	Dt_t		*dt;

	if(!(dt = dtopen(&Disc,Dtset)) )
		terror("Opening Dtset");

	if(dtuserlock(dt, 0, 1) >= 0 )
		terror("dtuserlock() should have failed because key == 0");

	if(dtuserlock(dt, 1111, 1) <  0 )
		terror("dtuserlock() should have succeeded to lock");

	/* note the key value below must be 1111; else, a deadlock will happen */
	if(dtuserdata(dt, (Void_t*)11, 1111) != (Void_t*)11)
		terror("dtuserdata() should have succeeded to set value to 11");

	if(dt->user->lock != 0) /* after setting value, the lock is unlocked */
		terror("lock should be opened");

	/* relocking with key == 1111 */
	if(dtuserlock(dt, 1111, 1) <  0 )
		terror("dtuserlock() should have succeeded to lock");

	/* reading the data value is always allowed (key == 0) */
	if(dtuserdata(dt, (Void_t*)0, 0) != (Void_t*)11)
		terror("dtuserdata() should have return 11");

	/* unlocking with key == 1111 */
	if(dtuserlock(dt, 1111, -1) <  0 )
		terror("dtuserlock() should have succeeded to lock");

	/* setting a new value */
	if(dtuserdata(dt, (Void_t*)22, 1) != (Void_t*)22)
		terror("dtuserdata() should have succeeded to set value to 22");

	if(dt->user->lock != 0)
		terror("user->lock should be zero");
	if(dt->user->data != (Void_t*)22)
		terror("user->data should be 22");

	texit(0);
}
