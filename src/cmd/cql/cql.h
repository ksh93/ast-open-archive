/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1991-2001 AT&T Corp.                *
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
*                 This software was created by the                 *
*                 Network Services Research Center                 *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * cql definitions
 */

#include <ast.h>
#include <cdb.h>
#include <ctype.h>
#include <error.h>
#include <expr.h>
#include <ls.h>
#include <tm.h>
#include <hix.h>
#include <debug.h>
#include <sfstr.h>

#define CACHE		8		/* minimum record cache size	*/
#define DECAY		4		/* cache miss decay		*/
#define DATA		32		/* cache data size roundup	*/
#define DELIMITER	';'		/* default field delimiter	*/
#define SEPARATOR	','		/* default list delimiter	*/
#define ELAPSED		100		/* elapsed type fraction of sec	*/
#define MATCHPATTERN	"*[*?(|)\\[\\]]*" /* strmatch() pattern pattern	*/
#define SCANLIMIT	0		/* default partition scan limit	*/
#define TERMINATOR	'\n'		/* default record delimiter	*/
#define VERSION		"cql 2.0"	/* change with hix hash changes	*/
#define WIDTH		64		/* allocated field width	*/

#define DIRECTCHUNK	16		/* direct access field chunk	*/
#define FIELDCHUNK	128		/* field allocation chunk	*/
#define PIPECHUNK	(2*1024*1024)	/* pipe slurp chunk		*/
#define FILECHUNK	(8*1024)	/* file input allocation chunk	*/
#define HERECHUNK	(1*1024)	/* here input allocation chunk	*/
#define MEMBERCHUNK	32		/* member allocation chunk	*/

#define ASSOCIATIVE	(-('A'))	/* associative reference	*/
#define DIRECT		(-('D'))	/* direct reference		*/
#define INDEX		(-('I'))	/* index reference		*/
#define NONE		(-('N'))	/* no indexing			*/
#define RECORD		(-('R'))	/* record index reference	*/
#define SCALAR		(-('S'))	/* scalar value reference	*/

#define DELDEF		(-1)		/* default delimiter		*/
#define DELOFF		(-2)		/* no delimiter			*/

#define context(p)	(excontext(p,state.text,sizeof(state.text)),state.text)
#define filename(f)	((f)->hix?(f)->hix->name:(f)->name?(f)->name:(f)->record?(f)->record->symbol->name:(char*)0)

/*
 * cql extoken.index bit encodings
 *
 *	BBBBBKKKKTTTTTTTSFFFFFFFFFFFFFFF
 *
 *	B	builtin variable/function
 *	K	keyword index
 *	T	internal type index
 *	S	sorted bit
 *	F	field index
 */

#define W_B	5
#define W_K	4
#define W_T	7
#define W_F	16

#define BUILTIN(n)	((n)<<(W_K+W_T+W_F))
#define GETBUILTIN(p)	(((p)->index>>(W_K+W_T+W_F))&((1<<W_B)-1))
#define SETBUILTIN(p,n)	((p)->index|=(n)<<(W_K+W_T+W_F))

#define KEY(n)		((n)<<(W_T+W_F))
#define GETKEY(p)	(((p)->index>>(W_T+W_F))&((1<<W_K)-1))
#define SETKEY(p,k)	((p)->index|=((k)<<(W_T+W_F)))

#define GETTYPE(p)	(((p)->index>>W_F)&((1<<W_T)-1))
#define SETTYPE(p,t)	((p)->index|=(t)<<W_F,(p)->type=(t)<=T_BASE?UNSIGNED:state.type[(t)-T_BASE-1])

#define GETFIELD(p)	((p)->index&((1<<W_F)-1))
#define SETFIELD(p,n)	((p)->index&=~((1<<W_F)-1),(p)->index|=(n))

#define F_EASY		0x0001
#define F_INDEX		0x0002

#define GETEASY(p)	((p)->flags&F_EASY)
#define SETEASY(p)	((p)->flags|=F_EASY)
#define CLREASY(p)	((p)->flags&=~F_EASY)

#define GETINDEX(p)	((p)->flags&F_INDEX)
#define SETINDEX(p)	((p)->flags|=F_INDEX)

