/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1989-2002 AT&T Corp.                *
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
*            Information and Software Systems Research             *
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
 * tw common definitions
 */

#include <ast.h>
#include <expr.h>
#include <ls.h>
#include <find.h>
#include <fs3d.h>
#include <ftwalk.h>
#include <magic.h>
#include <error.h>
#include <debug.h>

#include "cmdarg.h"
#include "ftwlocal.h"

#define ignorecase	fts_number

#define PATH(f)		((f)->status==FTW_NAME?(f)->name:(f)->path)

#define ACT_CMDARG		1
#define ACT_CODE		2
#define ACT_CODETYPE		3
#define ACT_EVAL		4
#define ACT_INTERMEDIATE	5
#define ACT_LIST		6

#define CMD_IMPLICIT	(CMD_USER<<0)

#define MEMMAKE(s,o)	(((s)<<8)|((o)&((1<<8)-1)))
#define MEMINDEX(x)	(((x)>>8)&((1<<8)-1))
#define MEMOFFSET(x)	((x)&((1<<8)-1))

#define T_type		64
#define T_DATE		(T_type+1)
#define T_GID		(T_type+2)
#define T_MODE		(T_type+3)
#define T_PERM		(T_type+4)
#define T_UID		(T_type+5)

#define F_args		1
#define F_atime		2
#define F_blocks	3
#define F_blksize	4
#define F_checksum	5
#define F_ctime		6
#define F_dev		7
#define F_fstype	8
#define F_gid		9
#define F_gidok		10
#define F_info		11
#define F_ino		12
#define F_level		13
#define F_local		14
#define F_mode		15
#define F_magic		16
#define F_mime		17
#define F_mtime		18
#define F_name		19
#define F_nlink		20
#define F_parent	21
#define F_perm		22
#define F_path		23
#define F_rdev		24
#define F_size		25
#define F_status	26
#define F_symlink	27
#define F_type		28
#define F_uid		29
#define F_uidok		30
#define F_view		31
#define F_visit		32

#define C_AGAIN		33
#define C_BLK		34
#define C_C		35
#define C_CHR		36
#define C_CTG		37
#define C_D		38
#define C_DC		39
#define C_DIR		40
#define C_DNR		41
#define C_DNX		42
#define C_DP		43
#define C_FIFO		44
#define C_FMT		45
#define C_FOLLOW	46
#define C_LNK		47
#define C_NOPOST	48
#define C_NR		49
#define C_NS		50
#define C_NX		51
#define C_REG		52
#define C_SOCK		53
#define C_SKIP		54

#define X_cmdarg	55
#define X_cmdflush	56

#define M_MEMBER	57

typedef struct				/* unique file identifier	*/
{
	long		di[2];		/* dev,ino			*/
} Fileid_t;

typedef struct				/* Fileid_t visit		*/
{
	Dtlink_t	link;		/* table link			*/
	Fileid_t	id;		/* file id			*/
	Extype_t	value[1];	/* visit values			*/
} Visit_t;

typedef struct Local_s			/* local struct			*/
{
	struct Local_s*	next;		/* next in free list		*/
	Extype_t	value[1];	/* member values		*/
} Local_t;

typedef struct				/* program state		*/
{
	int		act;		/* leaf node ACT_*		*/
	int		actII;		/* real action for intermediate	*/
	Exnode_t*	action;		/* action expression		*/
	int		args;		/* command arg count		*/
	Cmdarg_t*	cmd;		/* command arg state		*/
	int		cmdflags;	/* cmdopen() flags		*/
	int		compiled;	/* excomp() complete		*/
	int		errors;		/* error count			*/
	Exdisc_t	expr;		/* expr discipline		*/
	Find_t*		find;		/* fast find handle		*/
	int		finderror;	/* fast find generation error	*/
	int		ftwflags;	/* tree walk flags		*/
	Dt_t*		fstab;		/* fs type hash table		*/
	int		icase;		/* ignore case in sort		*/
	int		ignore;		/* ignore cmd and dir errors	*/
	int		info;		/* ftw.info checked by user	*/
	int		intermediate;	/* generate intermediate dirs	*/
	Local_t*	local;		/* local struct free list	*/
	int		localfs;	/* restrict to local fs mounts	*/
	int		localmem;	/* ftw.local member count	*/
	Magic_t*	magic;		/* magic tests			*/
	Magicdisc_t	magicdisc;	/* magic discipline		*/
	char*		pattern;	/* fast find pattern		*/
	Expr_t*		program;	/* compiled expressions		*/
	int		reverse;	/* reverse sort sense		*/
	Exnode_t*	select;		/* select expression		*/
	int		separator;	/* xargs list separator		*/
	int		(*sort)(Ftw_t*, Ftw_t*); 	/* sorter	*/
	Exid_t*		sortkey;	/* sort identifier symbol	*/
	Dt_t*		vistab;		/* visit hash table		*/
	int		visitmem;	/* visit member count		*/
} State_t;

extern State_t		state;

extern void		compile(char*);
extern long		eval(Exnode_t*, Ftw_t*);
extern long		getnum(Exid_t*, Ftw_t*);
