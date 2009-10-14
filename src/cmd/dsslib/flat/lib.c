/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2002-2009 AT&T Intellectual Property          *
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

#include <dss.h>

extern Dsslib_t	dss_lib_flat;

Dsslib_t*
dss_lib(const char* name, Dssdisc_t* disc)
{
	return &dss_lib_flat;
}
