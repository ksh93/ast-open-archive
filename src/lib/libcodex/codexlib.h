/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 2003-2004 AT&T Corp.                  *
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
 * codex private interface
 * included by <codex.h>
 */

#ifndef _CODEXLIB_H
#define _CODEXLIB_H

#ifndef SFDCEVENT
#define SFDCEVENT(a,b,n)	((((a)-'A'+1)<<11)^(((b)-'A'+1)<<6)^(n))
#endif

#define CODEX_DATA		SFDCEVENT('C','X','d')	/* get data	*/
#define CODEX_GETPOS		SFDCEVENT('C','X','g')	/* get position	*/
#define CODEX_SETPOS		SFDCEVENT('C','X','s')	/* set position	*/
#define CODEX_SIZE		SFDCEVENT('C','X','z')	/* set size	*/

typedef struct Codexcache_s
{
	Codex_t*	code;
	Codexnum_t	flags;
	Codexnum_t	cached;
	char		name[CODEX_NAME];
} Codexcache_t;

typedef struct Codexstate_s
{
	const char*	id;
	Codexmeth_t*	first;
	Codexdisc_t	disc;
	Codex_t		error;
	Codex_t*	last;
	char*		trace;
	char*		verbose;
	Codexnum_t	index;
	int		serial;
	unsigned int	initialized;
	unsigned int	scanned;
	unsigned int	cached;	
	Codexcache_t	cache[16];
} Codexstate_t;

#define CODEXERROR		(&codexstate.error)

#define codex_first		(&codex_uu)
#define codex_uu_next		(&codex_qp)
#define codex_qp_next		(&codex_rot13)
#define codex_rot13_next	(&codex_iconv)
#define codex_iconv_next	0

extern Codexmeth_t	codex_iconv;
extern Codexmeth_t	codex_qp;
extern Codexmeth_t	codex_rot13;
extern Codexmeth_t	codex_uu;

extern Codexstate_t	codexstate;

#endif
