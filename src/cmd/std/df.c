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
 * df -- free disk block report
 */

static const char usage[] =
"[-?\n@(#)df (AT&T Labs Research) 1999-06-11\n]"
USAGE_LICENSE
"[+NAME?df - summarize disk free space]"
"[+DESCRIPTION?\bdf\b displays the available disk space for the filesystem"
"	of each file argument. If no \afile\a arguments are given then the"
"	all mounted filesystems are displayed.]"

"[b:blockbytes?Measure disk usage in 512 byte blocks. This is the default"
"	if \bgetconf CONFORMANCE\b is \bstandard\b.]"
"[i:inodes?Display inode usage instead of block usage. There is at least one"
"	inode for each active file and directory on a filesystem.]"
"[k:kilobytes?Measure disk usage in 1024 byte blocks.]"
"[l:local?List information on local filesystems only, i.e., network"
"	mounts are omitted.]"
"[m:megabytes?Measure disk usage in 1024K byte blocks. This is the default"
"	if \bgetconf CONFORMANCE\b is not \bstandard\b.]"
"[n:native-block?Measure disk usage in the native filesystem block size."
"	This size may vary between filesystems, so i is displayed in the"
"	\bSiz\b column.]"
"[O:options?Display the \bmount\b(1) options.]"
"[P:portable?Display each filesystem on one line. By default output is"
"	folded for readability. Also implies \b--blockbytes\b.]"
"[s!:sync?Call \bsync\b(2) before querying the filesystems.]"
"[F:type?Display all filesystems of type \atype\a. Unknown types are"
"	listed as \blocal\b. Typical (but not supported on all systems) values"
"	are:]:[type]{"
"		[+ufs?default UNIX file system]"
"		[+ext2?default linux file system]"
"		[+xfs?sgi XFS]"
"		[+nfs?network file system version 2]"
"		[+nfs3?network file system version 3]"
"		[+afs3?Andrew file system]"
"		[+proc?process file system]"
"		[+fat?DOS FAT file system]"
"		[+ntfs?nt file system]"
"		[+reg?windows/nt registry file system]"
"		[+lofs?loopback file system for submounts]"
"}"
"[v:verbose?Report all filesystem query errors.]"
"[f|q|t?Ignored by this implementation.]"

"\n"
"\n[ file ... ]\n"
"\n"
"[+SEE ALSO?\bgetconf\b(1), \bmount\b(1), \bmount\b(2)]"
;

#include <ast.h>
#include <error.h>
#include <ls.h>
#include <mnt.h>

/*
 * man page and header comments notwithstanding
 * some systems insist on reporting block counts in 512 units
 * let me know how you probe for this
 */

#if _SCO_COFF || _SCO_ELF
#define F_FRSIZE(v)	(512)
#else
#if _mem_f_frsize_statvfs
#define F_FRSIZE(v)	((v)->f_frsize?(v)->f_frsize:(v)->f_bsize)
#else
#define F_FRSIZE(v)	((v)->f_bsize)
#endif
#endif

#if _mem_f_basetype_statvfs
#define F_BASETYPE(v,p)	((v)->f_basetype)
#else
static char*
basetype(const char* path)
{
	struct stat	st;

	return stat(path, &st) ? "ufs" : fmtfs(&st);
}
#define F_BASETYPE(v,p)	(basetype(p))
#endif

#define REALITY(n)	((((long)(n))<0)?0:(n))
#define UNKNOWN		"local"

#if _lib_sync
extern void	sync(void);
#endif

static struct
{
	int		block;		/* block unit			*/
	int		inode;		/* report free inodes too	*/
	int		local;		/* local mounts only		*/
	int		options;	/* list mount options		*/
	int		posix;		/* posix format			*/
	int		sync;		/* sync() first			*/
	int		verbose;	/* verbose message level {-1,1}	*/
	char*		sep;		/* size separation		*/
	char*		type;		/* type pattern			*/
} state;

static void
title(void)
{
	state.sep = state.block == 1024 * 1024 ? "" : "  ";
	sfprintf(sfstdout, "Filesystem   ");
	if (state.options)
	{
		state.posix = 0;
		sfprintf(sfstdout, "       Type  Options                      ");
	}
	else
	{
		if (!state.posix)
			sfprintf(sfstdout, "       Type");
		if (!state.block)
			sfprintf(sfstdout, " Siz");
		if (state.posix)
			sfprintf(sfstdout, " %7d-blocks", state.block);
		else
			sfprintf(sfstdout, " %s", state.block == 1024 * 1024 ? "Mbytes" : state.block == 1024 ? "  Kbytes" : "  Blocks");
		if (!state.inode)
			sfprintf(sfstdout, "%s   Used", state.sep);
		if (state.posix)
			sfprintf(sfstdout, " Available Capacity");
		else
			sfprintf(sfstdout, "%s  Avail  Cap", state.sep);
		if (state.inode)
			sfprintf(sfstdout, " Inodes Iavail Icap");
	}
	sfprintf(sfstdout, "  Mounted on\n");
}

