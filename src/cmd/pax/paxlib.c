/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1987-2003 AT&T Corp.                *
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
 * Glenn Fowler
 *
 * pax pax external library support
 */

#include "pax.h"

#undef	paxchecksum
#undef	paxcorrupt
#undef	paxdata
#undef	paxget
#undef	paxnospace
#undef	paxput
#undef	paxread
#undef	paxseek
#undef	paxstash
#undef	paxunread
#undef	paxwrite

static const char	null[] = "";
static const char	sep[] = ": ";

#define TXT(s,m)	(s)?sep:null,(s)?(m):null

static int
paxdata(Pax_t* pax, Paxarchive_t* ap, Paxfile_t* f, int fd, void* b, off_t n)
{
	return holewrite(fd, b, n) == n ? 0 : -1;
}

static void*
paxget(Pax_t* pax, Paxarchive_t* ap, off_t n, off_t* p)
{
	return bget(ap, n, p);
}

static int
paxput(Pax_t* pax, Paxarchive_t* ap, off_t n)
{
	bput(ap, n);
	return 0;
}

static off_t
paxread(Pax_t* pax, Paxarchive_t* ap, void* b, off_t n, off_t m, int must)
{
	return bread(ap, b, n, m, must);
}

static off_t
paxseek(Pax_t* pax, Paxarchive_t* ap, off_t pos, int op, int hard)
{
	return bseek(ap, pos, op, hard);
}

static char*
paxstash(Pax_t* pax, Value_t* v, const char* s, size_t z)
{
	return stash(v, s, z);
}

static int
paxunread(Pax_t* pax, Paxarchive_t* ap, void* b, off_t n)
{
	bunread(ap, b, n);
	return 0;
}

static int
paxwrite(Pax_t* pax, Paxarchive_t* ap, void* b, off_t n)
{
	bwrite(ap, b, n);
	return 0;
}

static int
paxcorrupt(Pax_t* pax, Paxarchive_t* ap, Paxfile_t* f, const char* msg)
{
	(*pax->errorf)(NiL, pax, 2, "%s%s%s: %s archive corrupt at %I*u%s%s", ap->name, TXT(f, f->name), ap->format->name, sizeof(off_t), paxseek(pax, ap, 0, SEEK_CUR, 0), TXT(msg, msg));
	return -1;
}

static int
paxchecksum(Pax_t* pax, Paxarchive_t* ap, Paxfile_t* f, unsigned long expected, unsigned long value)
{
	int	z;

	if (expected != value)
	{
		z = ((expected | value) & 0xff00) ? 4 : 2;
		(*pax->errorf)(NiL, pax, 2, "%s%s%s: %s archive checksum error -- expected %0*lx != %0*lx", ap->name, TXT(f, f->name), ap->format->name, z, expected, z, value);
		return -1;
	}
	return 0;
}

static int
paxnospace(Pax_t* pax)
{
	(*pax->errorf)(NiL, pax, 2, "out of space");
	return -1;
}

/*
 * initialize the external library callbacks
 */

void
paxinit(register Pax_t* pax, const char* id)
{
	pax->id = id;
	pax->errorf = errorf;

	pax->dataf = paxdata;
	pax->getf = paxget;
	pax->putf = paxput;
	pax->readf = paxread;
	pax->seekf = paxseek;
	pax->stashf = paxstash;
	pax->unreadf = paxunread;
	pax->writef = paxwrite;

	pax->corruptf = paxcorrupt;
	pax->checksumf = paxchecksum;
	pax->nospacef = paxnospace;
}
