/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1984-2003 AT&T Corp.                *
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
*                                                                  *
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * make common definitions
 */

#include <ast.h>
#include <ls.h>
#include <ctype.h>
#include <dirent.h>
#include <fs3d.h>
#include <hash.h>
#include <swap.h>
#include <namval.h>
#include <error.h>
#include <coshell.h>
#include <times.h>
#include <tok.h>
#include <setjmp.h>
#include <sfdisc.h>
#include <sfstr.h>

#if DEBUG
#define	PANIC		(ERROR_PANIC|ERROR_SOURCE|ERROR_SYSTEM),__FILE__,__LINE__
#else
#define	PANIC		ERROR_PANIC
#endif

#define COMMENT		'#'		/* make comment char		*/
#define MARK_CONTEXT	'\002'		/* context mark -- not in input!*/
#define MARK_QUOTE	'\003'		/* quote mark -- not in input!	*/
#define SALT		'#'		/* preprocessor control char	*/
#define VIEWOFFSET	'0'		/* char offset for view==0	*/

#if _WINIX
#define FILE_SPACE	'\001'		/* file name space		*/
#else
#define FILE_SPACE	'?'		/* file name space		*/
#endif

#define ATTRNAME	'.'		/* prefix to name an attribute	*/
#define ATTRSET		'+'		/* prefix to set an attribute	*/
#define ATTRCLEAR	'-'		/* prefix to clear an attribute	*/

#define CMDTRACE	(-6)		/* coshell trace debug level	*/
#define EXPTRACE	(-5)		/* explanation trace debug lev	*/

#undef	atoi
#undef	atol
#undef	bind
#undef	clrbit		/* netbsd has one in <sys/param.h> */
#undef	setbit		/* netbsd has one in <sys/param.h> */

#define bind		bindrule /* avoids possible socket clash */
#define canon(x)	((state.context&&iscontextp(x,state.tmppchar))?(state.tmppchar+1):(state.mam.statix?mamcanon(x):pathcanon(x,0)))
#define clrbit(v,b)	((v)&=~(1L<<(b)))
#define getar(name)	((struct dir*)hashget(table.ar,(name)))
#define getbound(name)	((char*)hashget(table.bound,(name)))
#define getdir(id)	((struct dir*)hashget(table.dir,(char*)(id)))
#define getfile(name)	((struct file*)hashget(table.file,(name)))
#define getold(name)	((char*)hashget(table.oldvalue,(name)))
#define getreg(name)	((int*)hashget(table.regress,(name)))
#define getrule(name)	((struct rule*)hashget(table.rule,(name)))
#define getvar(name)	((struct var*)hashget(table.var,(name)))
#define isaltstate(x)	(*((x)+1)=='+')
#define iscontext(x)	(*(x)==MARK_CONTEXT&&*((x)+strlen(x)-1)==MARK_CONTEXT)
#define iscontextp(x,p)	(*(x)==MARK_CONTEXT&&*(p=(x)+strlen(x)-1)==MARK_CONTEXT)
#define isintvar(x)	(*(x)=='.'&&*((x)+strlen(x))=='.')
#define isstate(x)	(*(x)=='('&&strchr(x,')'))
#define message(x)	do if (error_info.trace < 0) { error x; } while (0)
#define notfile(r)	(((r)->property&(P_attribute|P_functional|P_make|P_operator|P_state|P_use|P_virtual))||((r)->dynamic&D_scope)||(r)->semaphore||((r)->property&P_dontcare)&&((r)->dynamic&D_bound)&&!(r)->time)
#define oldname(r)	do{if(getbound(r->uname))putbound(0,0);if(r->dynamic&D_alias)r->dynamic&=~D_alias;else putrule(r->name,0);r->name=r->uname;r->uname=0;}while(0)
#define putbound(n,d)	hashput(table.bound,(char*)(n),(char*)(d))
#define putar(name,d)	hashput(table.ar,(name),(char*)(d))
#define putdir(id,d)	hashput(table.dir,(char*)(id),(char*)(d))
#define putfile(name,f)	hashput(table.file,(char*)(name),(char*)(f))
#define putold(name,v)	hashput(table.oldvalue,(char*)(name),(char*)(v))
#define putptr(f,p)	(internal.ptr=(char*)(p),sfwrite(f,&internal.ptr,sizeof(internal.ptr)))
#define putreg(name,r)	hashput(table.regress,(char*)(name),(char*)(r))
#define putrule(name,r)	hashput(table.rule,(char*)(name),(char*)(r))
#define putvar(name,v)	hashput(table.var,(char*)(name),(char*)(v))
#define reason(x)	do if(state.user&&(state.explain||error_info.trace<=EXPTRACE))explain x;while(0)
#define ropen(f,m)	((f)==internal.openfile?(internal.openfile=0,internal.openfd):open(f,m))
#define rsfopen(f)	((f)==internal.openfile?(internal.openfile=0,sfnew(NiL,NiL,SF_UNBOUND,internal.openfd,SF_READ)):sfopen(NiL,f,"r"))
#define setbit(v,b)	((v)|=(1L<<(b)))
#define shquote		shellquote /* netbsd has one in <stdlib.h>! */
#define timecmp(a,b)	((a)==(b)?0:cmptime(a,b))
#define timeq(a,b)	!timecmp(a,b)
#define timefix(t)      t-=(t<state.tolerance)?t:state.tolerance
#define trap()		(state.caught?handle():0)
#define tstbit(v,b)	((v)&(1L<<(b)))
#define unviewname(s)	(*(s)='(')
#define viewable(r)	((r->property&P_state)&&*r->name=='(')
#define viewname(s,v)	(*(s)=VIEWOFFSET+(v))
#define zero(x)		memzero(&(x),sizeof(x))

