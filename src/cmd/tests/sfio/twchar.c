/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1999-2002 AT&T Corp.                *
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
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*******************************************************************/
#include	"sftest.h"

#if _PACKAGE_ast
#ifndef _hdr_wchar
#define _hdr_wchar	1
#endif
#include	<stdio.h>
#endif

#if _hdr_wchar
#include	<wchar.h>
#endif

MAIN()
{
	int		n;
	wchar_t		wuf[256];
	char		buf[256];
	char		str[256];

	static char	tst[] = "hello-world";

#if defined(__STDC__) && defined(_hdr_wchar)

#if _PACKAGE_ast

	swprintf(wuf, sizeof(wuf), L"%ls", L"hello-world");
	wcstombs(str, wuf, sizeof(str));
	if (strcmp(tst, str))
		terror("swprintf %%ls \"%s\" expected, \"%s\" returned\n", tst, str);

	swprintf(wuf, sizeof(wuf), L"%S", L"hello-world");
	wcstombs(str, wuf, sizeof(str));
	if (strcmp(tst, str))
		terror("swprintf %%S \"%s\" expected, \"%s\" returned\n", tst, str);

	n = swscanf(L" hello-world ", L"%ls", &wuf);
	if (n != 1)
		terror("swscanf %%ls %d expected, %d returned\n", 1, n);
	else
	{
		wcstombs(str, wuf, sizeof(str));
		if (strcmp(tst, str))
			terror("swscanf %%ls \"%s\" expected, \"%s\" returned\n", tst, str);
	}

	n = swscanf(L" hello-world ", L"%S", &wuf);
	if (n != 1)
		terror("swscanf %%S %d expected, %d returned\n", 1, n);
	else
	{
		wcstombs(str, wuf, sizeof(str));
		if (strcmp(tst, str))
			terror("swscanf %%S \"%s\" expected, \"%s\" returned\n", tst, str);
	}

	swprintf(wuf, sizeof(wuf), L"%lc%lc%lc%lc%lc%lc%lc%lc%lc%lc%lc",
		L'h',L'e',L'l',L'l',L'o',L'-',L'w',L'o',L'r',L'l',L'd');
	wcstombs(str, wuf, sizeof(str));
	if (strcmp(tst, str))
		terror("swprintf %%lc \"%s\" expected, \"%s\" returned\n", tst, str);

	swprintf(wuf, sizeof(wuf), L"%C%C%C%C%C%C%C%C%C%C%C",
		L'h',L'e',L'l',L'l',L'o',L'-',L'w',L'o',L'r',L'l',L'd');
	wcstombs(str, wuf, sizeof(str));
	if (strcmp(tst, str))
		terror("swprintf %%C \"%s\" expected, \"%s\" returned\n", tst, str);

#endif

	sfsprintf(buf, sizeof(buf), "%ls", L"hello-world");
	if (strcmp(tst, buf))
		terror("sfsprintf %%ls \"%s\" expected, \"%s\" returned\n", tst, buf);

	sfsprintf(buf, sizeof(buf), "%S", L"hello-world");
	if (strcmp(tst, buf))
		terror("sfsprintf %%S \"%s\" expected, \"%s\" returned\n", tst, buf);

	n = sfsscanf(" hello-world ", "%ls", &wuf);
	if (n != 1)
		terror("sfsscanf %%ls %d expected, %d returned\n", 1, n);
	else
	{
		wcstombs(str, wuf, sizeof(str));
		if (strcmp(tst, str))
			terror("sfsscanf %%ls \"%s\" expected, \"%s\" returned\n", tst, str);
	}

	n = sfsscanf(" hello-world ", "%S", &wuf);
	if (n != 1)
		terror("sfsscanf %%S %d expected, %d returned\n", 1, n);
	else
	{
		wcstombs(str, wuf, sizeof(str));
		if (strcmp(tst, str))
			terror("sfsscanf %%S \"%s\" expected, \"%s\" returned\n", tst, str);
	}

	sfsprintf(buf, sizeof(buf), "%lc%lc%lc%lc%lc%lc%lc%lc%lc%lc%lc",
		L'h',L'e',L'l',L'l',L'o',L'-',L'w',L'o',L'r',L'l',L'd');
	if (strcmp(tst, buf))
		terror("sfsprintf %%lc \"%s\" expected, \"%s\" returned\n", tst, buf);

	sfsprintf(buf, sizeof(buf), "%C%C%C%C%C%C%C%C%C%C%C",
		L'h',L'e',L'l',L'l',L'o',L'-',L'w',L'o',L'r',L'l',L'd');
	if (strcmp(tst, buf))
		terror("sfsprintf %%C \"%s\" expected, \"%s\" returned\n", tst, buf);

#endif

	TSTEXIT(0);
}
