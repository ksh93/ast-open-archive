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
#pragma prototyped
                  
/*
 * Glenn Fowler
 * AT&T Labs Research
 *
 * connect stream server support
 */

#ifndef _CSSLIB_H
#define _CSSLIB_H

#include "cslib.h"

#include <error.h>
#include <sig.h>
#include <tok.h>

#define EXPIRE		60
#define KEYEXPIRE	(60*5)
#define KEYMASK		0x7fffffff
#define TOSS(k)		while((k=CSTOSS(k,cs.time))<=CS_KEY_MAX)

typedef struct
{
	int		seq;
	unsigned long	expire;
	time_t		atime;
	time_t		mtime;
	Csid_t		id;
} Auth_t;

typedef struct
{
	dev_t		dev;
	ino_t		ino;
} Fid_t;

struct Common_s;
typedef struct Common_s Common_t;

#define _CSS_FD_PRIVATE_		\
	Css_t*		css;		\
	int		events;

#define _CSS_PRIVATE_			\
	Cssdisc_t*	disc;		\
	Css_t*		next;		\
	char		buf[PATH_MAX];	\
	char		tmp[PATH_MAX];	\
	int		auth;		\
	int		challenge;	\
	int		fdpending;	\
	int		fdpolling;	\
	int		fdlistening;	\
	int		fduser;		\
	Fid_t		fid[2];		\
	gid_t		gid;		\
	uid_t		uid;		\
	unsigned long	newkey;		\
	unsigned long	oldkey;		\
	unsigned long	conkey;		\
	unsigned long	timeout_last;	\
	unsigned long	timeout_remain;	\
	unsigned long	wakeup_last;	\
	unsigned long	wakeup_remain;

#include <css.h>

struct Common_s
{
	Css_t*		servers;
	Css_t*		main;
	Auth_t*		auth;
	int		nauth;
	Cspoll_t*	fdpoll;
	Cssfd_t*	fdinfo;
	unsigned long	flags;
	int		fdpending;
	int		fdpolling;
	int		fdbefore;
	int		fdloop;
	int		fdnext;
	int		fdmax;
	int		pid;
	int		polling;
	unsigned long	expire;
};

#endif
