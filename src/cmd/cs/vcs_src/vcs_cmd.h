/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1990-2004 AT&T Corp.                  *
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
