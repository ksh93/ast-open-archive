/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1986-2000 AT&T Corp.              *
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
 * preprocessor library private definitions
 */

#ifndef _PPLIB_H
#define _PPLIB_H

/*
 * the first definitions control optional code -- 0 disables
 */

#ifndef ARCHIVE
#define ARCHIVE		1	/* -I can specify header archives	*/
#endif
#ifndef CATSTRINGS
#define CATSTRINGS	1	/* concatenate adjacent strings		*/
#endif
#ifndef CHECKPOINT
#define CHECKPOINT	1	/* checkpoint preprocessed files	*/
#endif
#ifndef COMPATIBLE
#define COMPATIBLE	1	/* enable COMPATIBILITY related code	*/
#endif
#ifndef MACKEYARGS
#define MACKEYARGS 	_BLD_DEBUG /* name=value macro formals and actuals */
#endif
#ifndef POOL
#define POOL		1	/* enable loop on input,output,error	*/
#endif
#ifndef PROTOTYPE
#define PROTOTYPE	1	/* enable ppproto code			*/
#endif

#define TRUNCLENGTH	8	/* default TRUNCATE length		*/

#if _BLD_DEBUG
#undef	DEBUG
#define DEBUG		(TRACE_message|TRACE_count|TRACE_debug)
#else
#ifndef DEBUG
#define DEBUG		(TRACE_message)
#endif
#endif

/*
 * the lower tests are transient
 */

#define TEST_count		(1L<<24)
#define TEST_hashcount		(1L<<25)
#define TEST_hashdump		(1L<<26)
#define TEST_hit		(1L<<27)
#define TEST_noinit		(1L<<28)
#define TEST_nonoise		(1L<<29)
#define TEST_noproto		(1L<<30)

#define TEST_INVERT		(1L<<31)

#define PROTO_CLASSIC		(1<<0)	/* classic to prototyped	*/
#define PROTO_DISABLE		(1<<1)	/* disable conversion		*/
#define PROTO_FORCE		(1<<2)	/* force even if no magic	*/
#define PROTO_HEADER		(1<<3)	/* header defines too		*/
#define PROTO_INCLUDE		(1<<4)	/* <prototyped.h> instead	*/
#define PROTO_INITIALIZED	(1<<5)	/* internal initialization	*/
#define PROTO_LINESYNC		(1<<6)	/* force standalone line syncs	*/
#define PROTO_NOPRAGMA		(1<<7)	/* delete pragma prototyped	*/
#define PROTO_PASS		(1<<8)	/* pass blocks if no magic	*/
#define PROTO_PLUSPLUS		(1<<9)	/* extern () -> extern (...)	*/
#define PROTO_RETAIN		(1<<10)	/* defines retained after close	*/
#define PROTO_TEST		(1<<12)	/* enable test code		*/

#define PROTO_USER		(1<<13)	/* first user flag		*/

#define SEARCH_EXISTS		0	/* ppsearch for existence	*/
#define SEARCH_INCLUDE		(1<<0)	/* ppsearch for include		*/
#define SEARCH_USER		(1<<1)	/* first user flag		*/

#define STYLE_gnu		(1<<0)	/* gnu style args		*/

#define IN_c			(1<<0)	/* C language file		*/
#define IN_defguard		(1<<1)	/* did multiple include check	*/
#define IN_disable		(1<<2)	/* saved state&DISABLE		*/
#define IN_endguard		(1<<3)	/* did multiple include check	*/
#define IN_eof			(1<<4)	/* reached EOF			*/
#define IN_expand		(1<<5)	/* ppexpand buffer		*/
#define IN_flush		(1<<6)	/* flush stdout on file_refill()*/
#define IN_hosted		(1<<7)	/* saved mode&HOSTED		*/
#define IN_ignoreline		(1<<8)	/* ignore #line until file	*/
#define IN_newline		(1<<9)	/* newline at end of last fill	*/
#define IN_noguard		(1<<10)	/* no multiple include guard	*/
#define IN_prototype		(1<<11)	/* ppproto() input		*/
#define IN_regular		(1<<12)	/* regular input file		*/
#define IN_static		(1<<13)	/* static buffer - don't free	*/
#define IN_sync			(1<<14)	/* line sync required on pop	*/
#define IN_tokens		(1<<15)	/* non-space tokens encountered	*/

