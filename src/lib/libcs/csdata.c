/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1990-2001 AT&T Corp.                *
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
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * cs library data
 */

static const char id_cs[] = "\n@(#)$Id: libcs (AT&T Research) 2001-05-11 $\0\n";

#include "cslib.h"

static Csdisc_t	disc = { CS_VERSION };

Cs_t		cs = { "libcs:cs", &disc };