#define freelist(x)	do{if(x){(x)->rule=(struct rule*)internal.freelists;internal.freelists=(char*)(x);}}while(0)
#define freerule(r)	do{zero(*r);*((char**)r)=internal.freerules;internal.freerules=(char*)(r);}while(0)
#define freevar(v)	do{(v)->property&=(V_free|V_import);*((char**)v)=internal.freevars;internal.freevars=(char*)(v);}while(0)

#define newlist(x)	do{if(x=(struct list*)internal.freelists){if(x->next){x=x->next;*((char**)internal.freelists)=(char*)x->next;}else internal.freelists=(char*)x->rule;}else x=(struct list*)newchunk(&internal.freelists,sizeof(struct list));}while(0)
#define newrule(r)	do{if(r=(struct rule*)internal.freerules){internal.freerules=(*((char**)r));zero(*r);}else r=(struct rule*)newchunk(&internal.freerules,sizeof(struct rule));}while(0)
#define newvar(v)	do{if(v=(struct var*)internal.freevars){internal.freevars=(*((char**)v));}else v=(struct var*)newchunk(&internal.freevars,sizeof(struct var));}while(0)

#if CHAR_MIN < 0
#define ctable		(ctypes-(CHAR_MIN)+1)
#else
#define ctable		(ctypes)
#endif

#define istype(c,t)	(ctable[c]&(t))
#define settype(c,t)	(ctable[c]|=(t))
#define unsettype(c,t)	(ctable[c]&=~(t))

#define NOTYET		0	/* don't know what to do yet		*/
#define UPDATE		1	/* rule in process of being updated	*/
#define MAKING		2	/* executing update action		*/
#define TOUCH		3	/* archive member to be touched		*/
#define EXISTS		4	/* rule already exists in desired state	*/
#define IGNORE		5	/* rule make failed but ignore errors	*/
#define FAILED		6	/* rule make failed			*/
#define OLDRULE		7	/* makefile compiler old rule mark	*/

#define RECOMPILE	1	/* recompile make object		*/
#define COMPILED	2	/* make object compiled (if necessary)	*/
#define SAVED		3	/* make state saved			*/

#define DELETE	NiL		/* delete path component in edit()	*/
#define KEEP	((char*)1)	/* keep path component in edit()	*/

#define NOTIME	(unsigned long)(-1)	/* not checked time		*/
#define OLDTIME	(unsigned long)(1)	/* oldest valid time		*/
#define CURTIME	(unsigned long)time(NiL)/* current time			*/

/*
 * VAR and RULE must not change -- the rest must be in sequence
 */

#define VAR		0	/* state var from var in staterule()	*/
#define CONSISTENT	VAR	/* consistency check bits		*/
#define RULE		1	/* state rule in staterule()		*/
#define PREREQS		2	/* alternate prereqs in staterule()	*/
#define STATERULES	2	/* last staterule() index		*/

#define MINVALUE	32	/* minimum variable value length	*/
#define MAXJOBS		128	/* maximum number concurrent jobs	*/
#define MAXNAME		1024	/* maximum file pathname length		*/
#define PCTGARBAGE	10	/* maximum state garbage percentage	*/

#define MAXVIEW		(sizeof(viewvector)*CHAR_BIT) /* max view index	*/

#if BINDINDEX
#define MAXBIND		UCHAR_MAX/* maximum bind index			*/
#endif

#define A_clear		(1<<0)	/* assertion() .CLEAR			*/
#define A_copy		(1<<1)	/* assertion() .COPY			*/
#define A_delete	(1<<2)	/* assertion() .DELETE			*/
#define A_group		(1<<3)	/* assertion() metarule (...) grouping	*/
#define A_insert	(1<<4)	/* assertion() .INSERT			*/
#define A_metarule	(1<<5)	/* assertion() pattern metarule 	*/
#define A_negate	(1<<6)	/* assertion() -attribute		*/
#define A_norhs		(1<<7)	/* assertion() empty rhs		*/
#define A_null		(1<<8)	/* assertion() .NULL			*/
#define A_scan		(1<<9)	/* assertion() .SCAN			*/
#define A_scope		(1<<10)	/* assertion() metarule scope prereqs	*/
#define A_special	(1<<11)	/* assertion() .SPECIAL		*/

#define C_ID1		(1<<0)	/* istype() first identifier char	*/
#define C_ID2		(1<<1)	/* istype() remaining id chars		*/
#define C_MATCH		(1<<2)	/* istype() shell pattern match chars	*/
#define C_OPTVAL	(1<<3)	/* istype() option value separator	*/
#define C_SEP		(1<<4)	/* istype() token separator		*/
#define C_TERMINAL	(1<<5)	/* istype() terminal chars		*/
#define C_VARIABLE1	(1<<6)	/* istype() first variable name char	*/
#define C_VARIABLE2	(1<<7)	/* istype() remaining variable chars	*/

#define C_VARPOS1	(1<<8)	/* istype() var superimposed code 1	*/
#define C_VARPOS2	(1<<9)	/* istype() var superimposed code 2	*/
#define C_VARPOS3	(1<<10)	/* istype() var superimposed code 3	*/
#define C_VARPOS4	(1<<11)	/* istype() var superimposed code 4	*/
#define C_VARPOS5	(1<<12)	/* istype() var superimposed code 5	*/
#define C_VARPOS6	(1<<13)	/* istype() var superimposed code 6	*/
#define C_VARPOS7	(1<<14)	/* istype() var superimposed code 7	*/
#define C_VARPOS8	(1L<<15)/* istype() var superimposed code 8	*/

#define ARG_ASSIGN	(1<<0)	/* command line assignment arg flag	*/
#define ARG_SCRIPT	(1<<1)	/* command line script arg flag		*/
#define ARG_TARGET	(1<<2)	/* command line target arg flag		*/

#define BIND_DOT	(1<<0)	/* bindfile in .			*/
#define BIND_FORCE	(1<<1)	/* force bindfile current time		*/
#define BIND_MAKEFILE	(1<<2)	/* bindfile using makefile dirs		*/
#define BIND_RULE	(1<<3)	/* force bindfile makerule		*/

