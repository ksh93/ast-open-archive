/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1989-2000 AT&T Corp.              *
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
 * AT&T Bell Laboratories
 *
 * make abstract machine library private definitions
 */

#ifndef _MAMLIB_H
#define _MAMLIB_H

struct frame				/* target stack frame		*/
{
	struct frame*	next;		/* next in list			*/
	struct frame*	prev;		/* prev in list			*/
	struct rule*	rule;		/* rule for this frame		*/
};

#define _MAM_RULE_PRIVATE		/* rule private			*/ \
	struct block*	atail;		/* action tail			*/

#define _MAM_PROC_PRIVATE		/* proc private			*/ \
	struct frame*	bp;		/* proc frame base pointer	*/ \
	struct frame*	fp;		/* proc frame pointer		*/ \
	struct proc*	next;		/* next in list of all procs	*/ \
	struct proc*	stail;		/* sibling tail			*/

#define _MAM_MAM_PRIVATE		/* mam private			*/ \
	struct proc*	procs;		/* list of all procs		*/

#include <mam.h>

#endif
