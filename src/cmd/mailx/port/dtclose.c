/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*Copyright (c) 1978-2004 The Regents of the University of Californi*
*                                                                  *
*          Permission is hereby granted, free of charge,           *
*       to any person obtaining a copy of THIS SOFTWARE FILE       *
*            (the "Software"), to deal in the Software             *
*              without restriction, including without              *
*           limitation the rights to use, copy, modify,            *
*                merge, publish, distribute, and/or                *
*            sell copies of the Software, and to permit            *
*            persons to whom the Software is furnished             *
*          to do so, subject to the following disclaimer:          *
*                                                                  *
*THIS SOFTWARE IS PROVIDED BY The Regents of the University of Cali*
*         ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,         *
*            INCLUDING, BUT NOT LIMITED TO, THE IMPLIED            *
*            WARRANTIES OF MERCHANTABILITY AND FITNESS             *
*             FOR A PARTICULAR PURPOSE ARE DISCLAIMED.             *
*IN NO EVENT SHALL The Regents of the University of California and *
*         BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,          *
*           SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES           *
*           (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT            *
*          OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,           *
*           DATA, OR PROFITS; OR BUSINESS INTERRUPTION)            *
*          HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,          *
*          WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT          *
*           (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING            *
*           IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,            *
*        EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.        *
*                                                                  *
*            Information and Software Systems Research             *
*The Regents of the University of California and AT&T Labs Research*
*                         Florham Park NJ                          *
*                                                                  *
*                        Kurt Shoens (UCB)                         *
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
#include	"dthdr.h"

/*	Close a dictionary
**
**	Written by Kiem-Phong Vo (07/15/95)
*/
#if __STD_C
int dtclose(reg Dt_t* dt)
#else
int dtclose(dt)
reg Dt_t*	dt;
#endif
{
	if(dt->nview > 0 ) /* can't close if being viewed */
		return -1;

	if(dt->view)	/* turn off viewing */
		dtview(dt,NIL(Dt_t*));

	/* announce the close event */
	if(dt->disc->eventf &&
	   (*dt->disc->eventf)(dt,DT_CLOSE,NIL(Void_t*),dt->disc) < 0)
		return -1;

	/* release all allocated data */
	(void)(*(dt->meth->searchf))(dt,NIL(Void_t*),DT_CLEAR);
	if(dtsize(dt) > 0)
		return -1;

	if(dt->data->ntab > 0)
		(*dt->memoryf)(dt,(Void_t*)dt->data->htab,0,dt->disc);
	(*dt->memoryf)(dt,(Void_t*)dt->data,0,dt->disc);

	free((Void_t*)dt);

	return 0;
}
