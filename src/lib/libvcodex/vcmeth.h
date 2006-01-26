/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 2003-2006 AT&T Corp.                  *
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
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
/*	Builtin method list definitions.
**
**	Written by Kiem-Phong Vo (kpv@research.att.com)
*/

#ifndef _VCMETH_H
#define _VCMETH_H	1

#if _PACKAGE_ast
#include	<ast.h>
#else
#include	<ast_common.h>
#endif

#if __STD_C
#define VCNEXT(m)	_##m##_next
#define VCLIB(m)	Vcmethod_t* m = &_##m;
#else
#define VCNEXT(m)	_/**/m/**/_next
#define VCLIB(m)	Vcmethod_t* m = &_/**/m;
#endif

#include	"vchdr.h"

/* List of data transforms under test */

#define _Vc2gdiff_next		0

/* List of builtin data transforms */

_BEGIN_EXTERNS_
#define VCFIRST			&_Vcdelta
extern Vcmethod_t		_Vcdelta;
#define _Vcdelta_next		&_Vchuffman
extern Vcmethod_t		_Vchuffman;
#define _Vchuffman_next		&_Vchuffgroup
extern Vcmethod_t		_Vchuffgroup;
#define _Vchuffgroup_next	&_Vchuffpart
extern Vcmethod_t		_Vchuffpart;
#define _Vchuffpart_next	&_Vcbwt
extern Vcmethod_t		_Vcbwt;
#define _Vcbwt_next		&_Vcrle
extern Vcmethod_t		_Vcrle;
#define _Vcrle_next		&_Vcmtf
extern Vcmethod_t		_Vcmtf;
#define _Vcmtf_next		&_Vcmap
extern Vcmethod_t		_Vcmap;
#define _Vcmap_next		&_Vctranspose
extern Vcmethod_t		_Vctranspose;
#define _Vctranspose_next	0
_END_EXTERNS_

/* List of plugin data transforms via vcgetmeth(0,0,0) */

#endif