#define ISINTERNAL(t)	((t)>0&&(t)<=(T_BASE+T_TYPES))
#define ISSTRING(t)	((t)>T_BASE)

#define INDIRECT(f,n)	((f)->field->first+(n)*(f)->record->fields+(f)->key)

#define ISDATA(p)	((p)->local.number&(1<<31))
#define ISSILENT(p)	((p)->local.number&(1<<30))
#define GETDATA1(p)	(((p)->local.number>>16)&((1<<14)-1))
#define GETDATA2(p)	((p)->local.number&((1<<16)-1))
#define SETDATA(p,a,b)	((p)->local.number|=((1<<31)|((a)<<16)|(b)))
#define SETSILENT(p)	((p)->local.number|=(1<<30))

/*
 * main loop function indices
 */

#define CLOSURE		0
#define BEGIN		1
#define SELECT		2
#define ACTION		3
#define END		4
#define LOOP		5

/*
 * builtin symbol indices
 */

#define B_COMMENT	1
#define B_CLOCK		2
#define B_DATE		3
#define B_DELIMITER	4
#define B_ERRORS	5
#define B_FORMAT	6
#define B_INPUT		7
#define B_ITERATION	8
#define B_LINE		9
#define B_OFFSET	10
#define B_PERMANENT	11
#define B_RECORD	12
#define B_SCANLIMIT	13
#define B_SCHEMA	14
#define B_SELECTED	15
#define B_SIZE		16
#define B_STATE		17
#define B_TERMINATOR	18
#define B_TIME		19
#define B_getenv	20
#define B_length	21
#define B_loop		22
#define B_path		23
#define B_present	24
#define B_sub		25

/*
 * keyword symbol indices
 */

#define K_access	1
#define K_comment	2
#define K_delimiter	3
#define K_details	4
#define K_edge		5
#define K_format	6
#define K_index		7
#define K_input		8
#define K_key		9
#define K_permanent	10
#define K_scanlimit	11
#define K_schema	12
#define K_sort		13
#define K_terminator	14

/*
 * pragma indices
 */

#define P_pragma	1
#define P_static	2
#define P_struct	3

/*
 * sym.local.number flags
 */

#define S_sorted	(1<<0)		/* sorted key field		*/

/*
 * internal type indices
 */

#define T_BASE		64		/* <= T_BASE -> base rep	*/
#define T_DATE		(T_BASE+1)
#define T_ELAPSED	(T_BASE+2)
#define T_TYPES		2		/* number of internal types	*/

typedef union				/* record field types		*/
{
	double		u_double;
	long		u_long;
	unsigned long	u_unsigned;
} Number_t;

#define F_CONVERTED	(CDB_LONG|CDB_INTEGER|CDB_FLOATING)
#define F_STRING	CDB_STRING

#define Field_t		Cdbdata_t

#define f_converted	flags
#define f_data		string.base
#define f_double	number.floating
#define f_long		number.linteger
#define f_longlong	number.linteger
#define f_size		string.length
#define f_string	string.base
#define f_unsigned	number.winteger

typedef struct Cache			/* record cache			*/
{
	struct Cache*	next;		/* next in list			*/
	Cdbrecord_t*	record;		/* cached record		*/
	Cdbdata_t*	last;		/* last field			*/
	long		elements;	/* # array elements		*/
	long		hash;		/* record hash			*/
	long		sequence;	/* cache sequence number	*/
	int		cached;		/* ok to cdbdrop()		*/
	int		index;		/* record index			*/
	int		hit;		/* hit count for this record	*/
	int		partition;	/* partition			*/
	Cdbdata_t	first[1];	/* first field			*/
} Cache_t;

typedef struct Tree			/* field update tree		*/
{
	struct Tree*	left;		/* left child			*/
	struct Tree*	right;		/* right child			*/
	unsigned long	offset;		/* record offset		*/
	char**		value[1];	/* new and old field values	*/
} Update_t;

#define HIXDISC2FILE(dp)	((File_t*)(dp))
#define CDBDISC2FILE(dp)	((File_t*)((char*)(dp)-sizeof(Hixdisc_t)))

