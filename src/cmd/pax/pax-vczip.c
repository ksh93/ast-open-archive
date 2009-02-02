/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1987-2009 AT&T Intellectual Property          *
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
 * pax vczip format
 */

#include "format.h"

static int
vczip_getprologue(Pax_t* pax, Format_t* fp, Archive_t* ap, File_t* f, unsigned char* buf, size_t size)
{
	if (size < 4 || buf[0] != 0326 || buf[1] != 0303 || buf[2] != 0304 || buf[3] && buf[3] != 0330)
		return 0;
	ap->uncompressed = ap->io->size * 25;
	return 1;
}

static Compress_format_t	pax_vczip_data =
{
	0,
	{ "vcunzip" },
	{ 0 },
};

Format_t	pax_vczip_format =
{
	"vczip",
	0,
	"vcodex compression; the details value is the compression \b--method\b=\amethod\a",
	0,
	COMPRESS|IN|OUT,
	0,
	0,
	0,
	PAXNEXT(pax_vczip_next),
	&pax_vczip_data,
	0,
	vczip_getprologue,
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

PAXLIB(&pax_vczip_format)