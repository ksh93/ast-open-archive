/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1989-2003 AT&T Corp.                *
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
*                David Korn <dgk@research.att.com>                 *
*                 Eduardo Krell <ekrell@adexus.cl>                 *
*                                                                  *
*******************************************************************/
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
		if (!(f & (O_CREAT|O_WRONLY)) && state.vcs.fd)\
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
