/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1989-2004 AT&T Corp.                  *
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
*                  David Korn <dgk@research.att.com>                   *
*                   Eduardo Krell <ekrell@adexus.cl>                   *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * 3D interface to version control system
 *
 * Herman Rao
 * AT&T Research
 */

#ifndef _VCS3D_H
#define _VCS3D_H

#define vcs_checkout	_3d_vcs_checkout
#define vcs_real	_3d_vcs_real
#define vcs_set		_3d_vcs_set

#define VCS_STATE				\
	struct					\
	{					\
	dev_t		dev;			\
	ino_t		ino;			\
	Fs_t*		fs;			\
	short		fd;			\
	time_t		now;			\
	}		vcs

#define VCS_PATH_STATE				\
	struct					\
	{					\
	char 		rfile[PATH_MAX]; 	\
	char		version[PATH_MAX];	\
	}		vcs

#define VCS_FS							\
	FSINIT("vcs", 0, vcs_set, FS_CLOSE, HASHKEY3('v','c','s'))

#define VCS_OPEN(p,f,m,s) 					\
	do 							\
	{ 							\
		if (!(f & O_CREAT) && (f & O_ACCMODE != O_WRONLY) && state.vcs.fd)\
		{						\
			if (vcs_checkout(p, s) < 0) 		\
			{ 					\
				errno = ENOENT; 		\
				return(-1); 			\
			}					\
			p = state.path.name;			\
		} 						\
	} while (0)

#define VCS_REAL(p,s) 						\
	(fson(state.vcs.fs) ? vcs_real(p,s) : 0)

extern int	vcs_checkout(const char*, struct stat*);
extern int	vcs_real(const char*, struct stat*);
extern int	vcs_set(Fs_t*, const char*, int, const char*, int);

#endif
