/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1990-2000 AT&T Corp.              *
*      and it may only be used by you under license from       *
*                     AT&T Corp. ("AT&T")                      *
*       A copy of the Source Code Agreement is available       *
*              at the AT&T Internet web site URL               *
*                                                              *
*     http://www.research.att.com/sw/license/ast-open.html     *
*                                                              *
*     If you received this software without first entering     *
*       into a license with AT&T, you have an infringing       *
*           copy and cannot use it without violating           *
*             AT&T's intellectual property rights.             *
*                                                              *
*               This software was created by the               *
*               Network Services Research Center               *
*                      AT&T Labs Research                      *
*                       Florham Park NJ                        *
*                                                              *
*             Glenn Fowler <gsf@research.att.com>              *
*                                                              *
***************************************************************/
/*
 *	rs_dir()
 */

#include "vcs_rscs.h"

static time_t	now;

static rdirent_t* add_entry(head, tp)
	rdirent_t*	head;
	tag_t*		tp;
{
	register rdirent_t*	ndp;
	register rdirent_t*	dp;
	register rdirent_t**	prev_posn;
	int			result;
	tag_t*			ntp;
	char*			link = NULL;
	int			marker;
	

	if (R_ISLINK(tp))
		link = rs_readlink(tp->version);

	marker = (R_ISMARKER(tp) ? 1 : 0);


		
	dp = head;
	prev_posn = &head;
	while (dp != NULL)
	{
		if (!marker && R_ISMARKER(dp->tag))
		{
			if ((dp->tag->stat.st_ctime < tp->stat.st_ctime) && markermatch(dp->tag->version, tp->version))
			{
				*prev_posn = dp->next;
				ndp = dp->next;
				free((char *)dp->tag);
				free((char *)dp);
				dp = ndp;
				continue;
			}
		}
		if ((result = strcmp(dp->tag->version, tp->version)) == 0)
		{
			/* check if the minor key (domain) is the same */
			if (dp->tag->domain == tp->domain)
			{
				if (dp->tag->stat.st_ctime > tp->stat.st_ctime)
					return (head);
				ntp = (tag_t *)malloc(tp->length);
				memcpy((char *)ntp, (char *)tp, tp->length);
				free((char *)dp->tag);
				dp->tag = ntp;
				if (R_ISLINK(tp) && link)
					dp->link = strdup(link);
				return (head);
			}
		}
		else if (result > 0)
		{
			ndp = (rdirent_t *)malloc(sizeof(rdirent_t));
			ntp = (tag_t *)malloc(tp->length);
			memcpy((char *)ntp, (char *)tp, tp->length);
			ndp->tag = ntp;
			ndp->next = dp;
			if (R_ISLINK(tp) && link)
				ndp->link = strdup(link);
			*prev_posn = ndp;
			return (head);
		}
		prev_posn = &(dp->next);
		dp = dp->next;
	}
	ndp = (rdirent_t *)malloc(sizeof(rdirent_t));
	ntp = (tag_t *)malloc(tp->length);
	memcpy((char *)ntp, (char *)tp, tp->length);
	ndp->tag = ntp;
	ndp->next = NULL;
	if (R_ISLINK(tp) && link)
		ndp->link = strdup(link);
	*prev_posn = ndp;
	return (head);
}


rdirent_t* rs_dir(rf, ap)
	Sfio_t*		rf;
	register attr_t*	ap;
{
	tag_t			tag;
	register tag_t*		tp;
	rdirent_t*		head;
	register rdirent_t*	dp;
	register rdirent_t*	ndp;
	register rdirent_t**	prev_posn;

	now = cs.time;
	
	tp = &tag;
	head = NULL;
	TOLOG(rf, ap);
	while(get_tag(rf, tp))
		head = add_entry(head, tp);

	TOTAG(rf, ap);
	while((WHERE(rf)<ap->del_reg) && get_tag(rf, tp))
		head = add_entry(head, tp);

	/*
	 * remove expired marker 
	 */
	dp = head;
	prev_posn = &head;
	while (dp != NULL)
	{
		if (R_ISMARKER(dp->tag) && dp->tag->stat.st_mtime < now)
		{
			*prev_posn = dp->next;
			ndp = dp->next;
			free((char *)dp->tag);
			free((char *)dp);
			dp = ndp;
		}
		else
		{
			prev_posn = &(dp->next);
			dp = dp->next;
		}

	}
	
	return (head);

}




