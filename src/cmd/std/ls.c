/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1989-2000 AT&T Corp.              *
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
 * ls -- list file status
 */

static const char usage[] =
"[-?\n@(#)ls (AT&T Labs Research) 2000-02-07\n]"
USAGE_LICENSE
"[+NAME?ls - list files and/or directories]"
"[+DESCRIPTION?For each directory argument \bls\b lists the contents; for each"
"	file argument the name and requested information are listed."
"	The current directory is listed if no file arguments appear."
"	The listing is sorted by file name by default, except that file"
"	arguments are listed before directories.]"
"[+?\bgetconf PATH_RESOLVE\b determines how symbolic links are handled. This"
"	can be explicitly overridden by the \b--logical\b, \b--metaphysical\b,"
"	and \b--physical\b options below. \bPATH_RESOLVE\b can be one of:]{"
"		[+logical?Follow all symbolic links.]"
"		[+metaphysical?Follow command argument symbolic links,"
"			otherwise don't follow.]"
"		[+physical?Don't follow symbolic links.]"
"}"

"[a:all?List entries starting with \b.\b; turns off \b--almost-all\b.]"
"[A:almost-all?List all entries but \b.\b and \b..\b; turns off \b--all\b.]"
"[b:escape?Print escapes for nongraphic characters.]"
"[B:ignore-backups?Do not list entries ending with ~.]"
"[c:ctime?Sort by change time; list ctime with \b--long\b.]"
"[C:multi-column?List entries by columns.]"
"[d:directory?List directory entries instead of contents.]"
"[D:define?Define \akey\a with optional \avalue\a. \akey\a may be a"
"	\b--format\b specification.]:[key[=value]]]"
"[f:format?Append to the listing format string. \aformat\a follows"
"	\bprintf\b(3) conventions, except that \bsfio\b(3) inline ids"
"	are used instead of arguments:"
"	%[-+]][\awidth\a[.\aprecis\a[.\abase\a]]]]]](\aid\a[:\asubformat\a]])\achar\a."
"	If \achar\a is \bs\b then the string form of the item is listed,"
"	otherwise the corresponding numeric form is listed. \asubformat\a"
"	overrides the default formatting for \aid\a. Supported \aid\as"
"	and \asubformat\as are:]:[format]{"
"		[+atime?access time]"
"		[+blocks?size in blocks]"
"		[+ctime?change time]"
"		[+device?device number]"
"		[+devmajor?major device number]"
"		[+devminor?minor device number]"
"		[+dir.blocks?directory blocks]"
"		[+dir.bytes?directory size in bytes]"
"		[+dir.count?directory entry count]"
"		[+dir.files?directory file count]"
"		[+dir.octets?directory size in octets]"
"		[+flags?command line flags in effect]"
"		[+gid?group id]"
"		[+header?listing header]"
"		[+ino?serial number]"
"		[+linkop?link operation: => for symbolic, == for hard]"
"		[+linkname?symbolic link text]"
"		[+linkpath?symbolic link text]"
"		[+mark?file or directory mark character]"
"		[+markdir?directory mark character]"
"		[+mode?access mode]"
"		[+mtime?modification time]"
"		[+name?entry name]"
"		[+nlink?hard link count]"
"		[+path?file path from original root dir]"
"		[+size?file size in bytes]"
"		[+summary?listing summary info]"
"		[+total.blocks?running total block count]"
"		[+total.bytes?running total size in bytes]"
"		[+total.files?running total file count]"
"		[+total.octets?running total size in octets]"
"		[+trailer?listing trailer]"
"		[+uid?owner id]"
"		[+----?subformats ----]"
"		[+case\b::\bp\b\a1\a::\bs\b\a1\a::...::\bp\b\an\a::\bs\b\an\a?Expands"
"			to \bs\b\ai\a if the value of \aid\a matches the shell"
"			pattern \bp\b\ai\a, or the empty string if there is no"
"			match.]"
"		[+mode?The integral value as a \bfmtmode\b(3) string.]"
"		[+time[=\aformat\a]]?The integral value as a \bstrftime\b(3)"
"			string. For example,"
"			\b--format=\"%8(mtime)u %(ctime:time=%H:%M:%S)s\"\b"
"			lists the mtime in seconds since the epoch and the"
"			ctime as hours:minutes:seconds.]"
"	}"
"[F:classify?Append a character for typing each entry.]"
"[g?\b--long\b with no owner info.]"
"[G:group?\b--long\b with no group info.]"
"[i:inode?List the file serial number.]"
"[I:ignore?Do not list implied entries matching shell \apattern\a.]:[pattern]"
"[k:kilobytes?Use 1024 blocks instead of 512.]"
"[K:shell-quote?Enclose entry names in shell $'...' if necessary.]"
"[l:long|verbose?Use a long listing format.]"
"[m:commas|comma-list?List names as comma separated list.]"
"[n:numeric-uid-gid?List numeric user and group ids instead of names.]"
"[N:literal?Print raw entry names (don't treat e.g. control characters specially).]"
"[o?\b--long\b with no group info.]"
"[O:owner?\b--long\b with no owner info.]"
"[p:markdir?Append / to each directory name.]"
"[q:hide-control-chars?Print ? instead of non graphic characters.]"
"[Q:quote-name?Enclose all entry names in \"...\".]"
"[r:reverse?Reverse order while sorting.]"
"[R:recursive?List subdirectories recursively.]"
"[s:size?Print size of each file, in blocks.]"
"[S:bysize?Sort by file size.]"
"[t:?Sort by modification time; list mtime with \b--long\b.]"
"[T:tabsize?Ignored by this implementation.]"
"[u:access?Sort by last access time; list atime with \b--long\b.]"
"[U?Equivalent to \b--sort=none\b.]"
"[V:colors|colours?\akey\a determines when color is used to distinguish"
"	types:]:?[key]{"
"		[+never?never use color (default)]"
"		[+always?always use color]"
"		[+tty|auto?use color when output is a tty]"
"}"
"[w:width?\ascreen-width\a is the current screen width.]#[screen-width]"
"[W:time?Display \akey\a time instead of the modification time:]:[key]{"
"	[+atime|access|use?access time]"
"	[+ctime|status?status change time]"
"	[+mtime|time?modify time]"
"}"
"[x:across?List entries by lines instead of by columns.]"
"[X:extension?Sort alphabetically by entry extension.]"
"[y:sort?Sort by \akey\a:]:?[key]{"
"	[+atime|access|use?access time]"
"	[+ctime|status?status change time]"
"	[+extension?file name extension]"
"	[+mtime|time?modify time]"
"	[+name?file name]"
"	[+none?don't sort]"
"}"
"[Y:layout?Listing layout \akey\a:]:[key]{"
"	[+across|horizontal?multi-column accros the page]"
"	[+comma?comma separated names across the page]"
"	[+long|verbose?long listing]"
"	[+single-column?one column down the page]"
"	[+multi-column|vertical?multi-column by column]"
"}"
"[z:numeric-time?List both full date and full time in numeric form.]"
"[Z:full-time?List both full date and full time.]"
"[1:one-column?List one file per line.]"
"[L:logical|follow?Follow symbolic links. The default is determined by"
"	\bgetconf PATH_RESOLVE\b.]"
"[H:metaphysical?Follow command argument symbolic links, otherwise don't"
"	follow. The default is determined by \bgetconf PATH_RESOLVE\b.]"
"[P:physical?Don't follow symbolic links. The default is determined by"
"	\bgetconf PATH_RESOLVE\b.]"

