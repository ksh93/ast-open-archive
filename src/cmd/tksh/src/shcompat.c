#pragma prototyped
#include "tksh.h"

#ifndef NEWKSH

#define SH_DEBUGTRAP            4       /* must be last internal trap */
typedef void History_t;
#include <setjmp.h>


struct sh_scoped
{
	struct slnod	*staklist;	/* link list of function stacks */
	int		states;
	int		breakcnt;
	int		execbrk;
	int		loopcnt;
	int		firstline;
	int		dolc;
	char		**dolv;
	long		optindex;
	long		optnum;
	short		optchar;
	short		opterror;
	char		*cmdname;
	int		ioset;
	short		trapmax;
	char		*trap[SH_DEBUGTRAP+1];
	char		**trapcom;
	char		**otrapcom;
	__V_		*timetrap;
	__V_		*par_tree;	/* for calling function */
	struct sh_scoped *prevst;	/* pointer to previous state */
};

struct limits
{
	int		open_max;	/* maximum number of file descriptors */
	int		clk_tck;	/* number of ticks per second */
	int		child_max;	/* maxumum number of children */
	int		ngroups_max;	/* maximum number of process groups */
	unsigned char	posix_version;	/* posix version number */
	unsigned char	posix_jobcontrol;/* non-zero for job control systems */
	unsigned char	fs3d;		/* non-zero for 3-d file system */
};

#define _SH_PRIVATE \
	struct sh_scoped st;		/* scoped information */ \
	struct limits	lim;		/* run time limits */ \
	Hashtab_t	*track_tree;	/* for tracked aliases*/ \
	Hashtab_t	*bltin_tree;	/* for builtin commands */ \
	Hashtab_t	*var_base;	/* global level variables */ \
	History_t	*hist_ptr;	/* history file pointer */ \
	Sfio_t		*outpool;	/* ouput stream pool */ \
	long		timeout;	/* read timeout */ \
	long		tmout;		/* value for TMOUT */ \
	int		curenv;		/* current subshell number */ \
	int		jobenv;		/* subshell number for jobs */ \
	int		nextprompt;	/* next prompt is PS<nextprompt> */ \
	Namval_t	*bltin_cmds;	/* pointer to built-in commands */ \
	Namval_t	*posix_fun;	/* points to last name() function */ \
	int		infd;		/* input file descriptor */ \
	char		*outbuff;	/* pointer to output buffer */ \
	char		*errbuff;	/* pointer to stderr buffer */ \
	char		*prompt;	/* pointer to prompt string */ \
	char		*shname;	/* shell name */ \
	char		*comdiv;	/* points to sh -c argument */ \
	char		*prefix;	/* prefix for compound assignment */ \
	sigjmp_buf	*jmplist;	/* longjmp return stack */ \
	char		**sigmsg;	/* points to signal messages */ \
	int		oldexit; \
	uid_t 		userid,euserid;	/* real and effective user id */ \
	gid_t 		groupid,egroupid;/* real and effective group id */ \
	pid_t		pid;		/* process id of shell */ \
	pid_t		bckpid;		/* background process id */ \
	pid_t		cpid; \
	long		ppid;		/* parent process id of shell */ \
	int		topfd; \
	int		sigmax;		/* maximum number of signals */ \
	unsigned char	*sigflag;	/* pointer to signal states */ \
	char		intrap; \
	char		login_sh; \
	char		lastbase; \
	char		universe; \
	char		forked;	\
	char		binscript;	\
	char		used_pos;	/* used postional parameter */\
	unsigned char	lastsig;	/* last signal received */ \
	char		*readscript;	/* set before reading a script */ \
	int		*inpipe;	/* input pipe pointer */ \
	int		*outpipe;	/* output pipe pointer */ \
	int		cpipe[2]; \
	int		coutpipe; \
	struct argnod	*envlist; \
	struct dolnod	*arglist; \
	int		fn_depth; \
	int		dot_depth; \
	long		nforks; \
	char		ifstable[256];


