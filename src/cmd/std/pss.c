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
 * process status stream implementation
 */


static const char id[] = "\n@(#)$Id: pss library (AT&T Labs Research) 2002-02-02 $\0\n";

static const char lib[] = "std:pss";

#define _PSS_PRIVATE_ \
	Pssdisc_t*	disc;		/* user discipline		*/ \
	Vmalloc_t*	vm;		/* vm region			*/ \
	DIR*		dir;		/* /proc dir stream		*/ \
	Pssent_t*	ent;		/* last entry			*/ \
	unsigned long	boot;		/* boot time			*/ \
	int		cmdsiz;		/* sizeof(pss->entry->command)	*/ \
	Dt_t*		ttys;		/* tty dev<=>name hash		*/ \
	void*		data;		/* private data			*/ \
	char		buf[1024];	/* work and return value buffer	*/

#include <ast.h>
#include <ast_dir.h>
#include <cdt.h>
#include <ctype.h>
#include <dirent.h>
#include <error.h>
#include <ls.h>
#include <vmalloc.h>

#if __CYGWIN__
#undef	dev_t
#define dev_t		int
#endif

#if _WIN32
#undef	SF_ERROR
#endif

#include <pss.h>

#include "FEATURE/procfs"

typedef struct Tty_s			/* tty hash			*/
{
	Dtlink_t	link;		/* dict link			*/
	dev_t		dev;		/* dev				*/
	char		name[1];	/* base name			*/
} Tty_t;

#if defined(_PS_dir) || __CYGWIN__ || _hdr_procinfo && _lib_getprocs

#if __CYGWIN__

#include <windows.h>
#include <sys/cygwin.h>

#ifndef PR_HZ
#define PR_HZ			100
#endif
#ifndef PRNODEV
#define PRNODEV			((dev_t)(-1))
#endif

#define PR_TTYMAP(d)		minor(d)

#undef	PR_CTIME
#define PR_CTIME(p)		(((p)->rusage_children.ru_utime.tv_sec+(p)->rusage_children.ru_stime.tv_sec)*PR_HZ+((((p)->rusage_children.ru_utime.tv_usec+(p)->rusage_children.ru_stime.tv_usec)*PR_HZ)/1000000))
#undef	PR_START
#define PR_START(p)		((p)->start_time)
#undef	PR_TIME
#define PR_TIME(p)		(((p)->rusage_self.ru_utime.tv_sec+(p)->rusage_self.ru_stime.tv_sec)*PR_HZ+((((p)->rusage_self.ru_utime.tv_usec+(p)->rusage_self.ru_stime.tv_usec)*PR_HZ)/1000000))

#undef	PSS_addr
#define PSS_addr	0
#undef	PSS_sched
#define PSS_sched	0
#undef	PSS_cpu
#define PSS_cpu		0
#undef	PSS_nice
#define PSS_nice	0
#undef	PSS_pri
#define PSS_pri		0
#undef	PSS_proc
#define PSS_proc	0
#undef	PSS_refcount
#define PSS_refcount	0
#undef	PSS_tgrp
#define PSS_tgrp	0
#undef	PSS_wchan
#define PSS_wchan	0

#else

#if !_mem_st_rdev_stat
#define st_rdev			st_dev
#endif

#if _hdr_procinfo && _lib_getprocs

#include <procinfo.h>
#include <sys/proc.h>

#if _mem_pi_pri_procsinfo64
#define procsinfo		procsinfo64
#else
#undef	PSS_pri
#define PSS_pri			0
#endif

typedef struct Getprocs_s
{
	struct procsinfo	entry[64];
	pid_t			pid;
	int			last;
	int			index;
	int			count;
} Getprocs_t;

#undef	PSS_gid
#define PSS_gid			0
#undef	PSS_npid
#define PSS_npid		0
#undef	PSS_proc
#define PSS_proc		0
#undef	PSS_sched
#define PSS_sched		0
#undef	PSS_tgrp
#define PSS_tgrp		0

#define PR_HZ			1

#else

#undef	_lib_getprocs

#ifdef _PS_dir

#if !_mem_pr_clname_prpsinfo
#undef	PSS_sched
#define PSS_sched			0
#endif

