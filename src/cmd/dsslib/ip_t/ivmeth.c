/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2000-2009 AT&T Intellectual Property          *
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
#include "ivlib.h"

typedef struct Method_s
{
	const char*	name;
	Ivmeth_t**	meth;
} Method_t;

#define IVMETHOD(m)	extern Ivmeth_t ## Iv ## m;

#include "ivmethods.h"

#define IVMETHOD(m)	#m, & Iv ## m,

static Method_t		methods[] =
{
#include "ivmethods.h"
};

Ivmeth_t*
ivmeth(const char* name)
{
	int	i;

	if ((name[0] == 'i' || name[0] == 'I') && (name[1] == 'v' || name[1] == 'V'))
		name += 2;
	for (i = 0; i < elementsof(methods); i++)
		if (!strncmp(methods[i].name, name, 4))
			return *methods[i].meth;
	return 0;
}
