/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1996-2002 AT&T Corp.                *
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
*                 Phong Vo <kpv@research.att.com>                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
#ifndef _RECSORT_H
#define	_RECSORT_H		1

#define RS_VERSION	19961031L
#define RSKEY_VERSION	19961031L

#include	<sfio.h>

typedef struct _rsobj_s		Rsobj_t;
typedef struct _rs_s		Rs_t;
typedef struct _rsmethod_s	Rsmethod_t;
typedef struct _rsdisc_s	Rsdisc_t;
typedef int 			(*Rsdefkey_f)
				  _ARG_((Rs_t*, unsigned char*, int,
					 unsigned char*, int, Rsdisc_t*));
typedef int			(*Rsevent_f)_ARG_((Rs_t*, int, Void_t*, Rsdisc_t*));

typedef struct _rskey_s		Rskey_t;
typedef struct _rskeydisc_s	Rskeydisc_t;
typedef int			(*Rskeyerror_f)
				  _ARG_((Void_t*, Void_t*, int, ...));

struct _rsmethod_s
{	int		(*insertf)_ARG_((Rs_t*, Rsobj_t*));
	Rsobj_t*	(*listf)_ARG_((Rs_t*));
	ssize_t		size;	/* size of private data			*/
	int		type;	/* method type				*/
	char*		name;	/* method name				*/
	char*		desc;	/* method description			*/
};

struct _rsdisc_s
{	unsigned long	version;/* interface version			*/
	int		type;	/* types of key&data			*/
	ssize_t		data;	/* length or separator			*/
	ssize_t		key;	/* key offset or expansion factor	*/
	ssize_t		keylen;	/* >0 for key length else end-offset	*/
	Rsdefkey_f	defkeyf;/* to define key from data		*/
	Rsevent_f	eventf;	/* to announce various events		*/
};

struct _rsobj_s
{	unsigned long	order;	/* for fast compare or ordinal 		*/
	Rsobj_t*	left;	/* left/last link or out of order	*/
	Rsobj_t*	right;	/* next record in sorted list		*/
	Rsobj_t*	equal;	/* equivalence class			*/
	unsigned char*	key;	/* object key				*/
	ssize_t		keylen;	/* key length				*/
	unsigned char*	data;	/* object data				*/
	ssize_t		datalen;/* data length				*/
};

struct _rskeydisc_s
{	unsigned long	version;	/* interface version		*/
	unsigned long	flags;		/* RSKEY_* flags		*/
	Rskeyerror_f	errorf;		/* error function		*/
};

struct _rskey_s
{	const char*	id;		/* library id			*/
	Rskeydisc_t*	keydisc;	/* rskey discipline		*/
	Rsdisc_t	disc;		/* rsopen() discipline		*/
	Rsmethod_t*	meth;		/* rsopen() method		*/
	int		type;		/* rsopen() type		*/

	char**		input;		/* input files			*/

	char*		output;		/* output file name		*/

	size_t		alignsize;	/* buffer alignment size	*/
	size_t		fixed;		/* fixed record size		*/
	size_t		insize;		/* input buffer size		*/
	size_t		outsize;	/* output buffer size		*/
	size_t		procsize;	/* process buffer size		*/
	size_t		recsize;	/* max record size		*/
	unsigned long	test;		/* test mask			*/

	int		merge;		/* merge sorted input files	*/
	int		nproc;		/* max number of processes	*/
	int		tab;		/* global tab char		*/
	int		verbose;	/* trace execution		*/
#ifdef _RSKEY_PRIVATE_
	_RSKEY_PRIVATE_
#endif
};

struct _rs_s
{	Rsmethod_t*	meth;	/* method to sort			*/
	Rsdisc_t*	disc;	/* discipline describing data		*/
	int		count;	/* number of accumulated objects	*/
	int		type;
#ifdef _RS_PRIVATE_
	_RS_PRIVATE_
#endif
};

/* events */
#define RS_CLOSE	1		/* sort context is being closed	*/
#define RS_DISC		2		/* discipline is being changed	*/
#define RS_METHOD	3		/* method is being changed	*/
#define RS_VERIFY	4		/* objects out of order		*/
#define RS_SUMMARY	5		/* outputting in RS_UNIQ mode	*/

/* sort controls */
#define RS_UNIQ		000001		/* remove duplicates		*/
#define RS_REVERSE	000002		/* reverse sort order		*/
#define RS_DATA		000004		/* sort by key, then by data	*/

/* discipline data */
#define RS_KSAMELEN	000010		/* key has fixed length		*/
#define RS_DSAMELEN	000020		/* data has fixed length	*/
#define RS_TYPES	000037

/* input/output control */
#define RS_ITEXT	000100		/* input is plain text		*/
#define RS_OTEXT	000200		/* output is plain text		*/
#define RS_TEXT		000300

/* method type */
#define RS_MTVERIFY	000400
#define RS_MTRASP	001000
#define RS_MTRADIX	002000
#define RS_MTSPLAY	004000

#define RSKEY_ERROR	000001		/* unrecoverable error		*/

#define RSKEYDISC(p)	(((Rskey_t*)((char*)(p)-offsetof(Rskey_t,disc)))->keydisc)

#define rscount(rs)	((rs)->count)	/* count # of rsprocess() objs	*/

_BEGIN_EXTERNS_	/* public data */
#if _BLD_recsort && defined(__EXPORT__)
#define extern	__EXPORT__
#endif
#if !_BLD_recsort && defined(__IMPORT__)
#define extern	__IMPORT__
#endif

extern Rsmethod_t*	Rsrasp;		/* radix + splay trees		*/
extern Rsmethod_t*	Rsradix;	/* radix only			*/
extern Rsmethod_t*	Rssplay;	/* splay insertion		*/
extern Rsmethod_t*	Rsverify;	/* check for correct order	*/

#undef extern
_END_EXTERNS_

_BEGIN_EXTERNS_	/* public functions */
#if _BLD_recsort && defined(__EXPORT__)
#define extern	__EXPORT__
#endif

extern Rs_t*		rsopen _ARG_((Rsdisc_t*, Rsmethod_t*, ssize_t, int));
extern int		rsclear _ARG_((Rs_t*));
extern int		rsclose _ARG_((Rs_t*));
extern ssize_t		rsprocess _ARG_((Rs_t*, Void_t*, ssize_t));
extern Rsobj_t*		rslist _ARG_((Rs_t*));
extern int		rswrite _ARG_((Rs_t*, Sfio_t*, int));
extern int		rsmerge _ARG_((Rs_t*, Sfio_t*, Sfio_t**, int, int));
extern Rsdisc_t*	rsdisc _ARG_((Rs_t*, Rsdisc_t*));
extern Rsmethod_t*	rsmethod _ARG_((Rs_t*, Rsmethod_t*));

extern Rskey_t*		rskeyopen _ARG_((Rskeydisc_t*));
extern int		rskey _ARG_((Rskey_t*, const char*, int));
extern int		rskeyopt _ARG_((Rskey_t*, const char*, int));
extern Rsmethod_t*	rskeymeth _ARG_((Rskey_t*, const char*));
extern int		rskeylist _ARG_((Rskey_t*, Sfio_t*, int));
extern void		rskeydump _ARG_((Rskey_t*, Sfio_t*));
extern int		rskeyinit _ARG_((Rskey_t*));
extern int		rskeyclose _ARG_((Rskey_t*));

#undef extern
_END_EXTERNS_

#endif /*_RECSORT_H*/
