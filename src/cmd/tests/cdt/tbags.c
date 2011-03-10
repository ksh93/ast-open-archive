/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1999-2005 AT&T Corp.                  *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                            by AT&T Corp.                             *
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

typedef struct _obj_s
{	Dtlink_t	link;
	int		key;
	int		ord;
} Obj_t;

static int objcmp(Dt_t* dt, Void_t* arg1, Void_t* arg2, Dtdisc_t* disc)
{
	Obj_t	*o1 = (Obj_t*)arg1, *o2 = (Obj_t*)arg2;

	return o1->key - o2->key;
}

static void objfree(Dt_t* dt, Void_t* arg, Dtdisc_t* disc)
{
	Obj_t	*o = (Obj_t*)arg;
	o->ord = -o->ord;
}

static unsigned int objhash(Dt_t* dt, Void_t* arg, Dtdisc_t* disc)
{
	Obj_t	*o = (Obj_t*)arg;
	return o->key;
}

Dtdisc_t Disc = { 0, 0, 0, 0, objfree, objcmp, objhash, 0, 0 };

main()
{
	Dt_t*	dt;
	Obj_t	*o, obj[20], proto;
	int	i, k, count;

	for(i = 0; i < sizeof(obj)/sizeof(obj[0]); i = k)
	{	for(k = i; k < i+5; ++k)
		{	obj[k].key = i;
			obj[k].ord = k;
		}
	}

	for(k = 0; k < 2; ++k)
	{	dt = dtopen(&Disc, k == 0 ? Dtobag : Dtbag);
		for(i = 0; i < sizeof(obj)/sizeof(obj[0]); ++i)
			if(dtinsert(dt, obj+i) != obj+i)
				terror("Insert %d,%d", obj[i].key, obj[i].ord);

		proto.key = 10;
		for(count = 0, o = dtsearch(dt, &proto); o; o = dtnext(dt,o), count++)
			if(o->key != 10)
				break;
		if(count != 5)
			terror("Bad count %d", count);
	
		dtdelete(dt, obj+7);
		if(obj[7].ord != -7)
			terror("Bad delete");
		obj[7].ord = 7;

		dtdelete(dt, obj+17);
		if(obj[17].ord != -17)
			terror("Bad delete2");
		obj[17].ord = 17;

		dtdelete(dt, obj+11);
		if(obj[11].ord != -11)
			terror("Bad delete3");
		obj[11].ord = 11;

		o = dtdelete(dt, &proto);
		if(!o || o->ord >= 0)
			terror("Bad delete4");
		o->ord = -o->ord;
	}

	return 0;
}
