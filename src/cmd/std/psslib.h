/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1989-2003 AT&T Corp.                *
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
 * process status stream private interface
 */

#ifndef _PSSLIB_H
#define _PSSLIB_H	1

#define _PSS_PRIVATE_ \
	Pssdisc_t*	disc;		/* user discipline		*/ \
	Vmalloc_t*	vm;		/* vm region			*/ \
	Pssent_t*	ent;		/* last entry			*/ \
	Pss_id_t	pid;		/* scan pid			*/ \
	unsigned long	boot;		/* boot time			*/ \
	Dt_t*		ttybyname;	/* tty by name hash		*/ \
	Dt_t*		ttybydev;	/* tty by dev hash		*/ \
	Dtdisc_t	ttybynamedisc;	/* tty by name disc		*/ \
	Dtdisc_t	ttybydevdisc;	/* tty by dev disc		*/ \
	void*		data;		/* private data			*/ \
	int		ttyscan;	/* -1:bad 0:init 2:scanned	*/ \
	char		buf[1024];	/* work and return value buffer	*/

#include <ast.h>
#include <ast_dir.h>
#include <dt.h>
#include <ctype.h>
#include <dirent.h>
#include <error.h>
#include <ls.h>
#include <vmalloc.h>

#if __CYGWIN__

#include <ast_windows.h>
#include <sys/cygwin.h>

#endif

#include <pss.h>

#include "FEATURE/procfs"

typedef struct Tty_s			/* tty hash			*/
{
	Dtlink_t	byname;		/* by name dict link		*/
	Dtlink_t	bydev;		/* by dev dict link		*/
	Pss_dev_t	dev;		/* dev				*/
	char		name[1];	/* base name			*/
} Tty_t;

#if !_mem_st_rdev_stat
#define st_rdev		st_dev
#endif

extern Pssmeth_t*	_pss_method;
extern Pssmeth_t*	_pss_ps;

#endif
