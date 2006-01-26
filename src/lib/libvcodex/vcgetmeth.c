/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 2003-2006 AT&T Corp.                  *
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
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#include	"vcmeth.h"

/*	Return a method by its string name.
**
**	Written by Kiem-Phong Vo (kpv@research.att.com)
*/

#if _PACKAGE_ast
#include <dlldefs.h>
#endif

static Vcmethod_t*	methods = VCFIRST;
static int		scanned = 0;

#if _PACKAGE_ast

static const char	id[] = "vcodex";

typedef Vcmethod_t* (*Vclib_f) _ARG_((const char*));

#if __STD_C
Vcmethod_t* plugin(Void_t* dll, const char* path)
#else
Vcmethod_t* plugin(dll, path)
Void_t*		dll;
char*		path;
#endif
{
	Vcmethod_t*	meth;
	Vclib_f		libf;
	char		sym[sizeof(id)+4];

	sprintf(sym, "%s_lib", id);
	if ((libf = (Vclib_f)dlllook(dll, sym)) && (meth = (*libf)(path)))
	{
		if (!meth->link)
		{
			meth->link = methods;
			methods = meth;
		}
		return meth;
	}
	dlclose(dll);
	return 0;
}

#endif

#if __STD_C
Vcmethod_t* vcgetmeth(const char* name, const char* ident, char** next)
#else
Vcmethod_t* vcgetmeth(name, ident, next)
char*	name;	/* != NULL if search by name	*/
char*	ident;	/* != NULL if search by ident	*/
char**	next;
#endif
{
	Vcmethod_t	*p;
	char		*s, *m;
#if _PACKAGE_ast
	Dllscan_t	*dls;
	Dllent_t	*dle;
	Void_t		*dll;
	char		path[PATH_MAX];
	char		buf[256];
#endif

	if(!name && !ident)
	{
#if _PACKAGE_ast
		if(!scanned)
		{	scanned = 1;
			if (dls = dllsopen(id, NiL, NiL))
			{	while (dle = dllsread(dls))
					if (dll = dlopen(dle->path, RTLD_LAZY))
						plugin(dll, dle->path);
				dllsclose(dls);
			}
		}
#endif
		return methods;
	}

	if(name)
	{	for (p = methods; p; p = p->link)
		{	s = (char*)name;
			m = p->name;
			do
			{	if (ISWINSEP(*s) || ISPUNCT(*s) || ISEOS(*s))
				{	if (ISEOS(*m))
					{	if(next)
							*next = s;
						return p;
					}
					break;
				}
			} while (*s++ == *m++);
		}
#if _PACKAGE_ast
		m = buf;
		for(s = (char*)name; !ISPUNCT(*s) && !ISEOS(*s); s++)
			if(m < &buf[sizeof(buf)-1])
				*m++ = *s;
		*m = 0;
		if (dll = dllplug(id, buf, NIL(char*), RTLD_LAZY, path, sizeof(path)))
		{
			if(next)
				*next = s;
			return plugin(dll, path);
		}
#endif
	}
	else
	{	for (p = methods; p; p = p->link)
			if (strcmp(ident, p->ident) == 0)
				return p;
	}

	return NIL(Vcmethod_t*);
}

#if __STD_C
Vcmethod_t* vcnextmeth(Vcmethod_t* meth)
#else
Vcmethod_t* vcnextmeth(meth)
Vcmethod_t*	meth;
#endif
{
	return meth ? meth->link : meth;
}
