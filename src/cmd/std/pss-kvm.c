/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1989-2006 AT&T Corp.                  *
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
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * process status stream PSS_METHOD_kvm implementation
 */

#include "psslib.h"

#if PSS_METHOD != PSS_METHOD_kvm

NoN(pss_kvm)

#else

#include <kvm.h>
#if _sys_time
#include <sys/time.h>
#endif
#if _sys_param
#include <sys/param.h>
#endif
#if _sys_proc
#include <sys/proc.h>
#endif
#include <sys/sysctl.h>
#include <sys/tty.h>

#if !_mem_p_pid_extern_proc
#define extern_proc		proc
#endif

typedef struct State_s
{
	kvm_t*			kd;
	struct kinfo_proc*	kp;
	struct kinfo_proc*	ke;
	struct extern_proc*	pr;
	struct eproc*		px;
} State_t;

static int
kvm_init(Pss_t* pss)
{
	register State_t*	state;

	if (!(state = vmnewof(pss->vm, 0, State_t, 1, 0)))
	{
		if (pss->disc->errorf)
			(*pss->disc->errorf)(pss, pss->disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	if (!(state->kd = kvm_open(NiL, NiL, 0, O_RDONLY, NiL)))
	{
		if (pss->disc->errorf)
			(*pss->disc->errorf)(pss, pss->disc, ERROR_SYSTEM|1, "kvm open error");
		return -1;
	}
	pss->data = state;
	error(1, "%s: pss method is currently incomplete", pss->meth->name);
	return 1;
}

static int
kvm_done(Pss_t* pss)
{
	register State_t*	state = (State_t*)pss->data;

	kvm_close(state->kd);
	return 1;
}

static int
kvm_readf(Pss_t* pss, Pss_id_t pid)
{
	register State_t*	state = (State_t*)pss->data;
	int			count;

	if (pid)
	{
		if (!(state->kp = kvm_getprocs(state->kd, KERN_PROC_PID, pid, &count)))
			return -1;
		state->ke = state->kp + count;
	}
	else if (!state->kp)
	{
		if (!(state->kp = kvm_getprocs(state->kd, KERN_PROC_ALL, 0, &count)))
			return -1;
		state->ke = state->kp + count;
	}
	if (state->kp >= state->ke)
		return 0;
	pss->pid = state->kp->kp_proc.p_pid;
	state->pr = &state->kp->kp_proc;
	state->px = &state->kp->kp_eproc;
	state->kp++;
	return 1;
}

static int
kvm_part(register Pss_t* pss, register Pssent_t* pe)
{
	register State_t*	state = (State_t*)pss->data;

	pe->pid = state->pr->p_pid;
	pe->pgrp = state->px->e_pgid;
	pe->tty = state->px->e_tdev;
	pe->uid = state->px->e_ucred.cr_uid;
#if 0
	pe->sid = state->px->e_sess->s_sid;
#endif
	switch (state->pr->p_stat)
	{
	case SIDL:	pe->state = 'I'; break;
	case SRUN:	pe->state = 'R'; break;
	case SSLEEP:	pe->state = 'S'; break;
	case SSTOP:	pe->state = 'T'; break;
	case SZOMB:	pe->state = 'Z'; break;
	default:	pe->state = 'O'; break;
	}
	return 1;
}

static int
kvm_full(register Pss_t* pss, register Pssent_t* pe)
{
	register State_t*		state = (State_t*)pss->data;
	unsigned long			fields = pss->disc->fields & pss->meth->fields;
	char*				s;
	int				i;

	if (pe->state != PSS_ZOMBIE)
	{
		if (fields & PSS_args)
		{
			s = state->pr->p_comm;
			if (s[0] == '(' && s[i = strlen(s) - 1] == ')')
			{
				s[i] = 0;
				s++;
			}
			pe->args = s;
		}
		if (fields & PSS_command)
		{
			s = state->pr->p_comm;
			if (s[0] == '(' && s[i = strlen(s) - 1] == ')')
			{
				s[i] = 0;
				s++;
			}
			pe->command = s;
		}
	}
#if 0
	pe->addr = state->pr->p_addr;
#endif
	pe->wchan = state->pr->p_wchan;
	pe->flags = state->pr->p_flag;
	pe->nice = state->pr->p_nice;
	pe->ppid = state->px->e_ppid;
#if PSS_pri
	pe->pri = state->pr->p_usrpri;
#endif
#ifdef FWIDTH
	pe->cpu = state->pr->p_pctcpu >> FWIDTH;
#endif
	pe->refcount = state->px->e_xccount;
	pe->rss = state->px->e_xrssize;
#if _mem_e_xsize_eproc
	pe->size = state->px->e_xsize;
#else
	pe->size = state->px->e_vm.vm_tsize + state->px->e_vm.vm_dsize + state->px->e_vm.vm_ssize;
#endif
#if _mem_p_starttime_extern_proc
	pe->start = state->pr->p_starttime.tv_sec;
#endif
	pe->time = state->pr->p_rtime.tv_sec;
	return 1;
}

static Pssmeth_t kvm_method =
{
	"kvm",
	"[-version?@(#)$Id: pss kvm (AT&T Labs Research) 2006-01-11 $\n]"
	"[-author?Glenn Fowler <gsf@research.att.com>]",
	PSS_all,
	kvm_init,
	kvm_readf,
	kvm_part,
	kvm_full,
	0,
	0,
	0,
	kvm_done
};

Pssmeth_t*	_pss_method = &kvm_method;

#endif
