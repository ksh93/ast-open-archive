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
 * Glenn Fowler
 * AT&T Research
 *
 * cql db format support library method private interface
 */

#ifndef _CDBMETHOD_H
#define _CDBMETHOD_H

#define _CDB_RECORD_METHOD_ \
	Sfoff_t		offset;		/* record offset		*/

#define _CDB_METHOD_ \
	struct								   \
	{								   \
	void*		data;		/* method private data		*/ \
	unsigned long	flags;		/* method private flags		*/ \
	}		details;	/* method private info		*/ \
	Cdbrecord_t*	record;		/* current record		*/

#include <cdb.h>

#endif
