/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1992-2002 AT&T Corp.                *
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
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                David Korn <dgk@research.att.com>                 *
*******************************************************************/
#pragma prototyped
/*
 * who.c
 * Written by David Korn
 * Thu Aug 15 00:12:29 EDT 1996
 */

static const char usage[] =
"[-?\n@(#)$Id: who (AT&T Labs Research) 2001-06-06 $\n]"
USAGE_LICENSE
"[+NAME?who - display who is on the system]"
"[+DESCRIPTION?\bwho\b displays various pieces of information about "
	"logged in users to standard output.]"
"[+?\bwho\b lists the user's name, terminal line, login time, and remote "
	"host name, if applicable, for each logged "
	"in user it reports.]"
"[+?The second form, \bwho am I\b is equivalent to \bwho -m\b.]"

"[i:idle?Also writes the idle time for each displayed user.  The idle time "
	"is the time since last activity on that line.]"
"[m?Write only information about the current terminal.]"
"[q:count?Writes only the login names and the number of users logged on.  "
	"This options overrides other options.]"
"[s?This option is ignored.]"
"[u:?Equivalent to \b-i\b.]"
"[w:writable?Equivalent to \b-T\b.]"
"[H:heading?Put a heading line in before of the output.]"
"[T:mesg?Specifies the state of the terminal as one of the following:]{"
	"[++?The terminal allows write access to other users.]"
	"[+-?The terminal denies write access to other users.]"
	"[+???The write access cannot be determined.]"
	"}"
"\n"
"\n[am I]\n"
"\n"
"[+EXIT STATUS]{"
	"[+0?Successful completion.]"
	"[+>0?One or more errors occurred.]"
"}"
"[+SEE ALSO?\bdate\b(1), \blogin\b(1)]"
;


#include	<cmd.h>
#include	<time.h>

#include "FEATURE/utmp"

#if _hdr_utmp
#	include	<utmp.h>
#endif

#if _mem_ut_host_utmp && _mem_ut_type_utmp
#	undef	_hdr_utmpx
#endif

#if _hdr_utmpx
#	include	<utmpx.h>
#	undef	_mem_ut_host_utmp
#	undef	_mem_ut_type_utmp
#	if _mem_ut_tv_utmpx
#		undef	ut_time
#		define	ut_time	ut_tv.tv_sec
#	endif
#	if ! _mem_ut_user_utmpx
#		define ut_user ut_name
#	endif
#	ifdef UTMPX_FILE
#		define UTMP	UTMPX_FILE
#	else
#		if _hdr_paths
#			include <paths.h>
#		endif
#		ifdef _PATH_UTMPX
#			define UTMP	_PATH_UTMPX
#		else
#			ifdef UTMP_FILE
#				define UTMP	UTMP_FILE
#			else
#				ifdef _PATH_UTMP
#					define UTMP	_PATH_UTMP
#				else
#					define UTMP	"/etc/utmpx"
#				endif
#			endif
#		endif
#	endif
#else
#	undef	_mem_ut_host_utmpx
#	undef	_mem_ut_type_utmpx
#	define utmpx	utmp
#	if _mem_ut_tv_utmp
#		undef	ut_time
#		define	ut_time	ut_tv.tv_sec
#	endif
#	if ! _mem_ut_user_utmp
#		define ut_user ut_name
#	endif
#	ifdef UTMP_FILE
#		define UTMP	UTMP_FILE
#	else
#		if _hdr_paths
#			include <paths.h>
#		endif
#		ifdef _PATH_UTMP
#			define UTMP	_PATH_UTMP
#		else
#			define UTMP	"/etc/utmp"
#		endif
#	endif
#endif

#ifdef	nonuser
#	define dead(ut)	(nonuser(ut))
#else
#	if _mem_ut_type_utmp || _mem_ut_type_utmpx
#		ifdef USER_PROCESS
#			define dead(ut)		((ut).ut_type != USER_PROCESS)
#		else
#			ifndef	DEAD_PROCESS
#				define DEAD_PROCESS	8
#			endif
#			define dead(ut)		((ut).ut_type == DEAD_PROCESS)
#		endif
#	else
#		define dead(ut)	0
#	endif
#endif

#define skip(ut)	(!*ut.ut_user||!*ut.ut_line||dead(ut))

typedef struct
{
	int	flags;
	int	siz_user;
	int	siz_line;
	int	siz_host;
} State_t;

#define FLAG_M	1
#define FLAG_T	2
#define FLAG_U	4
#define FLAG_H	8
#define FLAG_Q	0x10

