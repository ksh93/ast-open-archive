/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1997-2000 AT&T Corp.              *
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
#pragma prototyped

/*
 * Glenn Fowler
 * AT&T Research
 *
 * cql db format support library private interface
 */

#ifndef _CDBLIB_H
#define _CDBLIB_H

#include <ast.h>
#include <ctype.h>
#include <error.h>
#include <debug.h>
#include <namval.h>
#include <sfstr.h>

/*
 * private library global names
 */

#define cdbdata		_cdb_data
#define cdbexternal	_cdb_external
#define cdbnospace	_cdb_nospace
#define cdbparse	_cdb_parse

#define state		_cdb_state

#define	PANIC	(ERROR_PANIC|ERROR_SOURCE|ERROR_SYSTEM),__FILE__,__LINE__

#define SETFLOATING(dp,fp,iv)	((((fp)->flags&CDB_INVALID)&&(iv)||((fp)->flags&CDB_NEGATIVE)&&(dp)->number.floating<0.0||((fp)->flags&CDB_ZERO)&&(dp)->number.floating==0.0)?0:((dp)->flags|=CDB_FLOATING))
#define SETINTEGER(dp,fp,iv)	((((fp)->flags&CDB_INVALID)&&(iv)||((fp)->flags&CDB_NEGATIVE)&&(dp)->number.integer<0||((fp)->flags&CDB_ZERO)&&(dp)->number.integer==0)?0:((dp)->flags|=CDB_INTEGER))
#define SETUINTEGER(dp,fp,iv)	((((fp)->flags&CDB_INVALID)&&(iv)||((fp)->flags&CDB_ZERO)&&(dp)->number.uinteger==0)?0:((dp)->flags|=CDB_UNSIGNED|CDB_INTEGER))
#define SETLINTEGER(dp,fp,iv)	((((fp)->flags&CDB_INVALID)&&(iv)||((fp)->flags&CDB_NEGATIVE)&&(dp)->number.linteger<0||((fp)->flags&CDB_ZERO)&&(dp)->number.linteger==0)?0:((dp)->flags|=CDB_LONG|CDB_INTEGER))
#define SETWINTEGER(dp,fp,iv)	((((fp)->flags&CDB_INVALID)&&(iv)||((fp)->flags&CDB_ZERO)&&(dp)->number.winteger==0)?0:((dp)->flags|=CDB_UNSIGNED|CDB_LONG|CDB_INTEGER))

#define strsame(a,b)	((a)==(b)||(a)&&(b)&&streq(a,b))

struct Cdbdll_s; typedef struct Cdbdll_s Cdbdll_t;
struct Cdbconvert_s; typedef struct Cdbconvert_s Cdbconvert_t;

typedef struct				/* schema map			*/
{
	unsigned long	offset;		/* used by cdbmap()		*/
	int		ifields;	/* # input fields		*/
	int		ofields;	/* # output fields		*/
	Cdbconvert_t*	convert;	/* field data conversion info	*/
} Cdbmapschema_t;

#define _CDB_MAP_PRIVATE_ \
	Cdbmapschema_t	schema[1];	/* map for each schema type	*/

#define _CDB_RECORD_PRIVATE_ \
	struct								   \
	{								   \
	Cdbrecord_t*	next;		/* next in cache chain		*/ \
	Cdbrecord_t*	prev;		/* previous in cache chain	*/ \
	void*		image;		/* cdbimage() state		*/ \
	}		cache;		/* cdbcache() pointers		*/

#define _CDB_SCHEMA_PRIVATE_ \
	Cdbschema_t*	head;		/* cdbparse() partition chain	*/ \
	Cdbschema_t*	tail;		/* cdbparse() partition chain	*/ \
	int		code;		/* last ccode (for terminator)	*/ \
	int		delimterm;	/* >=1 delimiter==terminator	*/ \
	int		sep;		/* cdbparse() partition sep chr	*/ \
	int		strings;	/* highest referenced string+1	*/ \
	int		referenced;	/* highest referenced field+1	*/ \
	int		variable;	/* records are variable length	*/

#define _CDB_PRIVATE_ \
	Cdbmeth_t*	methods;	/* original cdbopen() methods	*/ \
	Cdbrecord_t*	cache;		/* cached records		*/ \
	Cdbrecord_t*	free;		/* free records			*/ \
	Cdbdisc_t*	odisc;		/* original discipline pointer	*/ \
	char*		label;		/* optional schema label	*/ \
	char*		identify;	/* record type id function	*/ \
	Sfio_t*		buf;		/* tmp buffer string stream	*/ \
	Sfio_t*		cvt;		/* tmp conversion string stream	*/ \
	Sfio_t*		txt;		/* fixed schema directive text	*/ \
	int		clear;		/* cdbread() vmclear() ok	*/ \
	int		closeio;	/* cdbclose() to close io	*/ \
	int		common;		/* # common fields		*/ \
	int		strings;	/* max referenced string+1	*/

#include <cdbmethod.h>

typedef int (*Cdbconvert_f)(Cdbmap_t*, Cdbconvert_t*);

struct Cdbdll_s				/* loaded dlls			*/
{
	Cdbdll_t*	next;		/* next in list			*/
	char*		name;		/* name				*/
	void*		dll;		/* handle			*/
};

struct Cdbconvert_s			/* field conversion info	*/
{
	Cdbconvert_f	convertf;	/* conversion function		*/
	struct
	{
	int		index;		/* field index			*/
	Cdbdata_t*	data;		/* field data			*/
	Cdbformat_t*	format;		/* field format			*/
	}		input, output;	/* input/output field info	*/
};

typedef struct				/* private library state	*/
{
	const char*	id;		/* library id			*/
	Cdbdll_t*	dll;		/* loaded dlls			*/
	Cdbmapmeth_t*	maps;		/* map method list		*/
	Cdbmeth_t*	methods;	/* format method list		*/
	Cdbtype_t*	types;		/* external type method list	*/
} Cdbstate_t;

extern Cdbmeth_t	_Cdb;		/* cdb format			*/
extern Cdbmeth_t	_Cdbflat;	/* flat file format		*/
extern Cdbmeth_t	_Cdbgzip;	/* gzip compression method	*/
extern Cdbmeth_t	_Cdbpzip;	/* pzip compression method	*/
extern Cdbmeth_t	_Cdbvdelta;	/* vdelta compression method	*/

extern Cdbstate_t	state;
extern Cdbtype_t	cdbexternal[];

extern int		cdbdata(Cdb_t*);
extern int		cdbnospace(Cdb_t*);
extern int		cdbparse(Cdb_t*, const char*);

#endif
