/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1989-2005 AT&T Corp.                  *
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
*                  David Korn <dgk@research.att.com>                   *
*                   Eduardo Krell <ekrell@adexus.cl>                   *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * libast 3d symbol mappings to minimize pollution
 */

#ifndef _NAME3D_AST_H
#define _NAME3D_AST_H

#define _ast_state	_3d_ast_state
#define _ast_info	_3d_ast_info
#define _hash_info_	_3d_hash_info
#define fmterror	_3d_fmterror
#define fmttime		_3d_fmttime
#define hashalloc	_3d_hashalloc
#define hashdone	_3d_hashdone
#define hashfree	_3d_hashfree
#define hashlook	_3d_hashlook
#define hashnext	_3d_hashnext
#define hashscan	_3d_hashscan
#define hashsize	_3d_hashsize
#define hashwalk	_3d_hashwalk
#define pathbin		_3d_pathbin
#define pathcat		_3d_pathcat
#define pathcheck	_3d_pathcheck
#define pathshell	_3d_pathshell
#define pathtemp	_3d_pathtemp
#define strgrpmatch	_3d_strgrpmatch
#define strmatch	_3d_strmatch
#define strsubmatch	_3d_strsubmatch
#define tokscan		_3d_tokscan
#define touch		_3d_touch

#include <ast_lib.h>

#undef	_lib_fmterror

#endif