struct ppsymbol;
struct ppindex;

typedef char*	(*PPBUILTIN)(char*, const char*, const char*);
typedef void	(*PPCOMMENT)(const char*, const char*, const char*, int);
typedef void	(*PPINCREF)(const char*, const char*, int, int);
typedef void	(*PPLINESYNC)(int, const char*);
typedef void	(*PPMACREF)(struct ppsymbol*, const char*, int, int, unsigned long);
typedef int	(*PPOPTARG)(int, int, const char*);
typedef void	(*PPPRAGMA)(const char*, const char*, const char*, const char*, int);

struct ppinstk				/* input stream stack frame	*/
{
	char*		nextchr;	/* next input char (first elt)	*/
	struct ppinstk*	next;		/* next frame (for allocation)	*/
	struct ppinstk*	prev;		/* previous frame		*/
	long*		control;	/* control block level		*/
	char*		buffer;		/* buffer base pointer		*/
	char*		file;		/* saved file name		*/
	char*		prefix;		/* directory prefix		*/
	struct ppsymbol* symbol;	/* macro info			*/
#if CHECKPOINT
	struct ppindex*	index;		/* checkpoint include index	*/
	int		buflen;		/* buffer count			*/
#endif
	int		line;		/* saved line number		*/
	short		fd;		/* file descriptor		*/
	short		hide;		/* hide index (from pp.hide)	*/
	short		flags;		/* IN_[a-z]* flags		*/
	char		type;		/* input type			*/
};

#if MACKEYARGS
struct ppkeyarg				/* pp macro keyword arg info	*/
{
	char*		name;		/* keyword arg name		*/
	char*		value;		/* keyword arg value		*/
};
#endif

struct pplist				/* string list			*/
{
	char*		value;		/* string value			*/
	struct pplist*	next;		/* next in list			*/
};

struct oplist				/* queue op until PP_INIT	*/
{
	int		op;		/* PP_* op			*/
	char*		value;		/* op value			*/
	struct oplist*	next;		/* next op			*/
};

struct pphide				/* hidden symbol info		*/
{
	struct ppmacro*	macro;		/* saved macro info		*/
	unsigned long	flags;		/* saved symbol flags if macro	*/
	int		level;		/* nesting level		*/
};

struct ppmacstk				/* macro invocation stack frame	*/
{
	struct ppmacstk* next;		/* next frame (for allocation)	*/
	struct ppmacstk* prev;		/* previous frame		*/
	int		line;		/* line number of first arg	*/
	char*		arg[1];		/* arg text pointers		*/
};

struct ppmember				/* archive member pun on ppfile	*/
{
	struct ppdirs*	archive;	/* archive holding file		*/
	unsigned long	offset;		/* data offset			*/
	unsigned long	size;		/* data size			*/
};

struct counter				/* monitoring counters		*/
{
	int		candidate;	/* macro candidates		*/
	int		function;	/* function macros		*/
	int		macro;		/* macro hits			*/
	int		pplex;		/* pplex() calls		*/
	int		push;		/* input stream pushes		*/
	int		terminal;	/* terminal states		*/
	int		token;		/* emitted tokens		*/
};