"\n"
"\n[ file ... ]\n"
"\n"
"[+SEE ALSO?\bchmod\b(1), \bfind\b(1), \bgetconf\b(1), \btw\b(1)]"
;

#include <ast.h>
#include <ls.h>
#include <ctype.h>
#include <error.h>
#include <ftwalk.h>
#include <sfdisc.h>
#include <sfstr.h>
#include <hash.h>

#define LS_ACROSS	(LS_USER<<0)	/* multi-column row order	*/
#define LS_ALL		(LS_USER<<1)	/* list all			*/
#define LS_COLUMNS	(LS_USER<<2)	/* multi-column column order	*/
#define LS_COMMAS	(LS_USER<<3)	/* comma separated name list	*/
#define LS_DIRECTORY	(LS_USER<<4)	/* list directories as files	*/
#define LS_ESCAPE	(LS_USER<<5)	/* list directories as files	*/
#define LS_EXTENSION	(LS_USER<<6)	/* sort by name extension	*/
#define LS_LABEL	(LS_USER<<7)	/* label for all dirs		*/
#define LS_MARKDIR	(LS_USER<<8)	/* marks dirs with /		*/
#define LS_MOST		(LS_USER<<9)	/* list all but . and ..	*/
#define LS_NOBACKUP	(LS_USER<<10)	/* omit *~ names		*/
#define LS_NOSORT	(LS_USER<<11)	/* don't sort			*/
#define LS_NOSTAT	(LS_USER<<12)	/* leaf FTW_NS ok		*/
#define LS_PRINTABLE	(LS_USER<<13)	/* ? for non-printable chars	*/
#define LS_QUOTE	(LS_USER<<14)	/* "..." file names		*/
#define LS_RECURSIVE	(LS_USER<<15)	/* recursive directory descent	*/
#define LS_REVERSE	(LS_USER<<16)	/* reverse sort order		*/
#define LS_SEPARATE	(LS_USER<<17)	/* dir header needs separator	*/
#define LS_SHELL	(LS_USER<<18)	/* $'...' file names		*/
#define LS_TIME		(LS_USER<<19)	/* sort by time			*/

#define LS_STAT		LS_NOSTAT

#define VISIBLE(f)	((f)->level<=0||(!state.ignore||!strmatch((f)->name,state.ignore))&&(!(state.lsflags&LS_NOBACKUP)||(f)->name[(f)->namelen-1]!='~')&&((state.lsflags&LS_ALL)||(f)->name[0]!='.'||(state.lsflags&LS_MOST)&&((f)->name[1]&&(f)->name[1]!='.'||(f)->name[2])))

#define INVISIBLE	1
#define LISTED		2

#define KEY_environ		(-1)

#define KEY_atime		1
#define KEY_blocks		2
#define KEY_ctime		3
#define KEY_device		4
#define KEY_devmajor		5
#define KEY_devminor		6
#define KEY_dir_blocks		7
#define KEY_dir_count		8
#define KEY_dir_files		9
#define KEY_dir_octets		10
#define KEY_flags		11
#define KEY_gid			12
#define KEY_header		13
#define KEY_ino			14
#define KEY_linkop		15
#define KEY_linkpath		16
#define KEY_mark		17
#define KEY_markdir		18
#define KEY_mode		19
#define KEY_mtime		20
#define KEY_name		21
#define KEY_nlink		22
#define KEY_path		23
#define KEY_size		24
#define KEY_summary		25
#define KEY_total_blocks	26
#define KEY_total_files		27
#define KEY_total_octets	28
#define KEY_trailer		29
#define KEY_uid			30

#define PRINTABLE(s)	((state.lsflags&LS_PRINTABLE)?printable(s):(s))

