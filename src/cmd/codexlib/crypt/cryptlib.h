/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1986-2004 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*    If you have copied or used this software without agreeing     *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                David Korn <dgk@research.att.com>                 *
*                 Phong Vo <kpv@research.att.com>                  *
*                                                                  *
*******************************************************************/
#pragma prototyped

/*
 * crypt decoder/encoder private definitions
 */

#ifndef _CRYPTLIB_H
#define _CRYPTLIB_H	1

#include <codex.h>

typedef unsigned _ast_int1_t ui1;
typedef unsigned _ast_int2_t ui2;
typedef unsigned _ast_int4_t ui4;

#define crypt_first		(&crypt_rar)
#define crypt_rar_next		(&crypt_zip)
#define crypt_zip_next		0

extern Codexmeth_t		crypt_rar;
extern Codexmeth_t		crypt_rar_13;
extern Codexmeth_t		crypt_rar_15;
extern Codexmeth_t		crypt_rar_20;
extern Codexmeth_t		crypt_zip;

extern const ui4		crc_tab[256];

#endif