#if !_mem_pr_gid_prpsinfo
#undef	PSS_gid
#define PSS_gid				0
#endif

#if !_mem_pr_lttydev_prpsinfo
#undef	_mem_pr_lttydev_prpsinfo
#define _mem_pr_lttydev_prpsinfo	0
#endif

#if !_mem_pr_ntpid_prpsinfo
#undef	PSS_npid
#define PSS_npid			0
#else
#define pr_npid				pr_ntpid
#endif

#if !_mem_pr_pgrp_prpsinfo
#if _mem_pr_pgid_prpsinfo
#undef	_mem_pr_pgrp_prpsinfo
#define _mem_pr_pgrp_prpsinfo		1
#define pr_pgrp				pr_pgid
#else
#undef	PSS_pgrp
#define PSS_pgrp			0
#endif
#endif

#if !_mem_pr_psargs_prpsinfo
#undef	_mem_pr_psargs_prpsinfo
#define _mem_pr_psargs_prpsinfo		0
#endif

#if !_mem_pr_refcount_prpsinfo
#undef	PSS_refcount
#define PSS_refcount			0
#endif

#if !_mem_pr_rssize_prpsinfo
#undef	PSS_rss
#define PSS_rss				0
#endif

#if !_mem_pr_sonproc_prpsinfo
#undef	PSS_proc
#define PSS_proc			0
#endif

#if !_mem_pr_sid_prpsinfo
#undef	PSS_sid
#define PSS_sid				0
#define pr_sid				pr_tgrp
#endif

#if !_mem_pr_tgrp_prpsinfo
#undef	PSS_tgrp
#define PSS_tgrp			0
#define pr_tgrp				pr_pgrp
#endif

#endif

#endif

#endif

#ifdef PR_TTYMAP
#define PR_TTYDEV(d)			PR_TTYMAP(d)
#else
#define PR_TTYDEV(d)			(d)
#endif

/*
 * return the status entry for pid
 */