typedef struct				/* dir/total counts		*/
{
	unsigned long	blocks;		/* number of blocks		*/
	unsigned long	files;		/* number of files		*/
	unsigned long	octets;		/* number of octets		*/
} Count_t;

typedef struct				/* sfkeyprintf() keys		*/
{
	char*		name;		/* key name			*/
	short		index;		/* index			*/
	short		disable;	/* macro being expanded		*/
	char*		macro;		/* macro definition		*/
} Key_t;

typedef struct				/* list state			*/
{
	Count_t		count;		/* directory counts		*/
	Ftw_t*		ftw;		/* ftw info			*/
	char*		dirnam;		/* pr() dirnam			*/
	int		dirlen;		/* pr() dirlen			*/
} List_t;

typedef struct				/* program state		*/
{
	char		flags[64];	/* command line option flags	*/
	long		ftwflags;	/* FTW_* flags			*/
	long		lsflags;	/* LS_* flags			*/
	long		sortflags;	/* sort LS_* flags		*/
	long		timeflags;	/* time LS_* flags		*/
	long		blocksize;	/* file block size		*/
	unsigned long	directories;	/* directory count		*/
	Count_t		total;		/* total counts			*/
	int		comma;		/* LS_COMMAS ftw.level crossing	*/
	int		width;		/* output width in chars	*/
	char*		endflags;	/* trailing 0 in flags		*/
	char*		format;		/* sfkeyprintf() format		*/
	char*		ignore;		/* ignore files matching this	*/
	char*		timefmt;	/* time list format		*/
	Hash_table_t*	keys;		/* sfkeyprintf() keys		*/
	Sfio_t*		tmp;		/* tmp string stream		*/
	Ftw_t*		top;		/* top directory -- no label	*/
} State_t;

static char	DEF_header[] =
"%(dir.count:case;0;;1;%(path)s:\n;*;\n%(path)s:\n)s"
"%(flags:case;*d*;;*[ls]*;total %(dir.blocks)u\n)s"
;

static Key_t	keys[] =
{
	{ 0 },
	{ "atime",		KEY_atime		},
	{ "blocks",		KEY_blocks		},
	{ "ctime",		KEY_ctime		},
	{ "device",		KEY_device		},
	{ "devmajor",		KEY_devmajor		},
	{ "devminor",		KEY_devminor		},
	{ "dir.blocks",		KEY_dir_blocks		},
	{ "dir.count",		KEY_dir_count		},
	{ "dir.files",		KEY_dir_files		},
	{ "dir.octets",		KEY_dir_octets		},
	{ "flags",		KEY_flags		},
	{ "gid",		KEY_gid			},
	{ "header",		KEY_header, 0, DEF_header },
	{ "ino",		KEY_ino			},
	{ "linkop",		KEY_linkop		},
	{ "linkpath",		KEY_linkpath		},
	{ "mark",		KEY_mark		},
	{ "markdir",		KEY_markdir		},
	{ "mode",		KEY_mode		},
	{ "mtime",		KEY_mtime		},
	{ "name",		KEY_name		},
	{ "nlink",		KEY_nlink		},
	{ "path",		KEY_path		},
	{ "size",		KEY_size		},
	{ "summary",		KEY_summary		},
	{ "total.blocks",	KEY_total_blocks	},
	{ "total.files",	KEY_total_files		},
	{ "total.octets",	KEY_total_octets	},
	{ "trailer",		KEY_trailer		},
	{ "uid",		KEY_uid			},

	/* aliases */

	{ "dir.bytes",		KEY_dir_octets		},
	{ "linkname",		KEY_linkpath		},
	{ "total.bytes",	KEY_total_octets	},
};

static State_t		state;

/*
 * return a copy of s with unprintable chars replaced by ?
 */

static char*
printable(register char* s)
{
	register char*	t;
	register int	c;

	static char*	prdata;
	static int	prsize;

	if (state.lsflags & LS_ESCAPE)
	{
		if (!(state.lsflags & LS_QUOTE))
			return fmtesc(s);
		if (state.lsflags & LS_SHELL)
			return fmtquote(s, "$'", "'", strlen(s), 0);
		return fmtquote(s, "\"", "\"", strlen(s), 1);
	}
	c = strlen(s) + 4;
	if (c > prsize)
	{
		prsize = roundof(c, 512);
		if (!(prdata = newof(prdata, char, prsize, 0)))
			error(3, "out of space [printable data]");
	}
	t = prdata;
	if (state.lsflags & LS_QUOTE)
		*t++ = '"';
	while (c = *s++)
		*t++ = (iscntrl(c) || !isprint(c)) ? '?' : c;
	if (state.lsflags & LS_QUOTE)
		*t++ = '"';
	*t = 0;
	return prdata;
}

/*
 * sfkeyprintf() lookup
 */

