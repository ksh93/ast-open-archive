/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1986-2008 AT&T Intellectual Property          *
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
	return error_info.errors;
}
