/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1987-2004 AT&T Corp.                *
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
#pragma prototyped

#include "format.h"

/*
 * pax delta format
 */

static Delta_format_t	data = { "94" };

Format_t	pax_delta_format =
{
	"delta",
	"delta94|vdelta",
	"vdelta difference/compression",
	DELTA_94,
	DELTA|IN|OUT,
	0,
	0,
	0,
	PAXNEXT(pax_delta_next),
};

PAXLIB(&pax_delta_format)
