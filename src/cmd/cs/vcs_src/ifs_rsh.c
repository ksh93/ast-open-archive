/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1990-2001 AT&T Corp.                *
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
/*
 * File: ifs_rsh.c
 */

#include "ifs_agent.h"
#include <stdio.h>

struct {
    int		version;
} rsh_data;

/*
 * name: RshGetFile
 *	get a file/directory from remote host
 */
int
RshGetFile( srv )
struct server_info *srv;
{
    struct mount_item	*mitem;
    char	cmd[ STRLEN ];
    char	tmpfile[ STRLEN ], buf[ STRLEN ];
    char	*lpath, *rpath, *hostid, *userid;
    FILE	*fp;
    int		fd;

    lpath = srv->lpath;
    rpath = srv->rpath;
    mitem = srv->mitem;
    hostid = mitem->host;
    userid = mitem->user;
    if( *rpath == '\0' || DashD( lpath ) ) {
	if( *rpath == '\0' ) rpath = "/";
	if( userid == NULL || *userid == '\0' ) {
	    sprintf( cmd, "rsh %s", hostid );
	} else {
	    sprintf( cmd, "rsh -l %s %s", userid, hostid );
	}
	sprintf( buf, "%s /bin/ls -alL %s", cmd, rpath );
	logit( buf );
	logit( "\n" );
	if( (fp = popen( buf, "rw" )) == NULL )
	    return -1;
	MakeTmpFile( lpath, tmpfile, sizeof(tmpfile) );
	if( (fd = open( tmpfile, O_WRONLY|O_CREAT, 0644 )) < 0 ) {
	    cserrno = E_DATAXFER;
	    return -1;
	}
	while( fgets( buf, sizeof(buf), fp ) != NULL )
	    write( fd, buf, strlen(buf) );
	close( fd );
	fclose( fp );
	sprintf( buf, "%s/._dir", lpath );
	MakePath( buf );
	chdir( lpath );
	ftp_makedents( tmpfile );
	rename( tmpfile, "._dir" );
	symlink( mitem->timeout, "._cache_time" );
    } else {
	if( userid == NULL || *userid == '\0' ) {
	    sprintf( cmd, "%s", hostid );
	} else { 
	    sprintf( cmd, "%s@%s", userid, hostid );
	}
	MakeTmpFile( lpath, tmpfile, sizeof(tmpfile) );
	sprintf( buf, "rcp %s:%s %s", cmd, rpath, tmpfile );
	logit( buf );
	logit( "\n" );
	if( (fp = popen( buf, "rw" )) == NULL )
	    return -1;
	fclose( fp );
	if( !DashF( tmpfile ) )
	    return -1;
	MakePath( lpath );
	rename( tmpfile, lpath );
    }
    return 0;
}

/*
 * name: RshPutFile
 *	put a file/directory to remote host
 */
int
RshPutFile( srv )
struct server_info *srv;
{
    return -1;
}

/*
 * name: RshNop
 *      unimplement command
 */
int
RshNop()
{
    return 0;
}

int
RshInit( tbl )
struct agent_item	*tbl;
{
    tbl->localdata	= (char *) &rsh_data;
    tbl->connect	= RshNop;
    tbl->disconnect	= RshNop;
    tbl->listdents	= RshGetFile;
    tbl->getfile	= RshGetFile;
    tbl->putfile	= RshPutFile;
    tbl->userdef	= RshNop;
    return 0;
}