#define _PP_CONTEXT_PRIVATE_		/* ppglobals private context	*/ \
	struct ppcontext* context;	/* current context		*/ \
	long		state;		/* pp state flags		*/ \
	long		mode;		/* uncoupled pp state flags	*/ \
	long		option;		/* option flags			*/ \
	long		test;		/* implementation tests		*/ \
	Sfio_t*		filedeps;	/* FILEDEPS output stream	*/ \
	struct ppdirs*	firstdir;	/* first include dir		*/ \
	struct ppdirs*	lastdir;	/* last include dir		*/ \
	int		hide;		/* current include hide index	*/ \
	int		column;		/* FILEDEPS column		*/ \
	int		pending;	/* ppline() pending output	*/ \
	char*		firstfile;	/* ppline() first file		*/ \
	char*		lastfile;	/* ppline() most recent file	*/ \
	char*		ignore;		/* include ignore list file	*/ \
	char*		probe;		/* ppdefault probe key		*/ \
	Hash_table_t*	filtab;		/* file name hash table		*/ \
	Hash_table_t*	prdtab;		/* predicate hash table		*/ \
	char*		date;		/* start date string		*/ \
	char*		time;		/* start time string		*/ \
	char*		maps;		/* directive maps		*/ \
	long		ro_state;	/* readonly state		*/ \
	long		ro_mode;	/* readonly mode		*/ \
	long		ro_option;	/* readonly option		*/ \
	char*		cdir;		/* arg C dir			*/ \
	char*		hostdir;	/* arg host dir			*/ \
	char*		ppdefault;	/* arg default info file	*/ \
	struct ppindex*	firstindex;	/* first include index entry	*/ \
	struct ppindex*	lastindex;	/* last include index entry	*/ \
	struct oplist*	firstop;	/* first arg op			*/ \
	struct oplist*	lastop;		/* last arg op			*/ \
	struct oplist*	firsttx;	/* first text file		*/ \
	struct oplist*	lasttx;		/* last text file		*/ \
	unsigned char	arg_file;	/* arg file index		*/ \
	unsigned char	arg_mode;	/* arg mode			*/ \
	unsigned char	arg_style;	/* arg style			*/ \
	unsigned char	c;		/* arg C state			*/ \
	unsigned char	hosted;		/* arg hosted state		*/ \
	unsigned char	ignoresrc;	/* arg ignore source state	*/ \
	unsigned char	initialized;	/* arg initialized state	*/ \
	unsigned char	standalone;	/* arg standalone state		*/ \
	unsigned char	spare_1;	/* padding spare		*/

#define _PP_GLOBALS_PRIVATE_		/* ppglobals private additions	*/ \
	char*		checkpoint;	/* checkpoint version		*/ \
	int		constack;	/* pp.control size		*/ \
	struct ppinstk*	in;		/* input stream stack pointer	*/ \
	char*		addp;	    	/* addbuf pointer		*/ \
	char*		args;		/* predicate args		*/ \
	char*		addbuf;		/* ADD buffer			*/ \
	char*		catbuf;		/* catenation buffer		*/ \
	char*		hdrbuf;		/* HEADEREXPAND buffer		*/ \
	char*		path;		/* full path of last #include	*/ \
	char*		tmpbuf;		/* very temporary buffer	*/ \
	char*		valbuf;		/* builtin macro value buffer	*/ \
		/* the rest are implicitly initialized */ \
	char*		include;	/* saved path of last #include	*/ \
	char*		prefix;		/* current directory prefix	*/ \
	struct ppmember* member;	/* include archive member data	*/ \
	int		hidden;		/* hidden newline count		*/ \
	int		hiding;		/* number of symbols in hiding	*/ \
	int		level;		/* pplex() recursion level	*/ \
	struct								   \
	{								   \
	int		input;		/* pool input			*/ \
	int		output;		/* pool output			*/ \
	}		pool;		/* loop on input,output,error	*/ \
	int		truncate;	/* identifier truncation length	*/ \
	struct ppmacstk* macp;		/* top of macro actual stack	*/ \
	char*		maxmac;		/* maximum size of macro stack	*/ \
	char*		mactop;		/* top of current macro frame	*/ \
	char*		toknxt;		/* '\0' of pp.token		*/ \
	long*		control;	/* control block flags pointer	*/ \
	long*		maxcon;		/* max control block frame	*/ \
	struct oplist*	chop;		/* include prefix chop list	*/ \
	struct ppfile*	insert;		/* inserted line sync file	*/ \
	struct ppfile*	original;	/* original include name	*/ \
	Hash_table_t*	dirtab;		/* directive hash table		*/ \
	Hash_table_t*	strtab;		/* string hash table		*/ \
	PPBUILTIN	builtin;	/* builtin macro handler	*/ \
	PPCOMMENT	comment;	/* pass along comments		*/ \
	PPINCREF	incref;		/* include file push/return	*/ \
	PPLINESYNC	linesync;	/* pass along line sync info	*/ \
	PPLINESYNC	olinesync;	/* original linesync value	*/ \
	PPMACREF	macref;		/* called on macro def/ref	*/ \
	PPOPTARG	optarg;		/* unknown option arg handler	*/ \
	PPPRAGMA	pragma;		/* pass along unknown pragmas	*/ \
	struct counter	counter;	/* monitoring counters		*/

