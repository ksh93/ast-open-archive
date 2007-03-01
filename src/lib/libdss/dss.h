/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*           Copyright (c) 2002-2007 AT&T Knowledge Ventures            *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                      by AT&T Knowledge Ventures                      *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * data stream scan interface definitions
 */

#ifndef _DSS_H
#define _DSS_H		1

#define DSS_ID		"dss"
#define DSS_VERSION	CX_VERSION

#define Dssdisc_t	Cxdisc_t
#define Dsslocation_f	Cxlocation_f
#define Dsstype_t	Cxtype_t
#define Dssvalue_t	Cxvalue_t
#define Dssvariable_t	Cxvariable_t

#include <ast.h>
#include <cx.h>
#include <error.h>
#include <vmalloc.h>

#define DSS_DEBUG	CX_DEBUG		/* debug trace		*/
#define DSS_QUIET	CX_QUIET		/* no non-fatal messages*/
#define DSS_REGRESS	CX_REGRESS		/* regression output	*/
#define DSS_TRACE	CX_TRACE		/* algorithm trace	*/
#define DSS_VALIDATE	CX_VALIDATE		/* validate constraints	*/
#define DSS_VERBOSE	CX_VERBOSE		/* verbose feedback	*/

#define DSS_CX_FLAGS	(CX_FLAGS-1)

#define DSS_APPEND	(CX_FLAGS<<0)		/* DSS_FILE_APPEND	*/
#define DSS_FORCE	(CX_FLAGS<<1)		/* populate all fields	*/

#define DSS_METH_FLAGS	(CX_FLAGS<<2)		/* first method flag	*/
#define DSS_FLAGS	(DSS_METH_FLAGS<<6)	/* first caller flag	*/

#define DSS_FILE_READ	(1<<0)			/* read mode		*/
#define DSS_FILE_WRITE	(1<<1)			/* write mode		*/
#define DSS_FILE_APPEND	(1<<2)			/* append (no header)	*/
#define DSS_FILE_KEEP	(1<<3)			/* keep stream on close	*/
#define DSS_FILE_ERROR	(1<<4)			/* an error occurred	*/

struct Dss_s; typedef struct Dss_s Dss_t;
struct Dssfile_s; typedef struct Dssfile_s Dssfile_t;
struct Dssformat_s; typedef struct Dssformat_s Dssformat_t;
struct Dsslib_s; typedef struct Dsslib_s Dsslib_t;
struct Dssmeth_s; typedef struct Dssmeth_s Dssmeth_t;
struct Dssoptdisc_s; typedef struct Dssoptdisc_s Dssoptdisc_t;
struct Dssrecord_s; typedef struct Dssrecord_s Dssrecord_t;
struct Dssstate_s; typedef struct Dssstate_s Dssstate_t;

typedef uint32_t Dssflags_t;
typedef double Dssnumber_t;
typedef Cxexpr_t Dssexpr_t;

struct Dssoptdisc_s			/* dssopt*() optget() disc	*/
{
	Optdisc_t	optdisc;	/* optget disc			*/
	Dssdisc_t*	disc;		/* dss disc			*/
	Cxheader_t*	header;		/* current item header		*/
};

struct Dssformat_s			/* file format method		*/
{
	_CX_NAME_HEADER_
	int		(*identf)(Dssfile_t*, void*, size_t, Dssdisc_t*);
	int		(*openf)(Dssfile_t*, Dssdisc_t*);
	int		(*readf)(Dssfile_t*, Dssrecord_t*, Dssdisc_t*);
	int		(*writef)(Dssfile_t*, Dssrecord_t*, Dssdisc_t*);
	Sfoff_t		(*seekf)(Dssfile_t*, Sfoff_t, Dssdisc_t*);
	int		(*closef)(Dssfile_t*, Dssdisc_t*);
	Dssrecord_t*	(*savef)(Dssfile_t*, Dssrecord_t*, Dssdisc_t*);
	int		(*dropf)(Dssfile_t*, Dssrecord_t*, Dssdisc_t*);
#ifdef _DSS_FORMAT_PRIVATE_
	_DSS_FORMAT_PRIVATE_
#endif
};

struct Dssrecord_s			/* record info			*/
{
	Dssfile_t*	file;		/* file handle if not cloned	*/
	void*		data;		/* actual data			*/
	size_t		size;		/* total data size		*/
	Dssflags_t	flags;		/* DSS_RECORD_* flags		*/
};

struct Dssfile_s			/* typed file handle		*/
{
	Sfoff_t		offset;		/* current record offset	*/
	size_t		count;		/* current record count		*/
	Dss_t*		dss;		/* dss handle			*/
	Dssflags_t	flags;		/* DSS_FILE_* flags		*/
	Sfio_t*		io;		/* io stream			*/
	char*		path;		/* original path		*/
	Dssformat_t*	format;		/* file format			*/
#ifdef _DSS_FILE_PRIVATE_
	_DSS_FILE_PRIVATE_
#endif
};

struct Dsslib_s				/* dss_lib() library info	*/
{
	_CX_NAME_HEADER_
	const char**	libraries;	/* library list			*/
	Dssmeth_t*	meth;		/* method			*/
	Cxtype_t*	types;		/* type table			*/
	Cxcallout_t*	callouts;	/* callout table		*/
	Cxrecode_t*	recodes;	/* recode table			*/
	Cxmap_t**	maps;		/* map table			*/
	Cxquery_t*	queries;	/* query table			*/
	Cxconstraint_t*	constraints;	/* constraint table		*/
	Cxedit_t*	edits;		/* edit table			*/

