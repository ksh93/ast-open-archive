/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1990-2005 AT&T Corp.                  *
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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#include "vcs_rscs.h"

struct rfile_t
{
	char*		path;
	Sfio_t*	fd;
	attr_t*		ap;
	rdirent_t*	list;
};

typedef struct rfile_t	rfile_t;

struct version_t
{
	char*		version;
	char*		path;
	int		domain;
	Sfio_t*	fd;
	tag_t*		tp;
};

typedef struct version_t version_t;
#define RSCS_DIR	"RSCS"		/* or "..." inside 3d */
