/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1992-2004 AT&T Corp.                *
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
*                                                                  *
*******************************************************************/
#pragma prototyped

/*
 * Glenn Fowler
 * AT&T Research
 *
 * dll scan interface
 */

static const char usage[] =
"[-?\n@(#)$Id: dlls (AT&T Labs Research) 2004-01-28 $\n]"
USAGE_LICENSE
"[+NAME?dlls - list dlls and shared libraries on $PATH]"
"[+DESCRIPTION?\bdlls\b lists the base name and full path, one per line, of"
"	\adlls\a and \ashared libraries\a found in the directories or"
"	sibling directories in the \bPATH\b environment variable. The"
"	\aplugin\a operand matches plugin libraries for the particular"
"	\aplugin\a. If omitted or \b-\b then no plugins are matched. The"
"	\aname\a operand is the library base name. All base names are matched"
"	if omitted or \b-\b. The \aversion\a operand specifies a specific"
"	library version and is an implementation specific sequence of"
"	decimal digits and dots. Only the first path for each library"
"	base name is listed. If no options are specified then"
"	\b--base\b \b--path\b is assumed.]"
"[b:base?List the \adll\a or \ashared library\a base names.]"
"[i:info?List native dll naming and location information.]"
"[p:path?List the \adll\a or \ashared library\a path names.]"

"\n\n"
"[ plugin [ name [ version ] ] ]\n"
"\n"

"[+SEE ALSO?\bfind\b(1), \bdllscan\b(3)]"
;

#include <cmd.h>
#include <dlldefs.h>

#define LIST_BASE	01
#define LIST_INFO	02
#define LIST_PATH	04

int
b_dlls(int argc, char** argv, void* context)
{
	int		i;
	int		r;
	int		flags;
	char*		arg[3];
	Dllscan_t*	dls;
	Dllent_t*	dle;
	Dllinfo_t*	dli;

	NoP(argc);
	cmdinit(argv, context, ERROR_CATALOG, 0);
	flags = 0;
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 0:
			break;
			continue;
		case 'b':
			flags |= LIST_BASE;
			continue;
		case 'i':
			flags |= LIST_INFO;
			continue;
		case 'p':
			flags |= LIST_PATH;
			continue;
		case ':':
			error(2, "%s", opt_info.arg);
			continue;
		case '?':
			error(ERROR_usage(2), "%s", opt_info.arg);
			continue;
		}
		break;
	}
	argc -= opt_info.index;
	argv += opt_info.index;
	if (error_info.errors || argc > 3)
		error(ERROR_usage(2), "%s", optusage(NiL));
	r = 0;
	if (flags & LIST_INFO)
	{
		if (!(dli = dllinfo()))
			error(2, "cannot determine native dll info");
		sfprintf(sfstdout, "sibling=(%s", dli->sibling[0]);
		for (i = 1; dli->sibling[i]; i++)
			sfprintf(sfstdout, " %s", dli->sibling[i]);
		sfprintf(sfstdout, ") env=%s prefix=%s suffix=%s flags=", dli->env, dli->prefix, dli->suffix);
		i = 0;
		if (dli->flags & DLL_INFO_PREVER)
		{
			i = 1;
			sfprintf(sfstdout, "PREVER");
		}
		if (dli->flags & DLL_INFO_DOTVER)
		{
			if (i)
				sfputc(sfstdout, '|');
			sfprintf(sfstdout, "DOTVER");
		}
		sfputc(sfstdout, '\n');
	}
	else if (!(flags & (LIST_BASE|LIST_PATH)))
		flags = LIST_BASE|LIST_PATH;
	if (flags &= (LIST_BASE|LIST_PATH))
	{
		for (i = 0; i < elementsof(arg); i++)
			arg[i] = (i >= argc || streq(argv[i], "-")) ? (char*)0 : argv[i];
		r = 1;
		if (dls = dllsopen(arg[0], arg[1], arg[2]))
		{
			while (dle = dllsread(dls))
			{
				r = 0;
				switch (flags)
				{
				case LIST_BASE:
					sfprintf(sfstdout, "%s\n", dle->name);
					break;
				case LIST_PATH:
					sfprintf(sfstdout, "%s\n", dle->path);
					break;
				default:
					sfprintf(sfstdout, "%14s %s\n", dle->name, dle->path);
					break;
				}
			}
			dllsclose(dls);
		}
	}
	return r || error_info.errors;
}