static Pssent_t*
ps(register Pss_t* pss, pid_t pid, int force)
{
	register unsigned long		fields = pss->disc->fields & PSS_all;
	register unsigned long		flags = pss->disc->flags;
	register Pssent_t*		pe;
	register char*			s;
	int				i;
	unsigned long			x;
	Pssmatch_t*			mp;
	Pssdata_t*			dp;

	/*
	 * method declarations
	 */

#if __CYGWIN__
	struct external_pinfo*		pr;
#else
#if _lib_getprocs
	register Getprocs_t*		gp;
	register struct procsinfo*	pr;
#else
	register int			fd;
	register struct prpsinfo*	pr;
	int				n;
#if defined(_PS_scan_binary) || defined(_PS_scan_format)
	struct stat			st;
#endif
#endif
#endif

	/*
	 * generic allocation
	 */

	if (!pss->ent && !(pss->ent = vmnewof(pss->vm, 0, Pssent_t, 1, 0)))
	{
		if (pss->disc->errorf)
			(*pss->disc->errorf)(pss, pss->disc, ERROR_SYSTEM|2, "out of space [pssent]");
		return 0;
	}
	pe = pss->ent;

	/*
	 * method query
	 */

#if __CYGWIN__
	if (!(pr = (struct external_pinfo*)cygwin_internal(CW_GETPINFO, pid)) || !pr->pid)
		return 0;
	pss->data = (void*)pr;
	pe->gid = pr->gid;
	pe->pgrp = pr->pgid;
	pe->sid = pr->sid;
	pe->tty = pr->ctty == PRNODEV ? PSS_NODEV : pr->ctty;
	pe->uid = pr->uid;
	if (pr->process_state&(PID_EXITED|PID_ZOMBIE))
		pe->state = PSS_ZOMBIE;
	else if (pr->process_state & PID_STOPPED)
		pe->state = 'S';
	else if (pr->process_state & PID_TTYIN)
		pe->state = 'I';
	else if (pr->process_state & PID_TTYOU)
		pe->state = 'O';
	else
		pe->state = ' ';
#else
#if _lib_getprocs
	gp = (Getprocs_t*)pss->data;
	while (gp->index >= gp->count)
	{
		if (gp->last)
			return 0;
		gp->index = 0;
		gp->count = getprocs(gp->entry, sizeof(gp->entry[0]), NiL, 0, &gp->pid, elementsof(gp->entry));
		if (gp->count < elementsof(gp->entry))
		{
			gp->last = 1;
			if (gp->count <= 0)
				return 0;
		}
		if (!gp->entry[0].pi_pid)
			gp->index++;
	}
	pr = gp->entry + gp->index++;
	pe->pgrp = pr->pi_pgrp;
	pe->pid = pr->pi_pid;
	pe->sid = pr->pi_sid;
	pe->tty = pr->pi_ttyp ? pr->pi_ttyd : PSS_NODEV;
	pe->uid = pr->pi_uid;
	switch (pr->pi_state)
	{
	case SACTIVE:
		pe->state = 'R';
		break;
	case SIDL:
		pe->state = 'I';
		break;
	case SSTOP:
		pe->state = 'T';
		break;
	case SSWAP:
		pe->state = 'W';
		break;
	case SZOMB:
		pe->state = 'Z';
		break;
	default:
		pe->state = 'O';
		break;
	}
#else
	if (!(pr = (struct prpsinfo*)pss->data) && !(pss->data = (void*)(pr = vmnewof(pss->vm, 0, struct prpsinfo, 1, 0))))
	{
		if (pss->disc->errorf)
			(*pss->disc->errorf)(pss, pss->disc, ERROR_SYSTEM|2, "out of space [struct prpsinfo]");
		return 0;
	}
	sfsprintf(pss->buf, sizeof(pss->buf), _PS_path_num, (unsigned long)pid, _PS_status);
	if ((fd = open(pss->buf, O_RDONLY|O_BINARY)) < 0)
	{
		if (pss->disc->errorf && ((pss->disc->flags & PSS_VERBOSE) || errno != ENOENT && errno != EACCES))
			(*pss->disc->errorf)(pss, pss->disc, ERROR_SYSTEM|2, "%lu: cannot stat process", pid);
		return 0;
	}
#ifdef _PS_scan_format
	if ((n = read(fd, pss->buf, sizeof(pss->buf))) <= 0 || fstat(fd, &st))
	{
		n = -1;
		errno = EINVAL;
	}
	else
	{
		memset(pr, sizeof(*pr), 0);
		n = sfsscanf(pss->buf, _PS_scan_format, _PS_scan_args(pr));
		if (n != _PS_scan_count)
			error(1, "%lu: scan count %d, expected %d", (unsigned long)pid, n, _PS_scan_count);
#ifdef _PS_scan_fix
		_PS_scan_fix(pr);
#endif
		pr->pr_uid = st.st_uid;
		pr->pr_gid = st.st_gid;
		pr->pr_nice = pr->pr_priority - 15;
		pr->pr_size /= 1024;
#if _mem_pr_rssize_prpsinfo
		pr->pr_rssize /= 1024;
#endif
#ifdef _PS_scan_boot
		if (!pss->boot)
		{
			register char*	s;
			Sfio_t*		fp;

			pss->boot = 1;
			if (fp = sfopen(NiL, "/proc/stat", "r"))
			{
				while (s = sfgetr(fp, '\n', 0))
					if (strneq(s, "btime ", 6))
					{
						pss->boot = strtol(s + 6, NiL, 10);
						break;
					}
				sfclose(fp);
			}
		}
		pr->pr_start = pss->boot + pr->pr_start / PR_HZ;
#endif
	}
#else
#ifdef _PS_scan_binary
	n = read(fd, pr, sizeof(*pr)) == sizeof(*pr) ? 1 : -1;
#else
	n = ioctl(fd, PIOCPSINFO, pr);
#endif
#endif
	close(fd);
	if (n < 0)
	{
		if (pss->disc->errorf)
			(*pss->disc->errorf)(pss, pss->disc, ERROR_SYSTEM|2, "%lu: cannot get process info", pid);
		return 0;
	}
#if _UTS
	if (pr->pr_pri > 99)
		pr->pr_pri = 99;
	else if (pr->pr_pri < 0)
		pr->pr_pri = 0;
	pr->pr_ttydev = makedev((pr->pr_ttydev >> 18) & ((1<<14)-1), pr->pr_ttydev & ((1<<18)-1));
#endif
#if PSS_gid
	pe->gid = pr->pr_gid;
#endif
#if PSS_pgrp
	pe->pgrp = pr->pr_pgrp;
#endif
	pe->pid = pr->pr_pid;
#if PSS_sid
	pe->sid = pr->pr_sid;
#endif
	pe->state = PR_ZOMBIE(pr) ? PSS_ZOMBIE : pr->pr_sname;
#if PSS_tgrp
	pe->tgrp = pr->pr_tgrp;
#endif
	pe->tty = (pr->pr_ttydev == (dev_t)PRNODEV) ? PSS_NODEV : pr->pr_ttydev;
	pe->uid = pr->pr_uid;
#endif
#endif

	/*
	 * generic filter
	 */

	if (!force && !(flags & PSS_ALL))
	{
		if (flags & (PSS_TTY|PSS_UID))
		{
			if ((flags & PSS_TTY) && pe->tty != PR_TTYDEV(pss->disc->tty))
				return 0;
			if ((flags & PSS_UID) && pe->uid != pss->disc->uid)
				return 0;
		}
		else if ((flags & PSS_ATTACHED) && pe->tty == PSS_NODEV)
			return 0;
		else if ((flags & PSS_DETACHED) && (pe->tty != PSS_NODEV || pe->state == PSS_ZOMBIE))
			return 0;
#if PSS_sid
		else if (!(flags & PSS_LEADER) && pe->pid == pe->sid)
			return 0;
#else
#if PSS_tgrp
		else if (!(flags & PSS_LEADER) && pe->pid == pe->tgrp)
			return 0;
#endif
#endif
		if (mp = pss->disc->match)
		{
			do
			{
				switch (mp->field)
				{
#if PSS_gid
				case PSS_gid:
					x = pe->gid;
					break;
#endif
#if PSS_pgrp
				case PSS_pgrp:
					x = pe->pgrp;
					break;
#endif
#if PSS_sid
				case PSS_sid:
					x = pe->sid;
					break;
#else
#if PSS_tgrp
				case PSS_tgrp:
					x = pe->tgrp;
					break;
#endif
#endif
				case PSS_tty:
					x = pe->tty;
#ifdef PR_TTYMAP
					for (dp = mp->data; dp; dp = dp->next)
						dp->data = PR_TTYDEV(dp->data);
#endif
					break;
				case PSS_uid:
					x = pe->uid;
					break;
				default:
					if (pss->disc->errorf)
						(*pss->disc->errorf)(pss, pss->disc, 2, "%08lx selection not implemented", mp->field);
					return 0;
				}
				for (dp = mp->data; dp; dp = dp->next)
					if (dp->data == x)
						break;
			} while (!dp && (mp = mp->next));
			if (!mp)
				return 0;
		}
	}

	/*
	 * method translation
	 */

#if __CYGWIN__
	if (fields & PSS_args)
	{
		if (pe->state == PSS_ZOMBIE)
			s = "<defunct>";
		else
		{
			cygwin_conv_to_posix_path(pr->progname, pss->buf);
			s = pss->buf;
			if ((i = strlen(s)) > 4 && !strcasecmp(s + i - 4, ".exe"))
				*(s + i - 4) = 0;
		}
		pe->args = s;
	}
	if (fields & PSS_command)
	{
		if (pe->state == PSS_ZOMBIE)
			s = "<defunct>";
		else
		{
			cygwin_conv_to_posix_path(pr->progname, pss->buf);
			if (s = strrchr(pss->buf, '/'))
				s++;
			else
				s = pss->buf;
			if ((i = strlen(s)) > 4 && !strcasecmp(s + i - 4, ".exe"))
				*(s + i - 4) = 0;
		}
		pe->command = s;
	}
	pe->flags = pr->process_state;
	pe->pid = pr->pid;
	pe->npid = pr->dwProcessId;
	pe->ppid = pr->ppid;
	pe->rss = pr->rusage_self.ru_maxrss;
	pe->size = pr->rusage_self.ru_idrss;
	pe->start = PR_START(pr);
	pe->time = PR_TIME(pr);
#else
#if _lib_getprocs
	if (fields & PSS_args)
	{
		if (pe->state == PSS_ZOMBIE)
			s = "<defunct>";
		else
		{
			s = pr->pi_comm;
			if (s[0] == '(' && s[i = strlen(s) - 1] == ')')
			{
				s[i] = 0;
				s++;
			}
		}
		pe->args = s;
	}
	if (fields & PSS_command)
	{
		if (pe->state == PSS_ZOMBIE)
			s = "<defunct>";
		else
		{
			s = pr->pi_comm;
			if (s[0] == '(' && s[i = strlen(s) - 1] == ')')
			{
				s[i] = 0;
				s++;
			}
		}
		pe->command = s;
	}
	pe->addr = (void*)pr->pi_adspace;
	pe->flags = pr->pi_flags;
	pe->nice = pr->pi_nice;
	pe->ppid = pr->pi_ppid;
#if PSS_pri
	pe->pri = pr->pi_pri;
#endif
	pe->refcount = pr->pi_thcount;
	pe->rss = pr->pi_drss + pr->pi_trss;
	pe->size = pr->pi_size;
	pe->start = pr->pi_start;
	pe->time = pr->pi_ru.ru_utime.tv_sec + pr->pi_ru.ru_stime.tv_sec;
#else
	if (fields & PSS_args)
	{
		if (pe->state == PSS_ZOMBIE)
			s = "<defunct>";
		else
		{
#if _mem_pr_psargs_prpsinfo
			s = pr->pr_psargs;
#else
#ifdef _PS_args
			s = "<unknown>";
			sfsprintf(pss->buf, sizeof(pss->buf), _PS_path_num, pe->pid, _PS_args);
			if ((i = open(pss->buf, O_RDONLY|O_BINARY)) >= 0)
			{
				n = read(i, pss->buf, sizeof(pss->buf) - 1);
				close(i);
				if (n > 0)
				{
					s = pss->buf;
					for (i = 0; i < n; i++)
						if (!s[i])
							s[i] = ' ';
					s[i] = 0;
				}
			}
#else
			s = pr->pr_fname;
			if (s[0] == '(' && s[i = strlen(s) - 1] == ')')
			{
				s[i] = 0;
				s++;
			}
#endif
#endif
		}
		pe->args = s;
	}
	if (fields & PSS_command)
	{
		if (pe->state == PSS_ZOMBIE)
			s = "<defunct>";
		else
		{
			s = pr->pr_fname;
			if (s[0] == '(' && s[i = strlen(s) - 1] == ')')
			{
				s[i] = 0;
				s++;
			}
		}
		pe->command = s;
	}
	pe->addr = (void*)pr->pr_addr;
#if _mem_pr_clname_prpsinfo
	pe->sched = pr->pr_clname;
#endif
	pe->cpu = pr->pr_cpu;
	pe->flags = pr->pr_flag;
	pe->nice = pr->pr_nice;
#if _mem_pr_ntpid_prpsinfo
	pe->npid = pr->pr_ntpid;
#endif
	pe->ppid = pr->pr_ppid;
	pe->pri = pr->pr_pri;
#if _mem_pr_sonproc_prpsinfo
	pe->proc = pr->pr_sonproc;
#endif
#if _mem_pr_refcount_prpsinfo
	pe->refcount = pr->pr_refcount;
#endif
#if _mem_pr_rssize_prpsinfo
	pe->rss = pr->pr_rssize;
#endif
	pe->size = pr->pr_size;
	pe->start = PR_START(pr);
	pe->time = PR_TIME(pr);
	pe->wchan = (void*)pr->pr_wchan;
#endif
#endif

	/*
	 * generic return
	 */

	return pe;
}