static int
key(void* handle, register const char* name, const char* arg, int cc, char** ps, Sflong_t* pn)
{
	register Ftw_t*		ftw;
	register struct stat*	st;
	register char*		s = 0;
	register Sflong_t	n = 0;
	register Key_t*		kp;
	List_t*			lp;

	static Sfio_t*		mp;
	static const char	fmt_mode[] = "mode";

	if (lp = (List_t*)handle)
	{
		ftw = lp->ftw;
		st = &ftw->statb;
	}
	else
	{
		ftw = 0;
		st = 0;
	}
	if (!(kp = (Key_t*)hashget(state.keys, name)))
	{
		if (*name != '$')
			return 0;
		if (!(kp = newof(0, Key_t, 1, 0)))
			error(3, "out of space [key]");
		kp->name = hashput(state.keys, 0, kp);
		kp->macro = getenv(name + 1);
		kp->index = KEY_environ;
		kp->disable = 1;
	}
	if (kp->macro && !kp->disable)
	{
		kp->disable = 1;
		if (!mp && !(mp = sfstropen()))
			error(3, "out of space [macro]");
		sfkeyprintf(mp, handle, kp->macro, key, NiL);
		s = sfstruse(mp);
		kp->disable = 0;
	}
	else switch (kp->index)
	{
	case KEY_atime:
		if (st)
			n = st->st_atime;
		if (!arg)
			arg = state.timefmt;
		break;
	case KEY_blocks:
		if (st)
			n = iblocks(st);
		break;
	case KEY_ctime:
		if (st)
			n = st->st_ctime;
		if (!arg)
			arg = state.timefmt;
		break;
	case KEY_device:
		if (st && (S_ISBLK(st->st_mode) || S_ISCHR(st->st_mode)))
			s = fmtdev(st);
		else
			return 0;
		break;
	case KEY_devmajor:
		if (st)
			n = major(st->st_dev);
		break;
	case KEY_devminor:
		if (st)
			n = minor(st->st_dev);
		break;
	case KEY_dir_blocks:
		if (lp)
			n = lp->count.blocks;
		break;
	case KEY_dir_count:
		if (ftw != state.top)
		{
			if (state.lsflags & LS_SEPARATE)
				n = state.directories;
			else if (state.lsflags & LS_LABEL)
				n = 1;
		}
		break;
	case KEY_dir_files:
		if (lp)
			n = lp->count.files;
		break;
	case KEY_dir_octets:
		if (lp)
			n = lp->count.octets;
		break;
	case KEY_environ:
		if (!(s = kp->macro))
			return 0;
		break;
	case KEY_flags:
		s = state.flags;
		break;
	case KEY_gid:
		if (st)
		{
			if (cc == 's')
				s = fmtgid(st->st_gid);
			else
				n = st->st_gid;
		}
		break;
	case KEY_ino:
		if (st)
			n = st->st_ino;
		break;
	case KEY_linkpath:
		if (ftw && ftw->info == FTW_SL)
		{
			char*		dirnam;
			int		c;

			static char*	txtdata;
			static int	txtsize;

			if ((st->st_size + 1) > txtsize)
			{
				txtsize = roundof(st->st_size + 1, 512);
				if (!(txtdata = newof(txtdata, char, txtsize, 0)))
					error(3, "out of space [link text]");
			}
			if (*ftw->name == '/' || !lp->dirnam)
				dirnam = ftw->name;
			else
			{
				sfprintf(state.tmp, "%s/%s", lp->dirnam + streq(lp->dirnam, "/"), ftw->name);
				dirnam = sfstruse(state.tmp);
			}
			c = pathgetlink(dirnam, txtdata, txtsize);
			if (c > 0)
				s = PRINTABLE(txtdata);
		}
		else
			return 0;
		break;
	case KEY_linkop:
		if (ftw && ftw->info == FTW_SL)
			s = "->";
		else
			return 0;
		break;
	case KEY_mark:
		if (!st)
			return 0;
		else if (S_ISLNK(st->st_mode))
			s = "@";
		else if (S_ISDIR(st->st_mode))
			s = "/";
		else if (S_ISFIFO(st->st_mode))
			s = "|";
#ifdef S_ISSOCK
		else if (S_ISSOCK(st->st_mode))
			s = "=";
#endif
		else if (S_ISBLK(st->st_mode) || S_ISCHR(st->st_mode))
			s = "$";
		else if (st->st_mode & (S_IXUSR|S_IXGRP|S_IXOTH))
			s = "*";
		else
			return 0;
		break;
	case KEY_markdir:
		if (!st || !S_ISDIR(st->st_mode))
			return 0;
		s = "/";
		break;
	case KEY_mode:
		if (st)
			n = st->st_mode;
		if (!arg)
			arg = fmt_mode;
		break;
	case KEY_mtime:
		if (st)
			n = st->st_mtime;
		if (!arg)
			arg = state.timefmt;
		break;
	case KEY_name:
		if (ftw)
			s = PRINTABLE(ftw->name);
		break;
	case KEY_nlink:
		if (st)
			n = st->st_nlink;
		break;
	case KEY_path:
		if (ftw)
			s = ftw->path ? PRINTABLE(ftw->path) : PRINTABLE(ftw->name);
		break;
	case KEY_size:
		if (st)
			n = st->st_size;
		break;
	case KEY_total_blocks:
		n = state.total.blocks;
		break;
	case KEY_total_files:
		n = state.total.files;
		break;
	case KEY_total_octets:
		n = state.total.octets;
		break;
	case KEY_uid:
		if (st)
		{
			if (cc == 's')
				s = fmtuid(st->st_uid);
			else
				n = st->st_uid;
		}
		break;
	default:
		return 0;
	}
	if (s)
		*ps = s;
	else if (cc == 's' && arg)
	{
		if (strneq(arg, fmt_mode, 4))
			*ps = fmtmode(n, 1);
		else if (strneq(arg, state.timefmt, 4))
			*ps = fmttime((*(arg + 4) == '=' ? arg : state.timefmt) + 5, (time_t)n);
	}
	else
		*pn = n;
	return 1;
}

/*
 * print info on a single file
 * parent directory name is dirnam of dirlen chars
 */

