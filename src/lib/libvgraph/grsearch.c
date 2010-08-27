/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1986-2010 AT&T Intellectual Property          *
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
*                  David Korn <dgk@research.att.com>                   *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#include	"grhdr.h"

/*	Depth-first search
**
**	Written by Kiem-Phong Vo
*/

typedef struct _dfsgraph_s
{	Grdata_t	data;
	Grnode_t*	node;	/* search stack		*/
} Dfsgraph_t; 

typedef struct _dfsnode_s
{
	Grdata_t	data;
	Gredge_t*	edge;	/* edge being searched	*/
} Dfsnode_t;

Grnode_t* grdfs(Graph_t* gr)
{
	return 0;
}
