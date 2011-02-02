/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1986-2011 AT&T Intellectual Property          *
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
*                  David Korn <dgk@research.att.com>                   *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * zip decoder private interface
 */

#ifndef _ZIP_H
#define _ZIP_H		1

#include <codex.h>

typedef  uint8_t uch;
typedef uint16_t ush;
typedef uint32_t ulg;

extern Codexmeth_t	codex_zip_shrink;
extern Codexmeth_t	codex_zip_reduce;
extern Codexmeth_t	codex_zip_implode;
extern Codexmeth_t	codex_zip_deflate;

#endif
