/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1989-2004 AT&T Corp.                *
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
*                          AT&T Research                           *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                David Korn <dgk@research.att.com>                 *
*                 Eduardo Krell <ekrell@adexus.cl>                 *
*                                                                  *
*******************************************************************/
#pragma prototyped

#define NoN(x)

#include "3d.h"

static char	conf[] = "";

#define astconf(a,b,c)	conf

#include "state.c"

#include "getcwd.c"
#include "getwd.c"
#include "hashalloc.c"
#include "hashfree.c"
#include "hashlook.c"
#include "hashscan.c"
#include "hashsize.c"
#include "hashwalk.c"
#include "memset.c"

#if DEBUG
#include "fmterror.c"
#endif

#if FS
#include "eaccess.c"
#include "pathcat.c"
#include "pathtemp.c"
#include "pathtmp.c"

char*
pathbin(void)
{
	char*	p;

	if (!(p = state.envpath) && !(p = getenv("PATH")))
		p = ":/bin:/usr/bin:/usr/ucb";
	return(p);
}

#include "tokscan.c"
#include "touch.c"

#endif

#include "strmatch.c"
#include "sigcrit.c"
#include "waitpid.c"

#undef	_real_vfork

#define close	CLOSE
#define fcntl	FCNTL
#define read	READ
#define write	WRITE

#include "pathshell.c"

/*
 * 3d doesn't handle spawnve() yet
 * we need spawnveg3d()
 */

#if _lib_fork || _lib_vfork
#undef	_lib_spawnve
#endif

#include "spawnveg.c"	/* follows spawnve.c because of #undef's */

#include "gross.c"

#if _map_malloc

#undef	calloc
#undef	free
#undef	malloc
#undef	realloc
#undef	strdup

extern void*	calloc(size_t, size_t);
extern void	free(void*);
extern void*	malloc(size_t);
extern void*	realloc(void*, size_t);
extern char*	strdup(const char*);

extern void*	_ast_calloc(size_t n, size_t m) { return calloc(n, m); }
extern void	_ast_free(void* p) { free(p); }
extern void*	_ast_malloc(size_t n) { return malloc(n); }
extern void*	_ast_realloc(void* p, size_t n) { return realloc(p, n); }
extern char*	_ast_strdup(const char* s) { return strdup(s); }

#endif