#define _PP_SYMBOL_PRIVATE_		/* ppsymbol private additions	*/ \
	struct pphide*	hidden;		/* hidden symbol info		*/

#if MACKEYARGS
#define _PP_MACRO_PRIVATE_		/* ppmacro private additions	*/ \
	int		size;		/* body size			*/ \
	union								   \
	{								   \
	char*		formal;		/* normal formals list		*/ \
	struct ppkeyarg* key;		/* keyword formals table	*/ \
	}		args;		/* macro args info		*/
#define formals		args.formal	/* formal argument list		*/
#define formkeys	args.key	/* formal keyword argument list	*/
#else
#define _PP_MACRO_PRIVATE_		/* ppmacro private additions	*/ \
	int		size;		/* body size			*/ \
	char*		formals;	/* formal argument list		*/
#endif

#define _PP_DIRS_PRIVATE_		/* ppdirs private additions	*/ \
	unsigned char	c;		/* files here are C language	*/ \
	unsigned char	hosted;		/* files found here are hosted	*/ \
	unsigned char	index;		/* prefix,local,standard index	*/ \
	unsigned char	type;		/* dir type			*/ \
	union								   \
	{								   \
	char*		buffer;		/* TYPE_BUFFER buffer		*/ \
	Sfio_t*		sp;		/* archive stream		*/ \
	struct ppdirs*	subdir;		/* subdir list			*/ \
	}		info;		/* type info			*/

#if !PROTOMAIN
#include <ast.h>
#include <error.h>
#endif

#undef	newof
#define newof(p,t,n,x)	((p)?(t*)realloc((char*)(p),sizeof(t)*(n)+(x)):(t*)calloc(1,sizeof(t)*(n)+(x)))

#include "pp.h"
#include "ppdef.h"
#include "ppkey.h"

#undef	setstate			/* random clash!		*/

/*
 * DEBUG is encoded with the following bits
 */

#define TRACE_message		01
#define TRACE_count		02
#define TRACE_debug		04

#if DEBUG && !lint
#define	PANIC		(ERROR_PANIC|ERROR_SOURCE|ERROR_SYSTEM),__FILE__,__LINE__
#else
#define	PANIC		ERROR_PANIC
#endif

#if DEBUG & TRACE_count
#define count(x)	pp.counter.x++
#else
#define count(x)
#endif

#if DEBUG & TRACE_message
#define message(x)	do { if (tracing) error x; } while (0)
#else
#define message(x)
#endif

#if DEBUG & TRACE_debug
#define debug(x)	do { if (tracing) error x; } while (0)
#else
#define debug(x)
#endif

/*
 * note that MEMCPY advances the associated pointers
 */

#define MEMCPY(to,fr,n) \
	do switch(n) \
	{ default : memcpy(to,fr,n); to += n; fr += n; break; \
	  case  7 : *to++ = *fr++; \
	  case  6 : *to++ = *fr++; \
	  case  5 : *to++ = *fr++; \
	  case  4 : *to++ = *fr++; \
	  case  3 : *to++ = *fr++; \
	  case  2 : *to++ = *fr++; \
	  case  1 : *to++ = *fr++; \
	  case  0 : break; \
	} while (0)

#define NEWDIRECTIVE	(-1)

