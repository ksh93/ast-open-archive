/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2008 AT&T Intellectual Property          *
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
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#include	"vchdr.h"

/* Recode a chunk of data using a secondary coder.
** This is like calling vcapply() on the data but it must make
** sure that all data is consumed and the recoding is error-free.
**
** Written by Kiem-Phong Vo (08/04/2006)
*/

#if __STD_
ssize_t vcrecode(Vcodex_t* vc, Vcchar_t** dtp, ssize_t* dtz, ssize_t head)
#else 
ssize_t vcrecode(vc, dtp, dtz, head)
Vcodex_t*	vc;	/* coding handle	*/
Vcchar_t**	dtp;	/* input/output data	*/
ssize_t*	dtz;	/* input/output size	*/
ssize_t		head;	/* extra header		*/
#endif
{
	ssize_t		sz;
	Vcchar_t	*dt;

	if(!vc->coder)
		return *dtz;

	vc->coder->head += vc->head + head;
	sz = vcapply(vc->coder, *dtp, *dtz, &dt);
	if(vcundone(vc->coder) > 0) /* at this step, all data must be processed */
		sz = -1;
	vc->coder->head -= vc->head + head;

	if(sz >= 0)
	{	*dtp = dt;
		*dtz = sz;
	}

	return sz;
}