typedef struct sh_static_priv
{
	int		inlineno;
	Sfio_t		*heredocs;	/* current here-doc file */ 
	Shopt_t		options;
	Hashtab_t	*var_tree;	/* for shell variables */
	Hashtab_t	*fun_tree;	/* for shell functions */
	Hashtab_t	*alias_tree;	/* for alias names */
	Namval_t	*bltin_nodes;	/* pointer to built-in variables */
	int		**fdptrs;	/* pointer to file numbers */
	Sfio_t		**sftable;	/* pointer to stream pointer table */
	unsigned char	*fdstatus;	/* pointer to file status table */
	int		subshell;	/* set for virtual subshell */
	unsigned int	trapnote;
	char		*lastpath;	/* last alsolute path found */
	int		exitval;
	char		*lastarg;
	const char	*pwd;		/* present working directory */
	int		savexit;
#ifdef _SH_PRIVATE
	_SH_PRIVATE
#endif /* _SH_PRIVATE */
} Shell_priv_t;


#if 0
typedef struct _fcin
{
	Sfio_t		*_fcfile;	/* input file pointer */
	unsigned char	*fcbuff;	/* pointer to input buffer */
	unsigned char	*fclast;	/* pointer to end of input buffer */
	unsigned char	*fcptr;		/* pointer to next input char */
	unsigned char	fcchar;		/* saved character */
	void (*fcfun) __PROTO__((Sfio_t*,const char*,int));	/* advance function */
} Fcin_t;

#define fcfile()	(_Fcin._fcfile)
#define fcgetc(c)	(((c=fcget()) || (c=fcfill())), c)
#define	fcget()		((int)(*_Fcin.fcptr++))
#define	fcpeek(n)	((int)_Fcin.fcptr[n])
#define	fcseek(n)	((char*)(_Fcin.fcptr+=(n)))
#define fcfirst()	((char*)_Fcin.fcbuff)
#define fcsopen(s)	(_Fcin._fcfile=(Sfio_t*)0,_Fcin.fcbuff=_Fcin.fcptr=(unsigned char*)(s))
#define fcsave(x)	(*(x) = _Fcin)
#define fcrestore(x)	(_Fcin = *(x))
extern __MANGLE__ int		fcfill __PROTO__((void));
extern __MANGLE__ int		fcfopen __PROTO__((Sfio_t*));
extern __MANGLE__ int		fcclose __PROTO__((void));
void			fcnotify __PROTO__((void(*)(Sfio_t*,const char*,int)));

extern __MANGLE__ Fcin_t		_Fcin;		/* used by macros */

#endif

static void *sh_topscope;

int nv_getlevel()
{
	int index;
	struct sh_scoped *sp = & ((Shell_priv_t *)(&sh))->st;
	for (index=0; sp; index++)
		sp = sp->prevst;
	return index-1;
}

void *nv_currentscope()
{
	return (void *) & (((Shell_priv_t *)(&sh))->st);
}

/*
 * return the scope corresponding to <index>
 * if <index><=0, <index>th previous scope on function call stack returned
 * otherwise, absolute scope (0 - <index>) is returned
 * If <index> greater than or equal to number of scopes, NULL returned 
 */

CallFrame *sh_getscope(int index, int mode)
{
	static CallFrame temp_frame;

	struct sh_scoped *sp;
	CallFrame *frame = &temp_frame;

	if (mode != 2)
		sp = & ((Shell_priv_t *)(&sh))->st; 	/* &sh.st */
	else
	{
		sp = sh_topscope;
		mode = 0;
	}

	if (mode == 0)		/* Absolute: 0=global, 1=1st func, etc */
	{
		if(index==0)	/* return global scope */
		{
			while(sp && sp->prevst)
				sp = sp->prevst;
			frame->var_tree = nv_globalscope();
			frame->par_tree = sp->par_tree;
			frame->par_scope = (CallFrame *) sp->prevst;
			frame->argc = sp->dolc;
			frame->argv = sp->dolv;
			return frame;
		}
		index = nv_getlevel() - index;
	}
	/* Else relative: 0=current, 1=top func, etc */

	if (index == 0)
	{
		frame->var_tree = sh.var_tree;
		frame->par_tree = sp->par_tree;
		frame->par_scope = (CallFrame *) sp->prevst;
		frame->argc = sp->dolc;
		frame->argv = sp->dolv;
		return frame;
	}

	while(sp)
	{
		if(--index==0)
		{
			frame->var_tree = sp->par_tree ? sp->par_tree : hashscope(sh.var_tree);
			frame->par_tree = sp->prevst->par_tree;
			frame->par_scope = (CallFrame *) sp->prevst->prevst;
			frame->argc = sp->prevst->dolc;
			frame->argv = sp->prevst->dolv;
			return frame;
		}
		sp = sp->prevst;
	}
	return (CallFrame *) 0;
}

