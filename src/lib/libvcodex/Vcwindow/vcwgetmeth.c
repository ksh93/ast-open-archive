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
#include	"vchdr.h"

/*	Return a windowing method by its string name.
**
**	Written by Kiem-Phong Vo (kpv@research.att.com)
*/

/* List of currently supported windowing methods */
_BEGIN_EXTERNS_
extern Vcwmethod_t	_Vcwmirror;
extern Vcwmethod_t	_Vcwvote;
extern Vcwmethod_t	_Vcwprefix;
_END_EXTERNS_

static Vcwmethod_t* _Vcwmethods[] =
{	&_Vcwmirror,
	&_Vcwvote,
	&_Vcwprefix,
	0
};

#if __STD_C
Vcwmethod_t* vcwgetmeth(const char* name)
#else
Vcwmethod_t* vcwgetmeth(name)
char*		name;	/* != NULL if search by name	*/
#endif
{
	int	i;

	if(!name)
		return _Vcwmethods[0];

	for (i = 0; _Vcwmethods[i]; ++i)
		if (strcmp(name, _Vcwmethods[i]->name) == 0)
			return _Vcwmethods[i];

	return NIL(Vcwmethod_t*);
}

#if __STD_C
Vcwmethod_t* vcwnextmeth(Vcwmethod_t* meth)
#else
Vcwmethod_t* vcwnextmeth(meth)
Vcwmethod_t*	meth;
#endif
{
	int	i;

	for (i = 0; _Vcwmethods[i]; ++i)
		if (_Vcwmethods[i] == meth)
			return _Vcwmethods[i+1];
	return NIL(Vcwmethod_t*);
}