#define dirname(x)	ppkeyname(x,1)
#define error		pperror
#define keyname(x)	ppkeyname(x,0)
#define nextframe(m,p)	(m->next=m+(p-(char*)m+sizeof(struct ppmacstk)-1)/sizeof(struct ppmacstk)+1)
#define popframe(m)	(m=m->prev)
#define pptokchr(c)	pptokstr(NiL,(c))
#define pushcontrol()	do { if (pp.control++ >= pp.maxcon) ppnest(); } while (0)
#define pushframe(m)	(m->next->prev=m,m=m->next)
#define setmode(m,v)	((v)?(pp.mode|=(m)):(pp.mode&=~(m)))
#define setoption(m,v)	((v)?(pp.option|=(m)):(pp.option&=~(m)))
#define setstate(s,v)	((v)?(pp.state|=(s)):(pp.state&=~(s)))
#define tracing		(error_info.trace<0)

#define ppgetfile(x)	((struct ppfile*)hashlook(pp.filtab,x,HASH_LOOKUP,NiL))
#define ppsetfile(x)	((struct ppfile*)hashlook(pp.filtab,x,HASH_CREATE|HASH_SIZE(sizeof(struct ppfile)),NiL))

#define ppkeyget(t,n)	(struct ppsymkey*)hashlook(t,n,HASH_LOOKUP,NiL)
#define ppkeyref(t,n)	(struct ppsymkey*)hashlook(t,n,HASH_LOOKUP|HASH_INTERNAL,NiL)
#define ppkeyset(t,n)	(struct ppsymkey*)hashlook(t,n,HASH_CREATE|HASH_SIZE(sizeof(struct ppsymkey)),NiL)

#define MARK		'@'		/* internal mark		*/
#define ARGOFFSET	'1'		/* macro arg mark offset	*/

#define STRAPP(p,v,r)	do{r=(v);while((*p++)=(*r++));}while(0)
#define STRCOPY(p,v,r)	do{r=(v);while((*p++)=(*r++));p--;}while(0)
#define STRCOPY2(p,r)	do{while((*p++)=(*r++));p--;}while(0)

#define SETFILE(p,v)	(p+=sfsprintf(p,16,"%c%c%lx%c",MARK,'F',(long)v,MARK))
#define SETLINE(p,v)	(p+=sfsprintf(p,16,"%c%c%lx%c",MARK,'L',(long)v,MARK))

#define peekchr()	(*pp.in->nextchr)
#define ungetchr(c)	(*--pp.in->nextchr=(c))

#define MAXID		255		/* maximum identifier size	*/
#define MAXTOKEN	PPTOKSIZ	/* maximum token size		*/
#define MAXFORMALS	64		/* maximum number macro formals	*/
#define MAXHIDDEN	10		/* ppline if hidden>=MAXHIDDEN	*/
#define DEFMACSTACK	(MAXFORMALS*32*32)/* default macstack size	*/

#define FSM_COMPATIBILITY	1	/* compatibility mode		*/
#define FSM_IDADD	2		/* add to identifer set		*/
#define FSM_IDDEL	3		/* delete from identifer set	*/
#define FSM_INIT	4		/* initilize			*/
#define FSM_MACRO	5		/* add new macro		*/
#define FSM_OPSPACE	6		/* handle <binop><space>=	*/
#define FSM_PLUSPLUS	7		/* C++ lexical analysis		*/
#define FSM_QUOTADD	8		/* add to quote set		*/
#define FSM_QUOTDEL	9		/* delete from quote set	*/

#define IN_TOP		01		/* top level -- directives ok	*/

#define IN_BUFFER	(2|IN_TOP)	/* buffer of lines		*/
#define IN_COPY		2		/* macro arg (copied)		*/
#define IN_EXPAND	4		/* macro arg (expanded)		*/
#define IN_FILE		(4|IN_TOP)	/* file				*/
#define IN_INIT		(6|IN_TOP)	/* initialization IN_BUFFER	*/
#define IN_MACRO	8		/* macro text			*/
#define IN_MULTILINE	(8|IN_TOP)	/* multi-line macro text	*/
#define IN_QUOTE	10		/* "..." macro arg (copied)	*/
#define IN_RESCAN	(10|IN_TOP)	/* directive rescan buffer	*/
#define IN_SQUOTE	12		/* '...' macro arg (copied)	*/
#define IN_STRING	14		/* string			*/

