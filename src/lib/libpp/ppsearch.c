/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1986-2000 AT&T Corp.                *
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
*                                                                  *
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * include file search support
 */

#include "pplib.h"

#define SEARCH_NEXT	(SEARCH_USER<<1)/* search for next (uncover)	*/
#define SEARCH_SKIP	(SEARCH_USER<<2)/* current binding skipped	*/

#define COLUMN_TAB	7
#define COLUMN_MAX	72

#if ARCHIVE

#include <vdb.h>
#include <ls.h>

#endif

/*
 * multiple include test
 * fp is a canonicalized ppfile pointer
 *
 * test
 *
 *	INC_CLEAR	can be included again
 *	INC_TEST	test if include required
 *	<symbol>	ifndef guard symbol
 *
 * test!=INC_CLEAR returns 1 if file can be included again
 *
 * NOTE:
 *
 *  (1)	different hard links to the same file are treated as
 *	different files
 *
 *  (2)	symbolic links in combination with .. may cause two
 *	different files to be treated as the same file:
 *
 *	"../h/<file>" == "/usr/include/sys/../h/<file>" -> "/usr/include/h/<file>"
 *	"h/<file>" -> "/usr/include/h/<file>"
 */

int
ppmultiple(register struct ppfile* fp, register struct ppsymbol* test)
{
	register struct ppsymbol*	status;

	status = fp->guard;
	debug((-4, "ppmultiple(%s) status=%s%s test=%s", fp->name, status == INC_CLEAR ? "[CLEAR]" : status == INC_TEST ? "[ONCE]" : status == INC_IGNORE ? "[IGNORE]" : status->name, (pp.mode & HOSTED) ? "[HOSTED]" : "", test == INC_CLEAR ? "[CLEAR]" : test == INC_TEST ? "[TEST]" : test->name));
	if (status == INC_IGNORE)
	{
		message((-2, "%s: ignored [%s]", fp->name, pp.ignore));
		return(0);
	}
	if (test == INC_TEST)
	{
		if (status != INC_CLEAR)
		{
			if (status != INC_TEST && status->macro || !(pp.mode & ALLMULTIPLE) && !(pp.state & STRICT))
			{
				if ((pp.mode & (ALLMULTIPLE|LOADING)) == LOADING)
					fp->guard = INC_IGNORE;
				if (pp.state & WARN) error(1, "%s: ignored -- already included", fp->name);
				else message((-3, "%s: ignored -- already included", fp->name));
				return(0);
			}
			return(1);
		}
		if ((pp.mode & (ALLMULTIPLE|LOADING)) == LOADING)
			test = INC_IGNORE;
	}
	fp->guard = test;
	return(1);
}

/*
 * search for file using directories in dp
 */

