/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1999-2004 AT&T Corp.                  *
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
#include	"sftest.h"

#if _typ_long_double
#include	<float.h>
#endif

MAIN()
{
#if _typ_long_double
	long double	ldval, ldmax;
	char		*s, *str;

	ldmax = LDBL_MAX;

	if(!(s = sfprints("%Le",ldmax)) )
		terror("sfprints failed1\n");
	if(!(str = malloc(strlen(s)+1)) )
		terror("Malloc failed\n");
	strcpy(str,s);

	if(sfsscanf(str,"%Le",&ldval) != 1)
		terror("sfsscanf failed\n");
	if(!(s = sfprints("%Le",ldval)) )
		terror("sfprints failed2\n");

	if(strcmp(s,str) != 0)
		terror("Bad conversion, expecting %s and getting %s\n",str,s);
#endif

	TSTEXIT(0);
}
