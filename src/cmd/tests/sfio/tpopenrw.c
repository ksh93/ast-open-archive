/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1999-2000 AT&T Corp.              *
*      and it may only be used by you under license from       *
*                     AT&T Corp. ("AT&T")                      *
*       A copy of the Source Code Agreement is available       *
*              at the AT&T Internet web site URL               *
*                                                              *
*     http://www.research.att.com/sw/license/ast-open.html     *
*                                                              *
*     If you received this software without first entering     *
*       into a license with AT&T, you have an infringing       *
*           copy and cannot use it without violating           *
*             AT&T's intellectual property rights.             *
*                                                              *
*               This software was created by the               *
*               Network Services Research Center               *
*                      AT&T Labs Research                      *
*                       Florham Park NJ                        *
*                                                              *
*             Glenn Fowler <gsf@research.att.com>              *
*                                                              *
***************************************************************/
#include	"sftest.h"

#if __STD_C
main(int argc, char** argv)
#else
main(argc, argv)
int	argc;
char	**argv;
#endif
{
	Sfio_t	*f;
	char	buf[1024], *s;
	int	n;

	if(argc > 1)
	{	/* coprocess only */
		while(s = sfreserve(sfstdin,-1,0))
			sfwrite(sfstdout,s,sfvalue(sfstdin));
		return 0;
	}

	/* make coprocess */
	if(!(f = sfpopen(NIL(Sfio_t*), sfprints("%s -p",argv[0]), "r+")))
		terror("Opening for read/write\n");
	for(n = 0; n < 10; ++n)
	{	sfsprintf(buf,sizeof(buf),"Line %d",n);
		sfputr(f,buf,'\n');
		if(!(s = sfgetr(f,'\n',1)))
			terror("Did not read back line\n");
		if(strcmp(s,buf) != 0)
			terror("Input=%s, Expect=%s\n",s,buf);
	}

	sfputr(f,"123456789",'\n');
	sfsync(f);
	sleep(1);
	if(!(s = sfreserve(f,-1,1)) || sfvalue(f) != 10)
		terror("Did not get data back\n");
	if(strncmp(s,"123456789\n",10) != 0)
		terror("Wrong data\n");
	s[0] = s[1] = s[2] = '0';
	if(sfwrite(f,s,3) != 3 || sfputc(f,'\n') != '\n')
		terror("Fail on write\n");
	if(!(s = sfgetr(f,'\n',1)) )
		terror("Lost data\n"); 
	if(strcmp(s,"456789") != 0)
		terror("Wrong data2\n");
	if(!(s = sfgetr(f,'\n',1)) )
		terror("Lost data2\n"); 
	if(strcmp(s,"000") != 0)
		terror("Wrong data3\n");
	
	return 0;
}