/*
 * save entry data
 */

Pssent_t*
psssave(register Pss_t* pss, register Pssent_t* pe)
{
	register unsigned long		fields = pss->disc->fields & PSS_all;

	if (fields & PSS_args)
		pe->args = vmstrdup(pss->vm, pe->args);
	if (fields & PSS_command)
		pe->command = vmstrdup(pss->vm, pe->command);
	if (fields & PSS_sched)
		pe->sched = vmstrdup(pss->vm, pe->sched);
	pss->ent = 0;
	return pe;
}

/*
 * open a pss stream
 */

Pss_t*
pssopen(Pssdisc_t* disc)
{
	register Pss_t*	pss;
	Vmalloc_t*	vm;

	if (!disc || !(vm = vmopen(Vmdcheap, Vmbest, 0)))
		return 0;
	if (!(pss = vmnewof(vm, 0, Pss_t, 1, 0)))
	{
		vmclose(vm);
		return 0;
	}
	pss->id = lib;
	pss->fields = PSS_all;
	pss->hz = PR_HZ;
	pss->disc = disc;
	pss->vm = vm;
#if __CYGWIN__
	cygwin_internal(CW_LOCK_PINFO, 1000);
#else
#if _lib_getprocs
	if (!(pss->data = (void*)vmnewof(vm, 0, Getprocs_t, 1, 0)))
	{
		if (pss->disc->errorf)
			(*pss->disc->errorf)(pss, pss->disc, ERROR_SYSTEM|2, "out of space [Pscan_t]");
		vmclose(vm);
		return 0;
	}
#endif
#endif
	return pss;
}