static void
pr(register List_t* lp, Ftw_t* ftw, register int fill)
{
#ifdef S_ISLNK
	/*
	 * -H == --hairbrained
	 * no way around it - this is really ugly
	 * symlinks should be no more visible than mount points
	 * but I wear my user hat more than my administrator hat
	 */

	if (ftw->level == 0 && (state.ftwflags & (FTW_META|FTW_PHYSICAL)) == (FTW_META|FTW_PHYSICAL) && !(ftw->info & FTW_D) && !lstat(ftw->path ? ftw->path : ftw->name, &ftw->statb) && S_ISLNK(ftw->statb.st_mode))
		ftw->info = FTW_SL;
#endif
	lp->ftw = ftw;
	fill -= sfkeyprintf(sfstdout, lp, state.format, key, NiL);
	if (!(state.lsflags & LS_COMMAS))
	{
		if (fill > 0)
			while (fill-- > 0)
				sfputc(sfstdout, ' ');
		else
			sfputc(sfstdout, '\n');
	}
}

/*
 * pr() ftw directory child list in column order
 * directory name is dirnam of dirlen chars
 * count is the number of VISIBLE children
 * length is the length of the longest VISIBLE child
 */

static void
col(register List_t* lp, register Ftw_t* ftw, int length)
{
	register Ftw_t*	p;
	register int	i;
	register int	n;
	register int	files;
	int		m;

	lp->ftw = ftw;
	if (keys[KEY_header].macro && ftw->level >= 0)
		sfkeyprintf(sfstdout, lp, keys[KEY_header].macro, key, NiL);
	if ((files = lp->count.files) > 0)
	{
		if (!(state.lsflags & LS_COLUMNS) || length <= 0)
			n = 1;
		else
		{
			i = ftw->name[1];
			ftw->name[1] = 0;
			m = sfkeyprintf(state.tmp, lp, state.format, key, NiL) + 2;
			length += m;
			n = state.width / length;
			sfstrset(state.tmp, 0);
			ftw->name[1] = i;
		}
		if (state.lsflags & LS_COMMAS)
		{
			length = m - 1;
			i = 0;
			n = state.width;
			for (p = ftw->link; p; p = p->link)
				if (p->local.number != INVISIBLE)
				{
					if ((n -= length + p->namelen) < 0)
					{
						n = state.width - (length + p->namelen);
						if (i)
							sfputr(sfstdout, ",\n", -1);
					}
					else if (i)
						sfputr(sfstdout, ", ", -1);
					pr(lp, p, 0);
					i = 1;
				}
			if (i)
				sfputc(sfstdout, '\n');
		}
		else if (n <= 1)
		{
			for (p = ftw->link; p; p = p->link)
				if (p->local.number != INVISIBLE)
					pr(lp, p, 0);
		}
		else if (state.lsflags & LS_ACROSS)
		{
			i = 0;
			for (p = ftw->link; p; p = p->link)
				if (p->local.number != INVISIBLE)
					pr(lp, p, ++i >= n ? (i = 0) : length);
			if (i)
				sfputc(sfstdout, '\n');
		}
		else
		{
			register Ftw_t**	x;
			int			j;

			static Ftw_t**		vec;
			static int		vecsiz;

			if (n > (vecsiz - 1))
			{
				vecsiz = roundof(n + 1, 64);
				if (!(vec = newof(vec, Ftw_t*, vecsiz, 0)))
					error(3, "out of space");
			}
			x = vec;
			files = (files - 1) / n + 1;
			p = ftw->link;
			for (j = 0; j < n; j++)
				if (x[j] = p)
					for (i = 0; i < files; i++)
					{
						while (p && p->local.number == INVISIBLE) p = p->link;
						if (i == 0)
							x[j] = p;
						if (!p)
							break;
						p = p->link;
					}
			x[n] = 0;
			i = 0;
			for (;;)
			{
				if (p = x[i])
				{
					if (p->local.number == LISTED)
						x[i] = 0;
					else
					{
						p->local.number = LISTED;
						ftw = p;
						while ((p = p->link) && p->local.number == INVISIBLE);
						x[i++] = (p && p->local.number != LISTED) ? p : 0;
						pr(lp, ftw, x[i] ? length : 0);
					}
				}
				else if (i == 0)
					break;
				else
					i = 0;
			}
		}
	}
	if (keys[KEY_trailer].macro && ftw->level >= 0)
		sfkeyprintf(sfstdout, lp, keys[KEY_trailer].macro, key, NiL);
}

/*
 * order child entries
 */

static int
order(register Ftw_t* f1, register Ftw_t* f2)
{
	register int	n;
	char*		x1;
	char*		x2;

	if (state.sortflags & LS_NOSORT)
		return 0;
	if (!(state.lsflags & LS_DIRECTORY) && (state.ftwflags & FTW_MULTIPLE) && f1->level == 0)
	{
		if (f1->info == FTW_D)
		{
			if (f2->info != FTW_D)
				return 1;
		}
		else if (f2->info == FTW_D)
			return -1;
	}
	if (state.sortflags & LS_BLOCKS)
	{
		if (f1->statb.st_size < f2->statb.st_size)
			n = 1;
		else if (f1->statb.st_size > f2->statb.st_size)
			n = -1;
		else
			n = 0;
	}
	else if (state.sortflags & LS_TIME)
	{
		if (state.sortflags & LS_ATIME)
		{
			f1->statb.st_mtime = f1->statb.st_atime;
			f2->statb.st_mtime = f2->statb.st_atime;
		}
		else if (state.sortflags & LS_CTIME)
		{
			f1->statb.st_mtime = f1->statb.st_ctime;
			f2->statb.st_mtime = f2->statb.st_ctime;
		}
		if (f1->statb.st_mtime < f2->statb.st_mtime)
			n = 1;
		else if (f1->statb.st_mtime > f2->statb.st_mtime)
			n = -1;
		else
			n = 0;
	}
	else if (state.sortflags & LS_EXTENSION)
	{
		x1 = strrchr(f1->name, '.');
		x2 = strrchr(f2->name, '.');
		if (x1)
		{
			if (x2)
				n = strcoll(x1, x2);
			else
				n = 1;
		}
		else if (x2)
			n = -1;
		else
			n = 0;
		if (!n)
			n = strcoll(f1->name, f2->name);
	}
	else
		n = strcoll(f1->name, f2->name);
	if (state.sortflags & LS_REVERSE)
		n = -n;
	return n;
}

