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
#include "mnt_imount.h"

#define USAGE	"umount	mount-point1 mount-point2 ..."

int im_umount_help(s)
	char*	s;
{
	printf("\t%s\n", USAGE);
	return (0);
}

int im_umount(argc, argv)
	int		argc;
	register char**	argv;
{
	int		n;
	register char*	s;
	int		fd;
	char		buf[1024];
	char		reply[1024];

	error_info.id = argv[0];
	memset(buf, 0, sizeof(buf));
	memset(reply, 0, sizeof(reply));

	opt_info.index = 1;
	while (n = optget(argv, "s:[cs_server] [mount-point ...]"))
	 switch (n)
	{
	case 's':
		s = opt_info.arg;
		if ((fd = csopen(s, CS_OPEN_READ)) < 0)
		{
			printf("cannot connect cs server %s\n", s);
			return (-1);
		}
		istate.cs_svc = strdup(s);
		istate.fd = fd;
		break;
	case '?':
	case ':':
		printf(USAGE);
		return (1);
	}

	argv += opt_info.index;
	argc -= opt_info.index;

	while(argc > 0)
	{
		s = *argv;
		sprintf(buf, "m %s -\n", s);
		if (vcs_write(buf) > 0 &&  vcs_read(reply, 1024) > 0)
		{
			if (strncmp(reply, "I 0 ok 0", 8) == 0)
			{
				printf("%s deleted\n", s);
				(void)rm_entry(s);
			}
			else 
				printf(reply);
		}
		argc--;
		argv++;
	}

	return (0);
}