#define MERGE_ALL	(1<<0)	/* merge everything			*/
#define MERGE_ATTR	(1<<1)	/* merge just attributes		*/
#define MERGE_BOUND	(1<<2)	/* MERGE_ALL but no bind		*/
#define MERGE_FORCE	(1<<3)	/* override attributes			*/
#define MERGE_SCANNED	(1<<4)	/* MERGE_ALL but no entries|scanned	*/

#define COMP_BASE	(1<<0)	/* base rules prereq			*/
#define COMP_DONTCARE	(1<<1)	/* optional include prereq		*/
#define COMP_FILE	(1<<2)	/* -f prereq				*/
#define COMP_GLOBAL	(1<<3)	/* -g prereq				*/
#define COMP_INCLUDE	(1<<4)	/* include prereq			*/
#define COMP_OPTIONS	(1<<5)	/* -[DIU]* prereq			*/

#define PREREQ_APPEND	1	/* addprereq append			*/
#define PREREQ_DELETE	2	/* addprereq delete			*/
#define PREREQ_INSERT	3	/* addprereq insert			*/
#define PREREQ_LENGTH	4	/* addprereq insert by length		*/

#define SCAN_IGNORE	1	/* .SCAN.IGNORE scan index		*/
#define SCAN_NULL	2	/* .SCAN.NULL scan index		*/
#define SCAN_STATE	3	/* .SCAN.STATE scan index		*/
#define SCAN_USER	8	/* first user defined scan index	*/
#define SCAN_MAX	UCHAR_MAX/* max scan index			*/

#define CO_ALWAYS	(CO_USER<<0)	/* always exec			*/
#define CO_DATAFILE	(CO_USER<<1)	/* job output to state.tmpdata	*/
#define CO_ERRORS	(CO_USER<<2)	/* job had errors		*/
#define CO_FOREGROUND	(CO_USER<<3)	/* wait for completion		*/
#define CO_KEEPGOING	(CO_USER<<4)	/* keep going on job error	*/
#define CO_LOCALSTACK	(CO_USER<<5)	/* stack local vars for action	*/
#define CO_PRIMARY	(CO_USER<<6)	/* primary prereq added		*/
#define CO_URGENT	(CO_USER<<7)	/* enter job at top of queue	*/

/*
 * rule.property flags
 */

#define P_accept	(1<<0)		/* ignore state conflicts	*/
#define P_after		(1<<1)		/* make after parent update	*/
#define P_always	(1<<2)		/* execute even if !state.exec	*/
#define P_archive	(1<<3)		/* rule bound to archive file	*/
#define P_attribute	(1<<4)		/* rule is an attribute		*/
#define P_before	(1<<5)		/* make before parent update	*/
#define P_command	(1<<6)		/* command target -- no pattern	*/
#define P_dontcare	(1<<7)		/* don't care if rule made	*/
#define P_force		(1<<8)		/* target is always out of date	*/
#define P_foreground	(1<<9)		/* run action in foreground	*/
#define P_functional	(1<<10)		/* associated w/functional var	*/
#define P_ignore	(1<<11)		/* parent to ignore this prereq	*/
#define P_immediate	(1<<12)		/* rule needs immediate action	*/
#define P_implicit	(1<<13)		/* force implicit prereq checks	*/
#define P_internal	(1<<14)		/* don't compile unless prereq	*/
#define P_joint		(1L<<15)	/* pseudo for joint targets	*/
#define P_make		(1L<<16)	/* make (not shell) action	*/
#define P_metarule	(1L<<17)	/* metarule			*/
#define P_multiple	(1L<<18)	/* multi prereq occurrences OK	*/

#define P_local		(1L<<19)	/* local affinity		*/

#define P_operator	(1L<<20)	/* rule is an operator		*/
#define P_parameter	(1L<<21)	/* rule bound to parameter file	*/
#define P_readonly	(1L<<22)	/* no user modifications	*/
#define P_repeat	(1L<<23)	/* make even if already made	*/
#define P_state		(1L<<24)	/* state atom			*/
#define P_staterule	(1L<<25)	/* staterule			*/
#define P_statevar	(1L<<26)	/* statevar			*/
#define P_target	(1L<<27)	/* rule is an explicit target	*/
#define P_terminal	(1L<<28)	/* terminal target or metarule	*/
#define P_use		(1L<<29)	/* rule is a .USE script	*/
#define P_virtual	(1L<<30)	/* target is not a file		*/

#define P_read		(1L<<31)	/* read action output		*/

/*
 * rule.dynamic flags
 */

#define D_alias		(1<<0)		/* more than one unbound name	*/
#define D_aliaschanged	(1<<1)		/* alias changed		*/
#define D_bound		(1<<2)		/* rule has been bound		*/
#define D_built		(1<<3)		/* triggered action built target*/
#define D_cached	(1<<4)		/* post assertion info cached	*/
#define D_compiled	(1<<5)		/* rule has been compiled	*/
#define D_dynamic	(1<<6)		/* must do dynamic expansion	*/
#define D_entries	(1<<7)		/* scanned rule has entries	*/
#define D_garbage	(1<<8)		/* state file GC mark		*/
#define D_hasafter	(1<<9)		/* rule has after prereqs	*/
#define D_hasbefore	(1<<10)		/* rule has before prereqs	*/

#define D_membertoo	(1<<11)		/* D_member was also set	*/
#define D_hassemaphore	(1<<12)		/* has rule.semaphore prereq	*/
#define D_same		(1<<13)		/* target unchanged by action	*/

#define D_lower		(1<<14)		/* state from lower view	*/

#define D_source	(1L<<15)	/* .SOURCE directory		*/

#define D_member	(1L<<16)	/* rule bound to archive member	*/

#define D_global	(1L<<17)	/* global view if view==0	*/

