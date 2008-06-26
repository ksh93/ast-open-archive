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
