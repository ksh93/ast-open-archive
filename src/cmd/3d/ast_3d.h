/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1990-2000 AT&T Corp.              *
*      and it may only be used by you under license from       *
*                     AT&T Corp. ("AT&T")                      *
*       A copy of the Source Code Agreement is available       *
*              at the AT&T Internet web site URL               *
*                                                              *
*     http://www.research.att.com/sw/license/ast-open.html     *
*                                                              *
*     If you received this software without first entering     *
*       into a license with AT&T, you have an infringing       *
*           copy and cannot use it without violating           *
*             AT&T's intellectual property rights.             *
*                                                              *
*               This software was created by the               *
*               Network Services Research Center               *
*                      AT&T Labs Research                      *
*                       Florham Park NJ                        *
*                                                              *
*             Glenn Fowler <gsf@research.att.com>              *
*              David Korn <dgk@research.att.com>               *
*               Eduardo Krell <ekrell@adexus.cl>               *
*                                                              *
***************************************************************/
#pragma prototyped
/*
 * libast 3d symbol mappings to minimize pollution
 */

#ifndef _NAME3D_AST_H
#define _NAME3D_AST_H

#define _ast_state	_3d_ast_state
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

#include <ast_lib.h>

#undef	_lib_fmterror

#define _3d_fmttime(a,b)	"[NOW]"

#endif
