/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1987-2004 AT&T Corp.                  *
*                      and is licensed under the                       *
*          Common Public License, Version 1.0 (the "License")          *
*                        by AT&T Corp. ("AT&T")                        *
*      Any use, downloading, reproduction or distribution of this      *
*      software constitutes acceptance of the License.  A copy of      *
*                     the License is available at                      *
*                                                                      *
*         http://www.research.att.com/sw/license/cpl-1.0.html          *
*         (with md5 checksum 8a5e0081c856944e76c69a1cf29c2e8b)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
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
