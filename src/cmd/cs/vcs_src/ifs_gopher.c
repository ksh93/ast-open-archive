/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1990-2002 AT&T Corp.                *
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
*******************************************************************/
/*
 * File: ifs_gopher.c
 */

#include "ifs_agent.h"
#include <stdio.h>

struct {
    int		version;
} gopher_data;

int
GopherXfer( srv, rpath, tmpfile )
struct server_info *srv;
char	*rpath;
char	*tmpfile;
{
    struct mount_item	*mitem = srv->mitem;
    NetFile	*nFile;
    FILE	*fout;
    char	buf[ STRLEN ];
    int		port, len;

    if( (port = mitem->port) <= 0 )
	port = 70;
    if( (nFile = NetConnect( srv, mitem->host, port )) == NULL ) {
	logit( "<gopher>: connect failed!\n" );
	cserrno = E_CONNECT;
	return -1;
    }
    NetWrite( nFile, rpath, strlen(rpath) );
    logit( rpath );

    if( NetGets( nFile, buf, sizeof(buf) ) == NULL ||
	strmatch( buf, "*\terror.host\t*" ) ) {
	/* file/directory not found  */
	logit( buf );
	cserrno = E_CONNECT;
	NetClose( nFile );
	return -1;
    }
    logit( buf );

    if( (fout = fopen( tmpfile, "w" )) == NULL ) {
	cserrno = E_OPENDEST;
	NetClose( nFile );
	return -1;
    }
    fprintf( fout, "%s", buf );
    while( (len = NetRead( nFile, buf, sizeof(buf) )) > 0 ) {
	fwrite( buf, 1, len, fout );
    }
    fclose( fout );
    NetClose( nFile );
    return 0;
}

int
GopherXferFile( srv, tmpfile )
struct server_info *srv;
char	*tmpfile;
{
    char	buf[ STRLEN ];

    sprintf( buf, "0%s\r\n", srv->rpath );
    if( GopherXfer( srv, buf, tmpfile ) == -1 )
	return -1;
    MakePath( srv->lpath );
    rename( tmpfile, srv->lpath );
    return 0;
}

int
GopherXferDir( srv, tmpfile )
struct server_info *srv;
char	*tmpfile;
{
    char	buf[ STRLEN ];

    sprintf( buf, "1%s\r\n", srv->rpath );
    if( GopherXfer( srv, buf, tmpfile ) == -1 )
	return -1;
    sprintf( buf, "%s/._dir", srv->lpath );
    MakePath( buf );
    chdir( srv->lpath );
    rename( tmpfile, "._dir" );
    symlink( srv->mitem->timeout, "._cache_time" );
    return 0;
}

int
GopherGetFile( srv )
struct server_info *srv;
{
    char	tmpfile[ STRLEN ];
    char	*lpath = srv->lpath;
    char	*rpath = srv->rpath;
    int		ans;

    MakeTmpFile( lpath, tmpfile, sizeof(tmpfile) );
    if( *rpath == '\0' || DashD( lpath ) ) {
	ans = GopherXferDir( srv, tmpfile );
    } else if( DashF( lpath ) ) {
	ans = GopherXferFile( srv, tmpfile );
    } else {
	if( (ans = GopherXferDir( srv, tmpfile )) == -1 ) {
	    ans = GopherXferFile( srv, tmpfile );
	}
    }
    unlink( tmpfile );
    return ans;
}

int
GopherNop()
{
    return 0;
}

int
GopherInit( tbl )
struct agent_item	*tbl;
{
    tbl->localdata	= (char *) &gopher_data;
    tbl->connect	= GopherNop;
    tbl->disconnect	= GopherNop;
    tbl->listdents	= GopherGetFile;
    tbl->getfile	= GopherGetFile;
    tbl->putfile	= GopherNop;
    tbl->userdef	= GopherNop;
    return 0;
}

