/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1987-2008 AT&T Intellectual Property          *
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
	DELTA|DELTAIO|IN|OUT,
	0,
	0,
	0,
	PAXNEXT(pax_delta_next),
};

PAXLIB(&pax_delta_format)
