/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1985-2002 AT&T Corp.                *
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
*                 Phong Vo <kpv@research.att.com>                  *
*******************************************************************/
#include <ast_std.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>

#if 0
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <net/if.h>
#include <linux/sockios.h>
#endif

#ifndef sigmask
#define sigmask(n)	((unsigned long)1 << ((n) - 1))
#endif

extern void _sethtent(int f);
extern void _endhtent(void);
extern struct hostent *_gethtent(void);
extern struct hostent *_gethtbyname(const char *name);
extern struct hostent *_gethtbyaddr(const char *addr, int len,
		 int type);
extern int _validuser(FILE *hostf, const char *rhost,
		 const char *luser, const char *ruser, int baselen);
extern int _checkhost(const char *rhost, const char *lhost, int len);

#if 0
extern void putlong(u_long l, u_char *msgp);
extern void putshort(u_short l, u_char *msgp);
extern u_int32_t _getlong(register const u_char *msgp);
extern u_int16_t _getshort(register const u_char *msgp);
extern void p_query(char *msg);
extern void fp_query(char *msg, FILE *file);
extern char *p_cdname(char *cp, char *msg, FILE *file);
extern char *p_rr(char *cp, char *msg, FILE *file);
extern char *p_type(int type);
extern char * p_class(int class);
extern char *p_time(u_long value);
#endif

extern char * hostalias(const char *name);
extern void sethostfile(char *name);
extern void _res_close (void);
extern void ruserpass(const char *host, char **aname, char **apass);
extern char* index(const char*, int);
extern int stricmp(const char*, const char*);
extern void bcopy(const void*, void*, size_t);
