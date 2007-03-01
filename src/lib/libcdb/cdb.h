/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*           Copyright (c) 1997-2007 AT&T Knowledge Ventures            *
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
 * Glenn Fowler
 * AT&T Research
 *
 * cql db format interface definitions
 */

#ifndef _CDB_H
#define _CDB_H

#define CDB_VERSION	19980811L	/* interface version		*/

#include <ast.h>
#include <ccode.h>
#include <vmalloc.h>

#define CDB_MAJOR	2		/* data major version		*/
#define CDB_MINOR	0		/* data minor version		*/

#define CDB_CCODE	CC_ASCII	/* default ccode		*/
#define CDB_DELIMITER	":"		/* default delimiter		*/
#define CDB_TERMINATOR	"\n"		/* default terminator		*/

/*
 * Cdbdata_t Cdbformat_t Cdbmeth_t -- type flags
 *
 * NOTE: Cdbformat_t forces some bits to be in 0xff
 */

#define CDB_IGNORE	0x0000		/* ignore field			*/
#define CDB_STRING	0x0001		/* string field			*/
#define CDB_INTEGER	0x0002		/* integer field		*/
#define CDB_FLOATING	0x0004		/* floating point field		*/
#define CDB_PHYSICAL	0x0008		/* physical type modifier	*/

#define CDB_LONG	0x0010		/* long integer			*/
#define CDB_UNSIGNED	0x0020		/* unsigned integer		*/

#define CDB_BINARY	0x0040		/* binary field, no ccode	*/
#define CDB_VIRTUAL	0x0080		/* virtual field		*/

#define CDB_INVALID	0x0100		/* absent if invalid		*/
#define CDB_NEGATIVE	0x0200		/* absent if < 0		*/
#define CDB_SPACE	0x0400		/* absent if space or nul	*/
#define CDB_ZERO	0x0800		/* absent if == 0		*/

#define CDB_CACHED	0x1000		/* string data cached		*/
#define CDB_INITIALIZED	0x2000		/* initialization complete	*/
#define CDB_STACK	0x4000		/* stackable method		*/
#define CDB_TERMINATED	0x8000		/* string data 0-terminated	*/

#define CDB_OPT		(CDB_INVALID|CDB_NEGATIVE|CDB_SPACE|CDB_ZERO)
#define CDB_OPT_DEFAULT	(CDB_INVALID|CDB_SPACE|CDB_ZERO)

/*
 * cdbopen() cdbdump() cdbmap() -- flags
 */

#define CDB_READ	0x0001		/* open for read		*/
#define CDB_WRITE	0x0002		/* open for write		*/
#define CDB_HEADER	0x0004		/* optional data header		*/
#define CDB_PACK	0x0008		/* packed internal data		*/
#define CDB_RAW		0x0010		/* don't partition		*/

#define CDB_DUMP	0x0020		/* dump at cdbread()/cdbwrite()	*/
#define CDB_DUMPED	0x0040		/* cdb header already dumped	*/
#define CDB_TEST1	0x0080		/* internal test #1		*/
#define CDB_TEST2	0x0100		/* internal test #2		*/
#define CDB_VERBOSE	0x0200		/* really verbose dump		*/

/* CDB_TERMINATED used here too */

#define CDB_MAP_IGNORE	0x0001		/* map ignore fields		*/
#define CDB_MAP_PHYSICAL 0x002		/* map physical fields		*/
#define CDB_MAP_VIRTUAL	0x0004		/* map virtual fields		*/

#define CDB_MAP_DELIMIT	0x0010		/* map to delimited fields	*/
#define CDB_MAP_NATIVE	0x0020		/* map to CC_NATIVE		*/
#define CDB_MAP_SIZED	0x0040		/* map to sized records		*/
#define CDB_MAP_TYPES	0x0080		/* map external types		*/

#define CDB_MAP_NIL	(-1)		/* map the nil field		*/

#define CDB_MAP_DEFAULT	(CDB_MAP_IGNORE|CDB_MAP_PHYSICAL|CDB_MAP_TYPES|CDB_MAP_VIRTUAL)

/*
 * Cdbevent_f method -- events
 *
 * NOTE: CDB_RExxx = CDB_xxx + 1
 */

#define CDB_METH	0		/* method unknown/changed event	*/
#define CDB_INIT	1		/* init event			*/
#define CDB_REINIT	2		/* method reopen init event	*/
#define CDB_OPEN	3		/* open event			*/
#define CDB_REOPEN	4		/* reopen event			*/
#define CDB_PUSH	5		/* CDB_STACK push event		*/
#define CDB_CLOSE	6		/* close event			*/
#define CDB_RECLOSE	7		/* method reopen close event	*/

