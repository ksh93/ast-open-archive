/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 2003-2004 AT&T Corp.                *
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
*                David Korn <dgk@research.att.com>                 *
*                 Phong Vo <kpv@research.att.com>                  *
*                                                                  *
*******************************************************************/
#pragma prototyped

/*
 * codex public interface
 */

#ifndef _CODEX_H
#define _CODEX_H	1

#include <ast.h>
#include <error.h>

#define CODEX_VERSION	20040104L

#define CODEX_DECODE	0x0001		/* decode supported		*/
#define CODEX_ENCODE	0x0002		/* encode supported		*/

#define CODEX_RETAIN	0x0010		/* initf-donef retain state	*/
#define CODEX_INVERT	0x0020		/* invert composition		*/
#define CODEX_TRACE	0x0040		/* enable trace			*/
#define CODEX_VERBOSE	0x0080		/* enable verbose trace		*/

#define CODEX_COMPRESS	0x0100		/* compression			*/
#define CODEX_CRYPT	0x0200		/* encryption			*/
#define CODEX_ICONV	0x0400		/* character code translation	*/
#define CODEX_SUM	0x0800		/* checksum			*/
#define CODEX_UU	0x1000		/* uuencode (ISO text)		*/

#define CODEX_SERIAL	0x20000000L	/* serial number assigned	*/
#define CODEX_FLUSH	0x40000000L	/* flush-only sync		*/
#define CODEX_ACTIVE	0x80000000L	/* active cache entry		*/

#define CODEX_IDENT	1024		/* max ident buffer size	*/
#define CODEX_NAME	256		/* max coder name w/args	*/
#define CODEX_ARGS	32		/* max coder args in name	*/

#define CODEX(d)	((Codex_t*)(d))

typedef unsigned _ast_int4_t Codexnum_t;

struct Codex_s; typedef struct Codex_s Codex_t;
struct Codexdata_s; typedef struct Codexdata_s Codexdata_t;
struct Codexdisc_s; typedef struct Codexdisc_s Codexdisc_t;
struct Codexmeth_s; typedef struct Codexmeth_s Codexmeth_t;

typedef Codexmeth_t* (*Codexlib_f)(const char*);

struct Codex_s				/* coder public state		*/
{
	Sfdisc_t	sfdisc;		/* coder sfio discipline	*/
	Sfio_t*		sp;		/* data base stream		*/
	Sfio_t*		dp;		/* optional data delta stream	*/
	Sfoff_t		size;		/* expected size if != -1	*/
	Codexdisc_t*	disc;		/* coder discipline		*/
	Codexmeth_t*	meth;		/* coder method			*/
	Codexnum_t	flags;		/* CODEX_* flags		*/
	Codexnum_t	index;		/* per-process index		*/
	int		serial;		/* codex() serial number	*/
	void*		data;		/* coder private state		*/
};

struct Codexdata_s			/* codexdata() info		*/
{
	Codexnum_t	size;		/* value size			*/
	Codexnum_t	num;		/* value if buf!=0		*/
	void*		buf;		/* size byte value		*/
};

struct Codexdisc_s			/* coder discipline		*/
{
	Codexnum_t	version;	/* CODEX_VERSION		*/
	Error_f		errorf;		/* error message function	*/
	ssize_t		(*passf)(void*, size_t, Codexdisc_t*, Codexmeth_t*);
	const char*	passphrase;	/* passphrase			*/
};

struct Codexmeth_s			/* coder method			*/
{
	const char*	name;		/* coder name			*/
	const char*	description;	/* coder description		*/
	const char*	options;	/* static optget() options	*/
	Codexnum_t	flags;		/* CODEX_* flags		*/

	int		(*optionsf)(Codexmeth_t*, Sfio_t*);
	int		(*identf)(Codexmeth_t*, const void*, size_t, char*, size_t);
	int		(*openf)(Codex_t*, char* const[], Codexnum_t);
	int		(*closef)(Codex_t*);
	int		(*initf)(Codex_t*);
	int		(*donef)(Codex_t*);
	ssize_t		(*readf)(Sfio_t*, void*, size_t, Sfdisc_t*);
	ssize_t		(*writef)(Sfio_t*, const void*, size_t, Sfdisc_t*);
	int		(*syncf)(Codex_t*);
	Sfoff_t		(*seekf)(Codex_t*, Sfoff_t, int);
	int		(*dataf)(Codex_t*, Codexdata_t*);

	void*		vcmeth;		/* vcodex(3) method		*/
	void*		data;		/* coder private data		*/
	Codexmeth_t*	next;		/* next in list of all coders	*/
};

#if _BLD_codex

#include <codexlib.h>

#define CODEXLIB(m)
#define CODEXNEXT(m)	m

#else

#ifdef __STDC__
#define CODEXLIB(m)	extern Codexmeth_t* codex_lib(const char* name) { return m; }
#else
#define CODEXLIB(m)	extern Codexmeth_t* codex_lib(name) char* name; { return m; }
#endif

#define CODEXNEXT(m)	0

#if defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern Codexmeth_t*	codex_lib(const char*);

#undef	extern

#endif

#define codexinit(d,e)	(memset(d,0,sizeof(*(d))),(d)->version=CODEX_VERSION,(d)->errorf=(Error_f)(e))

#if _BLD_codex && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern int		codex(Sfio_t*, Sfio_t*, const char*, Codexnum_t, Codexdisc_t*, Codexmeth_t*);
extern int		codexpop(Sfio_t*, Sfio_t*, int);
extern int		codexcmp(const char*, const char*);
extern Codex_t*		codexcopy(void);
extern int		codexdata(Sfio_t*, Codexdata_t*);
extern ssize_t		codexgetpass(const char*, void*, size_t);
extern Codexmeth_t*	codexid(const void*, size_t, char*, size_t);
extern Codexmeth_t*	codexlist(Codexmeth_t*);
extern Codexmeth_t*	codexmeth(const char*);
extern int		codexadd(const char*, Codexmeth_t*);
extern Sfio_t*		codexnull(void);
extern ssize_t		codexpass(void*, size_t, Codexdisc_t*, Codexmeth_t*);
extern int		codexsize(Sfio_t*, Sfoff_t);

#undef	extern

#endif
