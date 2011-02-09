/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1999-2011 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#include	"sftest.h"

static char	*Mystr = "abc";
#if __STD_C
int myprint(Sfio_t* f, Void_t* v, Sffmt_t* fe)
#else
int myprint(f, v, fe)
Sfio_t*		f;
Void_t*		v;
Sffmt_t*	fe;
#endif
{
	switch(fe->fmt)
	{
	case 's' :
		*((char**)v) = Mystr;
		fe->flags |= SFFMT_VALUE;
		return 0;
	}

	return 0;
}

MAIN()
{
	char	buf1[1024], buf2[1024];
	Sffmt_t	fe;

	memset(&fe, 0, sizeof(Sffmt_t));
	fe.version = SFIO_VERSION;
	fe.form = "%1$s";
	fe.extf = myprint;

	sfsprintf(buf1,sizeof(buf1),"%s",Mystr);
	sfsprintf(buf2,sizeof(buf2),"%!", &fe);
	if(strcmp(buf1,buf2) != 0)
		terror("Failed testing $position\n");

	return 0;
}