#define Cdbint_t	int32_t
#define Cdbuint_t	uint32_t

struct Cdb_s; typedef struct Cdb_s Cdb_t;
struct Cdbdata_s; typedef struct Cdbdata_s Cdbdata_t;
struct Cdbdisc_s; typedef struct Cdbdisc_s Cdbdisc_t;
struct Cdbformat_s; typedef struct Cdbformat_s Cdbformat_t;
struct Cdbkey_s; typedef struct Cdbkey_s Cdbkey_t;
struct Cdbmap_s; typedef struct Cdbmap_s Cdbmap_t;
struct Cdbmapmeth_s; typedef struct Cdbmapmeth_s Cdbmapmeth_t;
struct Cdbmeth_s; typedef struct Cdbmeth_s Cdbmeth_t;
struct Cdbrecord_s; typedef struct Cdbrecord_s Cdbrecord_t;
struct Cdbschema_s; typedef struct Cdbschema_s Cdbschema_t;
struct Cdbtype_s; typedef struct Cdbtype_s Cdbtype_t;

typedef int (*Cdbevent_f)(Cdb_t*, int, void*, Cdbdisc_t*);
typedef int (*Cdbindex_f)(Cdb_t*, Cdbkey_t*, Cdbdisc_t*);

typedef int (*Cdbinit_f)(Cdb_t*, Cdbtype_t*);
typedef int (*Cdbinternal_f)(Cdb_t*, Cdbformat_t*, Cdbdata_t*, const char*, size_t, Cdbtype_t*);
typedef int (*Cdbexternal_f)(Cdb_t*, Cdbformat_t*, Cdbdata_t*, char*, size_t, Cdbtype_t*);
typedef int (*Cdbvalidate_f)(Cdb_t*, Cdbformat_t*, Cdbtype_t*);

typedef int (*Cdbmapevent_f)(Cdbmap_t*, int);
typedef int (*Cdbmap_f)(Cdbmap_t*, Cdbrecord_t*, Cdbrecord_t*);

struct Cdbmapmeth_s			/* external map method		*/
{
	const char*	name;		/* map name			*/
	const char*	description;	/* map description		*/
	unsigned int	flags;		/* CDB_MAP_* flags		*/
	Cdbmapevent_f	eventf;		/* event function		*/
	Cdbmap_f	mapf;		/* map function			*/
	void*		data;		/* map specific data		*/
	Cdbmapmeth_t*	next;		/* next in list of maps		*/
};

struct Cdbmap_s				/* cdbmapopen() handle		*/
{
	Cdbmapmeth_t	meth;		/* map method			*/
	char*		details;	/* map method details		*/
	unsigned int	flags;		/* CDB_MAP_* flags		*/
	Cdb_t*		idb;		/* initialized for this input	*/
	Cdb_t*		odb;		/* initialized for this output	*/
	Vmalloc_t*	vm;		/* all map data in this region	*/

#ifdef _CDB_MAP_METHOD_
	_CDB_MAP_METHOD_
#endif

#ifdef _CDB_MAP_PRIVATE_
	_CDB_MAP_PRIVATE_
#endif

};

struct Cdbdisc_s			/* user discipline		*/
{
	Cdbuint_t	version;	/* CDB_VERSION			*/
	const char*	schema;		/* schema descriptor		*/
	const char*	comment;	/* format specific comment	*/
	const char*	details;	/* format specific details	*/
	const char*	lib;		/* pathfind() lib		*/
	Error_f		errorf;		/* error function		*/
	Cdbevent_f	eventf;		/* event function		*/
	Cdbindex_f	indexf;		/* key index function		*/
};

struct Cdbdata_s			/* field data			*/
{
	union
	{
	Cdbint_t	integer;	/* CDB_INTEGER			*/
	Cdbuint_t	uinteger;	/* CDB_UNSIGNED|CDB_INTEGER	*/
	Sflong_t	linteger;	/* CDB_LONG|CDB_INTEGER		*/
	Sfulong_t	winteger;	/* CDB_UNSIGNED|LONG|INTEGER	*/
	double		floating;	/* CDB_FLOATING			*/
	}		number;
	struct
	{
	char*		base;		/* CDB_STRING			*/
	unsigned short	length;		/* length without terminator	*/
	}		string;
	unsigned short	flags;		/* CDB_* type reference flags	*/
	unsigned short	record;		/* cdb->table[] record index	*/
};