#define D_regular	(1L<<18)	/* rule bound to regular file	*/
#define D_scanned	(1L<<19)	/* has been scanned		*/
#define D_select0	(1L<<20)	/* $(...) select bit 0		*/
#define D_select1	(1L<<21)	/* $(...) select bit 1		*/
#define D_triggered	(1L<<22)	/* rule action triggered	*/

#define D_index		(1L<<23)	/* load time index consistency	*/

#define D_bindindex	(1L<<24)	/* bind index table entry	*/

#define D_intermediate	(1L<<25)	/* intermediate pretend target	*/

#define D_hasscope	(1L<<26)	/* has D_scope prereqs		*/
#define D_scope		(1L<<27)	/* scoped var assignment	*/

#define D_hasmake	(1L<<28)	/* rule has .MAKE after prereqs	*/

#define D_context	(1L<<29)	/* ref may be diff dir context	*/

#define D_CLEAROBJECT	(~(D_bindindex|D_built|D_compiled|D_context|D_dynamic|D_index|D_lower|D_scope))

#define M_bind		(1<<0)		/* bind recursion mark		*/
#define M_compile	(1<<1)		/* compilation mark		*/
#define M_directory	(1<<2)		/* bind directory mark		*/
#define M_generate	(1<<3)		/* prereq generation mark	*/
#define M_mark		(1<<4)		/* temporary mark		*/
#define M_metarule	(1<<5)		/* metarule closure mark	*/
#define M_scan		(1<<6)		/* scan recursion mark		*/
#define M_waiting	(1<<7)		/* waiting to complete mark	*/

/*
 * var.property flags
 */

#define V_compiled	(1<<0)		/* variable has been compiled	*/
#define V_frozen	(1<<1)		/* frozen in make object file	*/
#define V_functional	(1<<2)		/* make rule name before access	*/
#define V_import	(1<<3)		/* imported from environment	*/
#define V_local_D	(1<<4)		/* :T=D: localview reference	*/
#define V_oldvalue	(1<<5)		/* compile old value		*/
#define V_readonly	(1<<6)		/* only dynamic modifications	*/
#define V_retain	(1<<7)		/* retain value in state file	*/
#define V_scan		(1<<8)		/* scan for implicit state var	*/

#define V_local_E	(1<<9)		/* :T=E: localview reference	*/

#define V_auxiliary	(1<<10)		/* auxiliary &= assignment	*/
#define V_append	(1<<11)		/* cmd line += property or op	*/
#define V_free		(1<<12)		/* value may be freed		*/
#define V_builtin	(1<<13)		/* builtin V_functional		*/
#define V_scope		(1<<14)		/* scoped value			*/

#define V_restored	(1L<<15)	/* retained value stored	*/

#define V_CLEARSTATE	(V_free|V_restored)
#define V_CLEAROBJECT	(~(V_auxiliary|V_builtin|V_functional|V_import|V_retain|V_scan|V_scope))

/*
 * getval() flags
 */

#define VAL_AUXILIARY	(1<<0)		/* auxilliary value		*/
#define VAL_FILE	(1<<1)		/* !notfile(r)			*/
#define VAL_PRIMARY	(1<<2)		/* primary value		*/
#define VAL_UNBOUND	(1<<3)		/* unbound name			*/

struct fileid				/* unique file id		*/
{
	long		dev;		/* device number		*/
	long		ino;		/* inode number			*/
};

struct dir				/* scanned directory entry	*/
{
	char*		name;		/* directory name		*/
	unsigned long	time;		/* modify time			*/
	unsigned char	archive;	/* directory is an archive	*/
	unsigned char	directory;	/* directory is a real directory*/
	unsigned char	ignorecase;	/* pox on dirs that ignore case	*/
	unsigned char	truncate;	/* names truncated to this	*/
};

struct file				/* file table entry		*/
{
	struct file*	next;		/* next in list			*/
	struct dir*	dir;		/* directory containing file	*/
	unsigned long	time;		/* modify time			*/
};

typedef int viewvector;			/* view consistency bit vector	*/

/*
 * statevar data, staterule sync time and unbound rule name -- shared in rule.u1
 */

#define event		u1.u_event
#define statedata	u1.u_data
#define uname		u1.u_uname
#define unbound(r)	((r)->uname?(r)->uname:(r)->name)

struct rule				/* rule				*/
{
	char*		name;		/* rule name			*/
	struct frame*	active;		/* active target frame		*/

	union
	{
	char*		u_uname;	/* unbound name			*/
	char*		u_data;		/* state value			*/
	unsigned long	u_event;	/* state rule event time	*/
	}		u1;

	struct list*	prereqs;	/* prerequisites		*/
	char*		action;		/* update action		*/
	unsigned long	time;		/* modify time			*/

	long		attribute;	/* external named attributes	*/
	long		dynamic;	/* dynamic properties		*/
	long		property;	/* stable properties		*/

	unsigned char	scan;		/* file scan strategy index	*/
	unsigned char	semaphore;	/* semaphore + count		*/
	unsigned char	status;		/* disposition			*/
	unsigned char	view;		/* view bind index		*/

	unsigned char	mark;		/* M_* marks			*/
	unsigned char	preview;	/* min prereq view		*/
	unsigned short	must;		/* cancel if == 0		*/

	unsigned long	complink;	/* compilation link		*/
	viewvector	checked[STATERULES+1];	/* view state check	*/

#if BINDINDEX
	unsigned char	source;		/* source bind index		*/
#endif
};

struct frame				/* active target frame		*/
{
	struct frame*	parent;		/* parent frame			*/
	struct frame*	previous;	/* previous active frame	*/
	struct rule*	target;		/* target in frame		*/
	struct list*	prereqs;	/* original prereqs		*/
	char*		action;		/* original action		*/
	char*		original;	/* original bound name		*/
	char*		primary;	/* metarule primary prereq name	*/
	char*		stem;		/* metarule stem		*/