#define INC_CLEAR	((struct ppsymbol*)0)
#define INC_IGNORE	((struct ppsymbol*)pp.addbuf)
#define INC_TEST	((struct ppsymbol*)pp.catbuf)

#define INC_BOUND(n)	(1<<(n))
#define INC_MEMBER(n)	(1<<((n)+INC_MAX))
#define INC_PREFIX	0
#define INC_LOCAL	1
#define INC_STANDARD	2
#define INC_MAX		3
#define INC_MAP		INC_PREFIX
#define INC_SELF	(1<<(2*INC_MAX+0))
#define INC_EXISTS	(1<<(2*INC_MAX+1))
#define INC_MAPPED	(1<<(2*INC_MAX+2))

#define TYPE_ARCHIVE	(1<<0)
#define TYPE_BUFFER	(1<<1)
#define TYPE_CHECKPOINT	(1<<2)
#define TYPE_DIRECTORY	(1<<3)

#define PRAGMA_COMMAND	01		/* option had command prefix	*/
#define PRAGMA_PP	02		/* pp command prefix		*/

#define TOK_BUILTIN	(1<<0)		/* last token was #(		*/
#define TOK_FORMAL	(1<<1)		/* last token was arg formal id	*/
#define TOK_ID		(1<<2)		/* last token was identifier	*/
#define TOK_TOKCAT	(1<<3)		/* last token was ##		*/

#define HADELSE		(1<<0)		/* already had else part	*/
#define KEPT		(1<<1)		/* already kept part of block	*/
#define SKIP		(1<<2)		/* skip this block		*/
#define BLOCKBITS	3		/* block flag bits		*/

#define SETIFBLOCK(p)	(*(p)=(*((p)-1)&SKIP)|((long)error_info.line<<BLOCKBITS))
#define GETIFLINE(p)	((*(p)>>BLOCKBITS)&((1L<<(sizeof(long)*CHAR_BIT-BLOCKBITS))-1))

#define PUSH(t,p)		\
	do \
	{ \
		count(push); \
		if (!pp.in->next) \
		{ \
			pp.in->next = newof(0, struct ppinstk, 1, 0); \
			pp.in->next->prev = pp.in; \
		} \
		p = pp.in = pp.in->next; \
		p->type = t; \
		p->flags = 0; \
	} while (0)

#define PUSH_BUFFER(f,p,n)		\
	pppush(IN_BUFFER,f,p,n);

#define PUSH_COPY(p,n)		\
	do \
	{ \
		register struct ppinstk*	cur; \
		PUSH(IN_COPY, cur); \
		cur->line = error_info.line; \
		error_info.line = n; \
		cur->nextchr = p; \
		cur->prev->symbol->flags &= ~SYM_DISABLED; \
		debug((-7, "PUSH in=%s next=%s", ppinstr(pp.in), pptokchr(*pp.in->nextchr))); \
	} while (0)

#define PUSH_EXPAND(p,n)	\
	do \
	{ \
		register struct ppinstk*	cur; \
		PUSH(IN_EXPAND, cur); \
		cur->line = error_info.line; \
		error_info.line = n; \
		cur->prev->symbol->flags &= ~SYM_DISABLED; \
		cur->buffer = cur->nextchr = ppexpand(p); \
		if (!(cur->prev->symbol->flags & SYM_MULTILINE)) \
			cur->prev->symbol->flags |= SYM_DISABLED; \
		debug((-7, "PUSH in=%s next=%s", ppinstr(pp.in), pptokchr(*pp.in->nextchr))); \
	} while (0)

#define PUSH_FILE(f,d)	\
	pppush(IN_FILE,f,NiL,d)

