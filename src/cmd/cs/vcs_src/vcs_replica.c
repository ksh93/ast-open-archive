/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1990-2004 AT&T Corp.                  *
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
*                                                                      *
***********************************************************************/
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
