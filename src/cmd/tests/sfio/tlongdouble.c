/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1999-2005 AT&T Corp.                  *
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
