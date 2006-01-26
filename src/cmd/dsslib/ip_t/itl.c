/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 2000-2006 AT&T Corp.                  *
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
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * integer tuple list internal/external implementation
 *
 * Glenn Fowler
 * AT&T Labs Research
 */

#include "itl.h"

#include <error.h>

#define ITLINT		unsigned _ast_int2_t
#define ITLINTERNAL	itl2internal
#define ITLEXTERNAL	itl2external

#include "itlie.h"

#define ITLINT		unsigned _ast_int4_t
#define ITLINTERNAL	itl4internal
#define ITLEXTERNAL	itl4external

#include "itlie.h"