/*
 * list a directory and its children
 */

static void
dir(register Ftw_t* ftw)
{
	register Ftw_t*	p;
	register int	length;
	int		top = 0;
	List_t		list;

	if (ftw->status == FTW_NAME)
	{
		list.dirlen = ftw->namelen;
		list.dirnam = ftw->path + ftw->pathlen - list.dirlen;
	}
	else
	{
		list.dirlen = ftw->pathlen;
		list.dirnam = ftw->path;
	}
	if (ftw->level >= 0)
		state.directories++;
	else
		state.top = ftw;
	length = 0;
	list.count.blocks = 0;
	list.count.files = 0;
	list.count.octets = 0;
	for (p = ftw->link; p; p = p->link)
	{
		if (p->level == 0 && p->info == FTW_D && !(state.lsflags & LS_DIRECTORY))
		{
			p->local.number = INVISIBLE;
			top++;
		}
		else if (VISIBLE(p))
		{
			if (p->info == FTW_NS)
			{
				if (ftw->level < 0 || !(state.lsflags & LS_NOSTAT))
				{
					if (ftw->path[0] == '.' && !ftw->path[1])
						error(2, "%s: not found", p->name);
					else
						error(2, "%s/%s: not found", ftw->path, p->name);
					goto invisible;
				}
			}
			else
			{
				list.count.blocks += iblocks(&p->statb);
				list.count.octets += p->statb.st_size;
			}
			list.count.files++;
			if (p->namelen > length)
				length = p->namelen;
			if (!(state.lsflags & LS_RECURSIVE))
				p->status = FTW_SKIP;
		}
		else
		{
		invisible:
			p->local.number = INVISIBLE;
			p->status = FTW_SKIP;
		}
	}
	state.total.blocks += list.count.blocks;
	state.total.files += list.count.files;
	state.total.octets += list.count.octets;
	col(&list, ftw, length);
	state.lsflags |= LS_SEPARATE;
	if (top)
	{
		if (list.count.files)
		{
			state.directories++;
			state.top = 0;
		}
		else if (top > 1)
			state.top = 0;
		else
			state.top = ftw->link;
		for (p = ftw->link; p; p = p->link)
			if (p->level == 0 && p->info == FTW_D)
				p->local.number = 0;
	}
}

/*
 * list info on a single file
 */

static int
ls(register Ftw_t* ftw)
{
	if (!VISIBLE(ftw))
	{
		ftw->status = FTW_SKIP;
		return 0;
	}
	switch (ftw->info)
	{
	case FTW_NS:
		if (ftw->parent->info == FTW_DNX)
			break;
		error(2, "%s: not found", ftw->path);
		return 0;
	case FTW_DC:
		if (state.lsflags & LS_DIRECTORY)
			break;
		error(2, "%s: directory causes cycle", ftw->path);
		return 0;
	case FTW_DNR:
		if (state.lsflags & LS_DIRECTORY)
			break;
		error(2, "%s: cannot read directory", ftw->path);
		return 0;
	case FTW_D:
	case FTW_DNX:
		if ((state.lsflags & LS_DIRECTORY) && ftw->level >= 0)
			break;
		if (!(state.lsflags & LS_RECURSIVE))
			ftw->status = FTW_SKIP;
		else if (ftw->info == FTS_DNX)
		{
			error(2, "%s: cannot search directory", ftw->path, ftw->level);
			ftw->status = FTW_SKIP;
			if (ftw->level > 0 && !(state.lsflags & LS_NOSTAT))
				return 0;
		}
		dir(ftw);
		return 0;
	}
	ftw->status = FTW_SKIP;
	if (!ftw->level)
	{
		static List_t	list;

		list.ftw = ftw;
		pr(&list, ftw, 0);
	}
	return 0;
}

