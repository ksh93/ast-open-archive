/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1989-2004 AT&T Corp.                *
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

#include "3d.h"

int
access3d(const char* path, int mode)
{
	register char*	sp;
	mode_t		test;
	struct stat 	st;

#if FS
	if (!fscall(NiL, MSG_stat, 0, path, &st))
	{
		if (state.ret) return(-1);
		sp = 0;
	}
	else
#endif
	if (!(sp = pathreal(path, 0, &st)))
		return(-1);

	/*
	 * handle some frequent cases separately
	 */

	switch (mode)
	{
	case F_OK:
		return(0);
	case R_OK:
		if ((st.st_mode&(S_IRUSR|S_IRGRP|S_IROTH)) == (S_IRUSR|S_IRGRP|S_IROTH))
			return(0);
		break;
	case W_OK:
		if (state.path.level && (st.st_mode&(S_IWUSR|S_IWGRP|S_IWOTH)) && !ACCESS(sp, R_OK))
			return(0);
		break;
	case X_OK:
		if ((st.st_mode&(S_IXUSR|S_IXGRP|S_IXOTH)) == (S_IXUSR|S_IXGRP|S_IXOTH))
			return(0);
		break;
	}
#if FS
	if (sp)
#endif
	return(ACCESS(sp, mode));
#if FS

	/*
	 * simulate access()
	 */

	if (mode & (R_OK|W_OK|X_OK))
	{
		test = 0;
		if (st.st_uid == state.uid)
		{
			if (mode & R_OK) test |= S_IRUSR;
			if (mode & W_OK) test |= S_IWUSR;
			if (mode & X_OK) test |= S_IXUSR;
		}
		else if (st.st_gid == state.gid)
		{
			if (mode & R_OK) test |= S_IRGRP;
			if (mode & W_OK) test |= S_IWGRP;
			if (mode & X_OK) test |= S_IXGRP;
		}
		else 
		{
			if (mode & R_OK) test |= S_IROTH;
			if (mode & W_OK) test |= S_IWOTH;
			if (mode & X_OK) test |= S_IXOTH;
		}
		if ((st.st_mode & test) != test)
		{
			errno = EACCES;
			return(-1);
		}
	}
	return(0);
#endif
}
