/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1999-2001 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*                 This software was created by the                 *
*                 Network Services Research Center                 *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*******************************************************************/
#include	"FEATURE/sfio"

#define _next	next
#define _endw	endw
#define _endr	endr
#define _endb	endb
#define _push	push
#define _flags	flags
#define _file	file
#define _data	data
#define _size	size
#define _val	val

#if _SFIO_H_ONLY
#include	<sfio.h>
#else
#include	<sfio_t.h>
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

#if _PACKAGE_ast
#include	<limits.h>
#endif

#if _hdr_math
#include	<math.h>
#endif

#if _hdr_float
#include	<float.h>
#endif
