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

/*
 * pax compress format
 */

#include "format.h"

static int
compress_getprologue(Pax_t* pax, Format_t* fp, Archive_t* ap, File_t* f, unsigned char* buf, size_t size)
{
	if (size < 2 || buf[0] != 0x1f || buf[1] != 0x9d)
		return 0;
	ap->uncompressed = ap->io->size * 3;
	return 1;
}

static Compress_format_t	pax_compress_data =
{
	0,
	{ 0 },
	{ "zcat" },
};

Format_t	pax_compress_format =
{
	"compress",
	0,
	"Lempel-Ziv compression",
	0,
	COMPRESS|IN|OUT,
	0,
	0,
	0,
	PAXNEXT(pax_compress_next),
	&pax_compress_data,
	0,
	compress_getprologue,
};

PAXLIB(&pax_compress_format)
