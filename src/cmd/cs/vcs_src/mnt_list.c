/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1990-2003 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*    If you have copied or used this software without agreeing     *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
#include "mnt_imount.h"

#define USAGE "list [mount-point ...]"

int im_list_help(s)
	char*	s;
{
	printf("\t%s\n", USAGE);
	return (0);
}

int im_list(argc, argv)
	int 		argc;
	register char**	argv;
{
	register int	n;
	register char*	s;
	int		seq;
	int		fd;
	int		len;
	char*		mpoint;
	char		buf[1024];
	char		reply[1024];

	error_info.id = argv[0];
	memset(buf, 0, sizeof(buf));
	memset(reply, 0, sizeof(reply));

	opt_info.index = 1;
	while (n = optget(argv, "s:[server] "))
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
	argc -= opt_info.index;
	argv += opt_info.index;

	if (argc == 0)
	{
		seq = -1;
		do
		{
			if (seq == -1)
				sprintf(buf, "m -0\n");
			else
				sprintf(buf, "m %d\n", seq);
			if (vcs_write(buf) <= 0 || vcs_read(reply, 1024) <= 0)
				return (-1);
			if (seq == -1)
			{
				if ((s = strstr(reply, "ok 0 ")) == NULL)
				{
					printf("%s", buf);
					return (-1);
				}
				s += 5;
				len = (int)strtol(s, (char**)0, 0);
			}
			else
				printmtmsg(reply);
			seq++;
		} while (seq < len);
		return (0);
	}
	else
	{
		while  (argc> 0)
		{
			mpoint = *argv;
			sprintf(buf, "m %s\n", mpoint);	
			if (vcs_write(buf) > 0 && vcs_read(reply, 1024) > 0)
				printmtmsg(reply);
			argc--;
			argv++;
		}
	}
	return (0);
}
