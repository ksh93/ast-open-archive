/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1990-2002 AT&T Corp.                *
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
*                                                                  *
*******************************************************************/
/*
 *	tag.c
 *
 */

#include "vcs_rscs.h"

/*
 *	Always point to next tag after call 
 */

tag_t*	get_tag(f, tp)
	Sfio_t*	f;
	register tag_t*	tp;
{
	register char *s;
	int	len;

	s = (char *)tp + sizeof(int);
	(void) memset((char *)tp, 0, sizeof(tag_t));
	if (!sfread(f, (char *)&(tp->length), sizeof(int)) || (sfread(f, (char *)s, (len = tp->length - sizeof(int))) != len))
	{
		return (NULL);
	}

	if (tp->type & LOG)
	{
		tp->del = WHERE(f);
		ADVANCE(f, tp->dsize);
	}
	
	return (tp);
}



int new_tag(tp, sp, v, dsize, domain, type)
	register tag_t*	tp;
	struct stat*	sp;
	char*		v;
	int		dsize;
	int		domain;
	int		type;
{
	memset((char *)tp, 0, sizeof(tag_t));
	tp->type = type;
	tp->dsize = dsize;
	tp->stat = *sp;
	tp->stat.st_ctime = cs.time;
	tp->domain = (domain ? domain : getmydomain());
	strcpy(tp->version, v);
	tp->length = sizeof(tag_t) - MAXVID + strlen(v) + 1;
	return (tp->length);
}

int keycmp(tp1, tp2)
	register tag_t*	tp1;
	register tag_t*	tp2;
{
	int	n;

	if ((n = strcmp(tp1->version, tp2->version)))
		return (n);

	return (tp1->domain - tp2->domain);
}