static int
search(register struct ppfile* fp, register struct ppdirs* dp, int type, int flags)
{
	register char*		prefix;
	register struct ppdirs*	up;
	register struct ppfile*	xp;
	struct ppfile*		mp;
	int			fd;
	int			index;
	char*			t;

	if (!(pp.option & PREFIX)) prefix = 0;
	else if ((prefix = strrchr(fp->name, '/')) && prefix > fp->name)
	{
		*prefix = 0;
		t = ppsetfile(fp->name)->name;
		*prefix = '/';
		prefix = t;
	}
	message((-3, "search: %s %s%s type=%s prefix=%s flags=|%s%s%s%s%s start=%s=\"%s\" pre=%s lcl=%s std=%s",
		fp->name,
		(flags & SEARCH_INCLUDE) ? "include" : "exists",
		(flags & SEARCH_NEXT) ? " next" : "",
		type == T_HEADER ? "<*>" : "\"*\"", prefix,
		(fp->flags & INC_SELF) ? "SELF|" : "",
		(fp->flags & INC_EXISTS) ? "EXISTS|" : "",
		(fp->flags & INC_BOUND(INC_PREFIX)) ? "PREFIX|" : "",
		(fp->flags & INC_BOUND(INC_LOCAL)) ? "LOCAL|" : "",
		(fp->flags & INC_BOUND(INC_STANDARD)) ? "STANDARD|" : "",
		dp ? (dp->index == INC_PREFIX ? "pre" : dp->index == INC_LOCAL ? "lcl" : "std") : NiL,
		dp ? dp->name : NiL,
		!(fp->flags & INC_MEMBER(INC_PREFIX)) && (xp = fp->bound[INC_PREFIX]) ? xp->name : NiL,
		!(fp->flags & INC_MEMBER(INC_LOCAL)) && (xp = fp->bound[INC_LOCAL]) ? xp->name : NiL,
		!(fp->flags & INC_MEMBER(INC_STANDARD)) && (xp = fp->bound[INC_STANDARD]) ? xp->name : NiL
		));
	for (index = -1; dp; dp = dp->next) if (dp->name)
	{
#if ARCHIVE
		if (!dp->type)
		{
			struct stat	st;

			if (stat(dp->name, &st))
			{
				message((-3, "search: omit %s", dp->name));
				dp->name = 0;
				continue;
			}
			if (S_ISREG(st.st_mode))
			{
				register char*		s;
				char*			e;
				int			delimiter;
				int			variant;
				unsigned long		siz;
				unsigned long		off;
				struct ppmember*	ap;
				Sfio_t*			sp;

				/*
				 * check for vdb header archive
				 */

				if (!(sp = sfopen(NiL, dp->name, "r")))
				{
					error(ERROR_SYSTEM|1, "%s: ignored -- cannot open", dp->name);
					dp->name = 0;
					continue;
				}
				variant = sfsprintf(pp.tmpbuf, MAXTOKEN, "%c%s%c%s:archive", VDB_DELIMITER, VDB_MAGIC, VDB_DELIMITER, pp.pass);
				if (!(s = sfgetr(sp, '\n', 1)) || !strneq(s, pp.tmpbuf, variant))
				{
					sfclose(sp);
					error(1, "%s: ignored -- not a directory or archive", dp->name);
					dp->name = 0;
					continue;
				}

				/*
				 * parse the options
				 */

				dp->type |= TYPE_ARCHIVE;
				for (s += variant;;)
				{
					while (*s == ' ') s++;
					e = s;
					for (t = 0; *s && *s != ' '; s++)
						if (*s == '=')
						{
							*s = 0;
							t = s + 1;
						}
					if (*s) *s++ = 0;
					if (!*e) break;
					switch ((int)hashref(pp.strtab, e))
					{
					case X_CHECKPOINT:
#if CHECKPOINT
						dp->type |= TYPE_CHECKPOINT;
						break;
#else
						error(1, "preprocessor not compiled with checkpoint enabled");
						goto notvdb;
#endif
					case X_HIDE:

						if (t) error(1, "%s: %s: archive option value ignored", e);
						if (e = strrchr(dp->name, '/')) *e = 0;
						else dp->name = ".";
						break;
					case X_MAP:
						if (!t) error(1, "%s: archive option value expected", e);
						else dp->name = strdup(t);
						break;
					default:
						error(1, "%s: unknown archive option", e);
						break;
					}
				}
				if (sfseek(sp, -(VDB_LENGTH + 1), SEEK_END) <= 0 || !(s = sfgetr(sp, '\n', 1)))
				{
				notvdb:
					sfclose(sp);
					error(1, "%s: ignored -- cannot load archive", dp->name);
					dp->name = 0;
					continue;
				}
				if (variant = *s != 0) s++;
				else if (!(s = sfgetr(sp, '\n', 1))) goto notvdb;
				if (sfvalue(sp) != (VDB_LENGTH + variant)) goto notvdb;
				if (!strneq(s, VDB_DIRECTORY, sizeof(VDB_DIRECTORY) - 1)) goto notvdb;
				delimiter = s[VDB_OFFSET - 1];
				off = strtol(s + VDB_OFFSET, NiL, 10) - sizeof(VDB_DIRECTORY);
				siz = strtol(s + VDB_SIZE, NiL, 10);
				if (sfseek(sp, off, SEEK_SET) != off) goto notvdb;
				if (!(s = sfreserve(sp, siz + 1, 0))) goto notvdb;
				s[siz] = 0;
				if (!strneq(s, VDB_DIRECTORY, sizeof(VDB_DIRECTORY)) - 1) goto notvdb;
				if (!(s = strchr(s, '\n'))) goto notvdb;
				s++;
				while (e = strchr(s, '\n'))
				{
					delimiter = variant ? *s++ : delimiter;
					if (!(t = strchr(s, delimiter))) break;
					*t = 0;
					if (!streq(s, VDB_DIRECTORY))
					{
						pathcanon(s, 0);
						ap = newof(0, struct ppmember, 1, 0);
						ap->archive = dp;
						ap->offset = strtol(t + 1, &t, 10);
						ap->size = strtol(t + 1, NiL, 10);
						xp = ppsetfile(s);
						xp->flags |= INC_MEMBER(dp->index);
						xp->bound[dp->index] = (struct ppfile*)ap;
if (pp.test & 0x0020) error(1, "VDB#%d %s %s index=%d data=<%lu,%lu>", __LINE__, dp->name, xp->name, index, ap->offset, ap->size);
					}
					s = e + 1;
				}
				if (sfseek(sp, 0L, SEEK_SET)) goto notvdb;
				if (!(pp.test & 0x4000) &&
#if POOL
					(pp.pool.input || !(dp->type & TYPE_CHECKPOINT))
#else
					!(dp->type & TYPE_CHECKPOINT)
#endif
					&& (dp->info.buffer = sfreserve(sp, off, 0)))
					dp->type |= TYPE_BUFFER;
				else
				{
					dp->info.sp = sp;
#if POOL
					if (pp.pool.input)
						sfset(sp, SF_SHARE, 1);
#endif
				}
			}
			else dp->type |= TYPE_DIRECTORY;
		}
#endif
		if (streq(fp->name, ".")) continue;
		if (prefix && *fp->name != '/' && dp->index != INC_PREFIX)
#if ARCHIVE
		if (dp->type & TYPE_DIRECTORY)
#endif
		{
			for (up = dp->info.subdir; up; up = up->next)
				if (up->name == prefix) break;
			if (!up)
			{
				up = newof(0, struct ppdirs, 1, 0);
				up->name = prefix;
				up->next = dp->info.subdir;
				dp->info.subdir = up;
				if (!*dp->name) t = prefix;
				else sfsprintf(t = pp.path, PATH_MAX - 1, "%s/%s", dp->name, prefix);
				if (!(up->hosted = !access(t, X_OK)))
				{
					message((-3, "search: omit %s", t));
					continue;
				}
			}
			else if (!up->hosted) continue;
		}
		mp = xp = 0;
		if (!(flags & SEARCH_NEXT) && index != dp->index)
		{
			if (index >= 0 && !(fp->flags & INC_MEMBER(index)))
				fp->flags |= INC_BOUND(index);
			index = dp->index;
			if (fp->flags & INC_BOUND(index))
			{
				xp = fp->bound[index];
				if (index == INC_PREFIX)
				{
					if (*fp->name == '/' || !*dp->name) strcpy(pp.path, fp->name);
					else sfsprintf(pp.path, PATH_MAX - 1, "%s/%s", dp->name, fp->name);
					pathcanon(pp.path, 0);
					if (!xp || !streq(xp->name, pp.path))
					{
						fp->bound[index] = xp = ppsetfile(pp.path);
						if (!(flags & SEARCH_INCLUDE) && (xp->flags & INC_EXISTS) || (flags & (SEARCH_INCLUDE|SEARCH_NEXT)) == SEARCH_INCLUDE)
						{
							if (!(flags & SEARCH_INCLUDE) || !ppmultiple(xp, INC_TEST))
								return(0);
							mp = xp;
						}
					}
				}
				else if (!xp)
				{
					while (dp->next && dp->next->index == index) dp = dp->next;
					message((-3, "search: omit %s/%s", dp->name, fp->name));
					continue;
				}
				else
				{
					strcpy(pp.path, xp->name);
					if (!(flags & SEARCH_INCLUDE) || !ppmultiple(xp, INC_TEST))
						return(0);
					mp = xp;
				}
			}
		}
		if (!(fp->flags & INC_BOUND(index)) || (flags & SEARCH_NEXT))
		{
			if (*fp->name == '/' || !*dp->name) strcpy(pp.path, fp->name);
			else sfsprintf(pp.path, PATH_MAX - 1, "%s/%s", dp->name, fp->name);
			pathcanon(pp.path, 0);
			if (!(flags & SEARCH_SKIP))
			{
				if (streq(error_info.file, pp.path))
					flags |= SEARCH_SKIP;
				continue;
			}
		}
		if ((xp || (xp = ppgetfile(pp.path))) && (xp->flags & INC_SELF))
		{
			if (xp->flags & INC_EXISTS)
			{
				if (!(flags & SEARCH_INCLUDE) || !(flags & SEARCH_NEXT) && mp != xp && (mp = xp) && !ppmultiple(xp, INC_TEST))
					return(0);
			}
			else if (*fp->name == '/') break;
			else continue;
		}
		message((-3, "search: file=%s path=%s", fp->name, pp.path));
#if ARCHIVE
if (pp.test & 0x0040) error(1, "SEARCH#%d dir=%s%s%s%s%s file=%s%s path=%s index=%d", __LINE__, dp->name, (dp->type & TYPE_ARCHIVE) ? " ARCHIVE" : "",  (dp->type & TYPE_BUFFER) ? " BUFFER" : "", (dp->type & TYPE_CHECKPOINT) ? " CHECKPOINT" : "", (dp->type & TYPE_DIRECTORY) ? " DIRECTORY" : "", fp->name, (fp->flags & INC_MEMBER(index)) ? " MEMBER" : "", pp.path, index);
		if ((fp->flags & INC_MEMBER(index)) && ((struct ppmember*)fp->bound[index])->archive == dp)
		{
			fd = 0;
			pp.member = (struct ppmember*)fp->bound[index];
if (pp.test & 0x0010) error(1, "SEARCH#%d file=%s path=%s index=%d data=<%lu,%lu>", __LINE__, fp->name, pp.path, index, pp.member->offset, pp.member->size);
		}
		else if (!(dp->type & TYPE_DIRECTORY))
			continue;
		else
#endif
		{
			pp.member = 0;
			fd = (flags & SEARCH_INCLUDE) ? open(pp.path, O_RDONLY) : access(pp.path, R_OK);
		}
		if (fd >= 0)
		{
			if ((pp.option & (PLUSPLUS|NOPROTO)) == PLUSPLUS && !(pp.test & TEST_noproto))
			{
				if (dp->c) pp.mode |= MARKC;
				else pp.mode &= ~MARKC;
			}
			if (dp->hosted) pp.mode |= MARKHOSTED;
			else pp.mode &= ~MARKHOSTED;
			message((-2, "search: %s -> %s%s%s", fp->name, pp.path, (pp.mode & MARKC) ? " [C]" : "", (pp.mode & MARKHOSTED) ? " [hosted]" : ""));
			xp = ppsetfile(pp.path);
#if ARCHIVE
			if (!pp.member)
			{
#endif
				fp->flags |= INC_BOUND(index);
				fp->bound[index] = xp;
				if (index == INC_STANDARD && type != T_HEADER && !(fp->flags & INC_BOUND(INC_LOCAL)))
				{
					fp->flags |= INC_BOUND(INC_LOCAL);
					fp->bound[INC_LOCAL] = xp;
				}
#if ARCHIVE
			}
#endif
			xp->flags |= INC_SELF|INC_EXISTS;
			if (flags & SEARCH_INCLUDE)
			{
				if ((pp.prefix = prefix) || (pp.prefix = pp.in->prefix))
					message((-2, "search: %s: next prefix = %s", xp->name, pp.prefix));
				if (!(pp.mode & ALLMULTIPLE))
				{
					if (xp->guard == INC_CLEAR || xp == mp)
						xp->guard = INC_TEST;
					else
					{
						if (pp.state & WARN) error(1, "%s: ignored -- already included", xp->name);
						else message((-3, "%s: ignored -- already included", xp->name));
						xp->guard = fp->guard = INC_IGNORE;
#if ARCHIVE
						if (!pp.member)
#endif
						close(fd);
						return(0);
					}
				}
				pp.include = xp->name;
				if ((pp.mode & (FILEDEPS|INIT)) == FILEDEPS && ((pp.mode & HEADERDEPS) || !(pp.mode & MARKHOSTED)))
				{
					if ((pp.column + strlen(xp->name)) >= COLUMN_MAX)
					{
						sfprintf(pp.filedeps, " \\\n");
						pp.column = COLUMN_TAB;
						index = '\t';
					}
					else index = ' ';
					pp.column += sfprintf(pp.filedeps, "%c%s", index, xp->name);
				}
			}
			return(fd);
		}
		if (xp) xp->flags |= INC_SELF;
		if (errno == EMFILE)
			error(3, "%s: too many open files", fp->name);
		else if (errno != ENOENT && errno != ENOTDIR)
			error(ERROR_SYSTEM|1, "%s: cannot open file for reading", pp.path);
		if (*fp->name == '/')
			break;
	}
	strcpy(pp.path, fp->name);
	message((-2, "search: %s%s not found", (flags & SEARCH_NEXT) ? "next " : "", fp->name));
	return(-1);
}

