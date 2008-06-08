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
#include	"vcwhdr.h"

/*	Close a Vcwindow_t handle
**
**	Written by Kiem-Phong Vo (kpv@research.att.com)
*/

#if __STD_C
int vcwclose(Vcwindow_t* vcw)
#else
int vcwclose(vcw)
Vcwindow_t*	vcw;
#endif
{
	if(!vcw)
		return -1;

	if(vcw->disc && vcw->disc->eventf &&
	   (*vcw->disc->eventf)(vcw, VCW_CLOSING, 0, vcw->disc) < 0 )
		return -1;

	if(vcw->meth && vcw->meth->eventf &&
	   (*vcw->meth->eventf)(vcw, VCW_CLOSING) < 0 )
		return -1;

	free(vcw);

	return 0;
}