/*
 * close a pss stream
 */

int
pssclose(Pss_t* pss)
{
	if (!pss || !pss->vm)
		return -1;
	vmclose(pss->vm);
#if __CYGWIN__
	cygwin_internal(CW_UNLOCK_PINFO);
#endif
	return 0;
}

/*
 * return a pss entry
 */

Pssent_t*
pssread(register Pss_t* pss, pid_t pid)
{
	Pssent_t*	pe;
	int		force;

	force = pid != PSS_SCAN;
	for (;;)
	{
#if __CYGWIN__
		if (!force)
		{
			if (!pss->data)
				pid = 0;
			else if (!(pid = ((struct external_pinfo*)pss->data)->pid))
			{
				pss->data = 0;
				return 0;
			}
			pid |= CW_NEXTPID;
		}
#else
#if _lib_getprocs
		register Getprocs_t*	gp;

		gp = (Getprocs_t*)pss->data;
		if (force)
		{
			gp->pid = pid;
			gp->index = 0;
			gp->count = getprocs(gp->entry, sizeof(gp->entry[0]), NiL, 0, &gp->pid, 1);
			gp->last = 0;
			gp->pid = 0;
		}
#else
		if (!force)
		{
			struct dirent*	ent;
			char*		e;

			if (!pss->dir && !(pss->dir = opendir(_PS_dir)))
				return 0;
			do
			{
				if (!(ent = readdir(pss->dir)))
					return 0;
				pid = (pid_t)strtol(ent->d_name, &e, 10);
			} while (*e);
		}
#endif
#endif
		if ((pe = ps(pss, pid, force)) || force)
			break;
#if _lib_getprocs
		if (gp->last)
			return 0;
#endif
	}
	return pe;
}

