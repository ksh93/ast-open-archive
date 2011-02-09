/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2000-2011 AT&T Intellectual Property          *
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
 * integer tuple list internal/external implementation
 *
 * Glenn Fowler
 * AT&T Research
 */

#include "itl.h"

#include <error.h>

#define ITLINT		uint16_t
#define ITLINTERNAL	itl2internal
#define ITLEXTERNAL	itl2external

#include "itlie.h"

#define ITLINT		uint8_t
#define ITLINTERNAL	itl1internal
#define ITLEXTERNAL	itl1external

#include "itlie.h"

#define ITLINT		uint32_t
#define ITLINTERNAL	itl4internal
#define ITLEXTERNAL	itl4external

#include "itlie.h"