#define PUSH_INIT(f,p)	\
	pppush(IN_INIT,f,p,1)

#define PUSH_MACRO(p)		\
	do \
	{ \
		register struct ppinstk*	cur; \
		PUSH(IN_MACRO, cur); \
		cur->symbol = p; \
		cur->nextchr = p->macro->value; \
		p->flags |= SYM_DISABLED; \
		if (p->flags & SYM_FUNCTION) pushframe(pp.macp); \
		pp.state &= ~NEWLINE; \
		debug((-7, "PUSH in=%s next=%s", ppinstr(pp.in), pptokchr(*pp.in->nextchr))); \
	} while (0)

#define PUSH_MULTILINE(p)		\
	do \
	{ \
		register struct ppinstk*	cur; \
		register int			n; \
		PUSH(IN_MULTILINE, cur); \
		cur->symbol = p; \
		cur->flags |= IN_defguard|IN_endguard|IN_noguard; \
		pushcontrol(); \
		cur->control = pp.control; \
		*pp.control = 0; \
		cur->file = error_info.file; \
		error_info.file = cur->buffer = malloc(n = strlen(error_info.file) + strlen(((struct ppsymbol*)p)->name) + 24); \
		sfsprintf(error_info.file, n, "%s:%s,%d", cur->file, p->name, error_info.line); \
		cur->line = error_info.line; \
		error_info.line = 1; \
		cur->nextchr = p->macro->value; \
		if (p->flags & SYM_FUNCTION) pushframe(pp.macp); \
		pp.state &= ~NEWLINE; \
		debug((-7, "PUSH in=%s next=%s", ppinstr(pp.in), pptokchr(*pp.in->nextchr))); \
	} while (0)

#define PUSH_QUOTE(p,n)		\
	do \
	{ \
		register struct ppinstk*	cur; \
		PUSH(IN_QUOTE, cur); \
		cur->nextchr = p; \
		pp.state |= QUOTE; \
		cur->line = error_info.line; \
		error_info.line = n; \
		debug((-7, "PUSH in=%s next=%s", ppinstr(pp.in), pptokchr(*pp.in->nextchr))); \
	} while (0)

#define PUSH_RESCAN(p)	\
	pppush(IN_RESCAN,NiL,p,0)

#define PUSH_SQUOTE(p,n)	\
	do \
	{ \
		register struct ppinstk*	cur; \
		PUSH(IN_SQUOTE, cur); \
		cur->nextchr = p; \
		pp.state |= SQUOTE; \
		cur->line = error_info.line; \
		error_info.line = n; \
		debug((-7, "PUSH in=%s next=%s", ppinstr(pp.in), pptokchr(*pp.in->nextchr))); \
	} while (0)

#define PUSH_STRING(p)		\
	do \
	{ \
		register struct ppinstk*	cur; \
		PUSH(IN_STRING, cur); \
		cur->nextchr = p; \
		if (pp.state & DISABLE) cur->flags |= IN_disable; \
		debug((-7, "PUSH in=%s next=%s", ppinstr(pp.in), pptokchr(*pp.in->nextchr))); \
	} while (0)

#define PUSH_LINE(p)		\
	do \
	{ \
		register struct ppinstk*	cur; \
		PUSH(IN_STRING, cur); \
		cur->nextchr = p; \
		pp.state |= DISABLE|NOSPACE|PASSEOF|STRIP; \
		debug((-7, "PUSH in=%s next=%s", ppinstr(pp.in), pptokchr(*pp.in->nextchr))); \
	} while (0)

#define POP_LINE()		\
	do \
	{ \
		debug((-7, "POP  in=%s", ppinstr(pp.in))); \
		pp.in = pp.in->prev; \
		pp.state &= ~(DISABLE|NOSPACE|PASSEOF|STRIP); \
	} while (0)

struct ppcontext			/* pp context			*/
{
	_PP_CONTEXT_PUBLIC_
	_PP_CONTEXT_PRIVATE_
};