void sh_setscope(CallFrame *new, CallFrame *old)
{
	old->var_tree = sh.var_tree;
	old->par_tree = ((Shell_priv_t *)(&sh))->st.par_tree;
	old->par_scope = (CallFrame *) (((Shell_priv_t *)(&sh))->st.prevst);
	old->argc = ((Shell_priv_t *)(&sh))->st.dolc;
	old->argv = ((Shell_priv_t *)(&sh))->st.dolv;
	sh.var_tree = new->var_tree;
	((Shell_priv_t *)(&sh))->st.par_tree = new->par_tree;
	((Shell_priv_t *)(&sh))->st.prevst = (struct sh_scoped *)
		(new->par_scope);
	((Shell_priv_t *)(&sh))->st.dolc = new->argc;
	((Shell_priv_t *)(&sh))->st.dolv = new->argv;
}

int sh_funscope(int argc, char **argv, int (*func)(void *), void *data, int f)
{
	int result;
	Shell_priv_t *shp = (Shell_priv_t *)(&sh);
	Hashtab_t *hp;
	struct sh_scoped sscope, *savst = &sscope;

	*savst = shp->st;
	shp->st.cmdname = argv[0];
	shp->st.dolc = argc-1;
	shp->st.dolv = argv;
	shp->st.opterror = shp->st.optchar = 0;
	shp->st.optindex = 1;
	shp->st.loopcnt = 0;
	shp->st.prevst = savst;
	/* shp->st.firstline = ??? */

	if (hashscope(sh.var_tree))
	{
		shp->st.par_tree = (void *) sh.var_tree;
		sh.var_tree = hashscope(sh.var_tree);
	}
	else
		shp->st.par_tree = 0;

	nv_scope(0);

	sh_topscope = nv_currentscope();
	result = func(data);

	hp = sh.var_tree;

	if (shp->st.par_tree)
	{
		sh.var_tree = (Hashtab_t*) shp->st.par_tree;
		shp->st = *savst;
		hashfree(hp);
	}
	else
	{
		shp->st = *savst;
		sh.var_tree = hashfree(hp);
	}

	sh_topscope = nv_currentscope();
	return result;
}

Hashtab_t *sh_bltin_tree()
{
	return ((Shell_priv_t *)&sh)->bltin_tree;
}

void sh_userinit()
{
	fprintf(stderr, "Tksh has been compiled with an old main.c, exiting\n");
	exit(1);
}

extern int sh_waitevent(int);

static int (*waitproc)(int, long);

void sh_waitnotify(int(*func)(int,long))
{
	waitproc = func;
}

int sh_waitevent(int fd)
{
	if (waitproc)
		return waitproc(fd, 0);
	else
		return 0;
}

#else

int nv_getlevel()
{
	int index=0;
	Shscope_t *sp = sh_getscope(0, 1);
	while ((sp = sp->par_scope))
		index++;
	return index;
}

#ifdef nv_scan
#undef nv_scan
int _nv_scan(Hashtab_t *root, void (*fn)(Namval_t *,void*), void *data, int mask, int flags)
{
	return nv_scan(root, (void (*)(Namval_t *)) fn, mask, flags);
}
#endif

#endif

Namval_t *nv_move(Namval_t *src, Namval_t *dst)
{
	*dst = *src;
	src->nvfun = NULL;
	src->nvalue = 0;
	src->nvenv = 0;
	src->nvflag = 0;
	
	nv_unset(src);	/* XX probably not necc */
	return dst;
}

Hashtab_t *nv_globalscope()
{
	Hashtab_t *hp;
	return ((hp=hashscope(sh.var_tree)) ? hp : sh.var_tree);
}

int sh_openmax()
{
	return sysconf(_SC_OPEN_MAX);
}
