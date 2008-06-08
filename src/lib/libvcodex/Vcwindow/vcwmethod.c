/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2008 AT&T Intellectual Property          *
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
extern Vcwmethod_t	_Vcwprefix;
extern Vcwmethod_t	_Vcwmirror;
extern Vcwmethod_t	_Vcwvote;
_END_EXTERNS_

static Vcwmethod_t* _Vcwmethods[] =
{	
	&_Vcwprefix,
	&_Vcwmirror,
	&_Vcwvote,
	0
};

#if __STD_C
Vcwmethod_t* vcwgetmeth(char* name)
#else
Vcwmethod_t* vcwgetmeth(name)
char*		name;
#endif
{
	int	i, k;

	if(!name)
		return NIL(Vcwmethod_t*);

	for (i = 0; _Vcwmethods[i]; ++i)
	{	for(k = 0; name[k] && _Vcwmethods[i]->name[k]; ++k)	
			if(name[k] != _Vcwmethods[i]->name[k])
				break;
		if(name[k] == 0) /* match a prefix */
			return _Vcwmethods[i];
	}

	return NIL(Vcwmethod_t*);
}

#if __STD_C
int vcwwalkmeth(Vcwalk_f walkf, Void_t* disc)
#else
int vcwwalkmeth(Vcwalk_f walkf, Void_t* disc)
Vcwalk_t	walkf;
Void_t*		disc;
#endif
{
	int	i, rv;

	if(!walkf)
		return -1;

	for (i = 0; _Vcwmethods[i]; ++i)
	{	rv = (*walkf)((Void_t*)_Vcwmethods[i],
				_Vcwmethods[i]->name, _Vcwmethods[i]->desc, disc);
		if(rv < 0)
			return rv;
	}

	return 0;
}
