/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1990-2005 AT&T Corp.                  *
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
/*
 *	return attr of a sfile 
 */


#include "vcs_rscs.h"

int get_attr(f, ap)
	Sfio_t*		f;
	register attr_t*	ap;
{
	if (sfread(f, (char *)ap, sizeof(attr_t)) != sizeof(attr_t) || !ISRSCS(ap))
	{
		rserrno = NOTRSCS;
		return (-1);
	}

	return (0);
}


