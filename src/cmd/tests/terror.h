/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1999-2011 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                 Eclipse Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
*                                                                      *
*                A copy of the License is available at                 *
*          http://www.eclipse.org/org/documents/epl-v10.html           *
*         (with md5 checksum b35adb5213ca9657e911e9befb180842)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#include	<stdio.h>
#include	<ast_common.h>

#ifndef NIL
#define NIL(t)	((t)0)
#endif

#if _hdr_stdlib
#include	<stdlib.h>
#endif
#if _hdr_unistd
#include	<unistd.h>
#endif

#include	<aso.h>
#include	<errno.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<string.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/wait.h>

#ifndef elementsof
#define elementsof(x)	(sizeof(x)/sizeof(x[0]))
#endif
#ifndef integralof
#define integralof(x)	(((char*)(x))-((char*)0))
#endif

_BEGIN_EXTERNS_

extern int	sprintf _ARG_((char*, const char*, ...));

#if !__STD_C && !_hdr_stdlib
extern int	atexit _ARG_((void (*)(void)));
extern void	exit _ARG_((int));
extern Void_t*	malloc _ARG_((size_t));
extern char*	getenv _ARG_((const char*));
extern int	system _ARG_((const char*));
#endif

#if !_hdr_unistd
extern int	alarm _ARG_((int));
extern int	sleep _ARG_((int));
extern int	fork();
extern int	access _ARG_((const char*, int));
extern int	write _ARG_((int, const void*, int));
extern int	unlink _ARG_((const char*));
extern Void_t*	sbrk _ARG_((int));
extern int	getpid();
#endif

extern void	tsterror _ARG_((char*, ...));
extern void	tstinfo _ARG_((char*, ...));
extern void	tstwarn _ARG_((char*, ...));
extern void	tstsuccess _ARG_((char*, ...));

_END_EXTERNS_

static int		Tstall;
static int		Tstline;
static char		Tstfile[16][256];

#ifdef __LINE__
#define terror		(Tstline=__LINE__),tsterror
#else
#define terror		(Tstline=-1),tsterror
#endif

#ifdef __LINE__
#define tinfo		(Tstline=__LINE__),tstinfo
#else
#define tinfo		(Tstline=-1),tstinfo
#endif

#ifdef __LINE__
#define twarn		(Tstline=__LINE__),tstwarn
#else
#define twarn		(Tstline=-1),tstwarn
#endif

#ifdef __LINE__
#define tpause		(Tstline=__LINE__),tstpause
#else
#define tpause		(Tstline=-1),tstpause
#endif

#ifdef __LINE__
#define twait(p,n)	((Tstline=__LINE__),tstwait(p,n))
#else
#define twait(p,n)	((Tstline=-1),tstwait(p,n))
#endif

#ifdef __LINE__
#define tsuccess	(Tstline=__LINE__),tstsuccess
#else
#define tsuccess	(Tstline=-1),tstsuccess
#endif

#ifdef __LINE__
#define taso(t)		((Tstline=__LINE__),tstaso(t,argv))
#else
#define taso(t)		((Tstline=-1),tstaso(t,argv))
#endif

#ifdef __LINE__
#define tchild()	((Tstline=__LINE__),tstchild(argv))
#else
#define tchild()	((Tstline=-1),tstchild(argv))
#endif

#ifdef __LINE__
#define topts()		((Tstline=__LINE__),tstopts(argv))
#else
#define topts()		((Tstline=-1),tstopts(argv))
#endif

#define tmesg		(Tstline=-1),tstwarn

#ifdef DEBUG
#ifdef __LINE__
#define TSTDEBUG(x)	(Tstline=__LINE__),tstwarn x
#else
#define TSTDEBUG(x)	(Tstline=-1),tstwarn x
#endif
#else
#define TSTDEBUG(x)
#endif

#ifndef tmain
#if __STD_C
#define tmain()		int main(int argc, char** argv)
#else
#define tmain()		int main(argc, argv) int argc; char** argv;
#endif
#endif /*tmain*/

#ifndef texit
#define texit(v)	{ tcleanup(); exit(v); }
#endif

