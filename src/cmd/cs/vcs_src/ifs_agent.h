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
 * File: ifs_agent.h
 */
#include <cs.h>
#include <ls.h>
#include "ifs_errno.h"

#define PROXY_HOST	"proxy.research.att.com"
#define PROXY_PORT	402

#define STRLEN		1024
#define SOCK_TIMEOUT	(60*1000)
#define IFS_PROXY	0x01

#ifdef DEBUG
#define debug_logit( msg )	logit( msg )
#else
#define debug_logit( msg )	(0)
#endif

typedef		int (FUNC)();

typedef struct sNetFile {
    char	netbuf[ STRLEN ];
    int		socket;
    int		head, size;
    int		err;
} NetFile;

struct agent_item {
    struct agent_item	*next;
    char	*name;
    char	*localdata;
    FUNC	*connect;
    FUNC	*disconnect;
    FUNC	*listdents;
    FUNC	*getfile;
    FUNC	*putfile;
    FUNC	*userdef;
};

struct mount_item {
    struct mount_item	*next;
    NetFile	*nFile;
    int		mode;
    char	*lpath;
    char	*proto;
    char	*user;
    int		pass;
    int		passlen;
    char	*host;
    int		port;
    char	*rpath;
    char	timeout[ 12 ];
};

struct mount_list {
    struct mount_list	*next;
    int			uid;
    struct mount_item	*mitem;
};

struct server_info {
    struct agent_item	*agent;
    struct mount_item	*mitem;
    char		*lpath;
    char		*proxy;
    char		rpath[ STRLEN ];
    int			flags;
};

extern int	cserrno;
extern char	csusrmsg[ STRLEN ];
extern int	IfsAbortFlag;

extern NetFile*	NetConnect();
extern int	NetClose();
extern int	NetRead();
extern char*	NetGets();
extern int	NetWrite();

