/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1990-2000 AT&T Corp.              *
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

