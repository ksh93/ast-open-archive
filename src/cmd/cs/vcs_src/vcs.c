/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1990-2004 AT&T Corp.                  *
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
/*
 * File: server.c
 */

static char id[] = "\n@(#)$Id: vcs+ifs (AT&T Research) 1996-02-29 $\0\n";

#include "ifs_agent.h"
#include "ifs_errno.h"
#include <cs.h>
#include <error.h>
#include <fs3d.h>

#define	CST	( (State_t *) handle )

/*
 *	replies
 *		I ... 	information
 *		W ...	warning message
 *		E ...	error message
 */

typedef struct
{
    time_t	uptime;
    char*	where;
    int		out_static;
    int		dormant;
    int		who;
    int		userid[ FD_SETSIZE ];
    Sfio_t*	logfd;
    char*	logfile;
} State_t;

int	cserrno;
char	csusrmsg[ STRLEN ];

char command_help[] = "\
help;	kill;	quit;	version;			\t\t\t\
 record [logfile];					\t\t\t\
 out pathname [version [pathname]]			\t\t\t\
 instances pathname					\t\t\t\
 mount url(pro:/usr:pw@host:pt/path) lpath time		\t\t\t\
 connect protocol server user-info;			\t\t\t\
 disconnect protocol server;				\t\t\t\
 listdents local-path 					\t\t\t\
 getfile   local-path					\t\t\t\
 putfile   [source] local-path				\t\t\t\
 userdef protocol arguments\n";

void*
svc_init( handle, fdmax )
void*	handle;
int	fdmax;
{
    char	pidfile[ 80 ];
    char	pid[ 80 ];

    (void) fs3d(FS3D_OP_OFF);
    (void) memset( CST, 0, sizeof(State_t) );
    CST->uptime = cs.time;
    CST->who = getuid();
    sprintf( pid, "%d", getpid() );
    GetUserFile( pidfile, "vcs.pid" );
    unlink( pidfile );
    symlink( pid, pidfile );
    IfsInitial();
    return( handle );
}

static int
svc_connect( handle, fd, id, clone, args )
void*	handle;
int	fd;
Cs_id_t* id;
int	clone;
char**	args;
{
    if ( CST->logfd ) {
	sfprintf( CST->logfd,
		"\n-- connect( fd=%d hid=%s pid=%d uid=%d gid=%d )\n",
		fd, csntoa(id->hid), id->pid, id->uid, id->gid );
	sfsync( CST->logfd );
    }
    if( fd < FD_SETSIZE )
	CST->userid[ fd ] = id->uid;
    return (0);
}

static int
svc_done( handle, sig )
void*	handle;
int	sig;
{
    if ( CST->logfd ) {
	sfprintf( CST->logfd, "-- svc_done( %d )\n", sig );
	sfsync( CST->logfd );
    }
    return (0);
}

static int
version_info( buf, uid, own, uptm )
char	*buf;
int	uid;
int	own;
time_t	uptm;
{
    sprintf( buf, "I Hi! %s, vcs[%d] by %s@%s at %s [%s]\n",
		fmtuid(uid), getpid(), fmtuid(own), csname(0), fmttime(NiL, uptm), id + 10 );
    return 0;
}

static int
svc_read( handle, fd )
void*	handle;
int	fd;
{
    char	msgbuf[ 4 * PATH_MAX ];
    char	ret[ 4 * PATH_MAX ];
    char	*msg = msgbuf;
    char	*argv[16], *ptr;
    int		uid, n, argc;

    (void) fs3d(FS3D_OP_OFF);
    csusrmsg[0] = '\0';
    uid = CST->userid[fd];
    if( (n = csread( fd, msg, sizeof(msgbuf), CS_LINE )) <= 0 )
	return -1;
    msg[ n - 1 ] = '\0';
    if( CST->logfd ) {
	time_t	t;

	t = cs.time;
	(void) tmform(ret, "%i", &t);
	sfprintf( CST->logfd, "[%s]\n%s\n", ret, msg );
    }
    while ((argc = tokscan( msg, &msg, " %v ", argv, elementsof(argv))) > 0) {
	n = sizeof(ret);
	switch ( argv[0][0] ) {
	    case 'c':
		n = IfsConnect( ret, n, uid, argc, argv );
		break;
	    case 'd':
		n = IfsDisconnect( ret, n, uid, argc, argv );
		break;
	    case 'g':
		n = IfsGetFile( ret, n, uid, argc, argv );
		break;
	    case 'h':
		n = sfsprintf(ret, n, command_help );
		break;
	    case 'i':
		n = rscs_instances( argc, argv, ret, &n );
		break;
	    case 'k':
		exit( 1 );
		break;
	    case 'l':
		n = IfsListDEnts( ret, n, uid, argc, argv );
		break;
	    case 'm':
		n = IfsMount( ret, n, uid, argc, argv );
		break;
	    case 'o':
		switch (argv[0][1]) {
		case 'p':
		    n = IfsReal(ret, n, uid, argc, argv);
		    break;
		default:
		    n = rscs_out( argc, argv, ret, &n );
		    break;
		}
		break;
	    case 'p':
		n = IfsPutFile( ret, n, uid, argc, argv );
		break;
	    case 'q':
		n = sfsprintf(ret, n, "I bye\n");
		break;
	    case 'r':
		ptr = (argc < 2 ? "/tmp/jvcs.log" : argv[1]);
		if ( CST->logfd ) {
		    sfclose( CST->logfd );
		    n = sfsprintf(ret, n, "I stop log %s (%d)\n",
			CST->logfile, CST->logfd );
		    CST->logfd = 0;
		} else if( (CST->logfd = sfopen(NULL,ptr,"a")) == NULL ) {
		    n = sfsprintf(ret, n, "E cannot open %s\n", ptr );
	 	} else {
		    CST->logfile = strdup( ptr );
		    sfprintf( CST->logfd, "\n" );
		    n = sfsprintf(ret, n, "I record %s\n", ptr );
		}
		break;
	    case 's':
		n = IfsReal(ret, n, uid, argc, argv);
		break;
	    case 'v':
		version_info( ret, uid, CST->who, CST->uptime );
		n = strlen( ret );
		break;
	    case 'u':
		n = IfsUserDef( ret, n, uid, argc, argv );
		break;
	    case 'D':
		error_info.trace = -(int)strtol(argv[1], (char**)0, 0);
		n = sfsprintf(ret, n, "I debug level %d\n", -error_info.trace );
		break;
	    default:
		n = sfsprintf(ret, n, "E invalid command %s\n", argv[0] );
		break;
	}
	if (write(fd, ret, n) != n || *argv[0] == 'q')
	    return -1;
    }
    if ( CST->logfd ) {
	if( ret[n-1] == '\n' )
	    ret[n-1] = '\0';
	sfprintf( CST->logfd, "\t%s\n", ret );
	sfsync( CST->logfd );
    }
    return (0);
}

static int
svc_timeout( handle )
void*	handle;
{
    return (0);
}

int
main( argc, argv )
int	argc;
char**	argv;
{
    static State_t	state;

    NoP(argc);
    csserve( &state, argv[1], svc_init, svc_done, svc_connect,
		svc_read, NULL, svc_timeout);
    exit(1);
}