#if __STD_C
static void tcleanup(void)
#else
static void tcleanup()
#endif
{
#ifdef DEBUG
	twarn("Temp files will not be removed");
#else
	int	i;
	for(i = 0; i < sizeof(Tstfile)/sizeof(Tstfile[0]); ++i)
		if(Tstfile[i][0]) {
			unlink(Tstfile[i]);
			Tstfile[i][0] = 0;
		}
#endif
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
	size_t	n;

	for(n = 0; n < sizeof(buf); ++n)
		buf[n] = 0;

	s = buf; n = 0;
	if(line >= 0)
	{
#if _SFIO_H
		sfsprintf(s, sizeof(buf), "\tLine=%d: ", line);
#else
		sprintf(s, "\tLine=%d: ", line);
#endif
		s += (n = strlen(s));
	}
#if _SFIO_H
	sfvsprintf(s, sizeof(buf)-n, form, args);
#else
	vsprintf(s, form, args);
#endif

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

#if _SFIO_H
	sfsprintf(failform, sizeof(failform), "FAILED %s [errno=%d]", form, errno);
#else
	sprintf(failform, "FAILED %s [errno=%d]", form, errno);
#endif

	tstputmesg(Tstline,failform,args);

	va_end(args);

	tcleanup();
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

	tcleanup();
	exit(0);
}

