/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1997-2005 AT&T Corp.                  *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                            by AT&T Corp.                             *
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
 * cdb image function
 *
 * NOTE: this code exposes the sfio write stream { data next endb } members
 */

#include <sfio_t.h>

#include "cdblib.h"

/*
 * write the image of the last record rp to op
 * if tc>=0 then it is appended to the record
 * the record image size is returned
 */

ssize_t
cdbimage(Cdb_t* cdb, Cdbrecord_t* rp, Sfio_t* op, int tc)
{
	unsigned char*		b;
	ssize_t			r;
	ssize_t			n;
	ssize_t			z;

	r = 0;
	z = cdb->io->_next - cdb->io->_data;
	n = sftell(cdb->io) - rp->offset;
	if (n > z)
	{
		/*
		 * record spans the buffer
		 */
		
		if (sfwrite(op, rp->image.data, rp->image.size) != rp->image.size)
			return -1;
		r += rp->image.size;
		n = z;
	}
	if (n)
	{
		/*
		 * the record (remainder) is in the stream buffer
		 */

		r += n;
		b = cdb->io->_next - n;
		if (rp->schema->terminator.chr >= 0)
			n--;
		if (n && sfwrite(op, b, n) != n)
			return 0;
		if (tc >= 0)
			sfputc(op, tc);
	}
	return r;
}
