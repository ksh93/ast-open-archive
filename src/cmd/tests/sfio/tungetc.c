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
	Sfio_t	*f;
	char	*str, *alpha, *s;
	char	buf[128];
	int	n;

	str = "0123456789";
	alpha = "abcdefghijklmnop";

	if(!(f = sfopen((Sfio_t*)0,alpha,"s")))
		terror("Opening stream\n");

	for(n = 9; n >= 0; --n)
		if(sfungetc(f,n+'0') != n+'0')
			terror("Ungetc\n");

	if(!(s = sfreserve(f,-1,0)) || sfvalue(f) != 10)
		terror("Peek stream1\n");
	if(strncmp(s,str,10) != 0)
		terror("Bad data1\n");

	if(!(s = sfreserve(f,-1,0)) || sfvalue(f) != (ssize_t)strlen(alpha))
		terror("Peek stream2\n");
	if(strncmp(s,alpha,strlen(alpha)) != 0)
		terror("Bad data2\n");

	sfseek(f,(Sfoff_t)0,0);
	for(n = 9; n >= 0; --n)
		if(sfungetc(f,n+'0') != n+'0')
			terror("Ungetc2\n");
	if(sfgetc(f) != '0')
		terror("Sfgetc\n");
	sfseek(f,(Sfoff_t)0,0);
	if(!(s = sfreserve(f,-1,0)) || sfvalue(f) != (ssize_t)strlen(alpha))
		terror("Peek stream3\n");
	if(strncmp(s,alpha,strlen(alpha)) != 0)
		terror("Bad data2\n");

	sfseek(f,(Sfoff_t)0,0);
	if(sfungetc(f,'0') != '0')
		terror("Ungetc3\n");

	strcpy(buf,"0123456789\n");
	if(!(f = sfopen(f,buf,"s+")) )
		terror("Reopening  string\n");
	if(sfungetc(f,'\n') != '\n')
		terror("Can't unget new-line2\n");
	if(sfungetc(f,'d') != 'd')
		terror("Can't unget d\n");
	if(sfungetc(f,'c') != 'c')
		terror("Can't unget c\n");
	if(sfungetc(f,'\n') != '\n')
		terror("Can't unget new-line\n");
	if(sfungetc(f,'b') != 'b')
		terror("Can't unget b\n");
	if(sfungetc(f,'a') != 'a')
		terror("Can't unget a\n");

	if(!(s = sfgetr(f,'\n',1)) || strcmp(s,"ab") != 0)
		terror("Did not get ab\n");
	if(!(s = sfgetr(f,'\n',1)) || strcmp(s,"cd") != 0)
		terror("Did not get cd\n");
	if(!(s = sfgetr(f,'\n',1)) || strcmp(s,"0123456789") != 0)
		terror("Did not get 0123456789\n");

	TSTEXIT(0);
}
