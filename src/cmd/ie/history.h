/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1984-2000 AT&T Corp.                *
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
*                David Korn <dgk@research.att.com>                 *
*                           Pat Sullivan                           *
*                                                                  *
*******************************************************************/
/*
 *	UNIX shell
 *	Header File for history mechanism
 *	written by David Korn
 *
 */



#ifndef IOBSIZE
#   define IOBSIZE	1024
#endif
#define FC_CHAR		'!'
#define HIS_DFLT	128		/* default size of history list */
#define HISMAX		(sizeof(int)*IOBSIZE)
#define HISBIG		(0100000-1024)	/* 1K less than maximum short */
#define HISLINE		16		/* estimate of average sized history line */
#define MAXLINE		258		/* longest history line permitted */

#define H_UNDO		0201		/* invalidate previous command */
#define H_CMDNO		0202		/* next 3 bytes give command number */
#define H_VERSION	1		/* history file format version no. */

struct history
{
	struct fileblk	*fixfp;		/* file descriptor for history file */
	int 		fixfd;		/* file number for history file */
	char		*fixname;	/* name of history file */
	off_t		fixcnt;		/* offset into history file */
	int		fixind;		/* current command number index */
	int		fixmax;		/* number of accessible history lines */
	int		fixflush;	/* set if flushed outside of hflush() */
	off_t		fixcmds[1];	/* byte offset for recent commands */
};

typedef struct
{
	int his_command;
	int his_line;
} histloc;

extern struct history	*hist_ptr;

#ifndef KSHELL
    extern void	p_flush();
    extern void	p_setout();
    extern void	p_char();
    extern void	p_str();
#   ifdef __STDC__
#	define nam_strval(s)	getenv(#s)
#   else
#	define nam_strval(s)	getenv("s")
#   endif /* __STDC__ */
#   define NIL		((char*)0)
#   define sh_fail	ed_failed
#   define sh_copy	ed_movstr
#ifdef PROTO
	extern char	*ed_movstr(const char*, char*);
#else
	extern char	*ed_movstr();
#endif /* PROTO */
#endif	/* KSHELL */

/* the following are readonly */
extern const char	hist_fname[];
extern const char	e_history[];

/* these are the history interface routines */
#ifdef PROTO
    extern int		hist_open(void);
    extern void 	hist_cancel(void);
    extern void 	hist_close(void);
    extern int		hist_copy(char*,int,int);
    extern void 	hist_eof(void);
    extern histloc	hist_find(char*,int,int,int);
    extern void 	hist_flush(void);
    extern void 	hist_list(off_t,int,char*);
    extern int		hist_match(off_t,char*,int);
    extern off_t	hist_position(int);
    extern void 	hist_subst(const char*,int,char*);
    extern char 	*hist_word(char*,int);
#   ifdef ESH
	extern histloc	hist_locate(int,int,int);
#   endif	/* ESH */
#else
    extern int		hist_open();
    extern void 	hist_cancel();
    extern void 	hist_close();
    extern int		hist_copy();
    extern void 	hist_eof();
    extern histloc	hist_find();
    extern void 	hist_flush();
    extern void 	hist_list();
    extern int		hist_match();
    extern off_t	hist_position();
    extern void 	hist_subst();
    extern char 	*hist_word();
#   ifdef ESH
	extern histloc	hist_locate();
#   endif	/* ESH */
#endif /* PROTO */
