/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1990-2004 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*    If you have copied or used this software without agreeing     *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
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


