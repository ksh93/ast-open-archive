/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1986-2004 AT&T Corp.                  *
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