typedef struct File_s			/* open file state		*/
{
	Hixdisc_t	hixdisc;	/* hix split discipline (1st!)	*/
	Cdbdisc_t	cdbdisc;	/* cdb split discipline (2nd!) */
	Cache_t*	field;		/* current record fields	*/
	Cache_t*	cache;		/* field cache			*/
	Dt_t*		assoc;		/* ASSOCIATIVE access hash	*/
	Dtdisc_t	assocdisc;	/* assoc table discipline	*/
	Cdb_t*		cdb;		/* open cdb handle		*/
	Cdbkey_t*	cdbkey;		/* cdbread() key		*/
	char*		comment;	/* data comment			*/
	int		access;		/* record access method		*/
	int		decay;		/* cache miss decay count	*/
	int		delimiter;	/* field delimiter		*/
	char*		details;	/* cdb file format details	*/
	int		fixed;		/* fixed length records		*/
	Cdbmeth_t*	format;		/* cdb file format method	*/
	int		force;		/* force all fields		*/
	long*		hash;		/* hash for generate field(s)	*/
	int		here;		/* file contents read in	*/
	Hix_t*		hix;		/* open hash index file handle	*/
	Sfio_t*		image;		/* current record image		*/
	int*		index;		/* hixend terminated field list	*/
	int		key;		/* ASSOCIATIVE access field	*/
	int		limit;		/* cache record limit		*/
	int		maxfield;	/* max field referenced		*/
	char*		name;		/* file name			*/
	int		offset;		/* RECORD access offset		*/
	Dt_t*		overlay;	/* partition overlay table	*/
	Dtdisc_t	overdisc;	/* overlay table discipline	*/
	int		permanent;	/* # permanent fields		*/
	struct Record_s*record;		/* field schema			*/
	long		records;	/* number of records		*/
	int		scanlimit;	/* partition scan limit		*/
	long		sequence;	/* cache sequence number	*/
	char*		schema;		/* cdb file schema		*/
	int		subfield;	/* direct access by subfields	*/
	Exid_t*		symbol;		/* field symbol accessing file	*/
	int		terminator;	/* record delimiter		*/
	int		termset;	/* terminator explicitly set	*/
	Update_t*	update;		/* field updates		*/
	struct
	{
	struct Record_s*record;		/* main record			*/
	Field_t*	field;		/* field data			*/
	Expr_t*		prog;		/* expression handle		*/
	int*		generate;	/* indexes to generate		*/
	char*		data[2];	/* compiled data		*/
	char*		pattern;	/* pivotal match pattern	*/
	}		scan;		/* main scan state		*/
} File_t;

typedef struct List_s			/* cql list			*/
{
	struct List_s*	next;		/* next in list			*/
	union
	{
	char*		string;		/* string element		*/
	Exid_t*		symbol;		/* symbol element		*/
	Exnode_t*	node;		/* node element			*/
	}		value;
} List_t;

typedef struct				/* field representation format	*/
{
	unsigned short	type;		/* expr type			*/
	unsigned short	width;		/* representation width		*/ 
	int		delimiter;	/* field delimiter		*/
	unsigned char	base;		/* representation base		*/ 
	unsigned char	direction;	/* delimiter dir {'+',0,'-'}	*/
	unsigned char	elements;	/* #elements if array		*/
	unsigned char	referenced;	/* referenced in expression	*/
	unsigned char	sorted;		/* memcmp() sorted index	*/
	unsigned char	string;		/* string representation	*/
	char*		format;		/* cdb field format string	*/
} Format_t;

typedef struct				/* record member		*/
{
	Exid_t*		symbol;		/* member symbol		*/
	struct Record_s*record;		/* if member is another schema	*/
	short		access;		/* data access			*/
	short		subfield;	/* subfield index		*/
	short		index;		/* member is candidate index	*/
	Format_t	format;		/* field representation format	*/
} Member_t;

typedef struct Record_s			/* schema record		*/
{
	int		fields;		/* number of fields (members)	*/
	int		subfields;	/* total number of subfields	*/
	int		key;		/* key (index) field		*/
	int		visit;		/* propagate() visit mark	*/
	Exid_t*		symbol;		/* record name			*/
	Member_t*	member;		/* member list			*/
	Format_t*	format;		/* field io formats		*/
} Record_t;

