/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1998-2001 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*                 This software was created by the                 *
*                 Network Services Research Center                 *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*******************************************************************/
#pragma prototyped

/*
 * sfio pzip discipline
 */

#include "pzlib.h"

#define GZ_MAGIC_1	0x1f		/* 1st gzip magic char		*/
#define GZ_MAGIC_2	0x8b		/* 2nd gzip magic char		*/
#define LZ_MAGIC_2	0x9d		/* 2nd lzw magic char		*/

#define PZ_GZ_MAGOFF	10		/* compressed magic offset	*/
#define PZ_GZ_MAGIC_1	0x92		/* 1st compressed magic char	*/
#define PZ_GZ_MAGIC_2	0x17		/* 2nd compressed magic char	*/

typedef struct
{
	Sfdisc_t	sfdisc;		/* sfio discipline		*/
	Pzdisc_t	disc;		/* pzip discipline		*/
	Pz_t*		pz;		/* pz handle			*/
} Sfpzip_t;

/*
 * pzip exception handler
 * free on close
 */

static int
sfpzexcept(Sfio_t* sp, int op, void* val, Sfdisc_t* dp)
{
	register Sfpzip_t*	pz = (Sfpzip_t*)dp;
	int			r;

	NoP(sp);
	switch (op)
	{
	case SF_ATEXIT:
		sfdisc(sp, SF_POPDISC);
		return 0;
	case SF_CLOSE:
	case SF_DPOP:
	case SF_FINAL:
		if (pz->pz)
		{
			r = pzclose(pz->pz);
			pz->pz = 0;
		}
		else
			r = 0;
		if (op != SF_CLOSE)
			free(dp);
		return r;
	case SF_SYNC:
		return val ? 0 : pzsync(pz->pz);
	case SFPZ_HANDLE:
		return (*((Pz_t**)val) = pz->pz) ? 1 : -1;
	}
	return 0;
}

/*
 * sfio pzip discipline read
 */

static ssize_t
sfpzread(Sfio_t* fp, Void_t* buf, size_t size, Sfdisc_t* dp)
{
	register Sfpzip_t*	pz = (Sfpzip_t*)dp;

	return pzread(pz->pz, buf, size);
}

/*
 * sfio pzip discipline write
 */

static ssize_t
sfpzwrite(Sfio_t* fp, const Void_t* buf, size_t size, Sfdisc_t* dp)
{
	register Sfpzip_t*	pz = (Sfpzip_t*)dp;

	/* NOTE: fp is wrong here! */
	return (pzwrite(pz->pz, fp, (void*)buf, size) < 0) ? -1 : size;
}

/*
 * create and push the sfio pzip discipline
 *
 * (flags&PZ_STAT) return
 *	>0	is a pzip file
 *	 0	not a pzip file
 *	<0	error
 * otherwise flags have pzopen() semantics and return
 *	>0	discipline pushed (one or more of { pzip gzip lzw })
 *	 0	discipline not needed
 *	<0	error
 */

int
sfdcpzip(Sfio_t* sp, Pzdisc_t* disc, const char* path, unsigned long flags)
{
	Sfio_t*		io;
	Sfpzip_t*	pz;
	Pz_t*		oz;

	if (flags & PZ_HANDLE)
	{
		oz = (Pz_t*)sp;
		sp = oz->io;
	}
	else
		oz = 0;
	if (sp->flags & SF_WRITE)
	{
		if (flags & PZ_STAT)
			return -1;
	}
	else if (!(flags & PZ_FORCE))
	{
		unsigned char*	s;
		int		r;
		int		m1;
		int		m2;

		if (!(s = (unsigned char*)sfreserve(sp, PZ_GZ_MAGOFF + 2, 1)))
			return -1;
		m1 = s[0];
		m2 = s[1];
		r = m1 == PZ_MAGIC_1 && m2 == PZ_MAGIC_2 ||
		    m1 == GZ_MAGIC_1 && m2 == GZ_MAGIC_2 &&
		    s[PZ_GZ_MAGOFF] == PZ_GZ_MAGIC_1 && s[PZ_GZ_MAGOFF+1] == PZ_GZ_MAGIC_2;
		sfread(sp, s, 0);
		if (flags & PZ_STAT)
			return r;
		if (!r)
		{
			if (!(flags & PZ_NOGZIP) && m1 == GZ_MAGIC_1)
			{
				if (m2 == GZ_MAGIC_2)
					r = sfdcgzip(sp, 0);
				else if (m2 == LZ_MAGIC_2)
					r = sfdclzw(sp, 0);
			}
			return r;
		}
		sfsync(sp);
	}
	if (!(io = sfnew(NiL, NiL, SF_UNBOUND, sffileno(sp), (sp->flags & (SF_READ|SF_WRITE)))))
		return -1;
	if (!(pz = newof(0, Sfpzip_t, 1, 0)))
	{
		io->file = -1;
		sfclose(io);
		return -1;
	}
	pz->disc.version = PZ_VERSION;
	flags &= ~(PZ_READ|PZ_WRITE|PZ_STAT|PZ_STREAM|PZ_INTERNAL);
	flags |= PZ_STREAM|((sp->flags & SF_READ) ? PZ_READ : PZ_WRITE);
	if (oz && (oz->flags & PZ_WRITE))
		flags |= PZ_DELAY;
	if (disc)
	{
		pz->disc.errorf = disc->errorf;
		pz->disc.window = disc->window;
		pz->disc.options = disc->options;
		if (disc->splitf)
			flags |= PZ_ACCEPT;
	}
	if (!(pz->pz = pzopen(&pz->disc, (char*)io, flags)))
	{
		io->file = -1;
		sfclose(io);
		free(pz);
		return -1;
	}
	if (path)
		pz->pz->path = path;
	pz->sfdisc.exceptf = sfpzexcept;
	if (sp->flags & SF_READ)
		pz->sfdisc.readf = sfpzread;
	else
		pz->sfdisc.writef = sfpzwrite;
	sp->file = open("/dev/null", 0);
	sfset(sp, SF_SHARE|SF_PUBLIC, 0);
	if (sfdisc(sp, &pz->sfdisc) != &pz->sfdisc)
	{
		close(sp->file);
		sp->file = io->file;
		sfseek(sp, sftell(io), SEEK_SET);
		io->file = -1;
		pzclose(pz->pz);
		free(pz);
		return -1;
	}
	if (oz)
		oz->flags |= pz->pz->flags & PZ_INTERNAL;
	return 1;
}
