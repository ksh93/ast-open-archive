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

			if (*sp == '/') continue;
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

						if (dp < begin) begin += 3;
						else
						{
							if (dp > begin)
								while (*--dp != '/')
									if (dp < begin)
										break;
							if (!c) break;
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
		if (!(c = *sp)) break;
		*++dp = c;
	}

	/*
	 * preserve trailing /.
	 */

	if (add_dot)
	{
		if (*dp != '/') *++dp = '/';
		*++dp = '.';
	}
	*++dp = 0;
	message((-6, "patcanon: -- %s", path));
	return(dp);
}
