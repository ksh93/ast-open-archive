/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1999-2000 AT&T Corp.                *
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
*                                                                  *
*******************************************************************/
#include	<ast_common.h>

#ifndef NIL
#define NIL(t)		((t)0)
#endif

#if __STD_C
#include	<stdarg.h>
#else
#include	<varargs.h>
#endif

_BEGIN_EXTERNS_

#if !_SFIO_H
extern int	sprintf _ARG_((char*, const char*, ...));
extern int	vsprintf _ARG_((char*, const char*, va_list));
#endif

extern int	atexit _ARG_((void (*)(void)));
extern void	exit _ARG_((int));
extern size_t	strlen _ARG_((const char*));
extern Void_t*	malloc _ARG_((size_t));
extern char*	getenv _ARG_((const char*));

extern int	strncmp _ARG_((const char*, const char*, size_t));
extern int	strcmp _ARG_((const char*, const char*));
extern int	system _ARG_((const char*));

#if !_hdr_unistd
extern int	alarm _ARG_((int));
extern int	sleep _ARG_((int));
extern int	fork();
extern int	wait _ARG_((int*));
extern int	access _ARG_((const char*, int));
extern int	write _ARG_((int, const void*, int));
extern int	unlink _ARG_((const char*));
extern Void_t*	sbrk _ARG_((int));
extern int	getpid();
#endif

_END_EXTERNS_

static int		Tstline;
static char		Tstfile[16][256];

#ifdef __LINE__
#define terror		(Tstline=__LINE__),tsterror
#else
#define terror		(Tstline=-1),tsterror
#endif

#ifdef __LINE__
#define twarn		(Tstline=__LINE__),tstwarn
#else
#define twarn		(Tstline=-1),tstwarn
#endif

#ifdef __LINE__
#define tsuccess	(Tstline=__LINE__),tstsuccess
#else
#define tsuccess	(Tstline=-1),tstsuccess
#endif

#define tmesg		(Tstline=-1),tstwarn

#ifndef MAIN
#if __STD_C
#define MAIN()		main(int argc, char** argv)
#else
#define MAIN()		main(argc, argv) int argc; char** argv;
#endif
#endif /*MAIN*/

#ifndef TSTRETURN
#define TSTRETURN(v)	{ tstcleanup(); return(v); }
#endif

static void tstcleanup()
{	int	i;

	for(i = 0; i < sizeof(Tstfile)/sizeof(Tstfile[0]); ++i)
		if(Tstfile[i][0])
			unlink(Tstfile[i]);
}

#if __STD_C
static void tstputmesg(int line, char* form, va_list args)
#else
static void tstputmesg(line, form, args)
int	line;
char*	form;
va_list	args;
#endif
{
	char	*s, buf[1024];
	int	n;

	for(n = 0; n < sizeof(buf); ++n)
		buf[n] = 0;

	s = buf;
	if(line >= 0)
	{	sprintf(s,"\tLine=%d: ", line);
		s += strlen(s);
	}
	vsprintf(s,form,args);

	if((n = strlen(buf)) > 0)
	{	if(buf[n-1] != '\n')
		{	buf[n] = '\n';
			n += 1;
		}
		write(2,buf,n);
	}
}


#if __STD_C
void tsterror(char* form, ...)
#else
void tsterror(va_alist)
va_dcl
#endif
{
	char	failform[1024];

	va_list	args;
#if __STD_C
	va_start(args,form);
#else
	char*	form;
	va_start(args);
	form = va_arg(args,char*);
#endif

	sprintf(failform, "Failure: %s", form);
	tstputmesg(Tstline,failform,args);

	va_end(args);

	tstcleanup();
	exit(1);
}


#if __STD_C
void tstsuccess(char* form, ...)
#else
void tstsuccess(va_alist)
va_dcl
#endif
{
	va_list	args;
#if __STD_C
	va_start(args,form);
#else
	char*	form;
	va_start(args);
	form = va_arg(args,char*);
#endif

	tstputmesg(Tstline,form,args);

	va_end(args);

	tstcleanup();
	exit(0);
}


#if __STD_C
void tstwarn(char* form, ...)
#else
void tstwarn(va_alist)
va_dcl
#endif
{
	va_list	args;
#if __STD_C
	va_start(args,form);
#else
	char*	form;
	va_start(args);
	form = va_arg(args,char*);
#endif

	tstputmesg(Tstline,form,args);

	va_end(args);
}


#if __STD_C
static char* tstfile(int n)
#else
static char* tstfile(n)
int	n;
#endif
{
	static int	Setatexit = 0;
	
	if(!Setatexit)
	{	Setatexit = 1;
		atexit(tstcleanup);
	}

	if(n >= sizeof(Tstfile)/sizeof(Tstfile[0]))
		terror("Bad temporary file request:%d\n", n);

	if(!Tstfile[n][0])
	{
#ifdef DEBUG
		sprintf(Tstfile[n], "Tstfile.%c%c%c", '0'+n, '0'+n, '0'+n);
#else
		static int	pid;
		static char*	tmp;
		if (!tmp)
		{	if (!(tmp = (char*)getenv("TMPDIR")) || access(tmp, 0) != 0)
				tmp = "/tmp";
			pid = (int)getpid() % 10000;
                }
                sprintf(Tstfile[n], "%s/sft.%c.%d", tmp, '0'+n, pid);
#endif
	}

	return Tstfile[n];
}
