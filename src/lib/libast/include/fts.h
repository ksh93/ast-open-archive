/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1985-2000 AT&T Corp.                *
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
*                David Korn <dgk@research.att.com>                 *
*                 Phong Vo <kpv@research.att.com>                  *
*                                                                  *
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * David Korn
 * Phong Vo
 * AT&T Research
 *
 * fts interface definitions
 */

#ifndef	_FTS_H
#define _FTS_H

#include <ls.h>

/*
 * fts_open flags
 */

#define FTS_LOGICAL	0	/* logical traversal, follow symlinks	*/
#define FTS_META	(1<<0)	/* follow top dir symlinks even if phys	*/
#define FTS_NOCHDIR	(1<<1)	/* don't chdir				*/
#define FTS_NOPOSTORDER	(1<<2)	/* no postorder visits			*/
#define FTS_NOPREORDER	(1<<3)	/* no preorder visits			*/
#define FTS_NOSEEDOTDIR	(1<<11)	/* never retain leading . dir		*/
#define FTS_NOSTAT	(1<<4)	/* don't stat children			*/
#define FTS_ONEPATH	(1<<5)	/* pathnames arg is one const char*	*/
#define FTS_PHYSICAL	(1<<6)	/* physical traversal, don't follow	*/
#define FTS_SEEDOT	(1<<7)	/* return . and ..			*/
#define FTS_SEEDOTDIR	(1<<10)	/* always retain leading . dir		*/
#define FTS_TOP		(1<<8)	/* don't traverse subdirectories	*/
#define FTS_XDEV	(1<<9)	/* don't cross mount points		*/

#define FTS_USER	(1<<12)	/* first user flag bit			*/

#define FTS_COMFOLLOW	FTS_META

/*
 * fts_info flags
 */

#define FTS_DEFAULT	0	/* ok, someone must have wanted this	*/

#define FTS_NS		(1<<0)	/* stat failed				*/
#define FTS_F		(1<<1)	/* file - not directory or symbolic link*/
#define FTS_SL		(1<<2)	/* symbolic link			*/
#define FTS_D		(1<<3)	/* directory - pre-order visit		*/

#define FTS_C		(1<<4)	/* causes cycle				*/
#define FTS_ERR		(1<<5)	/* some other error			*/
#define FTS_DD		(1<<6)	/* . or ..				*/
#define FTS_NR		(1<<7)	/* cannot read				*/
#define FTS_NX		(1<<8)	/* cannot search			*/
#define FTS_OK		(1<<9)	/* no info but otherwise ok		*/
#define FTS_P		(1<<10)	/* post-order visit			*/

#define FTS_DC		(FTS_D|FTS_C)	/* dir - would cause cycle	*/
#define FTS_DNR		(FTS_D|FTS_NR)	/* dir - no read permission	*/
#define FTS_DNX		(FTS_D|FTS_NX)	/* dir - no search permission	*/
#define FTS_DOT		(FTS_D|FTS_DD)	/* . or ..			*/
#define FTS_DP		(FTS_D|FTS_P)	/* dir - post-order visit	*/
#define FTS_NSOK	(FTS_NS|FTS_OK)	/* no stat (because you asked)	*/
#define FTS_SLNONE	(FTS_SL|FTS_NS)	/* symlink - to nowhere		*/

/*
 * fts_set flags
 */

#define FTS_AGAIN	FTS_TOP		/* process entry again		*/
#define FTS_FOLLOW	FTS_META	/* follow FTS_SL symlink	*/
#define FTS_SKIP	FTS_NOSTAT	/* skip FTS_D directory		*/
#define FTS_STAT	FTS_PHYSICAL	/* stat() done by user		*/

typedef struct Fts FTS;
typedef struct Ftsent FTSENT;

struct Ftsent
{
	char*		fts_accpath;	/* path relative to .		*/
	char*		fts_name;	/* file name			*/
	char*		fts_path;	/* path relative to top dir	*/
	FTSENT*		fts_cycle;	/* offender if cycle		*/
	FTSENT*		fts_link;	/* next child			*/
	FTSENT*		fts_parent;	/* parent directory		*/
	struct stat*	fts_statp;	/* stat info			*/
#ifdef _FTSENT_LOCAL_PRIVATE_
	_FTSENT_LOCAL_PRIVATE_
#else
	void*		fts_pointer;	/* local pointer value		*/
#endif
	long		fts_number;	/* local numeric value		*/
	int		fts_errno;	/* errno for this entry		*/
	unsigned short	fts_info;	/* info flags			*/
	unsigned short	fts_namelen;	/* strlen(fts_name)		*/
	unsigned short	fts_pathlen;	/* strlen(fts_path)		*/
	short		fts_level;	/* file tree depth, 0 at top	*/

#ifdef _FTSENT_PRIVATE_
	_FTSENT_PRIVATE_
#endif

};

struct Fts
{
	int		fts_errno;	/* last errno			*/

#ifdef _FTS_PRIVATE_
	_FTS_PRIVATE_
#endif

};

#if _BLD_ast && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern FTSENT*	fts_children(FTS*, int);
extern int	fts_close(FTS*);
extern int	fts_flags(void);
extern int	fts_notify(int(*)(FTS*, FTSENT*, void*), void*);
extern FTS*	fts_open(char* const*, int, int(*)(FTSENT* const*, FTSENT* const*));
extern FTSENT*	fts_read(FTS*);
extern int	fts_set(FTS*, FTSENT*, int);

#undef	extern

#endif
