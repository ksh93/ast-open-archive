/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1999-2001 AT&T Corp.                *
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
*                 This software was created by the                 *
*                 Network Services Research Center                 *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*******************************************************************/
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

	TSTRETURN(0);
}
