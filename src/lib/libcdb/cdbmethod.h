/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1997-2002 AT&T Corp.                *
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
