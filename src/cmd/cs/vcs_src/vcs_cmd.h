/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1990-2000 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*                 This software was created by the                 *
*                 Network Services Research Center                 *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
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
