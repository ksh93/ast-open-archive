/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1984-2004 AT&T Corp.                *
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
*                          AT&T Research                           *
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
 * make option attributes and flag mappings
 */

#define OPT(o)		(o&((1<<9)-1))	/* get opt char from code	*/
#define Of		(1<<8)		/* cannot be set by flag	*/

#define Oa		(1<<9)		/* multiple values added	*/
#define Ob		(1<<10)		/* boolean value		*/
#define Oi		(1<<11)		/* state.* sense inverted	*/
#define On		(1<<12)		/* numeric value		*/
#define Oo		(1<<13)		/* flag sense opposite		*/
#define Os		(1<<14)		/* string value			*/
#define Ov		(1L<<15)	/* value is optional		*/
#define Ox		(1L<<16)	/* not expanded in $(-)		*/

#define OPT_COMPILE	(1<<0)		/* compile into object file	*/
#define OPT_DECLARED	(1<<1)		/* OPT_EXTERNAL option declared	*/
#define OPT_DEFINED	(1<<2)		/* option explicitly defined	*/
#define OPT_EXPLICIT	(1<<3)		/* for OPT_rules checks		*/
#define OPT_EXTERNAL	(1<<4)		/* external by name		*/
#define OPT_FLAG	(1<<5)		/* external by flag		*/
#define OPT_GLOBAL	(1<<6)		/* defined in global file	*/
#define OPT_HIDDEN	(1<<7)		/* saved for listops(*,0)	*/
#define OPT_READONLY	(1<<8)		/* cannot reset til state.user	*/

#define OPT_accept	('A'|Ob)	/* accept existing targets	*/
#define OPT_alias	('a'|Ob)	/* directory aliasing enabled	*/
#define OPT_base	('b'|Ob|Ox)	/* compile base|global rules	*/
#define OPT_believe	('B'|On)	/* believe state from this level*/
#define OPT_byname	('o'|Oa|Os|Ox)	/* command line option by name	*/
#define OPT_compile	('c'|Ob|Ox)	/* force makefile compilation	*/
#define OPT_compatibility ('C'|Ob|Ox)	/* disable compatibility msgs	*/
#define OPT_corrupt	('X'|Os|Ov)	/* corrupt statefile action	*/
#define OPT_cross	('J'|Ob)	/* don't run gen'd executables	*/
#define OPT_debug	('d'|Oi|On)	/* debug trace level		*/
#define OPT_define	('D'|Os|Ox)	/* passed to preprocessor	*/
#define OPT_errorid	('E'|Os)	/* append to error output id	*/
#define OPT_exec	('n'|Ob|Oo)	/* execute shell actions	*/
#define OPT_expandview	('x'|Ob)	/* expand paths if fsview!=0	*/
#define OPT_explain	('e'|Ob)	/* explain actions		*/
#define OPT_file	('f'|Oa|Os|Ox)	/* next arg is makefile		*/
#define OPT_force	('F'|Ob)	/* force targets to be updated	*/
#define OPT_global	('g'|Oa|Os)	/* next arg is global makefile	*/
#define OPT_ignore	('i'|Ob)	/* ignore shell action errors	*/
#define OPT_ignorelock	('K'|Ob)	/* ignore state file locking	*/
#define OPT_import	('H'|Ob|Ox)	/* import var def precedence	*/
#define OPT_include	('I'|Os|Ox)	/* passed to preprocessor	*/
#define OPT_intermediate ('G'|Ob)	/* force intermediate targets	*/
#define OPT_jobs	('j'|On)	/* job concurrency level	*/
#define OPT_keepgoing	('k'|Ob)	/* do sibling prereqs on error	*/
#define OPT_list	('l'|Ob|Ox)	/* list info and don't make	*/
#define OPT_mam		('M'|Os)	/* generate mam			*/
#define OPT_never	('N'|Ob)	/* really - don't exec anything	*/
#define OPT_option	('O'|Oa|Of|Os|Ox)/* define new option by name	*/
#define OPT_override	('X'|Ob|Of|Ox)	/* override selected algorithms	*/
#define OPT_preprocess	('P'|Ob)	/* preprocess all makefiles	*/
#define OPT_questionable ('Q'|Oa|On)	/* enable questionable code	*/
#define OPT_readonly	('R'|Ob)	/* current vars|opts readonly	*/
#define OPT_readstate	('S'|On|Ov)	/* read state file on startup	*/
#define OPT_regress	('q'|Ob)	/* output for regression test	*/
#define OPT_reread	('Z'|Of|On|Ox)	/* force re-read all makefiles	*/
#define OPT_ruledump	('r'|Ob|Ox)	/* dump rule definitions	*/
#define OPT_scan	('s'|Ob|Of)	/* scan|check implicit prereqs	*/
#define OPT_silent	('s'|Ob)	/* run silently			*/
#define OPT_strictview	('V'|Ob)	/* strict views			*/
#define OPT_targetcontext ('L'|Ob|Of)	/* expand in target dir context	*/
#define OPT_test	('T'|Oa|On)	/* enable test code		*/
#define OPT_tolerance	('z'|On)	/* time comparison tolerance	*/
#define OPT_touch	('t'|Ob)	/* touch out of date targets	*/
#define OPT_undef	('U'|Os|Ox)	/* passed to preprocessor	*/
#define OPT_vardump	('v'|Ob|Ox)	/* dump variable definitions	*/
#define OPT_warn	('w'|Ob)	/* enable source file warnings	*/
#define OPT_writeobject	('y'|Of|Os|Ov|Ox)/* write recompiled object	*/
#define OPT_writestate	('Y'|Of|Os|Ov|Ox)/* write state file on exit	*/

struct option				/* option table entry		*/
{
	char*		name;		/* option name			*/
	unsigned long	flag;		/* OPT_[a-z]+ flags		*/
	char*		value;		/* overloaded value		*/
	int		status;		/* OPT_[A-Z]+ status		*/
	char*		set;		/* call this on set		*/
	char*		description;	/* description			*/
	char*		arg;		/* arg name			*/
};