#if !__CYGWIN__

/*
 * initialize the tty dev<=>name hash
 */

static int
ttyinit(register Pss_t* pss)
{
	register DIR*		dir;
	register struct dirent*	ent;
	register Tty_t*		tp;
	DIR*			sub = 0;
	Dtdisc_t*		dp;
	char*			base;
	char*			name;
	Vmalloc_t*		vo;
	struct stat		st;
	char			path[PATH_MAX];

	if (!(dp = vmnewof(pss->vm, 0, Dtdisc_t, 1, 0)))
	{
		if (pss->disc->errorf)
			(*pss->disc->errorf)(pss, pss->disc, ERROR_SYSTEM|2, "out of space [tty-disc]");
		return -1;
	}
	dp->key = offsetof(Tty_t, dev);
	dp->size = sizeof(dev_t);
	dp->link = offsetof(Tty_t, link);
	vo = Vmregion;
	pss->ttys = dtopen(dp, Dthash);
	Vmregion = vo;
	if (!pss->ttys)
	{
		if (pss->disc->errorf)
			(*pss->disc->errorf)(pss, pss->disc, ERROR_SYSTEM|2, "out of space [tty-dict]");
		return -1;
	}
	strcpy(path, "/dev");
	if (!(dir = opendir(path)))
	{
		if (pss->disc->errorf)
			(*pss->disc->errorf)(pss, pss->disc, ERROR_SYSTEM|2, "%s: cannot read", path);
		return -1;
	}
	path[4] = '/';
	name = base = path + 5;
	for (;;)
	{
		while (ent = readdir(dir))
		{
			if (D_NAMLEN(ent) + (base - path) + 1 > sizeof(path))
				continue;
			if (!sub && (ent->d_name[0] != 'c' && ent->d_name[0] != 't' && ent->d_name[0] != 'p' && ent->d_name[0] != 'v' || ent->d_name[1] != 'o' && ent->d_name[1] != 't'))
				continue;
			strcpy(base, ent->d_name);
			if (stat(path, &st))
				continue;
			if (!S_ISCHR(st.st_mode))
			{
				if (sub || !S_ISDIR(st.st_mode))
					continue;
				sub = dir;
				if (dir = opendir(path))
				{
					base = path + strlen(path);
					*base++ = '/';
				}
				else
				{
					dir = sub;
					sub = 0;
				}
				continue;
			}
			if (!(tp = vmnewof(pss->vm, 0, Tty_t, 1, strlen(name))))
			{
				if (pss->disc->errorf)
					(*pss->disc->errorf)(pss, pss->disc, ERROR_SYSTEM|2, "out of space [tty-entry]");
				return -1;
			}
			strcpy(tp->name, name);
			tp->dev = st.st_rdev;
			dtinsert(pss->ttys, tp);
		}
		if (!sub)
			break;
		closedir(dir);
		dir = sub;
		sub = 0;
		base = name;
	}
	closedir(dir);
	return 0;
}

