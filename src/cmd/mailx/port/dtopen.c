/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*Copyright (c) 1978-2004 The Regents of the University of California an*
*                                                                      *
*            Permission is hereby granted, free of charge,             *
*         to any person obtaining a copy of THIS SOFTWARE FILE         *
*              (the "Software"), to deal in the Software               *
*                without restriction, including without                *
*             limitation the rights to use, copy, modify,              *
*                  merge, publish, distribute, and/or                  *
*              sell copies of the Software, and to permit              *
*              persons to whom the Software is furnished               *
*            to do so, subject to the following disclaimer:            *
*                                                                      *
*THIS SOFTWARE IS PROVIDED BY The Regents of the University of Californ*
*           ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,           *
*              INCLUDING, BUT NOT LIMITED TO, THE IMPLIED              *
*              WARRANTIES OF MERCHANTABILITY AND FITNESS               *
*               FOR A PARTICULAR PURPOSE ARE DISCLAIMED.               *
*IN NO EVENT SHALL The Regents of the University of California and AT&T*
*           BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,            *
*             SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES             *
*             (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT              *
*            OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,             *
*             DATA, OR PROFITS; OR BUSINESS INTERRUPTION)              *
*            HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,            *
*            WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT            *
*             (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING              *
*             IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,              *
*          EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.          *
*                                                                      *
*              Information and Software Systems Research               *
*    The Regents of the University of California and AT&T Research     *
*                           Florham Park NJ                            *
*                                                                      *
*                          Kurt Shoens (UCB)                           *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#include	"dthdr.h"
static char     *Version = "\n@(#)$Id: Version 1995-10-25-cdt-kpv $\n";

/* 	Make a new dictionary
**
**	Written by Kiem-Phong Vo (7/15/95)
*/

#if __STD_C
Dt_t* dtopen(Dtdisc_t* disc, Dtmethod_t* meth)
#else
Dt_t*	dtopen(disc, meth)
Dtdisc_t*	disc;
Dtmethod_t*	meth;
#endif
{
	Dt_t*		dt = (Dt_t*)Version;	/* shut-up unuse warning */
	reg int		e;
	Dtdata_t*	data;

	if(!disc || !meth)
		return NIL(Dt_t*);

	/* allocate space for dictionary */
	if(!(dt = (Dt_t*) malloc(sizeof(Dt_t))))
		return NIL(Dt_t*);

	/* initialize all absolutely private data */
	dt->searchf = NIL(Dtsearch_f);
	dt->meth = NIL(Dtmethod_t*);
	dt->disc = NIL(Dtdisc_t*);
	dtdisc(dt,disc,0);
	dt->nview = 0;
	dt->view = dt->walk = NIL(Dt_t*);

	if(disc->eventf)
	{	/* if this is a shared dictionary, get the shared data */
		data = NIL(Dtdata_t*);
		if((e = (*disc->eventf)(dt,DT_OPEN,(Void_t*)(&data),disc)) != 0)
		{	if(e < 0 || !data || !(data->type&meth->type) )
			{	free((Void_t*)dt);
				return NIL(Dt_t*);
			}
			else	goto done;
		}
	}

	/* allocate sharable data */
	data = (Dtdata_t*)(dt->memoryf)(dt,NIL(Void_t*),sizeof(Dtdata_t),disc);
	if(!data)
	{	free((Void_t*)dt);
		return NIL(Dt_t*);
	}
	data->type = meth->type;
	data->here = NIL(Dtlink_t*);
	data->htab = NIL(Dtlink_t**);
	data->ntab = data->size = 0;

done:
	dt->data = data;
	dt->searchf = meth->searchf;
	dt->meth = meth;
	return dt;
}