	void*		pad[8];		/* pad for future expansion	*/

	/* the remaining are set by dsslib()				*/

	const char*	path;		/* library path name		*/
};

struct Dssmeth_s			/* method			*/
{
	_CX_NAME_HEADER_
	Dssmeth_t*	(*methf)(const char*, const char*, const char*, Dssdisc_t*, Dssmeth_t*);
	int		(*openf)(Dss_t*, Dssdisc_t*);
	int		(*closef)(Dss_t*, Dssdisc_t*);
	const char*	compress;	/* preferred compression	*/
	const char*	print;		/* default {print} format	*/
	Cx_t*		cx;		/* expression handle		*/
	Dt_t*		formats;	/* method format table		*/
#ifdef _DSS_METH_PRIVATE_
	_DSS_METH_PRIVATE_
#endif
};

struct Dssstate_s			/* global state			*/
{
	Dss_t*		dss;		/* most recent dssopen()	*/
	Dssmeth_t*	meth;		/* most recent dssmeth()	*/
	Cxstate_t*	cx;		/* cx global state		*/
#ifdef _DSS_STATE_PRIVATE_
	_DSS_STATE_PRIVATE_
#endif
};

struct Dss_s				/* dss handle			*/
{
	const char*	id;		/* interface id			*/
	Dssmeth_t*	meth;		/* method			*/
	Dssdisc_t*	disc;		/* discipline			*/
	Vmalloc_t*	vm;		/* handle vm			*/
	Dssflags_t	flags;		/* dssopen() flags		*/
	Dssflags_t	test;		/* dssopen() test mask		*/
	Cx_t*		cx;		/* expression handle		*/
	void*		data;		/* method data			*/
	Dssformat_t*	format;		/* first input file format	*/
	Dssstate_t*	state;		/* global state			*/
#ifdef _DSS_PRIVATE_
	_DSS_PRIVATE_
#endif
};

#define dssinit(d,e)	(memset(d,0,sizeof(*(d))),(d)->version=DSS_VERSION,(d)->errorf=(Error_f)(e),dssstate(d))
#define dssformat(n,d,m) ((Dssformat_t*)dtmatch((m)->formats,(char*)(n)))

#if DSS_STATIC || _BLD_STATIC || _BLD_STATIC_LINK
#define dssstatic(l,d)	{ extern Dsslib_t dss_lib_##l; dssadd(&dss_lib_##l,d); }
#else
#define dssstatic(l,d)
#endif

#define DSS(c)		((Dss_t*)(c)->caller)
#define DSSRECORD(v)	((Dssrecord_t*)(v))
#define DSSDATA(v)	(DSSRECORD(v)->data)

#if defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern Dsslib_t*	dss_lib(const char*, Dssdisc_t*);

#undef	extern

#if _BLD_dss && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern Dssstate_t*	dssstate(Dssdisc_t*);
extern Dsslib_t*	dsslib(const char*, Dssflags_t, Dssdisc_t*);
extern int		dssload(const char*, Dssdisc_t*);
extern int		dssadd(Dsslib_t*, Dssdisc_t*);

extern Dssmeth_t*	dssmeth(const char*, Dssdisc_t*);
extern Dssmeth_t*	dsstags(Sfio_t*, const char*, int, Dssflags_t, Dssdisc_t*, Dssmeth_t*);

extern Sfio_t*		dssfind(const char*, const char*, Dssflags_t, char*, size_t, Dssdisc_t*);

extern int		dssoptinfo(Opt_t*, Sfio_t*, const char*, Optdisc_t*);
extern int		dssoptlib(Sfio_t*, Dsslib_t*, Dssdisc_t*);

extern Dss_t*		dssopen(Dssflags_t, Dssflags_t, Dssdisc_t*, Dssmeth_t*);
extern int		dssrun(Dss_t*, const char*, const char*, const char*, char**);
extern int		dssclose(Dss_t*);

extern Dssfile_t*	dssfopen(Dss_t*, const char*, Sfio_t*, Dssflags_t, Dssformat_t*);
extern Dssrecord_t*	dssfread(Dssfile_t*);
extern int		dssfwrite(Dssfile_t*, Dssrecord_t*);
extern Sfoff_t		dssftell(Dssfile_t*);
extern int		dssfseek(Dssfile_t*, Sfoff_t);
extern int		dssfclose(Dssfile_t*);
extern Dssexpr_t*	dsscomp(Dss_t*, const char*, Sfio_t*);
extern int		dssbeg(Dss_t*, Dssexpr_t*);
extern int		dsseval(Dss_t*, Dssexpr_t*, Dssrecord_t*);
extern int		dssend(Dss_t*, Dssexpr_t*);
extern int		dsslist(Dss_t*, Dssexpr_t*, Sfio_t*);
extern int		dssfree(Dss_t*, Dssexpr_t*);
extern ssize_t		dssmagic(Dss_t*, Sfio_t*, const char*, const char*, uint32_t, size_t);
extern int		dssprintf(Dss_t*, Sfio_t*, const char*, Dssrecord_t*);

extern int		dssget(Dssrecord_t*, Dssvariable_t*, Dsstype_t*, const char*, Dssvalue_t*);
extern Dsstype_t*	dsstype(Dss_t*, const char*);
extern Dssvariable_t*	dssvariable(Dss_t*, const char*);

extern Dssrecord_t*	dsssave(Dssrecord_t*);
extern int		dssdrop(Dssrecord_t*);

#undef	extern

#endif
