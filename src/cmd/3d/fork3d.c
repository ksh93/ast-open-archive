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
*              David Korn <dgk@research.att.com>               *
*               Eduardo Krell <ekrell@adexus.cl>               *
*                                                              *
***************************************************************/
#pragma prototyped

#include "3d.h"

#if FS

#include <cs.h>

/*
 * called in the child process by fork()
 */

static void
forked(pid_t pid)
{
	register int		fd;
	register Fs_t*		fs;
	register Mount_t*	mp;
	char*			type;
	long			addr;
	long			port;
	long			clone;
	int			nfd;
	pid_t			ppid;
	char*			b;

	reclaim();
	ppid = state.pid;
	state.pid = pid;
	state.channel.internal = 0;
	for (fs = state.fs; fs < state.fs + elementsof(state.fs); fs++)
	{
		if ((fs->flags & FS_FORK) && fs->set)
			(*fs->set)(fs, state.null, 0, "fork", 4);
		if ((fs->flags & (FS_ERROR|FS_ON|FS_OPEN|FS_UNIQUE)) == (FS_ON|FS_OPEN|FS_UNIQUE))
			fsdrop(fs, 0);
	}
	for (mp = state.mount; mp < state.mount + elementsof(state.mount); mp++)
		if (mp->fs)
			switch (mp->fs->flags & (FS_ACTIVE|FS_ERROR|FS_GLOBAL|FS_MONITOR|FS_NAME|FS_ON))
	{
	case FS_ACTIVE|FS_ON:
		if (!fscall(mp, MSG_INIT(MSG_fork, 01511, 0), ppid, state.path.name, sizeof(state.path.name)) && state.ret > 0)
		{
			type = b = state.path.name;
			while (*b && *b != ' ') b++;
			if (*b) *b++ = 0;
			if (streq(type, "fdp")) fd = csrecv(&cs, mp->fs->fd, NiL, &nfd, 1) == 1 ? nfd : -1;
			else
			{
				addr = strtol(b, &b, 0);
				port = strtol(b, &b, 0);
				clone = strtol(b, &b, 0);
				fd = csbind(&cs, type, addr, port, clone);
			}
			if (fd >= 0)
			{
				FCNTL(fd, F_DUPFD, mp->fs->fd);
				CLOSE(fd);
			}
		}
		break;
	case FS_ACTIVE|FS_GLOBAL|FS_MONITOR|FS_ON:
	case FS_GLOBAL|FS_MONITOR|FS_ON:
	case FS_ACTIVE|FS_MONITOR|FS_ON:
	case FS_MONITOR|FS_ON:
		if (fssys(mp, MSG_fork))
			fscall(mp, MSG_fork, ppid);
		break;
	}

	if (state.cache)
		for (fd = 0; fd <= state.cache; fd++)
			if ((mp = state.file[fd].mount) && !(mp->fs->flags & FS_ACTIVE) && fssys(mp, MSG_dup))
				fscall(mp, MSG_dup, fd, fd);
}

#endif

#ifdef	fork3d

pid_t
fork3d(void)
{
	pid_t			pid;

	initialize();
	if (!(pid = FORK()) || pid == state.pid || pid == getpid())
	{
		pid = 0;
#if FS
		forked(getpid());
#else
		state.pid = getpid();
#endif
	}
#if FS
	else
	{
		register Mount_t*	mp;

		for (mp = state.global; mp; mp = mp->global)
			if (fssys(mp, MSG_fork))
				fscall(mp, MSG_fork, pid);
	}
#endif
	return(pid);
}

#else

NoN(fork)

#endif
