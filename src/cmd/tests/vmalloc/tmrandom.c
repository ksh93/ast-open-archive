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
#include	"vmtest.h"
#include	<stdlib.h>

#define TWARN		0		/* enable debug warnings	*/

static unsigned long	RAND_state = 1;

#define RAND()		(RAND_state = RAND_state * 0x63c63cd9L + 0x9c39c33dL)

#define N_OBJ		500000		/* number of allocated objects	*/

#define COMPACT		(N_OBJ/5)	/* period for compacting	*/

#define HUGESZ		(4*1024)	/* increase huge block by this	*/
#define H_FREQ		(N_OBJ/100)	/* 1/H_FREQ: resizing probab.	*/

/* size of an allocation or resize */
#define L_FREQ		100		/* one out of L_FREQ is LARGE	*/		
#define LARGE		10000		/* size of large objects	*/
#define SMALL		100		/* size of small objects	*/
#define MICRO		10
#define L_ALLOC()	(RAND()%LARGE + LARGE)
#define S_ALLOC()	(RAND()%SMALL + SMALL)
#define M_ALLOC()	(RAND()%MICRO + 1)
#define ALLOCSZ()	(RAND()%L_FREQ == 0 ? L_ALLOC() : \
			 RAND()%2 == 0 ? S_ALLOC() : M_ALLOC() )

/* when to resize or free */
#define L_RANGE		10000		/* when to resize/free large 	*/
#define S_RANGE		10		/* when to resize/free small	*/
#define M_RANGE		5		/* when to resize/free micro	*/
#define RANGE(sz)	(sz >= LARGE ? L_RANGE : sz >= SMALL ? S_RANGE : M_RANGE)
#define WHEN(sz)	((RAND() % RANGE(sz)) + 1)

typedef struct _obj_s	Obj_t;
struct _obj_s
{	Void_t*	obj;
	size_t	size;
	Obj_t*	head;
	Obj_t*	next;
};
static Obj_t		Obj[N_OBJ+1];

main()
{
	Obj_t		*o, *next, *last, *n, *f;
	Void_t		*huge;
#if TWARN
	unsigned char	*begaddr, *endaddr;
#endif
	size_t		sz, hugesz, busysz;
	int		counter;

#if TWARN
	begaddr = (unsigned char*)sbrk(0);
#endif

	hugesz = HUGESZ;
	if(!(huge = malloc(hugesz)) )
		terror("Can't allocate block");

	busysz = hugesz;
	for(counter = 1, o = Obj, last = Obj+N_OBJ; o < last; ++o, ++counter)
	{	
		/* free all stuff that should be freed */
		for(f = o->head; f; f = next)
		{	next = f->next;

			if(f->obj)
			{	if((RAND()%2) == 0 ) /* resize */
				{	sz = ALLOCSZ();
					f->obj = realloc(f->obj,sz);
					if(!f->obj)
						terror("realloc failed");
					busysz -= f->size;
					f->size = sz;
					busysz += f->size;

					if((n = o + WHEN(sz)) > last)
						n = last;
					f->next = n->head;
					n->head = f;
				}
				else
				{	free(f->obj);
					busysz -= f->size;
				}
			}
		}

#if TWARN
		if(counter%COMPACT == 0)
		{	endaddr = (unsigned char*)sbrk(0);
			twarn("%d: busy=%u arena=%u\n",
				counter, busysz, endaddr-begaddr);
		}
#endif

		if(((o-Obj)%H_FREQ) == 0 )
		{	hugesz += HUGESZ;
			if(!(huge = realloc(huge, hugesz)) )
				terror("Bad resize of huge block");
			busysz += HUGESZ;
		}

		sz = ALLOCSZ();
		o->obj = malloc(sz);
		if(!o->obj)
			terror("malloc failed");
		o->size = sz;
		busysz += sz;

		if((n = o + WHEN(sz)) > last)
			n = last;
		o->next = n->head;
		n->head = o;
	}

#if TWARN
	endaddr = (unsigned char*)sbrk(0);
	twarn("End: busy=%u arena=%u\n", counter, busysz, endaddr-begaddr);
#endif

	exit(0);
}
