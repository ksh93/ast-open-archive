/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1992-2007 AT&T Intellectual Property          *
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
*                  David Korn <dgk@research.att.com>                   *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * David Korn
 * AT&T Research
 *
 * pty
 */ 

static const char usage[] =
"[-?\n@(#)pty (AT&T Research) 2001-04-02\n]"
USAGE_LICENSE
"[+NAME?pty - create pseudo terminal and run command]"
"[+DESCRIPTION?\bpty\b creates a pseudo pty and then runs \bcommand\b "
    "with arguments given by \aarg\a with it standard input, standard "
    "output, and standard error connected to the pseudo terminal. By "
    "default, the \bpty\b creates a new session.]"
"[+?If \bcommand\b does not contain a \b/\b, the \bPATH\b variable will "
    "be used to locate the \bcommand\b.]"
"[+?Input to \bpty\b will be written to the standard input of this "
    "command. The standard output and standard error from the command will "
    "be written to the standard output of \bpty\b.]"
"[+?The \bpty\b commmand terminates when the command completes.]"
"[s!:session?Create a separate session for the process started by "
    "\bpty\b.]"

"\n"
"\ncommand [arg ...]\n"
"\n"

"[+EXIT STATUS?If the command determined by \bcommand\b is run the exit "
    "status of \bpty\b is that of this command. Otherwise, the exit status "
    "is one of the following:]"
    "{"
        "[+127?The command is found but cannot be executed.]"
        "[+128?The command could not be found.]"
    "}"
"[+SEE ALSO?\bcommand\b(1), \bexec\b(1)]"
;


#include	<cmd.h>
#include	<error.h>
#include	<fcntl.h>
#include	<termios.h>
#include	<proc.h>
#include	<sys/socket.h>
#include	<sys/wait.h>
#include	<sys/stat.h>
#include	"FEATURE/pty"

#define MODE_666	(S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
#define MAXNAME		64

#if !_lib_openpty && !_lib__getpty && !defined(_pty_clone)
#   if !_lib_ptsname
	static char *slavename(const char *name)
	{
		static char sname[MAXNAME];
		char *last;
		strncpy(sname,name,sizeof(sname));
		last = strrchr(sname,'/');
		last[1] = 't';
		return(sname);
	}
#   endif

    static char *master_name(char *name)
    {
	static char sname[MAXNAME];
	int n;
	if(!name)
	{
		strcpy(sname,_pty_first);
		return(sname);
	}
	n = strlen(_pty_first);
	if(name[n-1]=='9')
		name[n-1]='a';
	else if(name[n-1]=='f')
	{
		if(_pty_first[n-2]=='0' && name[n-2]=='9')
		{
			name[n-2]='0';
			if(name[n-3]=='9' || name[n-3]=='z')
				return(NULL);
			name[n-3]++;
		}
		if(_pty_first[n-2]=='p' && (name[n-2]=='z' || name[n-2]=='Z'))
		{
			if(name[n-2]=='z')
				name[n-2]=='P';
			else
				return(0);
		}
		else
			name[n-2]++;
		name[n-1]='0';
	}
	else
		name[n-1]++;
	return(name);
    }
#endif

#if !_lib_openpty
	static char *ptymopen(int *master)
	{
		char *slave=0;
#   if _lib__getpty
		return(_getpty(master,O_RDWR,MODE_666,0));
#   else
#	if defined(_pty_clone)
		*master = open(_pty_clone,O_RDWR|O_CREAT,MODE_666);
		if(*master>=0)
			slave = ptsname(*master);
#	else
		int fdm;
		char *name=0;
		while(name=master_name(name))
		{
			fdm = open(name,O_RDWR|O_CREAT,MODE_666);
			if(fdm >= 0)
			{
				*master = fdm;
#	   if _lib_ptsname
				slave = ptsname(fdm);
#	   else
				slave = slavename(name);
#	   endif
				break;
			}
		}
# 	endif
#   endif
		return(slave);
	}
#endif