#if __STD_C
void tstinfo(char* form, ...)
#else
void tstinfo(va_alist)
va_dcl
#endif
{
#ifdef INFO
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
#endif
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
void tstpause(char* form, ...)
#else
void tstpause(va_alist)
va_dcl
#endif
{
	char	pauseform[1024];

	va_list	args;
#if __STD_C
	va_start(args,form);
#else
	char*	form;
	va_start(args);
	form = va_arg(args,char*);
#endif

#ifdef INFO
#if _SFIO_H
	sfsprintf(pauseform, sizeof(pauseform), "Pausing: %s", form);
#else
	sprintf(pauseform, "Pausing: %s", form);
#endif
	tstputmesg(Tstline,pauseform,args);
#endif


	va_end(args);

	sleep(15);
}

#if __STD_C
int tstwait(pid_t* proc, int nproc)
#else
int tstwait(proc, nproc)
pid_t*	proc;
int	nproc;
#endif
{
	int	code = 2, n, status, reaped = 0;
	pid_t	pid, parent = getpid();

	tstinfo("Parent[pid=%d]: waiting for %d child%s", parent, nproc, nproc == 1 ? "" : "ren");
	while ((pid = wait(&status)) > 0)
	{	for (n = 0; n < nproc; n++)
		{	if (proc[n] == pid)
			{	tstinfo("Parent[pid=%d]: process %d[pid=%d] status=%d", parent, n, pid, status);
				reaped++;
				break;
			}
		}
		if (n >= nproc)
			tstwarn("Parent[pid=%d]: process UNKNOWN[pid=%d] status=%d", parent, pid, status);
		if (status)
			code = 1;
		else if (code > 1)
			code = 0;
	}
	if (reaped != nproc)
	{	tstwarn("Parent[pid=%d]: expected %d process%s, got %d", parent, nproc, nproc == 1 ? "" : "es", reaped);
		code = 2;
	}
	return code;
}

#if __STD_C
static char* tstfile(char* pfx, int n)
#else
static char* tstfile(pfx, n)
char*	pfx;
int	n;
#endif
{
	static int	Setatexit = 0;
	
	if(!Setatexit)
	{	Setatexit = 1;
		atexit(tcleanup);
	}

	if(n < 0)
		for(n = 0; n < (int)(sizeof(Tstfile)/sizeof(Tstfile[0])); ++n)
			if(Tstfile[n][0] == 0 )
				break;
	if(n >= sizeof(Tstfile)/sizeof(Tstfile[0]))
		terror("Bad temporary file request:%d\n", n);

	pfx = (pfx && pfx[0]) ? pfx : "tmp";

	if(!Tstfile[n][0])
	{
#ifdef DEBUG
#if _SFIO_H
		sfsprintf(Tstfile[n], sizeof(Tstfile[0]), "%s.%c%c%c.tst", pfx, '0'+n, '0'+n, '0'+n);
#else
		sprintf(Tstfile[n], "%s.%c%c%c.tst", pfx, '0'+n, '0'+n, '0'+n);
#endif
#else
		static int	pid;
		static char*	tmp;
		if (!tmp)
		{	if (!(tmp = (char*)getenv("TMPDIR")) || access(tmp, 0) != 0)
				tmp = "/tmp";
			pid = (int)getpid() % 10000;
                }
#if _SFIO_H
                sfsprintf(Tstfile[n], sizeof(Tstfile[0]), "%s/%s.%c.%d.tst", tmp, pfx, '0'+n, pid);
#else
                sprintf(Tstfile[n], "%s/%s.%c.%d.tst", tmp, pfx, '0'+n, pid);
#endif
#endif
	}

	return Tstfile[n];
}

static int asoerror(int type, const char* mesg)
{
	tsterror("aso error %d: %s", type, mesg);
	return 0;
}

typedef struct Asotype_s
{
	const char*	name;
	int		mask;
} Asotype_t;

static const Asotype_t	asotype[] =
{
	{ "SIGNAL",	ASO_SIGNAL },
	{ "THREAD",	ASO_THREAD },
	{ "PROCESS",	ASO_PROCESS },
	{ "INTRINSIC",	ASO_INTRINSIC },
};

static char* asotypes(int type)
{
	int	i;
	char*	p;

	static char	buf[128];
	static char*	b = buf;

	p = b;
	for (i = 0; i < elementsof(asotype); i++)
		if (asotype[i].mask & type)
		{
			if (b > p)
				*b++ = '|';
			strcpy(b, asotype[i].name);
			b += strlen(asotype[i].name);
		}
	*b++ = 0;
	return p;
}

#if __STD_C
static char* tstaso(int type, char** argv)
#else
static char* tstaso(type, argv)
int	type;
char**	argv;
#endif
{
	char*		a;
	char*		name = 0;
	Asometh_t*	meth;
	Asodisc_t	disc;

	ASODISC(&disc, asoerror);
	disc.hung = 16;
	asoinit(0, 0, &disc);
	while (a = *++argv)
		if (strcmp(a, "--all") == 0)
			Tstall++;
		else if (strncmp(a, "--aso=", 6) == 0)
		{
			if (a[6])
			{
				name = a + 6;
				Tstall++;
			}
			else
				a = 0;
			break;
		}
	if (type || name)
	{
		if (!(meth = asometh(0, name)))
			tsterror("aso method %s not found", name);
		if (!(meth->type & type))
		{
			if (name)
				tsterror("aso method %s only supports type %s, not %s", meth->name, asotypes(meth->type), asotypes(type));
			if (!(meth = asometh(type, name)))
				tsterror("aso method type %s not found", asotypes(type));
			if (asoinit(0, meth, 0))
				tsterror("aso method %s initialization error *and* asoerror() did not report it");
		}
	}
	return a;
}

static void
asodie(int sig)
{
	pid_t	pid;
	int	use;

	pid = getpid();
	signal(sig, SIG_IGN);
	if ((use = sig) == SIGALRM)
		use = SIGTERM;
	kill(-pid, use);
	tsterror("%d terminated with signal %d", pid, sig);
}

#if __STD_C
static int tstchild(char** argv)
#else
static int tstchild(argv)
char**	argv;
#endif
{
	char**		v = argv;
	char*		a;

	while (a = *++v)
		if (strcmp(a, "--all") == 0)
			Tstall++;
		else if (strcmp(a, "--child") == 0)
			return (int)(v - argv + 1);
		else if (strcmp(a, "--") == 0)
			break;
		else if (strncmp(a, "--", 2) != 0)
			break;
	setpgid(0, 0);
	signal(SIGALRM, asodie);
	signal(SIGINT, asodie);
	signal(SIGQUIT, asodie);
	signal(SIGTERM, asodie);
	/* no single test should take longer than 10 minutes, right? */
	alarm(10 * 60);
	return 0;
}

#if __STD_C
static int tstopts(char** argv)
#else
static int tstopts(argv)
char**	argv;
#endif
{
	char**		v = argv;
	char*		a;

	while (a = *++v)
		if (strcmp(a, "--all") == 0)
			Tstall++;
		else if (strcmp(a, "--") == 0)
			return (int)(v - argv + 1);
		else if (strncmp(a, "--", 2) != 0)
			break;
	return (int)(v - argv);
}