	struct
	{
	char*		name;		/* original target name		*/
	struct frame*	frame;		/* original target frame	*/
	unsigned long	time;		/* original target time		*/
	}		context;	/* context push/pop		*/
};

struct internal				/* internal rule and list info	*/
{
	/*
	 * read/write rule attributes
	 */

	struct rule*	accept;		/* .ACCEPT rule pointer		*/
	struct rule*	after;		/* .AFTER rule pointer		*/
	struct rule*	alarm;		/* .ALARM rule pointer		*/
	struct rule*	always;		/* .ALWAYS rule pointer		*/
	struct rule*	archive;	/* .ARCHIVE rule pointer	*/
	struct rule*	attribute;	/* .ATTRIBUTE rule pointer	*/
	struct rule*	before;		/* .BEFORE rule pointer		*/
	struct rule*	command;	/* .COMMAND rule pointer	*/
	struct rule*	dontcare;	/* .DONTCARE rule pointer	*/
	struct rule*	force;		/* .FORCE rule pointer		*/
	struct rule*	foreground;	/* .FOREGROUND rule pointer	*/
	struct rule*	functional;	/* .FUNCTIONAL rule pointer	*/
	struct rule*	ignore;		/* .IGNORE rule pointer		*/
	struct rule*	immediate;	/* .IMMEDIATE rule pointer	*/
	struct rule*	implicit;	/* .IMPLICIT rule pointer	*/
	struct rule*	insert;		/* .INSERT rule pointer		*/
	struct rule*	joint;		/* .JOINT rule pointer		*/
	struct rule*	local;		/* .LOCAL rule pointer		*/
	struct rule*	make;		/* .MAKE rule pointer		*/
	struct rule*	making;		/* .MAKING rule pointer		*/
	struct rule*	multiple;	/* .MULTIPLE rule pointer	*/
	struct rule*	op;		/* .OPERATOR rule pointer	*/
	struct rule*	parameter;	/* .PARAMETER rule pointer	*/
	struct rule*	read;		/* .READ rule pointer		*/
	struct rule*	readonly;	/* .READONLY rule pointer	*/
	struct rule*	regular;	/* .REGULAR rule pointer	*/
	struct rule*	repeat;		/* .REPEAT rule pointer		*/
	struct rule*	run;		/* .RUN rule pointer		*/
	struct rule*	semaphore;	/* .SEMAPHORE rule pointer	*/
	struct rule*	source;		/* .SOURCE rule pointer		*/
	struct rule*	state;		/* .STATE rule pointer		*/
	struct rule*	sync;		/* .SYNC rule pointer		*/
	struct rule*	terminal;	/* .TERMINAL rule pointer	*/
	struct rule*	use;		/* .USE rule pointer		*/
	struct rule*	virt;		/* .VIRTUAL rule pointer	*/
	struct rule*	wait;		/* .WAIT rule pointer		*/

	/*
	 * readonly rule attributes
	 */
	 
	struct rule*	active;		/* .ACTIVE rule pointer		*/
	struct rule*	bound;		/* .BOUND rule pointer		*/
	struct rule*	built;		/* .BUILT rule pointer		*/
	struct rule*	entries;	/* .ENTRIES rule pointer	*/
	struct rule*	exists;		/* .EXISTS rule pointer		*/
	struct rule*	failed;		/* .FAILED rule pointer		*/
	struct rule*	file;		/* .FILE rule pointer		*/
	struct rule*	member;		/* .MEMBER rule pointer		*/
	struct rule*	notyet;		/* .NOTYET rule pointer		*/
	struct rule*	scanned;	/* .SCANNED rule pointer	*/
	struct rule*	staterule;	/* .STATERULE rule pointer	*/
	struct rule*	statevar;	/* .STATEVAR rule pointer	*/
	struct rule*	target;		/* .TARGET rule pointer		*/
	struct rule*	triggered;	/* .TRIGGERED rule pointer	*/

	/*
	 * special rules and names
	 */

	struct rule*	args;		/* .ARGS rule pointer		*/
 	struct rule*	assert;		/* .ASSERT rule pointer		*/
 	struct rule*	assign;		/* .ASSIGN rule pointer		*/
	struct rule*	bind;		/* .BIND rule pointer		*/
	struct rule*	clear;		/* .CLEAR rule pointer		*/
	struct rule*	copy;		/* .COPY rule pointer		*/
	struct rule*	delete;		/* .DELETE rule pointer		*/
	struct rule*	dot;		/* . rule pointer		*/
	struct rule*	empty;		/* "" rule pointer		*/
	struct rule*	error;		/* error intercept rule pointer	*/
	struct rule*	exported;	/* .EXPORT rule pointer		*/
	struct rule*	globalfiles;	/* .GLOBALFILES rule pointer	*/
	struct rule*	include;	/* .INCLUDE rule pointer	*/
	struct rule*	internal;	/* .INTERNAL rule pointer	*/
	struct rule*	main;		/* .MAIN rule pointer		*/
	struct rule*	makefiles;	/* .MAKEFILES rule pointer	*/
	struct rule*	metarule;	/* .METARULE rule pointer	*/
	struct rule*	null;		/* .NULL rule pointer		*/
	struct rule*	options;	/* .OPTIONS rule pointer	*/
	struct rule*	preprocess;	/* .PREPROCESS rule pointer	*/
	struct rule*	query;		/* .QUERY rule pointer		*/
	struct rule*	rebind;		/* .REBIND rule pointer		*/
	struct rule*	reset;		/* .RESET rule pointer		*/
	struct rule*	retain;		/* .RETAIN rule pointer		*/
	struct rule*	scan;		/* .SCAN rule pointer		*/
	struct rule*	special;	/* .SPECIAL rule pointer	*/
	struct rule*	tmplist;	/* .TMPLIST rule pointer	*/
	struct rule*	unbind;		/* .UNBIND rule pointer		*/
	struct rule*	view;		/* .VIEW rule pointer		*/

