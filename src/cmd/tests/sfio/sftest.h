/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1999-2004 AT&T Corp.                  *
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
*                                                                      *
***********************************************************************/
#include	"FEATURE/sfio"

#if !_PACKAGE_ast
#ifdef __STDC__
#include	<stdarg.h>
#else
#include	<varargs.h>
#endif
#endif

#if _SFIO_H_ONLY
#include	<sfio.h>
#else
#include	<sfio_t.h>
#endif

#if _PACKAGE_ast
#include	<ast.h>
#endif

#if _hdr_unistd
#include	<unistd.h>
#endif

#include	<errno.h>

#include	"terror.h"

#ifndef EINTR
#define EINTR	4
#endif
#ifndef EBADF
#define EBADF	9
#endif
#ifndef EAGAIN
#define EAGAIN	11
#endif
#ifndef ENOMEM
#define ENOMEM	12
#endif
#ifndef ESPIPE
#define ESPIPE	29
#endif

#if _hdr_math
#include	<math.h>
#endif

#if _hdr_float
#include	<float.h>
#endif
