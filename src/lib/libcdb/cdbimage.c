/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1997-2000 AT&T Corp.                *
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
*                                                                  *
*******************************************************************/
#pragma prototyped

/*
 * cdb image function
 *
 * NOTE: this code exposes the sfio write stream { data next endb } members
 */

#include <sfio_t.h>

#include "cdblib.h"

typedef struct
{	
	Sfdisc_t	disc;		/* the sfio discipline		*/
	Cdb_t*		cdb;		/* cdb handle			*/
	ssize_t		size;		/* span prefix length		*/
	ssize_t		peek;		/* first real buffer		*/
	unsigned char	data[1];	/* span prefix buffer		*/
} Span_t;

/*
 * buffer span exception function
 */

static int
spanexcept(Sfio_t* f, int type, void* data, Sfdisc_t* disc)
{
	register Span_t*	span = (Span_t*)disc;

	switch (type)
	{
	case SF_DPOP:
	case SF_FINAL:
		free(span);
		break;
	case SF_READ:
		if (!span->peek)
		{
			if ((span->size = sftell(f) - span->cdb->record->offset) > f->size)
			{
				message((-11, "AHA OVER %6I*d              %9I*d s  %9I*d t  %9I*d o", sizeof(span->cdb->count), span->cdb->count, sizeof(span->size), span->size, sizeof(Sfoff_t), sftell(f), sizeof(span->cdb->record->offset), span->cdb->record->offset));
				span->size = 0;
			}
			else
			{
				memcpy(span->data, f->data + f->size - span->size, span->size);
				message((-11, "AHA KEEP %d %6I*d %9I*d c  %9I*d s  %9I*d t  %9I*d o", sffileno(f), sizeof(span->cdb->count), span->cdb->count, sizeof(ssize_t), *(ssize_t*)data, sizeof(span->size), span->size, sizeof(Sfoff_t), sftell(f), sizeof(span->cdb->record->offset), span->cdb->record->offset));
				sfsk(f, sftell(f), SEEK_SET, disc);
			}
		}
		break;
	}
	return 0;
}

/*
 * buffer span read function
 */

static ssize_t
spanread(Sfio_t* f, void* buf, size_t n, Sfdisc_t* disc)
{
	register Span_t*	span = (Span_t*)disc;
	ssize_t			m;

	if (span->peek)
	{
		m = span->size - span->peek;
		if (m > n)
			m = n;
		memcpy(buf, span->data + span->peek, m);
		message((-11, "AHA PEEK %d %6I*d %9I*d n  %9I*d m  %9I*d o %9I*d z `%-.64s'", sffileno(f), sizeof(span->cdb->count), span->cdb->count, sizeof(n), n, sizeof(m), m, sizeof(span->peek), span->peek, sizeof(span->size), span->size, buf));
		if ((span->peek += m) >= span->size)
			span->peek = 0;
		return m;
	}
	return sfrd(f, buf, n, disc);
}

/*
 * catch records that span sfio buffer boundaries
 */

static int
sfdcspan(Sfio_t* f, Cdb_t* cdb)
{
	register Span_t*	span;

	if (!(span = (Span_t*)newof(0, Span_t, 1, f->endb - f->data)))
		return -1;
	span->disc.readf = spanread;
	span->disc.exceptf = spanexcept;
	span->cdb = cdb;
	span->size = f->endb - f->data;
	span->peek = f->next - f->data;
	memcpy(span->data, f->data, span->size);
	sfpurge(f);
	cdb->record->cache.image = (void*)span;
	if (sfdisc(f, (Sfdisc_t*)span) != (Sfdisc_t*)span)
	{	
		free(span);
		return -1;
	}
	sfset(f, SF_IOCHECK, 1);
	return 0;
}

/*
 * return the image of the record rp
 */

ssize_t
cdbimage(Cdb_t* cdb, Cdbrecord_t* rp, Sfio_t* op, int tc)
{
	register Span_t*	span = (Span_t*)rp->cache.image;
	unsigned char*		b;
	ssize_t			n;
	ssize_t			z;

	z = cdb->io->next - cdb->io->data;
	n = sftell(cdb->io) - rp->offset;
	if (n > z && span)
	{
		/*
		 * record spans the buffer
		 */
		
		message((-11, "AHA SPAN %6I*d %9I*d n  %9I*d z  %9I*d t  %9I*d o", sizeof(cdb->count), cdb->count, sizeof(n), n, sizeof(z), z, sizeof(Sfoff_t), sftell(cdb->io), sizeof(span->cdb->record->offset), span->cdb->record->offset));
		if (span->size && sfwrite(op, span->data, span->size) != span->size)
			return -1;
		n = z;
	}
	if (n <= z)
	{
		/*
		 * the record (remainder) is in the stream buffer
		 */

		b = cdb->io->next - n;
		if (rp->schema->terminator.chr >= 0)
			n--;
		if (sfwrite(op, b, n) != n)
			return 0;
		message((-11, "AHA IMAGE tc=%03o chr=%03o `%-.*s'", tc, rp->schema->terminator.chr, n, b));
		if (tc >= 0)
		{
			sfputc(op, tc);
			n++;
		}
	}
	if (!span)
		sfdcspan(cdb->io, cdb);
	return n;
}