	/*
	 * pattern association rules
	 */

	struct rule*	append_p;	/* .APPEND. rule pointer	*/
	struct rule*	attribute_p;	/* .ATTRIBUTE. rule pointer	*/
	struct rule*	bind_p;		/* .BIND. rule pointer		*/
	struct rule*	dontcare_p;	/* .DONTCARE. rule pointer	*/
	struct rule*	insert_p;	/* .INSERT. rule pointer	*/
	struct rule*	require_p;	/* .REQUIRE. rule pointer	*/
	struct rule*	source_p;	/* .SOURCE. rule pointer	*/

	/*
	 * builtin functions
	 */

	struct var*	getconf;	/* .GETCONF var pointer		*/
	struct var*	outstanding;	/* .OUTSTANDING var pointer	*/

	/*
	 * miscellaneous internal info
	 */

	Sfio_t*		met;		/* metarule expansion buffer	*/
	Sfio_t*		nam;		/* name generation buffer	*/
	Sfio_t*		tmp;		/* very temporary work buffer	*/
	Sfio_t*		val;		/* initial getval return buffer	*/

	char*		freelists;	/* free lists list		*/
	char*		freerules;	/* free rules list		*/
	char*		freevars;	/* free variables list		*/

	char*		issource;	/* internal.source* match pat	*/
	char*		openfile;	/* bind()-scan() optimization	*/
	char*		pwd;		/* PWD value			*/
	char*		ptr;		/* temporary for sfstrptr()	*/

	int		openfd;		/* bind()-scan() optimization	*/
	int		pwdlen;		/* strlen(internal.pwd)		*/
};

struct external				/* external engine name info	*/
{
	/*
	 * names of variables defined by engine, init, or environment
	 */

	char*		args;		/* candidate args file name(s)	*/
	char*		convert;	/* makefile converter patterns	*/
	char*		file;		/* main input makefile name	*/
	char*		files;		/* candidate makefile name(s)	*/
	char*		import;		/* explicit env override vars	*/
	char*		lib;		/* related file lib directory	*/
	char*		make;		/* program path name		*/
	char*		nproc;		/* # jobs for compatibility	*/
	char*		old;		/* old program path name	*/
	char*		pwd;		/* pwd name			*/
	char*		rules;		/* candidate rules file name(s)	*/
	char*		skip;		/* order directory skip pattern	*/
	char*		version;	/* engine version stamp		*/
	char*		viewdot;	/* . view dir list		*/
	char*		viewnode;	/* view node dir list		*/

	/*
	 * infrequently used engine interface names
	 */

	char*		compdone;	/* made after makefile compiled	*/
	char*		compinit;	/* made before makefile compiled*/
	char*		done;		/* made just before exit	*/
	char*		init;		/* made before first user target*/
	char*		interrupt;	/* made on first interrupt	*/
	char*		jobdone;	/* made when each job done	*/
	char*		makedone;	/* made after done		*/
	char*		makeinit;	/* made after before init	*/
	char*		makeprompt;	/* made just before each prompt	*/
	char*		makerun;	/* made just before each job	*/
	char*		mamname;	/* external mam atom name	*/
	char*		mamaction;	/* external mam action		*/

	/*
	 * related file suffixes
	 */

	char*		lock;		/* make lock file suffix	*/
	char*		object;		/* make object file suffix	*/
	char*		source;		/* make source file suffix	*/
	char*		state;		/* make state file suffix	*/
};

struct tables				/* hash table pointers		*/
{
	Hash_table_t*	ar;		/* archives dir info by name	*/
	Hash_table_t*	bound;		/* directory of bound file	*/
	Hash_table_t*	dir;		/* directories and archives	*/
	Hash_table_t*	file;		/* files from scanned dirs	*/
	Hash_table_t*	oldvalue;	/* old variable values		*/
	Hash_table_t*	regress;	/* regression path maps		*/
	Hash_table_t*	rule;		/* rule names			*/
	Hash_table_t*	var;		/* variable names		*/
};

#define BIND_EXISTS	(1<<0)		/* statefile loaded		*/
#define BIND_LOADED	(1<<1)		/* statefile loaded		*/

struct binding				/* binding info			*/
{
#if BINDINDEX
	struct rule*	path;		/* path name component		*/
#else
	char*		path;		/* path name component		*/
#endif
	char*		root;		/* path root			*/
	short		pathlen;	/* path length			*/
	short		rootlen;	/* root length			*/
	unsigned char	flags;		/* BIND_* flags			*/
	unsigned char	map;		/* external index map		*/
};

struct label				/* resume label			*/
{
	jmp_buf		label;
};

struct mam				/* mam state			*/
{
	int		level;		/* next error() message level	*/
	int		parent;		/* mam parent label		*/
	int		rootlen;	/* strlen(state.mam.root)	*/

	char*		label;		/* instruction label		*/
	char*		options;	/* option string		*/
	char*		root;		/* names relative to this root	*/
	char*		type;		/* mam type name		*/

	Sfio_t*		out;		/* output stream pointer	*/

	unsigned char	dynamic;	/* dynamic mam			*/
	unsigned char	regress;	/* regression mam		*/
	unsigned char	statix;		/* static mam			*/

	unsigned char	dontcare;	/* emit dontcare rules too	*/
	unsigned char	port;		/* emit porting hints		*/
};