struct Cdbtype_s			/* type method			*/
{
	const char*	name;		/* type name			*/
	const char*	description;	/* type description		*/
	Cdbuint_t	flags;		/* CDB_* type support flags	*/
	Cdbinit_f	initf;		/* called once to initialize	*/
	Cdbinternal_f	internalf;	/* convert external=>internal	*/
	Cdbexternal_f	externalf;	/* convert internal=>external	*/
	Cdbvalidate_f	validatef;	/* validate schema usage	*/
	void*		data;		/* type specific data		*/
	Cdbtype_t*	next;		/* next in list of types	*/
};

typedef struct Cdbsep_s			/* delimiter as string or char	*/
{
	char*		str;		/* delimiter string if != 0	*/
	int		chr;		/* delimiter char		*/
	int		dir;		/* >0:push 0:replace <0:pop	*/
} Cdbsep_t;

struct Cdbformat_s			/* cdb field format		*/
{
	Cdbtype_t*	external;	/* external type method		*/
	char*		details;	/* external type method details	*/
	char*		name;		/* field name			*/
	Cdbsep_t	delimiter;	/* field delimiter		*/
	Cdbsep_t	escape;		/* field delimiter/quote escape	*/
	Cdbsep_t	quotebegin;	/* field quote begin		*/
	Cdbsep_t	quoteend;	/* field quote end		*/
	unsigned short	width;		/* field width			*/
	unsigned short	flags;		/* CDB_* type reference flags	*/
	unsigned char	type;		/* field type			*/
	unsigned char	base;		/* representation base		*/
	unsigned char	code;		/* representation ccode set	*/
	unsigned char	virtuals;	/* virtual field count		*/
	unsigned char	ptype;		/* type with CDB_PHYSICAL	*/
	unsigned char	ultype;		/* type with CDB_UNSIGNED|LONG	*/
};

struct Cdbschema_s			/* cdb schema			*/
{
	const char*	name;		/* schema name			*/
	Cdbdata_t	value;		/* identification value		*/
	unsigned int	index;		/* cdb->partition[] index	*/
	unsigned int	fields;		/* # fields			*/
	unsigned int	fixed;		/* fixed length record size	*/
	unsigned int	permanent;	/* # permanent fields		*/
	size_t		count;		/* # times this type processed	*/
	Cdbsep_t	terminator;	/* record terminator		*/
	Cdbformat_t*	format;		/* field formats		*/
	Cdbschema_t*	partition;	/* partition list		*/
	Cdbschema_t*	next;		/* parent partition chain	*/

#ifdef _CDB_SCHEMA_METHOD_
	_CDB_SCHEMA_METHOD_
#endif

#ifdef _CDB_SCHEMA_PRIVATE_
	_CDB_SCHEMA_PRIVATE_
#endif

};

struct Cdbrecord_s			/* cdb record			*/
{
	Cdbdata_t*	data;		/* record field data		*/
	Cdbschema_t*	schema;		/* record schema		*/
	Cdbrecord_t*	next;		/* next in partition		*/
	Vmalloc_t*	vm;		/* record local region		*/

#ifdef _CDB_RECORD_METHOD_
	_CDB_RECORD_METHOD_
#endif

#ifdef _CDB_RECORD_PRIVATE_
	_CDB_RECORD_PRIVATE_
#endif

};

struct Cdbmeth_s			/* file access method		*/
{
	const char*	name;		/* method name			*/
	const char*	description;	/* method description		*/
	const char*	suffix;		/* common file suffix		*/
	Cdbuint_t	flags;		/* CDB_* flags			*/
	int		(*eventf)(Cdb_t*, int);
					/* event function		*/
	int		(*recognizef)(Cdb_t*);
					/* format recognition function	*/
	int		(*hdrreadf)(Cdb_t*);
					/* header read function		*/
	int		(*hdrwritef)(Cdb_t*);
					/* header write function	*/
	Cdbrecord_t*	(*recreadf)(Cdb_t*, Cdbkey_t*);
					/* record read function		*/
	int		(*recwritef)(Cdb_t*, Cdbkey_t*, Cdbrecord_t*);
					/* record write function	*/
	Sfoff_t		(*recseekf)(Cdb_t*, Sfoff_t);
					/* record seek function		*/
	void*		data;		/* method specific data		*/
	Cdbmeth_t*	next;		/* next in list/stack		*/
};