typedef struct				/* local Exid_t info		*/
{
	File_t*		file;		/* open file for schema		*/
	File_t*		index;		/* pseudo file for index	*/
	Record_t*	record;		/* if symbol is a schema	*/
	Exref_t*	reference;	/* reference list for getval()	*/
} Local_t;

typedef struct				/* main loop function info	*/
{
	char*		name;		/* function name for this run	*/
	char*		next;		/* function name for next run	*/
	Exnode_t*	body;		/* function body		*/
} Function_t;

typedef struct				/* ASSOCIATIVE access field	*/
{
	Dtlink_t	link;
	Field_t*	field;
	char*		name;
} Assoc_t;

typedef struct				/* cdt allocated name marker	*/
{
	Dtlink_t	link;
	int		value;
	char		name[1];
} Mark_t;

typedef struct				/* program state		*/
{
	Function_t	loop[LOOP];	/* main loop functions		*/
	int		verbose;	/* verbose warning messages	*/
	Hixdisc_t	hix;		/* hix discipline defaults	*/
	unsigned long	cdb_flags;	/* cdbopen() flag defaults	*/

	/* the rest are implicitly initialized				*/

	int		active;		/* active connection		*/
	int		again;		/* run main loop again		*/
	struct
	{
	Exid_t*		entity;
	Exid_t*		relation;
	Dt_t*		member;
	struct File_s*	f;
	Record_t*	r;
	int		key;
	int		parent;
	int		child;
	int		on;
	int		selected;
	Dtdisc_t	disc;
	}		closure;	/* closure state		*/
	unsigned long	date;		/* start time			*/
	int		declaration;	/* parsing a declaration	*/
	List_t*		edge;		/* edge parent,child decl	*/
	Field_t*	empty;		/* empty record size maxfield	*/
	int		end;		/* in end()			*/
	Exdisc_t	expr;		/* expr discipline		*/
	int		generate;	/* gen main indices before eval	*/
	List_t*		index;		/* explicit main schema indices	*/
	int		insert;		/* parsing insert() list	*/
	int		iteration;	/* closure iteration count	*/
	int		maxfield;	/* max field of all schemas	*/
	int		operand;	/* parsing initializer operand	*/
	long		record;		/* current input record number	*/
	int		replace;	/* replace top db with updates	*/
	Exid_t*		schema;		/* main schema			*/
	int		selected;	/* number of selected records	*/
	List_t*		sort;		/* ordered symbol sort list	*/
	unsigned int	term_pad[2];	/* for DELDEF and DELOFF	*/
	unsigned int	term[UCHAR_MAX];/* record parse terminal chars	*/
	unsigned long	test;		/* test mask			*/
	char		text[64];	/* expression error context	*/
	int		type[T_TYPES];	/* conversion type -> type map	*/
	int		update;		/* only generate update records	*/
} State_t;

/*
 * cql externs
 */

extern State_t		state;

extern File_t*		attach(Exid_t*, Record_t*, int);
extern void		commit(File_t*);
extern int		declare(Expr_t*, const char*, Sfio_t*, int);
extern void		image(File_t*, Sfio_t*, int);
extern Exnode_t*	g2print(Expr_t*, Record_t*);
extern void		generate(Expr_t*, File_t*, const char*, Exid_t*);
extern int		hix_event(Hix_t*, int, void*, Hixdisc_t*);
extern Expr_t*		init(Exid_t*);
extern void		load(Expr_t*, File_t*, Exid_t*);
extern int		optimize(Expr_t*, Exnode_t*);
extern int		pivot(File_t*, Exnode_t*);
extern void		propagate(Record_t*);
extern Field_t*		record(File_t*, char*, long, int, int, int);
extern Extype_t		reference(Expr_t*, Exnode_t*, Exid_t*, Exref_t*, char*, int, Exdisc_t*);
extern int		subfields(Record_t*);
extern int		update(File_t*, Field_t*, int, char*);
extern Extype_t		value(Expr_t*, Exnode_t*, Exid_t*, Exref_t*, void*, int, Exdisc_t*);
