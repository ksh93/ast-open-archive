/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1985-2004 AT&T Corp.                  *
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
*                  David Korn <dgk@research.att.com>                   *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * process library definitions
 */

#ifndef _PROCLIB_H
#define _PROCLIB_H

#include <ast.h>
#include <errno.h>
#include <sig.h>

#if _lib_sigprocmask
typedef sigset_t Sig_mask_t;
#else
typedef unsigned long Sig_mask_t;
#endif

struct Mods_s;

#define _PROC_PRIVATE_ \
	struct Mod_s*	mods;		/* process modification state	*/ \
	long		flags;		/* original PROC_* flags	*/ \
	Sig_mask_t	mask;		/* original blocked sig mask	*/ \
	Sig_handler_t	sigchld;	/* PROC_FOREGROUND SIG_DFL	*/ \
	Sig_handler_t	sigint;		/* PROC_FOREGROUND SIG_IGN	*/ \
	Sig_handler_t	sigquit;	/* PROC_FOREGROUND SIG_IGN	*/

#include <proc.h>

#define proc_default	_proc_info_	/* hide external symbol		*/

extern Proc_t		proc_default;	/* first proc			*/

#ifndef errno
extern int		errno;
#endif

#endif
