/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1996-2007 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * rskey internal interface
 */

#ifndef _RSKEYHDR_H
#define _RSKEYHDR_H	1

#if _PACKAGE_ast
#include <ast.h>
#endif

#include <ctype.h>
#include <ccode.h>

#ifndef UCHAR_MAX
#define UCHAR_MAX	((unsigned char)(~0))
#endif
#ifndef SHRT_MAX
#define SHRT_MAX	((short)(~((unsigned short)0)) >> 1)
#endif
#ifndef INT_MAX
#define INT_MAX		((int)(~((unsigned int)0)) >> 1)
#endif
#ifndef LONG_MAX
#define LONG_MAX	((long)(~((unsigned long)0)) >> 1)
#endif

#define INSIZE		PROCSIZE	/* default insize		*/
#define OUTSIZE		(64*1024)	/* default outsize		*/
#define PROCSIZE	(4*1024*1024)	/* default procsize		*/

#define MAXFIELD	INT_MAX

#define blank(c)	((c)==' '||(c)=='\t')

struct Field_s;
struct _rskey_s;

typedef int (*Coder_t)(struct _rskey_s*, struct Field_s*, unsigned char*, unsigned char*, int);

typedef struct				/* field position		*/
{
	int		field;		/* field offset			*/
	int		index;		/* char offset			*/
} Position_t;

typedef struct Field_s			/* key field			*/
{
	struct Field_s*	next;		/* next in list			*/
	Coder_t		coder;		/* encode data into key		*/
	int		binary;		/* binary data			*/
	int		code;		/* coder ccode or conversion	*/
	int		index;		/* field definition index	*/
	int		flag;		/* code flag			*/
	int		freetrans;	/* free trans on close		*/
	unsigned char*	trans;		/* translation table		*/
	unsigned char*	keep;		/* deletion table		*/
	Position_t	begin;		/* key begins here		*/
	Position_t	end;		/* and ends here		*/
	unsigned char	aflag;		/* accumulate dups here		*/
	unsigned char	bflag;		/* skip initial blanks		*/
	unsigned char	eflag;		/* skip trailing blanks		*/
	unsigned char	rflag;		/* reverse order		*/
	unsigned char	standard;	/* 1:-k 0:+pos-pos		*/
} Field_t;

typedef struct
{
	unsigned char	ident[UCHAR_MAX + 1];	/* identity transform	*/
	unsigned char	fold[UCHAR_MAX + 1];	/* fold case		*/

	unsigned char	all[UCHAR_MAX + 1];	/* all significant	*/
	unsigned char	dict[UCHAR_MAX + 1];	/* dictionary order	*/
	unsigned char	print[UCHAR_MAX + 1];	/* printable significant*/
} State_t;

#define _RSKEY_PRIVATE_ \
	State_t*	state;		/* readonly state		*/ \
	Rsdisc_t*	tail;		/* rslib() disc stack tail	*/ \
	struct								   \
	{								   \
	Field_t		global;		/* global field info		*/ \
	Field_t*	head;		/* field list head		*/ \
	Field_t*	tail;		/* field list tail		*/ \
	Field_t*	prev;		/* previous field list tail	*/ \
	int		index;		/* last field index		*/ \
	int		maxfield;	/* max field position		*/ \
	unsigned char**	positions;	/* field start positions	*/ \
	}		field;		/* key field info		*/ \
	struct								   \
	{								   \
	Field_t*	head;		/* accumulate list head		*/ \
	Field_t*	tail;		/* accumulate list tail		*/ \
	}		accumulate;	/* accumulate field info	*/ \
	unsigned char*	xfrmbuf;	/* strxfrm buffer		*/ \
	unsigned int	xfrmsiz;	/* strxfrm buffer size		*/ \
	unsigned long	shuffle;	/* shuffle seed			*/ \
	unsigned char	coded;		/* coded keys specified		*/

#include "rshdr.h"

#if !__STD_C && !defined(const)
#define const
#endif

#if !_PACKAGE_ast
#if __STD_C
#include <string.h>
#endif
#define elementsof(x)	(sizeof(x)/sizeof(x[0]))
#define roundof(x,y)	(((x)+(y)-1)&~((y)-1))
#define streq(a,b)	(*(a)==*(b)&&!strcmp(a,b))
#endif

#endif
