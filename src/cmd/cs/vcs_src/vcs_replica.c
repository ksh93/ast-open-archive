/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1990-2002 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*******************************************************************/
#include "vcs_rscs.h"
#include "vcs_replica.h"
#include <dirent.h>
#include <tm.h>

char* getrdir(rp, buf)
	register char*	rp;
	char*		buf;
{
	register char*	s;

	strcpy(buf, rp);
	if (!(s = pathcanon(buf, PATH_PHYSICAL|PATH_EXISTS)))
		return 0;
	*s++ = '/';
	strcpy(s, REPL_DIR);
	return buf;
}

int replica_creat(path, sf)
	char*		path;
	Sfio_t*		sf;
{
	register char*		rf;
	register char*		rd;
	char			dirbuf[1024];
	register DIR*		dir;
	register struct dirent*	rca;
	register char*		s;
	Sfio_t*			fd;
	int			cnt = 0;
	struct stat		st;

	if (((rd = getrdir(path, dirbuf)) == NULL) || (dir = opendir(rd)) == NULL)
		return (-1);

	for(s = rd; *s; s++);

	if ((rf = strrchr(path, '/')))
		rf++;
	else
		rf = path;
	
	
	while((rca = readdir(dir)))
	{
		/*
		 * skip all entries staring with ``.''
		 */

		if (rca->d_name[0] == '.')
			continue;

		/*
		 * skip regular file 
		 */
		sprintf(s, "/%s", rca->d_name);
		if (stat(dirbuf, &st) || !S_ISDIR(st.st_mode))
			continue;

		sprintf(s, "/%s/%s", rca->d_name, rf);
		if ((fd = sfopen(NULL, dirbuf, "w")))
		{
			sfseek(sf, 0L, 0);
			sfmove(sf, fd, -1, -1);
			sfclose(fd);
			cnt++;
		}
	}
	if (cnt)
	{
		/* send message to the server */
		

	}

	return (cnt);

}

int replica(path, df, tp)
	char*		path;
	Sfio_t*		df;
	tag_t*		tp;
{
	register char*		rf;
	register char*		rd;
	char			dirbuf[1024];
	register DIR*		dir;
	register struct dirent*	rca;
	register char*		s;
	Sfio_t*			fd;
	int			cnt = 0;
	struct stat		st;


	if (((rd = getrdir(path, dirbuf)) == NULL) || (dir = opendir(rd)) == NULL)
		return (-1);

	for(s = rd; *s; s++);

	if ((rf = strrchr(path, '/')))
		rf++;
	else
		rf = path;
	
	
	while((rca = readdir(dir)))
	{
		/*
		 * skip all entries staring with ``.''
		 */

		if (rca->d_name[0] == '.')
			continue;

		/*
		 * skip regular file 
		 */
		sprintf(s, "/%s", rca->d_name);
		if (stat(dirbuf, &st) || !S_ISDIR(st.st_mode))
			continue;
		sprintf(s, "/%s/%s.%d", rca->d_name, rf, cs.time);
		if ((fd = sfopen(NULL, dirbuf, "a")))
		{
			sfwrite(fd,(char *)tp,tp->length); 
			if (df)
			{
				sfseek(df, 0L, 0); 
				sfmove(df,fd,-1,-1); 
			}
			sfclose(fd);
			cnt++;
		}
	}
	if (cnt)
	{
		/* send message to the server */
		

	}

	return (cnt);

}
