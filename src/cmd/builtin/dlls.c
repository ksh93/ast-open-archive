/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*           Copyright (c) 1992-2006 AT&T Knowledge Ventures            *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                      by AT&T Knowledge Ventures                      *
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
*                  David Korn <dgk@research.att.com>                   *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * Glenn Fowler
 * AT&T Research
 *
 * dll scan interface
 */

static const char usage[] =
"[-?\n@(#)$Id: dlls (AT&T Research) 2006-06-24 $\n]"
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
"[+?\asymbol\a operands, if specified, are searched for in each matched"
"	library. The address or lookup error is listed for each library"
"	and \asymbol\a.]"
"[b:base?List the \adll\a or \ashared library\a base names.]"
"[c:containing?Only list libraries containing at least one of the \asymbol\a"
"	operands.]"
"[i:info?List native dll naming and location information.]"
"[p:path?List the \adll\a or \ashared library\a path names.]"

"\n\n"
"[ plugin [ name [ version [ symbol ... ] ] ] ]\n"
"\n"

"[+SEE ALSO?\bfind\b(1), \bdllscan\b(3)]"
;

#include <cmd.h>
#include <dlldefs.h>

#define LIST_BASE	0x1
#define LIST_INFO	0x2
#define LIST_ONLY	0x4
#define LIST_PATH	0x8

int
b_dlls(int argc, char** argv, void* context)
{
	int		i;
	int		r;
	int		flags;
	int		only;
	char**		syms;
	char*		arg[3];
	void*		dll;
	void*		sym;
	Dllscan_t*	dls;
	Dllent_t*	dle;
	Dllinfo_t*	dli;

	NoP(argc);
	cmdinit(argv, context, ERROR_CATALOG, 0);
	flags = 0;
	only = 0;
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 0:
			break;
		case 'b':
			flags |= LIST_BASE;
			continue;
		case 'c':
			only = 1;
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
	argv += opt_info.index;
	if (error_info.errors)
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
			if (arg[i] = *argv)
				argv++;
		if (only && !*argv)
			error(ERROR_usage(2), "%s", optusage(NiL));
		r = 1;
		for (;;)
		{
			if (dls = dllsopen(arg[0], arg[1], arg[2]))
			{
				while (dle = dllsread(dls))
				{
					r = 0;
					if (!only)
					{
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
						if (*(syms = argv))
						{
							if (dll = dlopen(dle->path, RTLD_LAZY))
							{
								do
								{
									sfprintf(sfstdout, "               %14s ", *syms);
									if (sym = dlllook(dll, *syms))
										sfprintf(sfstdout, "%p\n", sym);
									else
										sfprintf(sfstdout, "%s\n", dlerror());
								} while (*++syms);
								dlclose(dll);
							}
							else
								sfprintf(sfstdout, "               %s\n", dlerror());
						}
					}
					else if (dll = dlopen(dle->path, RTLD_LAZY))
					{
						i = 1;
						for (syms = argv; *syms; syms++)
							if (sym = dlllook(dll, *syms))
							{
								if (i)
								{
									i = 0;
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
								sfprintf(sfstdout, "               %14s %p\n", *syms, sym);
							}
						dlclose(dll);
					}
				}
				dllsclose(dls);
			}
			if (!r || !arg[0] || streq(arg[0], "-") || !arg[1] || streq(arg[1], "-"))
				break;
			arg[0] = 0;
		}
	}
	return r || error_info.errors;
}