struct state				/* program state		*/
{
	unsigned char	accept;		/* accept all existing targets	*/
	unsigned char	alias;		/* enable directory aliasing	*/
	unsigned char	base;		/* compile base|global rules	*/
	unsigned char	caught;		/* a signal was caught		*/
	unsigned char	compile;	/* make object compile state	*/
	unsigned char	compileonly;	/* only compile (force)		*/
	unsigned char	compatibility;	/* disable compatibility msgs	*/
	unsigned char	cross;		/* don't run gen'd executables	*/
	unsigned char	exec;		/* execute shell actions	*/
	unsigned char	expandview;	/* expand paths if fsview!=0	*/
	unsigned char	explain;	/* explain reason for actions	*/
	unsigned char	finish;		/* in finish()			*/
	unsigned char	force;		/* force target updates		*/
	unsigned char	forceread;	/* force makefiles to be read	*/
	unsigned char	forcescan;	/* force implicit prereq scan	*/
	unsigned char	fsview;		/* file system handles views	*/
	unsigned char	fullscan;	/* scan for impl state prereqs	*/
	unsigned char	global;		/* reading global rules		*/
	unsigned char	ignore;		/* ignore sh action errors	*/
	unsigned char	ignorelock;	/* ignore state lock		*/
	unsigned char	init;		/* engine initialization	*/
	unsigned char	intermediate;	/* force intermediate targets	*/
	unsigned char	interpreter;	/* in interpreter main loop	*/
	unsigned char	keepgoing;	/* continue w/ sibling prereqs	*/
	unsigned char	list;		/* list readable definitions	*/
	unsigned char	localview;	/* automatics to local view	*/
#if BINDINDEX
	unsigned char	logical;	/* emit logical pathnames	*/
#endif
	unsigned char	never;		/* really - don't exec anything	*/
	unsigned char	op;		/* currently parsing operator	*/
	unsigned char	override;	/* override explicit rules	*/
	unsigned char	preprocess;	/* preprocess all makefiles	*/
	unsigned char	reading;	/* currently reading makefile	*/
	unsigned char	readonly;	/* current vars|opts readonly	*/
	unsigned char	ruledump;	/* dump rule information	*/
	unsigned char	savestate;	/* must save state variables	*/
	unsigned char	scan;		/* scan|check implicit prereqs	*/
	unsigned char	silent;		/* run silently			*/
	unsigned char	strictview;	/* strict views			*/
	unsigned char	targetcontext;	/* expand in target dir context	*/
	unsigned char	touch;		/* touch out of date targets	*/
	unsigned char	user;		/* user activities started	*/
	unsigned char	val;		/* internal.val in use		*/
	unsigned char	vardump;	/* dump variable information	*/
	unsigned char	virtualdot;	/* fsview . is virtual		*/
	unsigned char	waiting;	/* waiting for job completion	*/
	unsigned char	warn;		/* enable source file warnings	*/
	unsigned char	writeobject;	/* write recompiled object file	*/
	unsigned char	writestate;	/* write state file on exit	*/

	int		argc;		/* global argc			*/
	int		believe;	/* believe state from this level*/
	int		errors;		/* keepgoing error count	*/
	int		interrupt;	/* interrupt causing exit	*/
	int		jobs;		/* sh action concurrency level	*/
	int		pid;		/* make pid			*/
	int		readstate;	/* state files to this view ok	*/
	int		reread;		/* input makefile reread count	*/
	int		stateview;	/* state file view index	*/
	int		tabstops;	/* tab stops for makefile parse	*/
	int		targetview;	/* most recent active targ view	*/
	int		tolerance;	/* time comparison tolerance	*/
	int		unwind;		/* make() dontcare unwind level	*/

	unsigned long	questionable;	/* questionable code enable bits*/
	unsigned long	test;		/* test code enable bits	*/
	unsigned long	start;		/* start time of this make	*/

	char*		corrupt;	/* corrupt state file action	*/
	char*		errorid;	/* error message id		*/
	char*		hold;		/* hold error trap		*/
	char*		makefile;	/* first makefile name		*/
	char*		objectfile;	/* make object file name	*/
	char*		rules;		/* base rules base name		*/
	char*		statefile;	/* state variable file name	*/
	char*		tmppchar;	/* macro char* temporary	*/
	char*		tmpfile;	/* temporary file name		*/

	int*		argf;		/* global argv ARG_* flags	*/

	char**		argv;		/* global argv			*/

	struct dir*	archive;	/* .SCAN archive		*/

	Sfio_t*		context;	/* localview() target context	*/

	struct frame*	frame;		/* current target frame		*/

#if BINDINDEX
	struct binding	source[MAXBIND+1];/* source bind table		*/
	int		maxsource;	/* max source bind index	*/
#endif
	struct binding	view[MAXVIEW+1];/* view bind table		*/
	unsigned int	maxview;	/* max view bind index		*/

	struct label	resume;		/* if interpreter!=0		*/

	struct mam	mam;		/* mam state			*/

	Coshell_t*	coshell;	/* coshell handle		*/

	Sfio_t*		io[11];		/* print/read streams		*/
};

struct var				/* variable			*/
{
	char*		name;		/* name				*/
	char*		value;		/* value			*/
	long		property;	/* static and dynamic		*/
	long		length;		/* maximum length of value	*/
	char*		(*builtin)(char**);	/* builtin function	*/
};

struct list				/* rule cons cell		*/
{
	struct list*	next;		/* next in list			*/
	struct rule*	rule;		/* list item			*/
};

/*
 * make globals
 */

extern struct external	external;	/* external engine names	*/
extern struct internal	internal;	/* internal rule and list info	*/
extern struct state	state;		/* engine state			*/
extern struct tables	table;		/* hash table pointers		*/

extern char		null[];		/* null string			*/
extern char		tmpname[];	/* temporary name buffer	*/

extern short		ctypes[];	/* internal character types	*/

extern char*		idname;		/* interface id name		*/
extern char*		initdynamic;	/* dynamic initialization	*/
extern char*		initstatic;	/* static initialization	*/
extern char*		version;	/* program version stamp	*/

/*
 * make routines
 */

