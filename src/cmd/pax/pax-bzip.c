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
 * pax bzip format
 */

#include "format.h"

static int
bzip_getprologue(Pax_t* pax, Format_t* fp, Archive_t* ap, File_t* f, unsigned char* buf, size_t size)
{
	if (size < 3 || buf[0] != 0x42 || buf[1] != 0x5a || buf[2] != 0x68)
		return 0;
	ap->uncompressed = ap->io->size * 7;
	return 1;
}

static Compress_format_t	pax_bzip_data =
{
	0,
	{ "bunzip2" },
	{ 0 },
};

Format_t	pax_bzip_format =
{
	"bzip",
	"bzip2",
	"bzip compression",
	0,
	COMPRESS|IN|OUT,
	0,
	0,
	0,
	PAXNEXT(pax_bzip_next),
	&pax_bzip_data,
	0,
	bzip_getprologue,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0
};

PAXLIB(&pax_bzip_format)