static void
scale(int m, unsigned long w, unsigned long p)
{
	if (state.block != 1024 * 1024)
		sfprintf(sfstdout, " %8lu", w);
	else if (!m || !w && !p || w > 9)
		sfprintf(sfstdout, " %6lu", w);
	else
		sfprintf(sfstdout, " %4lu.%lu", w, p);
}

static void
entry(Mnt_t* mnt, struct statvfs* vfs, const char* name, const char* type, const char* mounted, int flags)
{
	unsigned long	a;
	unsigned long	ta;
	unsigned long	t;
	unsigned long	tt;
	unsigned long	u;
	unsigned long	tu;
	unsigned long	b;
	char*		x;
	int		p;
	int		s;
	int		m;

	if ((!state.type || strmatch(type, state.type)) && (!state.local || !(flags & MNT_REMOTE)))
	{
		if (REALITY(vfs->f_blocks) == 0)
		{
			a = t = u = 0;
			p = 0;
		}
		else
		{
			/*
			 * NOTE: on some systems vfs.f_* are unsigned,
			 *	 and on others signed negative values
			 *	 denote error
			 */

			t = vfs->f_blocks;
			u = ((long)vfs->f_blocks <= (long)vfs->f_bfree) ? 0 : (vfs->f_blocks - vfs->f_bfree);
			a = ((long)vfs->f_bavail < 0) ? 0 : vfs->f_bavail;
			p = (tt = u + a) ? (unsigned long)(((double)u / (double)tt + 0.005) * 100.0) : 0;
		}
		m = 0;
		ta = tt = tu = 0;
		if (state.block)
		{
			b = F_FRSIZE(vfs);
			if (b > state.block)
			{
				s = b / state.block;
				a *= s;
				t *= s;
				u *= s;
			}
			else if (b < state.block)
			{
				s = state.block / b;
				if (m = s / 10)
				{
					ta = (a / m) % 10;
					tt = (t / m) % 10;
					tu = (u / m) % 10;
				}
				a /= s;
				t /= s;
				u /= s;
			}
		}
		x = "";
		if (state.posix)
			sfprintf(sfstdout, "%-19s%s", name, state.block == 1024 * 1024 ? "  " : "");
		else
		{
			if ((s = 24 - strlen(name) - strlen(type)) <= 0)
			{
				if (state.posix) s = 1;
				else
				{
					x = "\n";
					s = 25 - strlen(type);
				}
			}
			sfprintf(sfstdout, "%s%-*.*s%s", name, s, s, x, type);
		}
		if (state.options)
			sfprintf(sfstdout, "  %-28s", mnt && mnt->options ? mnt->options : "");
		else
		{
			if (!state.block)
			{
				if (vfs->f_bsize >= 1024)
					sfprintf(sfstdout, "%3dk", vfs->f_bsize / 1024);
				else
					sfprintf(sfstdout, "%4d", vfs->f_bsize);
			}
			if (t || tt)
			{
				scale(m, t, tt);
				if (!state.inode)
					scale(m, u, tu);
				if (state.posix)
					sfprintf(sfstdout, " %s", state.block == 1024 * 1024 ? "  " : "");
				scale(m, a, ta);
				sfprintf(sfstdout, " %s%3d%%", state.posix ? "    " : "", p);
			}
			else if (state.inode)
			{
				if (state.posix)
					sfprintf(sfstdout, "      %s-      %s-         -", state.sep, state.sep);
				else
					sfprintf(sfstdout, "      %s-      %s-    -", state.sep, state.sep);
			}
			else if (state.posix)
				sfprintf(sfstdout, "      %s-      %s-       %s-        -", state.sep, state.sep, state.sep);
			else
				sfprintf(sfstdout, "      %s-      %s-      %s-    -", state.sep, state.sep, state.sep);
			if (state.inode)
			{
				t = REALITY(vfs->f_files);
				a = REALITY(vfs->f_ffree);
				if (t < a)
					u = 0;
				else
					u = t - a;
				a = REALITY(vfs->f_favail);
				p = (tt = u + a) ? (unsigned long)(((double)u / (double)tt + 0.005) * 100.0) : 0;
				if (t) sfprintf(sfstdout, " %6lu %6lu %3d%%", t, a, p);
				else
					sfprintf(sfstdout, "      -      -    -");
			}
		}
		sfprintf(sfstdout, "  %s\n", mounted);
	}
}