#endif

/*
 * return dev given tty base name
 */

dev_t
pssttydev(register Pss_t* pss, const char* name)
{
	register const char*	s;
	register Tty_t*		tp;
	struct stat		st;

	s = name;
	if (stat(s, &st))
	{
		sfsprintf(pss->buf, sizeof(pss->buf), "/dev/%s", name);
		s = (const char*)pss->buf;
		if (stat(s, &st))
		{
			sfsprintf(pss->buf, sizeof(pss->buf), "/dev/tty%s", name);
			if (stat(s, &st))
			{
				if (pss->disc->errorf)
					(*pss->disc->errorf)(pss, pss->disc, ERROR_SYSTEM|2, "%s: unknown tty", name);
				return PSS_NODEV;
			}
		}
	}
#if !__CYGWIN__
	if (!pss->ttys && ttyinit(pss))
		return PSS_NODEV;
	if (!(tp = (Tty_t*)dtmatch(pss->ttys, &st.st_rdev)))
	{
		if (!(tp = vmnewof(pss->vm, 0, Tty_t, 1, strlen(s))))
		{
			if (pss->disc->errorf)
				(*pss->disc->errorf)(pss, pss->disc, ERROR_SYSTEM|2, "out of space [tty]");
			return PSS_NODEV;
		}
		strcpy(tp->name, s);
		tp->dev = st.st_rdev;
		dtinsert(pss->ttys, tp);
	}
#endif
	return st.st_rdev;
}

/*
 * return tty base name given tty dev
 */

char*
pssttyname(register Pss_t* pss, dev_t dev)
{
#if __CYGWIN__
	if (dev < 0)
		return "?";
	else if (dev == (dev_t)TTY_CONSOLE)
		return "con";
	sfsprintf(pss->buf, sizeof(pss->buf), "%d", dev);
#else
	register Tty_t*	tp;

	if (dev == PSS_NODEV || !pss->ttys && ttyinit(pss))
		return "?";
	if (tp = (Tty_t*)dtmatch(pss->ttys, &dev))
		return tp->name;
	sfsprintf(pss->buf, sizeof(pss->buf), "%03d,%03d", major(dev), minor(dev));
#endif
	return pss->buf;
}

#else

/*
 * not under bozo's bigtop
 */

Pss_t*
pssopen(Pssdisc_t* disc)
{
	return 0;
}

int
pssclose(Pss_t* pss)
{
	return -1;
}

Pssent_t*
pssread(register Pss_t* pss, pid_t pid)
{
	return 0;
}

Pssent_t*
psssave(register Pss_t* pss, register Pssent_t* pe)
{
	return 0;
}

dev_t
pssttydev(register Pss_t* pss, const char* name)
{
	return PSS_NODEV;
}

char*
pssttyname(register Pss_t* pss, dev_t dev)
{
	return 0;
}

#endif
