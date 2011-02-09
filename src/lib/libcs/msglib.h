/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1990-2011 AT&T Intellectual Property          *
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
 * syscall message implementation interface
 */

#ifndef _MSGLIB_H
#define _MSGLIB_H

#define CS_INTERFACE	2

#include <ast.h>

#include "cs_lib.h"

#include <msg.h>
#include <ast_dir.h>
#include <errno.h>
#include <ls.h>

#ifndef D_FILENO
#define D_FILENO(d)	(1)
#endif

#ifndef errno
extern int	errno;
#endif

#endif
