/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1989-2002 AT&T Corp.                *
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
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * process status stream interface definitions
 */

#ifndef _PSS_H
#define _PSS_H		1

#include <ast.h>

#define PSS_VERSION		20020121L

#define PSS_ALL			(1<<0)		/* match all		*/
#define PSS_ATTACHED		(1<<1)		/* match attached	*/
#define PSS_DETACHED		(1<<2)		/* match detached	*/
#define PSS_LEADER		(1<<3)		/* match group leaders	*/
#define PSS_TTY			(1<<4)		/* match disc.tty	*/
#define PSS_UID			(1<<5)		/* match disc.uid	*/
#define PSS_VERBOSE		(1<<6)		/* report all errors	*/

#define PSS_FLAGS		0xff
#define PSS_NODEV		((dev_t)(-1))
#define PSS_SCAN		0
#define PSS_ZOMBIE		'Z'

#define PSS_addr		(1<<0)
#define PSS_args		(1<<1)
#define PSS_command		(1<<2)
#define PSS_cpu			(1<<3)
#define PSS_flags		(1<<4)
#define PSS_gid			(1<<5)
#define PSS_nice		(1<<6)
#define PSS_npid		(1<<7)
#define PSS_pgrp		(1<<8)
#define PSS_pid			(1<<9)
#define PSS_ppid		(1<<10)
#define PSS_pri			(1<<11)
#define PSS_proc		(1<<12)
#define PSS_refcount		(1<<13)
#define PSS_rss			(1<<14)
#define PSS_sched		(1L<<15)
#define PSS_sid			(1L<<16)
#define PSS_size		(1L<<17)
#define PSS_start		(1L<<18)
#define PSS_state		(1L<<19)
#define PSS_tgrp		(1L<<20)
#define PSS_time		(1L<<21)
#define PSS_tty			(1L<<22)
#define PSS_uid			(1L<<23)
#define PSS_wchan		(1L<<24)

#define PSS_all		(PSS_addr|PSS_args|PSS_command|PSS_cpu|PSS_flags|\
			PSS_gid|PSS_nice|PSS_npid|PSS_pgrp|PSS_pid|PSS_ppid|\
			PSS_pri|PSS_proc|PSS_refcount|PSS_rss|PSS_sched|\
			PSS_sid|PSS_size|PSS_start|PSS_state|PSS_tgrp|\
			PSS_time|PSS_tty|PSS_uid|PSS_wchan)
#define PSS_match	(PSS_gid|PSS_pgrp|PSS_sid|PSS_tty|PSS_uid)

#undef	hz			/* who gets the prize for this?		*/

struct Pss_s;
struct Pssdisc_s;

typedef struct Pssent_s
{
	void*		addr;
	void*		wchan;

	char*		args;
	char*		command;
	char*		sched;

	dev_t		tty;

	size_t		rss;
	size_t		size;

	time_t		start;

	unsigned long	time;

	pid_t		npid;
	pid_t		pid;
	pid_t		pgrp;
	pid_t		ppid;
	pid_t		sid;
	pid_t		tgrp;

	gid_t		gid;
	uid_t		uid;

	long		nice;
	long		pri;

	int		cpu;
	int		flags;
	int		proc;
	int		refcount;
	int		state;
} Pssent_t;

typedef struct Pssdata_s
{
	struct Pssdata_s*	next;
	unsigned long		data;
} Pssdata_t;

typedef struct Pssmatch_s
{
	struct Pssmatch_s*	next;
	unsigned long		field;
	struct Pssdata_s*	data;
} Pssmatch_t;

typedef struct Pssdisc_s
{
	unsigned long	version;	/* interface version		*/
	unsigned long	fields;		/* PSS_[a-z]* field requests	*/
	unsigned long	flags;		/* PSS_[A-Z]* flags		*/
	dev_t		tty;		/* PSS_TTY match value		*/
	pid_t		uid;		/* PSS_UID match value		*/
	Pssmatch_t*	match;		/* match these fields		*/
	Error_f		errorf;		/* error function		*/
} Pssdisc_t;

typedef struct Pss_s
{
	const char*	id;		/* library id string		*/
	unsigned long	fields;		/* supported fields		*/
	int		hz;		/* secs=Psent_t.time/Pss_t.hz	*/

#ifdef _PSS_PRIVATE_
	_PSS_PRIVATE_
#endif

} Pss_t;

extern Pss_t*		pssopen(Pssdisc_t*);
extern Pssent_t*	pssread(Pss_t*, pid_t);
extern Pssent_t*	psssave(Pss_t*, Pssent_t*);
extern int		pssclose(Pss_t*);

extern dev_t		pssttydev(Pss_t*, const char*);
extern char*		pssttyname(Pss_t*, dev_t);

#endif
