/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1997-2004 AT&T Corp.                  *
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