static void outline(Sfio_t *out,register struct utmpx *up,register State_t* sp)
{
	struct stat statb;
	char *date = ctime(&up->ut_time);
	char line[sizeof(up->ut_line)+6];
	int r = 0;
	if(sp->flags&(FLAG_T|FLAG_U))
	{
		sfsprintf(line,sizeof(line),"/dev/%s\0",up->ut_line);
		r = stat(line,&statb);
	}
	if(sp->flags&FLAG_T)
	{
		int state = '?';
		if(r>=0)
			state = (statb.st_mode&S_IWOTH)?'+':'-';
		sfprintf(out,"%-*.*s %c %-*.*s %.12s ",sp->siz_user,sp->siz_user,up->ut_user,state,sp->siz_line,sp->siz_line,up->ut_line,date+4);
	}
	else
		sfprintf(out,"%-*.*s %-*.*s %.12s ",sp->siz_user,sp->siz_user,up->ut_user,sp->siz_line,sp->siz_line,up->ut_line,date+4);
	if(sp->flags&FLAG_U)
	{
		time_t t = time(0)-30;
		sfsprintf(line,sizeof(line),"/dev/%s\0",up->ut_line);
		if(r>=0 && statb.st_atime<t)
		{
			t = (t-(up->ut_time+30))/60;
			if(t>24*60)
				sfprintf(out," old  ");
			else
				sfprintf(out, "%.2d:%.2d ",t/60,t%60);
		}
		else
			sfprintf(out,"  .   ");
	}
#if _mem_ut_host_utmp || _mem_ut_host_utmpx
	if (*up->ut_host)
		sfprintf(out,"  %-*.*s\n",sp->siz_host,sp->siz_host,up->ut_host);
	else
#endif
	sfputc(out,'\n');
}

static int who(Sfio_t *in, Sfio_t *out, int flags)
{
	struct utmpx ut;
	char *line = 0;
	int count = 0;
	State_t state;
	state.flags = flags;
	state.siz_user = sizeof(ut.ut_user);
	if (state.siz_user > 24)
		state.siz_user = 24;
	state.siz_line = sizeof(ut.ut_line);
	if (state.siz_line > 8)
		state.siz_line = 8;
#if _mem_ut_host_utmp || _mem_ut_host_utmpx
	state.siz_host = sizeof(ut.ut_host);
	if (state.siz_host > 30)
		state.siz_host = 30;
#endif
	if(state.flags&FLAG_Q)
		state.flags = FLAG_Q;
	if((state.flags&FLAG_M) && ((line=ttyname(0)) || (line=ttyname(2))))
		line += 5;
	if(state.flags&FLAG_H)
	{
		char *t = "";
		if(state.flags&FLAG_T)
			t = "MESG";
		if(state.flags&FLAG_U)
			sfprintf(out,"%-*.*s %2s %-*.*s %.12s %6s     %s\n",state.siz_user-3,state.siz_user-3,"USER",t,state.siz_line,state.siz_line,"LINE","LOGIN-TIME","IDLE","FROM");
		else
			sfprintf(out,"%-*.*s %2s %-*.*s %.12s     %s\n",state.siz_user-3,state.siz_user-3,"USER",t,state.siz_line,state.siz_line,"LINE","LOGIN-TIME","FROM");
	}
	while(sfread(in,&ut,sizeof(struct utmpx))>0)
	{
		if(skip(ut))
			continue;
		if(line && strcmp(line,ut.ut_line))
			continue;
		if(state.flags&FLAG_Q)
		{
			if(count)
				sfputc(out,' ');
			sfprintf(out,"%s",ut.ut_user);
		}
		else
			outline(out,&ut,&state);
		count++;
	}
	if(state.flags&FLAG_Q)
		sfprintf(out,"\n# users=%d\n",count);
	return(0);
}

b_who(int argc, char *argv[], void *context)
{
	register int n, flags=0;
	Sfio_t *sp;
	error_info.id = argv[0];
	cmdinit(argv,context, ERROR_CATALOG);
	while (n = optget(argv, usage)) switch (n)
	{
	    case 'm':
		flags |= FLAG_M;
		break;
	    case 'H':
		flags |= FLAG_H;
		break;
	    case 'T': case 'w':
		flags |= FLAG_T;
		break;
	    case 'q':
		flags |= FLAG_Q;
		break;
	    case 's':
		break;
	    case 'i':
	    case 'u':
		flags |= FLAG_U;
		break;
	    case ':':
		error(2, opt_info.arg);
		break;
	    case '?':
		error(ERROR_usage(2), "%s", opt_info.arg);
		break;
	}
	argv += opt_info.index;
	argc -= opt_info.index;
	if(argc==2)
	{
		flags |= FLAG_M;
		argc = 0;
	}
	if(error_info.errors || argc)
		error(ERROR_usage(2),"%s", optusage((char*)0));
	if(!(sp = sfopen((Sfio_t*)0,UTMP,"r")))
		error(ERROR_system(1),"%s: cannot open",UTMP);
	who(sp,sfstdout,flags);
	sfclose(sp);
	return(error_info.errors);
}
