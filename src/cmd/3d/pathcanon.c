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
 * pathcanon - generate canonical pathname from given pathname
 *
 * this routine works with both relative and absolute paths
 * relative paths can contain any number of leading ../
 * a pointer to the trailing '\0' is returned, 0 on error
 * the operator ... is also applied by this routine
 * phys is for libast compatibility and is currently ignored
 */

#include "3d.h"

char*
pathcanon(char* path, int phys)
{
	register char*	sp;
	register char*	dp = path;
	register int	c = '/';
	register char*	begin = dp;
	int		add_dot = 0;
	long		visits = 0;

	message((-6, "patcanon: ++ %s%s", path, phys ? " [PHYSICAL]" : ""));
#if PRESERVE_LEADING_SLASH_SLASH
	if (*dp == '/' && *(dp + 1) == '/')
		path = ++dp;
#endif
	if (*dp != '/') dp--;
	sp = dp;
	for (;;)
	{
		sp++;
		if (c == '/')
		{
			/*
			 * eliminate redundant /
			 */

			if (*sp == '/')
				continue;
			if (*sp == '.')
			{
				if ((c = *++sp) == '/')
				{
					/*
					 * ./
					 */

					add_dot++;
					continue;
				}
				if (!c)
				{
					/*
					 * .
					 */

					add_dot++;
					break;
				}
				if (c == '.')
				{
					if (!(c = *++sp) || c == '/')
					{
						/*
						 * ..
						 */

						if (dp < begin)
							begin += 3;
						else
						{
							if (dp > begin)
								while (*--dp != '/')
									if (dp < begin)
										break;
							if (!c)
								break;
							continue;
						}
					}
					else if (c == '.' && !state.in_2d)
					{
						if (!(c = *++sp) || c == '/')
						{
							/*
							 * ...
							 */

							*dp = 0;
							phys = state.path.level;
							if (!(dp = pathnext(path, sp, &visits)))
							{
								message((-6, "patcanon: -- %s", NiL));
								return(0);
							}
							if (!state.real)
								state.path.level = phys;
							if (!(*(sp = dp) = c))
							{
								dp--;
								break;
							}
							continue;
						}
						*++dp = '.';
					}
					*++dp = '.';
				}
				*++dp = '.';
			}
		}
		add_dot = 0;
		if (!(c = *sp))
			break;
		*++dp = c;
	}

	/*
	 * preserve trailing /.
	 */

	if (add_dot)
	{
		if (*dp != '/')
			*++dp = '/';
		*++dp = '.';
	}
	*++dp = 0;
	message((-6, "patcanon: -- %s", path));
	return(dp);
}
