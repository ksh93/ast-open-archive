/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1986-2003 AT&T Corp.                *
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
/*
 * archive scan/touch/extract implementation definitions
 */

#ifndef _ARDIRLIB_H
#define _ARDIRLIB_H	1

#include <ast.h>
#include <ardir.h>
#include <error.h>

#define ar_first_method		(&ar_omf)

#define ar_aix_next		(&ar_aixbig)
#define ar_aixbig_next		(&ar_local)
#define ar_local_next		(0)
#define ar_omf_next		(&ar_pdp11)
#define ar_pdp11_next		(&ar_s5r0)
#define ar_port_next		(&ar_aix)
#define ar_s5r0_next		(&ar_port)

#define ar_aix			_ar_aix
#define ar_aixbig		_ar_aixbig
#define ar_local		_ar_local
#define ar_omf			_ar_omf
#define ar_pdp11		_ar_pdp11
#define ar_port			_ar_port
#define ar_s5r0			_ar_s5r0

extern Ardirmeth_t		ar_aix;
extern Ardirmeth_t		ar_aixbig;
extern Ardirmeth_t		ar_local;
extern Ardirmeth_t		ar_omf;
extern Ardirmeth_t		ar_pdp11;
extern Ardirmeth_t		ar_port;
extern Ardirmeth_t		ar_s5r0;

#endif
