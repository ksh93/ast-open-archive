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
#if _SFIO_H_ONLY
#	if _PACKAGE_ast
#		include	<ast.h>
#	else
#		include	<sfio.h>
		_BEGIN_EXTERNS_
		extern int	unlink _ARG_((const char*));
		extern ssize_t	write _ARG_((int, const void*, size_t));
		extern int	exit _ARG_((int));
		extern size_t	strlen _ARG_((const char*));
		extern Void_t*	malloc _ARG_((size_t));
		_END_EXTERNS_
#	endif
#else
#	include	"sfhdr.h"
#endif

#ifndef NIL
#define NIL(t)	((t)0)
#endif

#if !_PACKAGE_ast
	_BEGIN_EXTERNS_
	extern int	strncmp _ARG_((const char*, const char*, size_t));
	extern int	strcmp _ARG_((const char*, const char*));

	extern int	system _ARG_((const char*));
	extern int	alarm _ARG_((int));
	_END_EXTERNS_
#endif

#ifdef __LINE__
static int	Sftline;
#define terror	(Sftline=__LINE__),t_error
#else
#define terror	t_error
#endif

/* temp files that may be used */
static char	Sft[4][256];

static void sftcleanup()
{	int	i;

	for(i = 0; i < sizeof(Sft)/sizeof(Sft[0]); ++i)
		if(Sft[i][0])
			unlink(Sft[i]);
}

#if __STD_C
void t_error(char* form, ...)
#else
void t_error(va_alist)
va_dcl
#endif
{
	Sfio_t*	f;
	char*	s;
	int	n;
	va_list	args;
#if __STD_C
	va_start(args,form);
#else
	char*	form;
	va_start(args);
	form = va_arg(args,char*);
#endif

	f = sfopen(NIL(Sfio_t*),NIL(char*),"sw");
	n = 0;
#ifdef __LINE__
	n += sfprintf(f,"Line=%d: ", Sftline);
#endif
	n += sfvprintf(f,form,args);
	sfseek(f,(Sfoff_t)0,0);
	s = sfreserve(f,SF_UNBOUND,1);

	va_end(args);

	if(n>0)
	{
#if _PACKAGE_ast
		if(s[n-1] == '\n')
			s[n-1] = ' ';
#else
		if(s[n-1] != '\n')
			s[n++] = '\n';
#endif
		write(2,s,n);
	}

	sftcleanup();
	exit(1);
}


#if __STD_C
static char* sftfile(int n)
#else
static char* sftfile(n)
int	n;
#endif
{
	if(n >= sizeof(Sft)/sizeof(Sft[0]))
		terror("Bad temporary file request:%d\n", n);

	if(!Sft[n][0])
	{
#ifdef DEBUG
		sfsprintf(Sft[n], sizeof(Sft[n]), "Sftfile.%c%c%c", '0'+n, '0'+n, '0'+n);
#else
		static int	pid;
		static char*	tmp;
		if (!tmp)
		{	if (!(tmp = (char*)getenv("TMPDIR")) || access(tmp, 0) != 0)
				tmp = "/tmp";
			pid = (int)getpid() % 10000;
                }
                sfsprintf(Sft[n], sizeof(Sft[n]), "%s/sft.%c.%d", tmp, '0'+n, pid);
#endif
	}

	return Sft[n];
}