int
main(int argc, register char** argv)
{
	register int	n;
	register char*	dir;
	int		rem;
	dev_t		dirdev;
	dev_t		mntdev;
	dev_t*		dev = 0;
	void*		mp;
	Mnt_t*		mnt;
	struct stat	st;
	struct statvfs	vfs;

	error_info.id = "df";
	state.block = -1;
	state.posix = -1;
	state.sync = 1;
	state.verbose = -1;
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'b':
			state.block = 512;
			continue;
		case 'F':
			state.type = opt_info.arg;
			continue;
		case 'i':
			state.inode = 1;
			continue;
		case 'k':
			state.block = 1024;
			continue;
		case 'l':
			state.local = 1;
			continue;
		case 'm':
			state.block = 1024 * 1024;
			continue;
		case 'n':
			state.block = 0;
			continue;
		case 'O':
			state.options = 1;
			continue;
		case 'P':
			state.posix = 1;
			continue;
		case 's':
			state.sync = opt_info.num;
			continue;
		case 'v':
			state.verbose = ERROR_SYSTEM|1;
			continue;
		case 'f':
			/* ignore or obsolete */
			continue;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			break;
		case ':':
			error(2, "%s", opt_info.arg);
			break;
		}
		break;
	}
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	argc -= opt_info.index;
	argv += opt_info.index;
	if (state.posix < 0)
		state.posix = !strcmp(astconf("CONFORMANCE", NiL, NiL), "standard");
	if (state.block < 0)
		state.block = state.posix ? 512 : 1024 * 1024;
#if _lib_sync
	if (state.sync)
		sync();
#endif
	if (!(mp = mntopen(NiL, "r")))
	{
		error(ERROR_SYSTEM|(argc > 0 ? 1 : 3), "cannot access mount table");
		state.options = 0;
		title();
		while (dir = *argv++)
			if (statvfs(dir, &vfs))
				error(ERROR_SYSTEM|2, "%s: cannot stat filesystem", dir);
			else
				entry(NiL, &vfs, dir, F_BASETYPE(&vfs, dir), UNKNOWN, 0);
	}
	else
	{
		title();
		if (rem = argc)
		{
			if (!(dev = newof(0, dev_t, argc, 0)))
				error(ERROR_SYSTEM|3, "out of space [dev_t]");
			for (n = 0; n < argc; n++)
				if (stat(argv[n], &st))
				{
					error(ERROR_SYSTEM|2, "%s: cannot stat", argv[n]);
					argv[n] = 0;
					rem--;
				}
				else
					dev[n] =
#if _mem_st_rdev_stat
					S_ISBLK(st.st_mode) ? st.st_rdev :
#endif
					st.st_dev;
		}
		while ((!argc || rem) && (mnt = mntread(mp)))
		{
			dir = mnt->dir;
			if (stat(dir, &st))
			{
				if (errno != ENOENT && errno != ENOTDIR)
					error(state.verbose, "%s: cannot stat", dir);
				continue;
			}
			dirdev = st.st_dev;
			mntdev = stat(mnt->fs, &st) ? dirdev : st.st_dev;
			if (argc)
			{
				for (n = 0; n < argc; n++)
					if (argv[n] && (dev[n] == dirdev || dev[n] == mntdev))
					{
						argv[n] = 0;
						rem--;
						break;
					}
				if (n >= argc)
					continue;
			}
			if (statvfs(dir, &vfs) && statvfs(mnt->fs, &vfs))
				error(ERROR_SYSTEM|2, "%s: cannot stat filesystem", dir);
			else
				entry(mnt, &vfs, mnt->fs, mnt->type, dir, mnt->flags);
			if (argc > 0)
			{
				while (++n < argc)
					if (dev[n] == dirdev || dev[n] == mntdev)
					{
						argv[n] = 0;
						rem--;
					}
				if (rem <= 0)
					break;
			}
		}
		mntclose(mp);
		if (argc > 0)
			for (n = 0; n < argc; n++)
				if (dir = argv[n])
				{
					if (statvfs(dir, &vfs))
						error(ERROR_SYSTEM|2, "%s: mount point not found", dir);
					else
						entry(NiL, &vfs, dir, F_BASETYPE(&vfs, dir), UNKNOWN, 0);
				}
	}
	exit(0);
}
