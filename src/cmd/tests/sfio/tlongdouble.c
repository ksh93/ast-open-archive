/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1999-2000 AT&T Corp.                *
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
*                                                                  *
*******************************************************************/
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

	TSTRETURN(0);
}
