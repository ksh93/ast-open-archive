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
 * pax gzip format
 */

#include "format.h"

static int
gzip_getprologue(Pax_t* pax, Format_t* fp, Archive_t* ap, File_t* f, unsigned char* buf, size_t size)
{
	off_t		pos;
	unsigned char	num[4];

	if (size < 2 || buf[0] != 0x1f || buf[1] != 0x8b)
		return 0;
	if ((pos = lseek(ap->io->fd, (off_t)0, SEEK_CUR)) < 0 || lseek(ap->io->fd, (off_t)-4, SEEK_END) <= 0 || read(ap->io->fd, num, sizeof(num)) != sizeof(num))
		ap->uncompressed = ap->io->size * 6;
	else
		ap->uncompressed = (num[0]) |
				   (num[1] << 8) |
				   (num[2] << 16) |
				   (num[3] << 24);
	lseek(ap->io->fd, pos, SEEK_SET);
	return 1;
}

static Compress_format_t	pax_gzip_data =
{
	"-9",
	{ "gunzip" },
	{ "ratz", "-c" },
};

Format_t	pax_gzip_format =
{
	"gzip",
	0,
	"gzip compression",
	0,
	COMPRESS|IN|OUT,
	0,
	0,
	0,
	PAXNEXT(pax_gzip_next),
	&pax_gzip_data,
	0,
	gzip_getprologue,
};

PAXLIB(&pax_gzip_format)