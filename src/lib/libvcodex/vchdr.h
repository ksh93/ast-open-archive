/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 2003-2006 AT&T Corp.                  *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                            by AT&T Corp.                             *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#ifndef _VCHDR_H
#define _VCHDR_H	1

#if _PACKAGE_ast
#include	<ast.h>
#else
#include	<ast_common.h>
#endif

#define ISPUNCT(x)	(ISBLANK(x) || (x) == ',' || (x) == '^')
#define ISWINSEP(x)	((x) == ',' || (x) == '.')
#define ISEOS(x)	((x) == 0 || (x) == ':')
#define ISBLANK(x)	((x) == ' ' || (x) == '\t')
#define ISDIGIT(x)	((x) >= '0' && (x) <= '9')

#include	<cdt.h>

typedef struct _vcbuf_s
{	struct _vcbuf_s*	next;
} Vcbuf_t;

#define _VCODEX_PRIVATE \
	int		ctop;	/* current top context	*/ \
	int		cnow;	/* current context	*/ \
	Void_t*		ctxt;	/* context data		*/ \
	ssize_t		csize;	/* context size		*/ \
	ssize_t		head;	/* required head room	*/ \
	Vcchar_t*	buf;	/* active output buffer	*/ \
	ssize_t		bsize;	/* size of above	*/ \
	ssize_t		bhead;	/* frozen head room	*/ \
	Vcbuf_t*	blist;	/* busy inactive bufs	*/ \
	Void_t*		mtdata;	/* method data		*/ 

#include	"vcodex.h"

#if __STD_C
#include	<stdarg.h>
#else
#include	<varargs.h>
#endif

#ifdef VMFL
#include	<vmalloc.h>
#endif

#ifdef DEBUG
_BEGIN_EXTERNS_
extern void	abort _ARG_((void));
_END_EXTERNS_
#ifndef __LINE__
#define __LINE__	0
#endif
#ifndef __FILE__
#define __FILE__	"Unknown"
#endif
static void _oops(char* file, int line)
{
	char buf[1024];
	sprintf(buf, "\nFailed at %s:%d\n", file, line);
	write(2,buf,strlen(buf));
	abort();
}

#include <sys/times.h>
#include <sys/resource.h>
static double _getTime ( void )
{	double		tm;
	struct rusage	u;
	getrusage ( RUSAGE_SELF, &u );
	tm = (double)u.ru_utime.tv_sec  + (double)u.ru_utime.tv_usec/1000000.0;
	return tm;
}

static double		_Vcdbtime;
#define BEGTIME()	(_Vcdbtime = _getTime())
#define ENDTIME()	(_getTime() - _Vcdbtime)
#define ASSERT(p)	((p) ? 0 : (_oops(__FILE__, __LINE__),0))
#define COUNT(n)	((n) += 1)
#define TALLY(c,n,v)	((c) ? ((n) += (v)) : (n))
#define DECLARE(t,v)	t v
#define SET(n,v)	((n) = (v))
#define PRINT(fd,s,v)	{char b[1024];sprintf(b,s,v);write((fd),b,strlen(b));}
#define WRITE(fd,d,n)	write((fd),(d),(n))
#define KPV(temp)	(temp) /* debugging stuff that should be removed */
#define RETURN(x)	(_oops(__FILE__, __LINE__), (x))
#define BREAK		(_oops(__FILE__, __LINE__))
#else
#define BEGTIME()
#define ENDTIME()
#define ASSERT(p)
#define COUNT(n)
#define TALLY(c,n,v)
#define DECLARE(t,v)	extern int _stub_	/* some compilers must have non empty decl */
#define SET(n,v)
#define PRINT(fd,s,v)
#define WRITE(fd,d,n)
#define KPV(x)
#define RETURN(x)	return(x)
#define BREAK		break
#endif /*DEBUG*/

#ifndef NIL
#define NIL(type)	((type)0)
#endif

#ifndef reg
#define reg		register
#endif

#define VC_LARGE	((ssize_t)((~((size_t)0)) >> 1) )

typedef unsigned _ast_int4_t	Vchash_t;		/* 4-byte hash values	*/
#define VCHASH(ky)	(((ky)>>13)^0x1add2b3^(ky) )	/* 1add2b3 is a prime!	*/
#define	VCINDEX(ky,msk)	(VCHASH(ky) & (msk) ) 		/* get index % (msk+1)	*/

/* hd: amount of extra space for header.
** dt,sz: data to be transformed.
*/
#define VCCODER(vc, coder, hd, dt, sz) \
	( ((coder)->head += (vc)->head + (hd) ), \
	  ((sz) = vcapply((coder), (dt), (sz), &(dt)) ), \
	  ((coder)->head -= (vc)->head + (hd) ), \
	  (sz) \
	)

/* dealing with compression contexts */
typedef struct _vcctxt_s /* default context structure	*/
{	int	ctxt;	/* context # of secondary coder */
} Vcctxt_t;
#define VCINITCTXT(vc, tp)	((vc)->csize = sizeof(tp) )
#define VCGETCTXT(vc, tp)	((tp)((Vcchar_t*)(vc)->ctxt + (vc)->cnow*(vc)->csize) )
#define VCSETCTXT(vc, ct)	((ct) = vccontext((vc), (ct)) ) 

/* dealing with private method data */
#define VCGETMETH(vc, tp)	((tp)(vc)->mtdata)
#define VCSETMETH(vc, mt)	((vc)->mtdata = (Void_t*)(mt))

/* dealing with disciplines */
#define VCGETDISC(vc)		((vc)->disc)
#define VCSETDISC(vc, dc)	((vc)->disc = (Vcdisc_t*)(dc))

/* functions to tell the coding size of an integer */
#define VCSIZEU(v)	((v) < (1<<7) ? 1 : (v) < (1<<14) ? 2 : (v) < (1<<21) ? 3 : 4)
#define VCSIZEM(v)	((v) < (1<<8) ? 1 : (v) < (1<<16) ? 2 : (v) < (1<<24) ? 3 : 4)

#define RL_ESC		255	/* default escape character	*/
#define RL_ZERO		254	/* (0,RL_ZERO) codes 0-runs	*/
#define RLLITERAL(c)	((c) >= RL_ZERO)

_BEGIN_EXTERNS_

extern ssize_t		_vcrle2coder _ARG_((Vcodex_t*, ssize_t,
					    Vcchar_t*, ssize_t,
					    Vcchar_t*, ssize_t,
					    Vcchar_t**, ssize_t));

extern Void_t*		memcpy _ARG_((Void_t*, const Void_t*, size_t));
extern Void_t*		malloc _ARG_((size_t));
extern Void_t*		realloc _ARG_((Void_t*, size_t));
extern Void_t*		calloc _ARG_((size_t, size_t));
extern void		free _ARG_((Void_t*));
_END_EXTERNS_

#endif /*_VCHDR_H*/
