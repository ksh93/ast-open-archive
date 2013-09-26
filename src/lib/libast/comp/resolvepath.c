/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1985-2013 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                 Eclipse Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
*                                                                      *
*                A copy of the License is available at                 *
*          http://www.eclipse.org/org/documents/epl-v10.html           *
*         (with md5 checksum b35adb5213ca9657e911e9befb180842)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                     Phong Vo <phongvo@gmail.com>                     *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * resolvepath implementation
 */

#define resolvepath	______resolvepath

#include <ast.h>
#include <error.h>

#undef	resolvepath

#undef	_def_map_ast
#undef	_AST_API_IMPLEMENT

#include <ast_map.h>
#include <ast_api.h>

#if defined(__EXPORT__)
#define extern	__EXPORT__
#endif

extern int
resolvepath(const char* file, char* path, size_t size)
{
	char*	s;

	return (s = pathdev(AT_FDCWD, file, path, size, PATH_PHYSICAL|PATH_ABSOLUTE|PATH_DOTDOT|PATH_EXISTS, NiL)) ? (s - path) : -1;
}
