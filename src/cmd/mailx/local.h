/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*Copyright (c) 1978-2004 The Regents of the University of California an*
*                                                                      *
*            Permission is hereby granted, free of charge,             *
*         to any person obtaining a copy of THIS SOFTWARE FILE         *
*              (the "Software"), to deal in the Software               *
*                without restriction, including without                *
*             limitation the rights to use, copy, modify,              *
*                  merge, publish, distribute, and/or                  *
*              sell copies of the Software, and to permit              *
*              persons to whom the Software is furnished               *
*            to do so, subject to the following disclaimer:            *
*                                                                      *
*THIS SOFTWARE IS PROVIDED BY The Regents of the University of Californ*
*           ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,           *
*              INCLUDING, BUT NOT LIMITED TO, THE IMPLIED              *
*              WARRANTIES OF MERCHANTABILITY AND FITNESS               *
*               FOR A PARTICULAR PURPOSE ARE DISCLAIMED.               *
*IN NO EVENT SHALL The Regents of the University of California and AT&T*
*           BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,            *
*             SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES             *
*             (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT              *
*            OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,             *
*             DATA, OR PROFITS; OR BUSINESS INTERRUPTION)              *
*            HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,            *
*            WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT            *
*             (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING              *
*             IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,              *
*          EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.          *
*                                                                      *
*              Information and Software Systems Research               *
*    The Regents of the University of California and AT&T Research     *
*                           Florham Park NJ                            *
*                                                                      *
*                          Kurt Shoens (UCB)                           *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
/*
 * Mail -- a mail program
 *
 * Local conventions and workarounds.
 */

#if _PACKAGE_ast
#include "FEATURE/lcl"
#endif

#if _hdr_paths
#include <paths.h>
#endif

#ifndef _PATH_ED
#define _PATH_ED	"/usr/bin/ed"
#endif
#ifndef _PATH_MASTER_RC
#define _PATH_MASTER_RC	"/etc/mailx.rc"
#endif
#ifndef _PATH_MAILDIR
#define _PATH_MAILDIR	"/usr/mail/"
#endif
#ifndef _PATH_MORE
#define _PATH_MORE	"/usr/bin/more"
#endif
#ifndef _PATH_RESCONF
#define _PATH_RESCONF	"/usr/etc/resolv.conf"
#endif
#ifndef _PATH_SENDMAIL
#define _PATH_SENDMAIL	"/usr/lib/sendmail"
#endif
#ifndef _PATH_SHELL
#define _PATH_SHELL	"/bin/sh"
#endif
#ifndef _PATH_TMP
#define _PATH_TMP	"/tmp/"
#endif
#ifndef _PATH_VI
#define _PATH_VI	"/usr/bin/vi"
#endif

#ifndef EFTYPE
#define EFTYPE		EINVAL
#endif

#ifndef FILENAME_MAX
#define FILENAME_MAX	PATH_MAX
#endif

#ifndef L_tmpnam
#define L_tmpnam	25
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN	PATH_MAX
#endif

#ifndef NOFILE
#define NOFILE		OPEN_MAX
#endif

#if !_PACKAGE_ast

#define SIG_REG_PENDING		(-1)
#define SIG_REG_POP		0
#define SIG_REG_EXEC		00001
#define SIG_REG_PROC		00002
#define SIG_REG_TERM		00004
#define SIG_REG_ALL		00777
#define SIG_REG_SET		01000

#if defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)
#define NiL	0
#else
#define NiL	((char*)0)
#endif

#define elementsof(x)		(sizeof(x)/sizeof(x[0]))
#define newof(p,t,n,x)		((p)?(t*)realloc((char*)(p),sizeof(t)*(n)+(x)):(t*)calloc(1,sizeof(t)*(n)+(x)))
#ifndef offsetof
#define offsetof(type,member) ((size_t)&(((type*)0)->member))
#endif
#define stracmp			strcmp
#define streq(a,b)		(*(a)==*(b)&&!strcmp(a,b))
#define strneq(a,b,n)		(*(a)==*(b)&&!strncmp(a,b,n))

typedef void (*Sig_handler_t)(int);

extern int		chresc(const char*, char**);
extern char*		fmtesc(const char*);
extern char*		fmtident(const char*);
extern char*		pathshell(void);
extern int		sigcritical(int);
extern int		sigunblock(int);
extern int		spawnvp(const char*, char* const*);
extern char*		strcopy(char*, const char*);
extern int		stresc(char*);
extern void*		strpsearch(const void*, size_t, size_t, const char*, char**);
extern void*		strsearch(const void*, size_t, size_t, int(*)(const char*, const char*), const char*, void*);
extern int		touch(const char*, time_t, time_t, int);

#endif
