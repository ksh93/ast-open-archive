/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1986-2001 AT&T Corp.                *
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
 * G. S. Fowler
 * AT&T Bell Laboratories
 *
 * standalone C preprocessor wrapper
 */

#include <ast.h>

#include "pp.h"

int
main(int argc, char** argv)
{
	NoP(argc);
	ppop(PP_LINE, ppline);
	ppop(PP_PRAGMA, pppragma);
	ppop(PP_DEFAULT, PPDEFAULT);
	optjoin(argv, ppargs, NiL);
	ppop(PP_STANDALONE);
	ppop(PP_INIT);
	ppcpp();
	ppop(PP_DONE);
	exit(error_info.errors);
}