static int mkpty(int *master, int *slave)
{
	struct termios tty,*ttyp=0;
#ifdef TIOCGWINSZ
	struct winsize win,*winp=0;
#endif
#if !_lib_openpty
	char *sname;
#endif
	if(tcgetattr(STDERR_FILENO, &tty)>=0)
		ttyp = &tty;
	else
		error(ERROR_warn(0),"unable to get standard error terminal attributes");
#ifdef TIOCGWINSZ
	if(ioctl(STDERR_FILENO, TIOCGWINSZ, &win)>=0)
		winp = &win;
	else
		error(ERROR_warn(0),"unable to get standard error window size");
#endif
#if _lib_openpty
	if (openpty(master, slave, NULL, ttyp, winp) < 0)
		return(-1);
#else
	if(!(sname=ptymopen(master)) || (*slave = open(sname,O_RDWR)) <0)
		return(-1);
	if(ttyp && tcsetattr(*slave,TCSANOW,ttyp) <0)
		error(ERROR_warn(0),"unable to set pty terminal attributes");
#   ifdef TIOCSWINSZ
	if(winp && ioctl(*slave,TIOCSWINSZ,winp) <0)
		error(ERROR_warn(0),"unable to set pty window size");
#   endif
#endif
	fcntl(*master,F_SETFD,FD_CLOEXEC);
	fcntl(*slave,F_SETFD,FD_CLOEXEC);
	return(0);
}

static Proc_t *runcmd(char **argv, int slave, int flags)
{
	long opts[4];
	Proc_t *pp;
	int fd1,fd2;
	fd1 = dup(slave);
	fd2 = dup(slave);
	opts[0] = PROC_FD_DUP(slave,0,PROC_FD_CHILD);
	opts[1] = PROC_FD_DUP(fd1,1,PROC_FD_CHILD);
	opts[2] = PROC_FD_DUP(fd2,2,PROC_FD_CHILD);
	opts[3] = 0;
	pp = procopen(argv[0],argv,(char**)0, opts, flags);
	close(fd1);
	close(fd2);
	return(pp);
}

static void process(int master)
{
	char buff[8192];
	int i,n,infd[2],outfd[2];
	fd_set readfds;
	FD_ZERO(&readfds);
	infd[0] = 0;
	infd[1] = master;
	outfd[0] = master;
	outfd[1] = 1;
	while(1)
	{
		if(infd[0]>=0)
			FD_SET(infd[0], &readfds);
		if(infd[1]>=0)
			FD_SET(infd[1], &readfds);
#if 0
sfprintf(sfstderr,"before select\n"); 
#endif
		errno = 0;
		n = select(master+1,&readfds,(fd_set*)0,(fd_set*)0,(struct timeval*)0);
#if 0
sfprintf(sfstderr,"after select n=%d\n",n); 
#endif
		if(n>0)
		{
			for(i=0; i < 2; i++)
			{
				if(FD_ISSET(infd[i], &readfds))
				{
#if 0
sfprintf(sfstderr,"ready on %d\n",i);
#endif
					if((n=read(infd[i],buff,sizeof(buff)))>0)
					{
buff[n] = 0;
#if 0
sfprintf(sfstderr,"n=%d fd=%d buff=%s\n",n,outfd[i],buff);
#endif
						write(outfd[i],buff,n);
					}
					else
					{
						close(outfd[i]);
						close(infd[i]);
sfprintf(sfstderr,"close i=%d\n",i);
						if(infd[1-i]<0)
							break;
						infd[i] = -1;
					}
				}
			}
		}
		else
		{
			if(n<0)
				sfprintf(sfstderr,"errno=%d\n",errno);
			return;
		}
	}
}

int b_pty(int argc, char *argv[], void *context)
{
	int master,slave,flags=PROC_SESSION;
	Proc_t *proc;

	cmdinit(argc, argv, context, ERROR_CATALOG, 0);
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 's':
			flags = opt_info.num ? PROC_SESSION : 0;
			continue;
		case ':':
			break;
		case '?':
			error(ERROR_usage(2), "%s", opt_info.arg);
			break;
		}
		break;
	}
	argv += opt_info.index;
	if(mkpty(&master, &slave)<0)
		error(ERROR_system(1),"unable to create pty");
	if(!(proc = runcmd(argv, slave,flags)))
		error(ERROR_system(1),"unable run %s",argv[0]);
	close(slave);
	process(master);
	return procclose(proc);
}
