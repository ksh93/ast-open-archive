/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2002-2011 AT&T Intellectual Property          *
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
 * dss library private implementation header
 *
 * Glenn Fowler
 * AT&T Research
 */

#ifndef _DSSHDR_H
#define _DSSHDR_H	1

struct Format_s; typedef struct Format_s Format_t;

#define _DSS_PRIVATE_ \
	Format_t*		print;

#include "dsslib.h"

/*
 * library private globals
 */

#define dssmethinit		_dss_meth_init

extern Dssmeth_t*		dssmethinit(const char*, const char*, const char*, Dssdisc_t*, Dssmeth_t*);

#endif
