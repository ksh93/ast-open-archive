#pragma prototyped
/* First, determine which version of KSH we're using.  For now,
   the best way to tell is by seeing if sh_bltin_tree is defined
 */

#ifdef SH_VERSION
#define NEWKSH
# if SH_VERSION >= 19961101
# define KSH93F
# endif
# if SH_VERSION >= 19990801
# define _COMPAT_SH_PRIVATE
# endif
#endif

#define NIL(t)		((t)0)

#ifdef NEWKSH

#if 0
#define sh_trap(cmd, val)	sh_interp((cmd), NULL, NULL)
#endif

#ifndef SH_SUBSHARE
#define SH_SUBSHARE	(1L<<27)	/* subshell shares state with parent */
#endif

#else
/* Older versions of libshell */
extern Sfio_t		*sh_sfeval(char*[]);

typedef struct Shscope_t
{
	Hashtab_t *var_tree;
	Hashtab_t *par_tree;
	struct Shscope_t *par_scope;
	int argc;
	char **argv;
} Shscope_t; 

int		nv_getlevel(void);

Hashtab_t *	sh_bltin_tree(void);
Shscope_t *	sh_getscope(int index, int mode);
void		sh_setscope(Shscope_t *new, Shscope_t *old);
int		sh_funscope(int argc, char **argv, int (*func)(void *), void *data, int f);

#endif

#ifndef KSH93F
/* Functionality to support ksh93d and ksh93e */

/* Functions to save and restore Fcin */
typedef struct _fcin
{
        Sfio_t          *_fcfile;       /* input file pointer */
        unsigned char   *fcbuff;        /* pointer to input buffer */
        unsigned char   *fclast;        /* pointer to end of input buffer */
        unsigned char   *fcptr;         /* pointer to next input char */
        unsigned char   fcchar;         /* saved character */
        void (*fcfun) __PROTO__((Sfio_t*,const char*,int));
} Fcin_t;
extern __MANGLE__ Fcin_t                _Fcin;          /* used by macros */
#define fcsave(x)       (*(x) = _Fcin)
#define fcrestore(x)    (_Fcin = *(x))

/* ksh93f changes nv_scan arguments, compatibility for 93e */
int _nv_scan(Hashtab_t *root, void (*fn)(Namval_t *,void*), void *data,
	int mask, int flags);
#define	nv_scan	_nv_scan

#else

/* typedef char Fcin_t[256]; */
typedef struct _fcin
{
	char data[256];
} Fcin_t;
extern void	fcsave(Fcin_t*);
extern void	fcrestore(Fcin_t*);

#endif

/* Functions all versions of tksh need to support */
Namval_t *		nv_move(Namval_t *src, Namval_t *dst);
extern int		TkshUpVar(Tcl_Interp *interp,char *,char *,char *,
			Hashtab_t *scope);
extern Namval_t *	TkshAccessVar(Tcl_Interp *interp, char *part1,
			char *part2);
Hashtab_t*		nv_globalscope(void);
void *			nv_currentscope(void);


/* Structures all versions of tksh need to support */
typedef Shscope_t CallFrame;
typedef int (*ShellProc_t)(int, char**, void*);
typedef int (*ShellNote_t)(int, long, int);
