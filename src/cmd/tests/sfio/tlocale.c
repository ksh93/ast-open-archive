/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1999-2001 AT&T Corp.                *
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
#include	"sftest.h"
#if _lib_locale
#include	<locale.h>
#endif

MAIN()
{
#if _lib_locale
	char		buf[128], cmp[128];
	float		d;
	int		n, decimal, thousand;
	struct lconv*	lv;

	setlocale(LC_ALL, "");

	if(!(lv = localeconv()))
		TSTEXIT(0);

	decimal = '.';
	if(lv->decimal_point && lv->decimal_point[0])
		decimal = lv->decimal_point[0];

	thousand = 0;
	if(lv->thousands_sep && lv->thousands_sep[0])
		thousand = lv->thousands_sep[0];
		
	if(thousand)
		sfsprintf(cmp, sizeof(cmp), "1%c000", thousand);
	else	sfsprintf(cmp, sizeof(cmp), "1000");
	sfsprintf(buf, sizeof(buf), "%'d", 1000);
	if(strcmp(buf, cmp) != 0)
		terror("Bad printing");
	
	if(thousand)
		sfsprintf(cmp, sizeof(cmp), "1%c000%c10", thousand, decimal);
	else	sfsprintf(cmp, sizeof(cmp), "1000%c10", decimal);
	d = 0.;
	if((n = sfsscanf(cmp, "%'f", &d)) != 1)
		terror("Scan error %d", n);
	if(d < 1000.099 || d > 1000.101)
		terror("Bad scanning");
	sfsprintf(buf, sizeof(buf), "%.2f", d);
	if(strcmp(buf, "1000.10") != 0)
		terror("Deep formatting error");
#endif

	TSTEXIT(0);
}
