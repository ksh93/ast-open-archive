/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1999-2004 AT&T Corp.                  *
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
#include	"sftest.h"

MAIN()
{
	char	buf[100];
	Sfio_t	*fp;
	int	i;
	char	*s;

	if(!(fp = sftmp(8)))
		terror("Can't open temp file\n");

	sfset(fp,SF_LINE,1);
	for(i = 0; i < 1000; ++i)
	{	sfsprintf(buf,sizeof(buf),"Number: %d",i);
		if(sfputr(fp,buf,'\n') <= 0)
			terror("Writing %s\n",buf);
	}

	sfseek(fp,(Sfoff_t)0,0);

	for(i = 0; i < 1000; ++i)
	{	sfsprintf(buf,sizeof(buf),"Number: %d",i);
		if(!(s = sfgetr(fp,'\n',1)))
			terror("Reading %s\n",buf);
		if(strcmp(s,buf) != 0)
			terror("Input=%s, Expect=%s\n",s,buf);
	}

	sfseek(fp,(Sfoff_t)0,0);
	s = sfgetr(fp,'\0',1);
	if(s)
		terror("Expecting a null string\n");
	s = sfgetr(fp,'\0',-1);
	if(!s)
		terror("Expecting a non-null string\n");
	if(sfvalue(fp) != sfsize(fp))
		terror("Wrong size\n");

	TSTEXIT(0);
}