int
main(int argc, register char** argv)
{
	register int	n;
	register char*	s;
	Key_t*		kp;
	Sfio_t*		fmt;

	static char	fmt_color[] = "%(mode:case:d*:\\E[01;34m%(name)s\\E[0m:l*:\\E[01;36m%(name)s\\E[0m:*x*:\\E[01;32m%(name)s\\E[0m:*:%(name)s)s";

	NoP(argc);
	setlocale(LC_ALL, "");
	if (s = strrchr(argv[0], '/'))
		s++;
	else
		s = argv[0];
	error_info.id = s;
	state.ftwflags = ftwflags() | FTW_CHILDREN;
	if (!(fmt = sfstropen()) || !(state.tmp = sfstropen()))
		error(3, "out of space [tmp]");
	if (!(state.keys = hashalloc(NiL, HASH_name, "keys", 0)))
		error(3, "out of space [hash]");
	for (n = 1; n < elementsof(keys); n++)
		hashput(state.keys, keys[n].name, &keys[keys[n].index]);
	hashset(state.keys, HASH_ALLOCATE);
	if (streq(s, "lc"))
		state.lsflags |= LS_COLUMNS;
	else if (streq(s, "lf") || streq(s, "lsf"))
		state.lsflags |= LS_MARK;
	else if (streq(s, "ll"))
		state.lsflags |= LS_LONG;
	else if (streq(s, "lsr"))
		state.lsflags |= LS_RECURSIVE;
	else if (streq(s, "lsx"))
		state.lsflags |= LS_ACROSS|LS_COLUMNS;
	else if (isatty(1))
		state.lsflags |= LS_COLUMNS|LS_PRINTABLE;
	state.endflags = state.flags;
	state.blocksize = 512;
	state.timefmt = "time=%?%l";
	while (n = optget(argv, usage))
	{
		switch (n)
		{
		case 'a':
			state.lsflags |= LS_ALL;
			break;
		case 'b':
			state.lsflags |= LS_PRINTABLE|LS_ESCAPE;
			break;
		case 'c':
			state.lsflags &= ~LS_ATIME;
			state.lsflags |= LS_CTIME;
			break;
		case 'd':
			state.lsflags |= LS_DIRECTORY;
			break;
		case 'f':
			if (!sfstrtell(fmt))
				state.lsflags &= ~LS_COLUMNS;
			sfputr(fmt, opt_info.arg, ' ');
			break;
		case 'g':
		case 'O':
			if (opt_info.num)
				state.lsflags |= LS_LONG|LS_NOUSER;
			else
				state.lsflags |= LS_LONG|LS_NOGROUP;
			break;
		case 'i':
			state.lsflags |= LS_INUMBER;
			break;
		case 'k':
			state.blocksize = 1024;
			break;
		case 'l':
			state.lsflags |= LS_LONG;
			break;
		case 'm':
			state.lsflags |= LS_COMMAS;
			break;
		case 'n':
			state.lsflags |= LS_NUMBER;
			break;
		case 'o':
		case 'G':
			if (opt_info.num)
				state.lsflags |= LS_LONG|LS_NOGROUP;
			else
				state.lsflags |= LS_LONG|LS_NOUSER;
			break;
		case 'p':
			state.lsflags |= LS_MARKDIR;
			break;
		case 'q':
			state.lsflags |= LS_PRINTABLE;
			break;
		case 'r':
			state.lsflags |= LS_REVERSE;
			break;
		case 's':
			state.lsflags |= LS_BLOCKS;
			break;
		case 't':
			state.lsflags |= LS_TIME;
			break;
		case 'u':
			state.lsflags &= ~LS_CTIME;
			state.lsflags |= LS_ATIME;
			break;
		case 'w':
			state.width = opt_info.num;
			break;
		case 'x':
			state.lsflags |= LS_ACROSS|LS_COLUMNS;
			break;
		case 'y':
			if (!(s = opt_info.arg))
				state.sortflags = LS_NOSORT;
			else
			{
				while (*s == '-')
					s++;
				switch (*s)
				{
				case 'a':
					state.sortflags = LS_TIME|LS_ATIME;
					break;
				case 'b':
					state.sortflags = LS_BLOCKS;
					break;
				case 'c':
					state.sortflags = LS_TIME|LS_CTIME;
					break;
				case 'e':
				case 'X':
					state.sortflags = LS_EXTENSION;
					break;
				case 'f':
				case 0:
					state.sortflags = 0;
					break;
				case 'm':
				case 't':
					state.sortflags = LS_TIME;
					break;
				case 'n':
					state.sortflags = (*(s + 1) == 'o') ? LS_NOSORT : 0;
					break;
				case 's':
					state.sortflags = (*(s + 1) == 'i') ? LS_BLOCKS : (LS_TIME|LS_CTIME);
					break;
				default:
					error(2, "%s: unknown %s key", opt_info.arg, opt_info.name);
					break;
				}
			}
			break;
		case 'z':
			state.timefmt = "time=%K";
			break;
		case 'A':
			state.lsflags |= LS_MOST;
			state.lsflags &= ~LS_ALL;
			break;
		case 'B':
			state.lsflags |= LS_NOBACKUP;
			break;
		case 'C':
			state.lsflags |= LS_COLUMNS;
			break;
		case 'D':
			if (s = strchr(opt_info.arg, '='))
				*s++ = 0;
			if (*opt_info.arg == 'n' && *(opt_info.arg + 1) == 'o')
			{
				opt_info.arg += 2;
				s = 0;
			}
			if (!(kp = (Key_t*)hashget(state.keys, opt_info.arg)))
			{
				if (!s)
					break;
				if (!(kp = newof(0, Key_t, 1, 0)))
					error(3, "out of space [key]");
				kp->name = hashput(state.keys, 0, kp);
			}
			if (kp->macro = s)
			{
				stresc(s);
				if (strmatch(s, "*:case:*"))
					state.lsflags |= LS_STAT;
			}
			break;
		case 'F':
			state.lsflags |= LS_MARK;
			break;
		case 'H':
			state.ftwflags |= FTW_META|FTW_PHYSICAL;
			break;
		case 'I':
			state.ignore = opt_info.arg;
			break;
		case 'K':
			state.lsflags |= LS_PRINTABLE|LS_SHELL|LS_QUOTE|LS_ESCAPE;
			break;
		case 'L':
			state.ftwflags &= ~(FTW_META|FTW_PHYSICAL);
			break;
		case 'N':
			state.lsflags &= ~LS_PRINTABLE;
			break;
		case 'P':
			state.ftwflags &= ~FTW_META;
			state.ftwflags |= FTW_PHYSICAL;
			break;
		case 'Q':
			state.lsflags |= LS_PRINTABLE|LS_QUOTE;
			break;
		case 'R':
			state.lsflags |= LS_RECURSIVE;
			break;
		case 'S':
			state.lsflags |= LS_BLOCKS;
			break;
		case 'V':
			if (!(s = opt_info.arg))
				s = "f";
			else
				while (*s == '-')
					s++;
			switch (*s)
			{
			case 'a':
			case 'f':
			case 't':
				if ((s[0] == 'f' || s[1] == 'l' || isatty(1)) && (kp = (Key_t*)hashget(state.keys, "name")))
				{
					stresc(kp->macro = fmt_color);
					state.lsflags |= LS_STAT;
				}
				break;
			case 'n':
			case 0:
				break;
			default:
				error(2, "%s: unknown %s key", opt_info.arg, opt_info.name);
				break;
			}
			break;
		case 'W':
			for (s = opt_info.arg; *s == '-'; s++);
			switch (*s)
			{
			case 'a':
				state.timeflags = LS_ATIME;
				break;
			case 'c':
			case 's':
				state.timeflags = LS_CTIME;
				break;
			case 'm':
			case 't':
			case 0:
				state.timeflags = 0;
				break;
			default:
				error(2, "%s: unknown %s key", opt_info.arg, opt_info.name);
				break;
			}
			break;
		case 'X':
			state.lsflags |= LS_EXTENSION;
			break;
		case 'Y':
			for (s = opt_info.arg; *s == '-'; s++);
			switch (*s++)
			{
			case 'a':
			case 'h':
				state.lsflags |= LS_ACROSS|LS_COLUMNS;
				break;
			case 'c':
				state.lsflags |= LS_COMMAS;
				break;
			case 'l':
				state.lsflags |= LS_LONG;
				break;
			case 's':
			case '1':
				state.lsflags &= ~(LS_ACROSS|LS_COLUMNS);
				break;
			case 'v':
				if (*s++ != 'e' || *s++ != 'r' || *s++ != 't')
				{
					state.lsflags |= LS_LONG;
					break;
				}
				/*FALLTHROUGH*/
			case 'x':
				state.lsflags &= ~LS_ACROSS;
				state.lsflags |= LS_COLUMNS;
				break;
			case 0:
				break;
			default:
				error(2, "%s: unknown %s key", opt_info.arg, opt_info.name);
				break;
			}
			break;
		case 'Z':
			state.timefmt = "time=%c";
			break;
		case '1':
			state.lsflags &= ~(LS_COLUMNS|LS_PRINTABLE);
			break;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			break;
		case ':':
			error(2, "%s", opt_info.arg);
			break;
		default:
			error(1, "%s: option not implemented", opt_info.name);
			continue;
		}
		if (!strchr(state.flags, n))
			*state.endflags++ = n;
	}
	argv += opt_info.index;
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	if (state.lsflags & LS_DIRECTORY)
		state.lsflags &= ~LS_RECURSIVE;
	if (!state.sortflags)
		state.sortflags = state.lsflags;
	if (state.lsflags & LS_REVERSE)
		state.sortflags |= LS_REVERSE;
	if (!state.timeflags)
		state.timeflags = state.lsflags;
	if (state.lsflags & (LS_COLUMNS|LS_COMMAS))
	{
		if (state.lsflags & LS_LONG)
			state.lsflags &= ~(LS_COLUMNS|LS_COMMAS);
		else if (!state.width)
		{
			astwinsize(1, NiL, &state.width);
			if (state.width <= 20)
				state.width = 80;
		}
	}
	if (state.lsflags & LS_STAT)
		state.lsflags &= ~LS_NOSTAT;
	else if (!(state.lsflags & (LS_DIRECTORY|LS_BLOCKS|LS_LONG|LS_MARK|LS_MARKDIR|LS_TIME
#if !_mem_d_fileno_dirent && !_mem_d_ino_dirent
		|LS_INUMBER
#endif
		)) && !sfstrtell(fmt))
	{
		state.lsflags |= LS_NOSTAT;
		state.ftwflags |= FTW_DELAY|FTW_DOT;
	}
	if (!sfstrtell(fmt))
	{
		if (state.lsflags & LS_INUMBER)
			sfputr(fmt, "%6(ino)u ", -1);
		if (state.lsflags & LS_BLOCKS)
			sfputr(fmt, "%5(blocks)u ", -1);
		if (state.lsflags & LS_LONG)
		{
			sfputr(fmt, "%(mode)s%3(nlink)u", -1);
			if (!(state.lsflags & LS_NOUSER))
				sfprintf(fmt, " %%-8(uid)%c", (state.lsflags & LS_NUMBER) ? 'd' : 's');
			if (!(state.lsflags & LS_NOGROUP))
				sfprintf(fmt, " %%-8(gid)%c", (state.lsflags & LS_NUMBER) ? 'd' : 's');
			sfputr(fmt, "%8(device:case::%(size)u:*:%(device)s)s", -1);
			sfprintf(fmt, " %%(%s)s ", (state.timeflags & LS_ATIME) ? "atime" : (state.timeflags & LS_CTIME) ? "ctime" : "mtime");
		}
		sfputr(fmt, "%(name)s", -1);
		if (state.lsflags & LS_MARK)
			sfputr(fmt, "%(mark)s", -1);
		else if (state.lsflags & LS_MARKDIR)
			sfputr(fmt, "%(markdir)s", -1);
		if (state.lsflags & LS_LONG)
			sfputr(fmt, "%(linkop:case:?*: %(linkop)s %(linkpath)s)s", -1);
	}
	else
		sfstrrel(fmt, -1);
	state.format = sfstruse(fmt);
	stresc(state.format);

	/*
	 * do it
	 */

	if (argv[0])
	{
		if (argv[1])
			state.lsflags |= LS_LABEL;
		state.ftwflags |= FTW_MULTIPLE;
		ftwalk((char*)argv, ls, state.ftwflags, order);
	}
	else
		ftwalk(".", ls, state.ftwflags, order);
	if (keys[KEY_summary].macro)
		sfkeyprintf(sfstdout, NiL, keys[KEY_summary].macro, key, NiL);
	exit(error_info.errors != 0);
}