/*
 * search for an include file
 * if (flags&SEARCH_INCLUDE) then
 *	if file found then open read file descriptor returned
 *		with pp.path set to the full path and
 *		pp.prefix set to the directory prefix
 *	otherwise 0 returned if file found but ignored
 *	otherwise -1 returned
 * otherwise
 *	if file found then 0 returned
 *	otherwise -1 returned
 */

int
ppsearch(char* file, int type, int flags)
{
	register struct ppfile*	fp;
	register char*		s;
	register struct ppdirs*	dp;
	struct oplist*		cp;
	int			dospath;
	int			fd;
	int			index;
	char			name[MAXTOKEN + 1];

	pp.include = 0;
	fd = -1;
	dospath = 0;
 again:
	pathcanon(file, 0);
	for (cp = pp.chop; cp; cp = cp->next)
		if (strneq(file, cp->value, cp->op))
		{
			if (cp->value[cp->op + 1])
			{
				sfsprintf(name, sizeof(name) - 1, "%s%s", cp->value + cp->op + 1, file + cp->op);
				message((-3, "chop: %s -> %s", file, name));
				file = name;
			}
			else if (strchr(file + cp->op, '/'))
			{
				message((-3, "chop: %s -> %s", file, file + cp->op));
				file += cp->op;
			}
			break;
		}
	fp = ppsetfile(file);
	while (fp->flags & INC_MAPPED)
		fp = fp->bound[INC_MAP];
	pp.original = fp;
	if (type == T_HEADER && strneq(fp->name, "...", 3) && (!fp->name[3] || fp->name[3] == '/'))
	{
		if (fp->name[3] == '/')
		{
			struct ppfile*	np;

			if ((fd = ppsearch(fp->name + 4, type, flags)) < 0 || !(np = ppgetfile(pp.path)))
				return(-1);
			s = error_info.file;
			error_info.file = np->name;
			fd = ppsearch(fp->name + 4, type, flags|SEARCH_NEXT);
			error_info.file = s;
			return(fd);
		}
		if (file = strrchr(error_info.file, '/')) file++;
		else file = error_info.file;
		if (pp.in->prefix)
		{
			sfsprintf(name, sizeof(name) - 1, "%s/%s", pp.in->prefix, file);
			file = name;
		}
		fp = ppsetfile(file);
		flags |= SEARCH_NEXT;
		message((-2, "search: nxt=%s cur=%s", fp->name, error_info.file));
	}
	else if ((flags & SEARCH_INCLUDE) && fp->guard == INC_IGNORE)
	{
		strcpy(pp.path, fp->name);
		message((-2, "%s: ignored", fp->name));
		return(0);
	}
	else if (!(flags & SEARCH_NEXT)) flags |= SEARCH_SKIP;
	pp.prefix = 0;
	if (type == T_HEADER) dp = pp.stddirs->next;
	else
	{
		dp = pp.lcldirs;
		if (dp == pp.firstdir)
		{
			/*
			 * look in directory of including file first
			 */

			if (error_info.file && (s = strrchr(error_info.file, '/')))
			{
				*s = 0;
				dp->name = ppsetfile(error_info.file)->name;
				*s = '/';
			}
			else dp->name = "";
		}
		else if (pp.in->prefix && pp.lcldirs != pp.firstdir)
		{
			/*
			 * look in prefix directory of including file first
			 */

			if (*fp->name != '/')
			{
				if ((s = strchr(fp->name, '/')) && (fp->name[0]
!= '.' || fp->name[1] != '.' || fp->name[2] != '/'))
				{
					*s = 0;
					if (!streq(fp->name, pp.in->prefix)) fd = 0;
					*s = '/';
				}
				else fd = 0;
			}
			if (fd >= 0)
			{
				struct ppfile*	xp;

				sfsprintf(name, sizeof(name) - 1, "%s/%s", pp.in->prefix, fp->name);
				pathcanon(name, 0);
				xp = ppsetfile(name);
				if ((fd = search(xp, dp, type, flags)) >= 0)
					return(fd);
			}
		}
	}
	if ((fd = search(fp, dp, type, flags)) < 0)
	{
		/*
		 * hackery for msdos files viewed through unix
		 */

		switch (dospath)
		{
		case 0:
			if (s = strchr(file, '\\'))
			{
				do *s++ = '/'; while (s = strchr(s, '\\'));
				pathcanon(file, 0);
				dospath = 1;
				goto again;
			}
			/*FALLTHROUGH*/
		case 1:
			if (ppisid(file[0]) && file[1] == ':' && file[2] == '/')
			{
				file[1] = file[0];
				file[0] = '/';
				pathcanon(file, 0);
				dospath = 2;
				goto again;
			}
			break;
		case 2:
			file += 2;
			goto again;
		}
		if ((flags & (SEARCH_INCLUDE|SEARCH_NEXT)) == SEARCH_INCLUDE)
		{
			if (!(pp.mode & GENDEPS))
			{
				if (!(pp.option & ALLPOSSIBLE) || pp.in->prev->prev)
					error(2, "%s: cannot find include file", file);
			}
			else if (!(pp.mode & INIT))
			{
				if ((pp.column + strlen(file)) >= COLUMN_MAX)
				{
					sfprintf(pp.filedeps, " \\\n");
					pp.column = COLUMN_TAB;
					index = '\t';
				}
				else index = ' ';
				pp.column += sfprintf(pp.filedeps, "%c%s", index, file);
			}
		}
	}
	return(fd);
}