struct Cdb_s				/* cdb info handle		*/
{
	const char*	id;		/* library id string		*/
	Cdbmeth_t	meth;		/* file access method		*/
	Cdbdisc_t*	disc;		/* user discipline		*/
	unsigned int	flags;		/* CDB_* flags			*/
	int		major;		/* major version		*/
	int		minor;		/* minor version		*/
	int		sized;		/* first field is record size	*/
	unsigned int	partitions;	/* # schema partitions		*/
	unsigned int	fields;		/* max # fields			*/
	size_t		count;		/* total # records processed	*/
	Sfio_t*		io;		/* io stream			*/
	Sfio_t*		tmp;		/* temporary string stream	*/
	char*		path;		/* io path			*/
	char*		comment;	/* data specific comment	*/
	char*		schema;		/* canonical schema descriptor	*/
	Cdbschema_t**	table;		/* table of all partitions	*/
	Vmalloc_t*	vm;		/* cdb local region		*/

#ifdef _CDB_METHOD_
	_CDB_METHOD_
#endif

#ifdef _CDB_PRIVATE_
	_CDB_PRIVATE_
#endif

};

#if _BLD_cdb && defined(__EXPORT__)
#define extern		__EXPORT__
#endif
#if !_BLD_cdb && defined(__IMPORT__)
#define extern		extern __IMPORT__
#endif

extern Cdbmeth_t* Cdb;		/* cdb format		*/
extern Cdbmeth_t* Cdbflat;	/* flat file format	*/
extern Cdbmeth_t* Cdbgzip;	/* gzip compression	*/
extern Cdbmeth_t* Cdbpzip;	/* pzip compression	*/
extern Cdbmeth_t* Cdbvdelta;	/* vdelta compression	*/

#undef	extern

#if !_BLD_cdb && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern int		cdb_lib(void);

#undef	extern

#if _BLD_cdb && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern Cdb_t*		cdbopen(Cdb_t*, Cdbdisc_t*, Cdbmeth_t*, Sfio_t*, const char*, int);
extern int		cdbclose(Cdb_t*);

extern Cdbmap_t*	cdbmapopen(Cdb_t*, const char*, Cdbmapmeth_t*);
extern char*		cdbschema(Cdb_t*, Cdbmap_t*);
extern int		cdbmapclose(Cdbmap_t*);

extern Cdbrecord_t*	cdbread(Cdb_t*, Cdbkey_t*);
extern int		cdbwrite(Cdb_t*, Cdbkey_t*, Cdbrecord_t*);
extern Sfoff_t		cdbseek(Cdb_t*, Sfoff_t);
extern ssize_t		cdbmove(Cdb_t*, Cdb_t*, Cdbmap_t*, ssize_t);

extern Cdbrecord_t*	cdbcache(Cdb_t*, Cdbrecord_t*);
extern int		cdbdrop(Cdb_t*, Cdbrecord_t*);
extern int		cdbempty(Cdb_t*, Cdbdata_t*, Cdbformat_t*, size_t);
extern ssize_t		cdbimage(Cdb_t*, Cdbrecord_t*, Sfio_t*, int);

extern int		cdblib(const char*, Cdbdisc_t*);

extern int		cdbaddmap(Cdbmapmeth_t*);
extern Cdbmapmeth_t*	cdbgetmap(const char*);
extern int		cdbaddmeth(Cdbmeth_t*);
extern Cdbmeth_t*	cdbgetmeth(const char*);
extern int		cdbaddtype(Cdbtype_t*);
extern Cdbtype_t*	cdbgettype(const char*);

extern void		cdbdump(Cdb_t*, Sfio_t*, const char*, Cdbrecord_t*);
extern char*		cdbflags(Cdb_t*, int);
extern char*		cdbmapflags(Cdb_t*, int);
extern char*		cdbtypes(Cdb_t*, int);

#undef	extern

#define _CDB_(cdb)		((Cdb_t*)cdb)

#define _cdb_read(p,k)		(*_CDB_(p)->meth.recreadf)(p,k)
#define _cdb_write(p,k,r)	(*_CDB_(p)->meth.recwritef)(p,k,r)
#define _cdb_seek(p,o)		(*_CDB_(p)->meth.recseekf)(p,o)

#define cdbread(p,k)		_cdb_read(p,k)
#define cdbwrite(p,k,r)		_cdb_write(p,k,r)
#define cdbseek(p,o)		_cdb_seek(p,o)

#endif