extern struct file*	addfile(struct dir*, char*, unsigned long);
extern void		addprereq(struct rule*, struct rule*, int);
extern struct list*	append(struct list*, struct list*);
extern int		apply(struct rule*, char*, char*, char*, unsigned long);
extern void		argcount(void);
extern void		arscan(struct rule*);
extern void		artouch(char*, char*);
extern char*		arupdate(char*);
extern struct rule*	associate(struct rule*, struct rule*, char*, struct list**);
extern struct var*	auxiliary(char*, int);
extern char*		b_getconf(char**);
extern char*		b_outstanding(char**);
extern struct rule*	bind(struct rule*);
extern void		bindattribute(struct rule*);
extern struct rule*	bindfile(struct rule*, char*, int);
extern struct rule*	bindstate(struct rule*, char*);
extern int		block(int);
extern void		candidates(void);
extern char*		call(struct rule*, char*);
extern struct rule*	catrule(char*, char*, char*, int);
extern int		cmptime(unsigned long, unsigned long);
extern char*		colonlist(Sfio_t*, char*, int);
extern void		compile(char*, char*);
extern int		complete(struct rule*, struct list*, unsigned long*, long);
extern void		compref(int, const char*, unsigned long);
extern struct list*	cons(struct rule*, struct list*);
extern void		dirscan(struct rule*);
extern void		drop(void);
extern void		dump(Sfio_t*, int);
extern void		dumpaction(Sfio_t*, const char*, char*, const char*);
extern void		dumpjobs(int);
extern void		dumpregress(Sfio_t*, const char*, const char*, char*);
extern void		dumprule(Sfio_t*, struct rule*);
extern void		dumpvar(Sfio_t*, struct var*);
extern void		dynamic(struct rule*);
extern void		edit(Sfio_t*, char*, char*, char*, char*);
extern void		expand(Sfio_t*, char*);
extern void		explain(int, ...);
extern long		expr(Sfio_t*, char*);
extern Sfio_t*		fapply(struct rule*, char*, char*, char*, unsigned long);
extern void		finish(int);
extern int		forcescan(const char*, char* v, void*);
extern char*		getarg(char**, int*);
extern void		getop(Sfio_t*, char*, int);
extern char*		getval(char*, int);
extern char**		globv(char*);
extern int		handle(void);
extern int		hasattribute(struct rule*, struct rule*, struct rule*);
extern void		immediate(struct rule*);
extern void		initcode(void);
extern void		inithash(void);
extern void		initrule(void);
extern void		initscan(int);
extern void		inittrap(void);
extern void		initview(void);
extern void		initwakeup(int);
extern void		interpreter(char*);
extern int		isdynamic(const char*);
extern int		isglob(const char*);
extern int		isstatevar(const char*);
extern struct list*	joint(struct rule*);
extern struct list*	listcopy(struct list*);
extern void		listops(Sfio_t*, int);
extern int		load(Sfio_t*, const char*, int);
extern int		loadable(Sfio_t*, struct rule*, int);
extern void		localvar(Sfio_t*, struct var*, char*, int);
extern char*		localview(struct rule*);
extern void		lockstate(char*);
extern int		make(struct rule*, unsigned long*, char*, long);
extern int		makeafter(struct rule*);
extern int		makebefore(struct rule*);
extern struct rule*	makerule(char*);
extern void		maketop(struct rule*, int, char*);
extern char*		mamcanon(char*);
extern ssize_t		mamerror(int, const void*, size_t);
extern char*		mamname(struct rule*);
extern void		mampop(Sfio_t*, struct rule*, long);
extern int		mampush(Sfio_t*, struct rule*, long);
extern Sfio_t*		mamout(struct rule*);
extern void		merge(struct rule*, struct rule*, int);
extern void		mergestate(struct rule*, struct rule*);
extern void		metaclose(struct rule*, struct rule*, int);
extern void		metaexpand(Sfio_t*, char*, char*);
extern struct rule*	metaget(struct rule*, struct list*, char*, struct rule**);
extern struct rule*	metainfo(int, char*, char*, int);
extern int		metamatch(char*, char*, char*);
extern struct rule*	metarule(char*, char*, int);
extern void		negate(struct rule*, struct rule*);
extern void*		newchunk(char**, size_t);
extern void		newfile(struct rule*, char*, unsigned long);
extern struct option*	opentry(int, int);
extern void		optcheck(void);
extern void		optinit(void);
extern void		parentage(Sfio_t*, struct rule*, char*);
extern int		parse(Sfio_t*, char*, char*, int);
extern char*		pathname(char*, struct rule*);
extern void		poplocal(void*);
extern int		prereqchange(struct rule*, struct list*, struct rule*, struct list*);
extern void		punt(int);
extern void*		pushlocal(void);
extern void		readcheck(void);
extern void		readclear(void);
extern void		readenv(void);
extern int		readfile(char*, int, char*);
extern void		readstate(char*);
extern void		rebind(struct rule*, int);
extern void		remdup(struct list*);
extern void		remtmp(int);
extern int		resolve(char*, int, int);
extern int		rstat(char*, struct stat*, int);
extern void		rules(char*);
extern struct rule*	rulestate(struct rule*, int);
extern void		savestate(void);
extern struct list*	scan(struct rule*, unsigned long*);
extern int		scanargs(int, char**, int*);
extern void		set(char*);
extern struct var*	setvar(char*, char*, int);
extern void		shquote(Sfio_t*, char*);
extern struct rule*	source(struct rule*);
extern int		special(struct rule*);
extern struct rule*	staterule(int, struct rule*, char*, int);
extern unsigned long	statetime(struct rule*, int);
extern int		strprintf(Sfio_t*, const char*, char*, int, int);
extern char*		strtime(unsigned long);
extern void		terminate(void);
extern void		trigger(struct rule*, struct rule*, char*, unsigned long);
extern int		unbind(const char*, char*, void*);
extern struct dir*	unique(struct rule*);
extern void		unparse(int);
extern struct var*	varstate(struct rule*, int);
extern void		wakeup(unsigned long, struct list*);