struct ppfile				/* include file info		*/
{
	HASH_HEADER;			/* this is a hash bucket too	*/
	struct ppsymbol* guard;		/* guard symbol			*/
	struct ppfile*	bound[INC_MAX];	/* include bindings		*/
	int		flags;		/* INC_* flags			*/
};

#if CHECKPOINT

struct ppindex				/* checkpoint include index	*/
{
	struct ppindex*	next;		/* next in list			*/
	struct ppfile*	file;		/* include file			*/
	unsigned long	begin;		/* beginning output offset	*/
	unsigned long	end;		/* ending output offset		*/
};

#endif

struct ppsymkey				/* pun for SYM_KEYWORD lex val	*/
{
	struct ppsymbol	sym;		/* symbol as usual		*/
	int		lex;		/* lex value for SYM_KEYWORD	*/
};

#if PROTOMAIN && PROTO_STANDALONE

#if defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)
#define NiL		0
#define NoP(x)		(&x,1)
#else
#define NiL		((char*)0)
#define NoP(x)
#endif

#define newof(p,t,n,x)	((p)?(t*)realloc((char*)(p),sizeof(t)*(n)+(x)):(t*)calloc(1,sizeof(t)*(n)+(x)))

#define size_t		int

extern int		access(const char*, int);
extern void*		calloc(size_t, size_t);
extern int		close(int);
extern int		creat(const char*, int);
extern char*		ctime(time_t*);
extern void		exit(int);
extern void		free(void*);
extern int		link(const char*, const char*);
extern int		open(const char*, int, ...);
extern int		read(int, void*, int);
extern time_t		time(time_t*);
extern int		unlink(const char*);
extern int		write(int, const void*, int);

#else

/*
 * library implementation globals
 */

#define ppassert	_pp_assert
#define ppbuiltin	_pp_builtin
#define ppcall		_pp_call
#define ppcontrol	_pp_control
#define ppdump		_pp_dump
#define ppexpand	_pp_expand
#define ppexpr		_pp_expr
#define ppfsm		_pp_fsm
#define ppinmap		_pp_inmap
#define ppinstr		_pp_instr
#define ppkeyname	_pp_keyname
#define pplexmap	_pp_lexmap
#define pplexstr	_pp_lexstr
#define ppload		_pp_load
#define ppmodestr	_pp_modestr
#define ppmultiple	_pp_multiple
#define ppnest		_pp_nest
#define ppoption	_pp_option
#define ppoptionstr	_pp_optionstr
#define pppclose	_pp_pclose
#define pppdrop		_pp_pdrop
#define pppopen		_pp_popen
#define pppread		_pp_pread
#define pppredargs	_pp_predargs
#define pppush		_pp_push
#define pprefmac	_pp_refmac
#define ppsearch	_pp_search
#define ppstatestr	_pp_statestr
#define pptokstr	_pp_tokstr
#define pptrace		_pp_trace

#endif

extern void		ppassert(int, char*, char*);
extern void		ppbuiltin(void);
extern int		ppcall(struct ppsymbol*, int);
extern int		ppcontrol(void);
extern void		ppdump(void);
extern char*		ppexpand(char*);
extern long		ppexpr(int*);
extern void		ppfsm(int, char*);
extern char*		ppinstr(struct ppinstk*);
extern char*		ppkeyname(int, int);
extern char*		pplexstr(int);
extern void		ppload(char*);
extern char*		ppmodestr(long);
extern int		ppmultiple(struct ppfile*, struct ppsymbol*);
extern void		ppnest(void);
extern int		ppoption(char*);
extern char*		ppoptionstr(long);
extern void		pppclose(char*);
extern int		pppdrop(char*);
extern char*		pppopen(char*, int, char*, char*, char*, char*, int);
extern int		pppread(char*);
extern int		pppredargs(void);
extern void		pppush(int, char*, char*, int);
extern struct ppsymbol*	pprefmac(char*, int);
extern int		ppsearch(char*, int, int);
extern char*		ppstatestr(long);
extern char*		pptokstr(char*, int);
extern void		pptrace(int);

#endif
