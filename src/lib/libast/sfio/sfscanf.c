/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1985-2004 AT&T Corp.                  *
*                      and is licensed under the                       *
*          Common Public License, Version 1.0 (the "License")          *
*                        by AT&T Corp. ("AT&T")                        *
*      Any use, downloading, reproduction or distribution of this      *
*      software constitutes acceptance of the License.  A copy of      *
*                     the License is available at                      *
*                                                                      *
*         http://www.research.att.com/sw/license/cpl-1.0.html          *
*         (with md5 checksum 8a5e0081c856944e76c69a1cf29c2e8b)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#include	"sfhdr.h"

/*	Read formated data from a stream
**
**	Written by Kiem-Phong Vo.
*/

#if __STD_C
int sfscanf(Sfio_t* f, const char* form, ...)
#else
int sfscanf(va_alist)
va_dcl
#endif
{
	va_list	args;
	reg int	rv;

#if __STD_C
	va_start(args,form);
#else
	reg Sfio_t*	f;
	reg char*	form;
	va_start(args);
	f = va_arg(args,Sfio_t*);
	form = va_arg(args,char*);
#endif

	rv = (f && form) ? sfvscanf(f,form,args) : -1;
	va_end(args);
	return rv;
}

#if __STD_C
int sfvsscanf(const char* s, const char* form, va_list args)
#else
int sfvsscanf(s, form, args)
char*	s;
char*	form;
va_list	args;
#endif
{
	Sfio_t	f;

	if(!s || !form)
		return -1;

	/* make a fake stream */
	SFCLEAR(&f,NIL(Vtmutex_t*));
	f.flags = SF_STRING|SF_READ;
	f.bits = SF_PRIVATE;
	f.mode = SF_READ;
	f.size = strlen((char*)s);
	f.data = f.next = f.endw = (uchar*)s;
	f.endb = f.endr = f.data+f.size;

	return sfvscanf(&f,form,args);
}

#if __STD_C
int sfsscanf(const char* s, const char* form,...)
#else
int sfsscanf(va_alist)
va_dcl
#endif
{
	va_list		args;
	reg int		rv;
#if __STD_C
	va_start(args,form);
#else
	reg char*	s;
	reg char*	form;
	va_start(args);
	s = va_arg(args,char*);
	form = va_arg(args,char*);
#endif

	rv = (s && form) ? sfvsscanf(s,form,args) : -1;
	va_end(args);
	return rv;
}
