/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1987-2003 AT&T Corp.                *
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
 * AT&T Bell Laboratories
 *
 * pax file support
 */

#include "pax.h"

/*
 * "nocomment" is a hardwired "nocom"
 * should be an sfio discipline
 */

#include "nocomment.c"

#if __STDC__
#define chmod(a,b)	(error(-1,"%s#%d: chmod(%s,%05o)",__FILE__,__LINE__,a,b),chmod(a,b))
#endif

/*
 * return read file descriptor for filtered current input file
 */

int
filter(register Archive_t* ap, register File_t* f)
{
	register int	n;
	char*		arg;
	int		rfd;
	int		wfd;

	if (state.filter.line <= 0)
		arg = f->path;
	else if (!*(arg = state.filter.command))
	{
		if ((rfd = open(f->st->st_size ? f->path : "/dev/null", O_RDONLY|O_BINARY)) < 0)
			error(ERROR_SYSTEM|2, "%s: cannot read", f->path);
		return rfd;
	}
	if ((wfd = open(state.tmp.file, O_CREAT|O_TRUNC|O_WRONLY|O_BINARY, S_IRUSR)) < 0)
	{
		error(2, "%s: cannot create filter temporary %s", f->path, state.tmp.file);
		return -1;
	}
	if ((rfd = open(state.tmp.file, O_RDONLY|O_BINARY)) < 0)
	{
		error(2, "%s: cannot open filter temporary %s", f->path, state.tmp.file);
		close(wfd);
		if (remove(state.tmp.file))
			error(1, "%s: cannot remove filter temporary %s", f->path, state.tmp.file);
		return -1;
	}
	if (remove(state.tmp.file))
		error(1, "%s: cannot remove filter temporary %s", f->path, state.tmp.file);
	if (ap->format == ASCHK)
		f->checksum = 0;
	f->st->st_size = 0;
	if (streq(*state.filter.argv, "nocomment"))
	{
		int	errors = error_info.errors;
		off_t	count;
		Sfio_t*	ip;
		Sfio_t*	op;

		if ((ip = sfopen(NiL, f->path, "r")) && (op = sfnew(NiL, NiL, SF_UNBOUND, wfd, SF_WRITE)) && (count = nocomment(ip, op)) < 0)
			error(2, "%s: %s: filter error", f->path, *state.filter.argv);
		if (ip)
		{
			sfclose(ip);
			if (op) sfclose(op);
			else error(2, "%s: cannot redirect filter", f->path);
		}
		else error(2, "%s: cannot read", f->path);
		if (errors != error_info.errors)
		{
			close(rfd);
			close(wfd);
			return -1;
		}
		f->st->st_size = count;
	}
	else
	{
		Proc_t*		proc;

		*state.filter.patharg = arg;
		if (!(proc = procopen(*state.filter.argv, state.filter.argv, NiL, NiL, PROC_READ)))
		{
			error(2, "%s: cannot execute filter %s", f->path, *state.filter.argv);
			close(rfd);
			close(wfd);
			return -1;
		}
		holeinit(wfd);
		while ((n = read(proc->rfd, state.tmp.buffer, state.buffersize)) > 0)
		{
			if (holewrite(wfd, state.tmp.buffer, n) != n)
			{
				error(2, "%s: filter write error", f->path);
				break;
			}
			if (ap->format == ASCHK)
				f->checksum = asc_checksum(state.tmp.buffer, n, f->checksum);
			f->st->st_size += n;
		}
		holedone(wfd);
		if (n < 0)
			error(ERROR_SYSTEM|2, "%s: %s filter read error", f->path, *state.filter.argv);
		if (n = procclose(proc))
			error(2, "%s: %s filter exit code %d", f->path, *state.filter.argv, n);
	}
	close(wfd);
	message((-1, "%s: filter file size = %ld", f->path, f->st->st_size));
	return rfd;
}

/*
 * return read file descriptor for current input file
 */

int
openin(register Archive_t* ap, register File_t* f)
{
	register int	n;
	int		rfd;

	if (f->type != X_IFREG)
		rfd = -1;
	else if (state.filter.argv && f->st->st_size)
		rfd = filter(ap, f);
	else if ((rfd = open(f->st->st_size ? f->path : "/dev/null", O_RDONLY|O_BINARY)) < 0)
		error(ERROR_SYSTEM|2, "%s: cannot read", f->path);
	else if (ap->format == ASCHK)
	{
		f->checksum = 0;
		if (lseek(rfd, (off_t)0, SEEK_SET) != 0)
			error(ERROR_SYSTEM|1, "%s: %s checksum seek error", f->path, format[ap->format].name);
		else
		{
			while ((n = read(rfd, state.tmp.buffer, state.buffersize)) > 0)
				f->checksum = asc_checksum(state.tmp.buffer, n, f->checksum);
			if (n < 0)
				error(ERROR_SYSTEM|2, "%s: %s checksum read error", f->path, format[ap->format].name);
			if (lseek(rfd, (off_t)0, SEEK_SET) != 0)
				error(ERROR_SYSTEM|1, "%s: %s checksum seek error", f->path, format[ap->format].name);
		}
	}
	if (rfd < 0)
		f->st->st_size = 0;
	return rfd;
}

/*
 * create directory and all path name components leading to directory
 */

static int
missdir(register Archive_t* ap, register File_t* f)
{
	register char*	s;
	register char*	t;
	long		pp;
	struct stat*	st;
	struct stat*	sp;
	struct stat	st0;
	struct stat	st1;

	s = f->name;
	pathcanon(s, 0);
	if (t = strchr(*s == '/' ? s + 1 : s, '/'))
	{
		if (!state.mkdir)
		{
			if (!state.warnmkdir)
			{
				state.warnmkdir = 1;
				error(1, "omit the --nomkdir option to create intermediate directories");
			}
			return -1;
		}
		st = 0;
		sp = &st0;
		do
		{
			*t = 0;
			if (stat(s, sp))
			{
				*t = '/';
				break;
			}
			*t = '/';
			st = sp;
			sp = (sp == &st0) ? &st1 : &st0;
		} while (t = strchr(t + 1, '/'));
		if (t)
		{
			if (!st && stat(".", st = &st0))
				error(ERROR_SYSTEM|3, "%s: cannot stat .", s);
			pp = f->perm;
			f->perm = st->st_mode & state.modemask;
			sp = f->st;
			f->st = st;
			do
			{
				*t = 0;
				if (mkdir(s, f->perm))
				{
					error(ERROR_SYSTEM|2, "%s: cannot create directory", s);
					*t = '/';
					f->perm = pp;
					f->st = sp;
					return -1;
				}
				setfile(ap, f);
				*t = '/';
			} while (t = strchr(t + 1, '/'));
			f->perm = pp;
			f->st = sp;
		}
	}
	return 0;
}

/*
 * open file for writing, set all necessary info
 */

int
openout(register Archive_t* ap, register File_t* f)
{
	register int	fd;
	int		exists;
	int		perm;
	struct stat	st;

	pathcanon(f->name, 0);

	/*
	 * if not found and state.update then check down the view
	 *
	 * NOTE: VPATH in app code is ugly but the benefits of the
	 *	 combination with state.update win over beauty
	 */

	if (f->ro)
	{
		f->name = "PAX-INTERNAL-ERROR";
		f->skip = 1;
		exists = 0;
	}
	else if (exists = !(*state.statf)(f->name, &st))
	{
		if (!state.clobber && !S_ISDIR(st.st_mode))
		{
			error(1, "%s: already exists -- not overwritten", f->name);
			return -1;
		}
		f->chmod = f->perm != (st.st_mode & (S_IRWXU|S_IRWXG|S_IRWXO)) &&
			(state.chmod || state.update || S_ISDIR(st.st_mode));
		st.st_mode = modex(st.st_mode);
	}
	else
	{
		typedef struct View
		{
			struct View*	next;
			char*		root;
			dev_t		dev;
			ino_t		ino;
		} View_t;

		View_t*			vp;
		View_t*			tp;
		char*			s;
		char*			e;

		static View_t*		view;
		static char*		offset;

		if (state.update && !offset)
		{
			if (s = getenv("VPATH"))
			{
				if (!(s = strdup(s)))
					nospace();
				do
				{
					if (e = strchr(s, ':')) *e++ = 0;
					if (!(vp = newof(0, View_t, 1, 0)))
						nospace();
					vp->root = s;
					if (stat(s, &st))
					{
						vp->dev = 0;
						vp->ino = 0;
					}
					else
					{
						vp->dev = st.st_dev;
						vp->ino = st.st_ino;
					}
					if (view) tp = tp->next = vp;
					else view = tp = vp;
				} while (s = e);
				s = state.pwd;
				e = 0;
				for (;;)
				{
					if (stat(s, &st))
						error(ERROR_SYSTEM|3, "%s: cannot stat pwd component", s);
					for (vp = view; vp; vp = vp->next)
						if (vp->ino == st.st_ino && vp->dev == st.st_dev)
						{
							offset = e ? e + 1 : ".";
							tp = view;
							view = vp->next;
							while (tp && tp != view)
							{
								vp = tp;
								tp = tp->next;
								free(vp);
							}
							goto found;
						}
					if (e) *e = '/';
					else e = s + strlen(s);
					while (e > s && *--e != '/');
					if (e <= s) break;
					*e = 0;
				}
			}
		found:
			if (!offset) offset = ".";
		}
		st.st_mode = 0;
		st.st_mtime = 0;
		if (*f->name != '/')
			for (vp = view; vp; vp = vp->next)
			{
				sfsprintf(state.tmp.buffer, state.tmp.buffersize - 1, "%s/%s/%s", vp->root, offset, f->name);
				if (!stat(state.tmp.buffer, &st))
					break;
			}
		f->chmod = state.chmod || state.update;
	}
	if (f->delta.op == DELTA_delete)
	{
		if (exists) switch (X_ITYPE(st.st_mode))
		{
		case X_IFDIR:
			if (!f->ro)
			{
				if (rmdir(f->name)) error(ERROR_SYSTEM|2, "%s: cannot remove directory", f->name);
				else listentry(f);
			}
			break;
		default:
			if (remove(f->name)) error(ERROR_SYSTEM|2, "%s: cannot remove file", f->name);
			else listentry(f);
			break;
		}
		return -1;
	}
	if (state.operation == (IN|OUT))
	{
		if (exists && f->st->st_ino == st.st_ino && f->st->st_dev == st.st_dev)
		{
			error(2, "attempt to pass %s to self", f->name);
			return -1;
		}
		if (state.linkf && f->type != X_IFDIR && (state.linkf == pathsetlink || f->st->st_dev == state.dev))
		{
			if (exists)
				remove(f->name);
			if ((*state.linkf)(f->path, f->name))
			{
				if (!exists && missdir(ap, f))
				{
					error(ERROR_SYSTEM|2, "%s: cannot create intermediate directories", f->name);
					return -1;
				}
				if (exists || (*state.linkf)(f->path, f->name))
				{
					error(ERROR_SYSTEM|2, "%s: cannot link to %s", f->path, f->name);
					return -1;
				}
			}
			setfile(ap, f);
			return -2;
		}
	}
	if (prune(ap, f, &st))
		return -1;
	switch (f->type)
	{
	case X_IFDIR:
		if (ap->format != ZIP)
			f->st->st_size = 0;
		if (f->ro)
			return -1;
		if (exists && X_ITYPE(st.st_mode) != X_IFDIR)
		{
			if (remove(f->name))
			{
				error(ERROR_SYSTEM|2, "cannot remove current %s", f->name);
				return -1;
			}
			exists = 0;
		}
		if (!exists && mkdir(f->name, f->perm) && (missdir(ap, f) || mkdir(f->name, f->perm)))
		{
			error(ERROR_SYSTEM|2, "%s: cannot create directory", f->name);
			return -1;
		}
		setfile(ap, f);
		if (!exists || f->chmod)
		{
			listentry(f);
			fd = -1;
		}
		else if (state.update && exists)
			fd = -1;
		else
			fd = -2;
		return fd;
	case X_IFLNK:
		if (!*f->linkpath)
			return -2;
		if (streq(f->name, f->linkpath))
		{
			error(1, "%s: symbolic link loops to self", f->name);
			return -1;
		}
		if (exists && remove(f->name))
		{
			error(ERROR_SYSTEM|2, "cannot remove current %s", f->name);
			return -1;
		}
		if (pathsetlink(f->linkpath, f->name))
		{
			if (!exists && missdir(ap, f))
			{
				error(ERROR_SYSTEM|2, "%s: cannot create intermediate directories", f->name);
				return -1;
			}
			if (exists || pathsetlink(f->linkpath, f->name))
			{
				error(ERROR_SYSTEM|2, "%s: cannot symlink to %s", f->name, f->linkpath);
				return -1;
			}
		}
		setfile(ap, f);
		listentry(f);
		return -1;
	}
	if (!addlink(ap, f))
		return -1;
	switch (f->type)
	{
	case X_IFIFO:
	case X_IFSOCK:
		IDEVICE(f->st, 0);
		/*FALLTHROUGH*/
	case X_IFBLK:
	case X_IFCHR:
		if (exists && remove(f->name))
		{
			error(ERROR_SYSTEM|2, "cannot remove current %s", f->name);
			return -1;
		}
		if (ap->format != ZIP)
			f->st->st_size = 0;
		if (mknod(f->name, f->st->st_mode, idevice(f->st)))
		{
			if (errno == EPERM)
			{
				error(ERROR_SYSTEM|2, "%s: cannot create %s special file", f->name, (f->type == X_IFBLK) ? "block" : "character");
				return -1;
			}
			if (!exists && missdir(ap, f))
			{
				error(ERROR_SYSTEM|2, "%s: cannot create intermediate directories", f->name);
				return -1;
			}
			if (exists || mknod(f->name, f->st->st_mode, idevice(f->st)))
			{
				error(ERROR_SYSTEM|2, "%s: cannot mknod", f->name);
				return -1;
			}
		}
		setfile(ap, f);
		return -2;
	default:
		error(1, "%s: unknown file type 0%03o -- creating regular file", f->name, f->type >> 12);
		/*FALLTHROUGH*/
	case X_IFREG:
		if (f->ro)
			return dup(1);
		if (state.intermediate)
		{
			char*	d;
			char*	e;
			int	n;
			int	ifd;

			/*
			 * copy to intermediate output file and rename
			 * to real file only on success - a handy
			 * backup option
			 *
			 * thanks to the amazing dr. ek
			 */

			if (missdir(ap, f))
			{
				error(ERROR_SYSTEM|2, "%s: cannot create intermediate directories", f->name);
				return -1;
			}
			d = (e = strrchr(f->name, '/')) ? f->name : ".";
			for (n = 0;; n++)
			{
				if (e)
					*e = 0;
				f->intermediate = pathtemp(ap->path.temp, sizeof(ap->path.temp), d, error_info.id, &ifd);
				if (e)
					*e = '/';
				message((-4, "%s: intermediate %s", f->name, f->intermediate));
				if (f->intermediate)
				{
					ap->errors = error_info.errors;
					return ifd;
				}
				if (n)
				{
					error(ERROR_SYSTEM|2, "%s: cannot create intermediate name", f->name);
					return -1;
				}
			}
		}

		/*
		 * ok, the exists bits are only used right here
		 * you do the defines if its that important
		 *
		 * <chmod u+w><remove><missdir>
		 *	4	don't attempt
		 *	2	attempted and succeeded
		 *	1	attempted and failed
		 */

		if (!exists)
			exists |= 0440;
		else if (!state.linkf)
			exists |= remove(f->name) ? 0010 : 0420;
		else if (st.st_mode & S_IWUSR)
			exists |= 0400;
		if ((perm = f->perm) & (S_ISUID|S_ISGID|S_ISVTX))
			perm &= ~(S_ISUID|S_ISGID|S_ISVTX);
		while ((fd = open(f->name, O_CREAT|O_TRUNC|O_WRONLY|O_BINARY, perm)) < 0)
		{
		again:
			if (!(exists & 0007))
			{
				if (missdir(ap, f))
				{
					error(ERROR_SYSTEM|2, "%s: cannot create intermediate directories", f->name);
					return -1;
				}
				exists |= 0002;
			}
			else if (!(exists & 0700))
			{
				if (chmod(f->name, perm | S_IWUSR))
				{
					exists |= 0100;
					goto again;
				}
				exists |= 0200;
			}
			else if (!(exists & 0070))
			{
				if (remove(f->name))
				{
					exists |= 0010;
					goto again;
				}
				exists ^= 0620;
			}
			else
			{
				error(ERROR_SYSTEM|2, "%s: cannot create%s%s", f->name, (exists & 0100) ? ERROR_translate(0, 0, 0, ", cannot enable user write") : "", (exists & 0010) ? ERROR_translate(0, 0, 0, ", cannot remove") : "");
				return -1;
			}
		}
		if (perm != f->perm)
			f->chmod = 1;
		else if ((exists & 0200) && chmod(f->name, f->perm))
			error(ERROR_SYSTEM|1, "%s: cannot restore original mode %s", f->name, fmtperm(st.st_mode & S_IPERM));
		return fd;
	}
}

/*
 * close an openout() fd, doing the intermediate rename if needed
 */

int
closeout(register Archive_t* ap, register File_t* f, int fd)
{
	register char*	s;
	int		r;

	r = close(fd);
	if (s = f->intermediate)
	{
		f->intermediate = 0;
		if (ap->errors != error_info.errors)
		{
			if (remove(s))
				error(ERROR_SYSTEM|2, "%s: cannot remove intermediate file %s", f->name, s);
			return -1;
		}
		if (rename(s, f->name) && (remove(f->name) || rename(s, f->name)))
		{
			error(ERROR_SYSTEM|2, "%s: cannot rename from intermediate file %s", f->name, s);
			return -1;
		}
		if (chmod(f->name, f->perm))
		{
			error(ERROR_SYSTEM|1, "%s: cannot change mode to %s", f->name, fmtperm(f->perm));
			return -1;
		}
	}
	return r;
}

/*
 * get file info for output
 */

int
getfile(register Archive_t* ap, register File_t* f, register Ftw_t* ftw)
{
	register char*		name;
	register int		n;

	name = ftw->path;
	message((-4, "getfile(%s)", name));
	switch (ftw->info)
	{
	case FTW_NS:
		error(2, "%s: not found", name);
		return 0;
	case FTW_DNR:
		if (state.files)
			error(2, "%s: cannot read directory", name);
		break;
	case FTW_D:
	case FTW_DNX:
	case FTW_DP:
		if (!state.descend)
			ftw->status = FTW_SKIP;
		else if (ftw->info == FTW_DNX)
		{
			error(2, "%s: cannot search directory", name);
			ftw->status = FTW_SKIP;
		}
		else if (!state.files)
		{
			/*
			 * stdin files most likely come from tw/find with
			 * directory descendents already included; in posix
			 * omitting -d would result in duplicate output copies
			 * so we avoid the problem by peeking ahead and
			 * pruning all paths with this dir prefix
			 */

			n = ftw->pathlen;
			name = stash(&ap->path.peek, name, n);
			name[n] = '/';
			if (!state.peekfile || !strncmp(state.peekfile, name, n))
				while ((state.peekfile = sfgetr(sfstdin, '\n', 1)) && !strncmp(state.peekfile, name, n));
			name[n] = 0;
		}
		break;
	}
	if (ap->delta)
		ap->delta->hdr = ap->delta->hdrbuf;
	name = stash(&ap->path.name, name, ftw->pathlen);
	pathcanon(name, 0);
	f->path = stash(&ap->path.path, name, ftw->pathlen);
	f->name = map(name);
	if (state.files && state.operation == (IN|OUT) && dirprefix(state.destination, name))
		return 0;
	f->namesize = strlen(f->name) + 1;
	ap->st = ftw->statb;
	f->st = &ap->st;
	f->perm = f->st->st_mode & S_IPERM;
	f->st->st_mode = modex(f->st->st_mode);
	f->uidname = 0;
	f->gidname = 0;
	f->link = 0;
	if ((f->type = X_ITYPE(f->st->st_mode)) == X_IFLNK)
	{
		f->linkpathsize = f->st->st_size + 1;
		f->linkpath = stash(&ap->path.link, NiL, f->linkpathsize);
		if (pathgetlink(f->path, f->linkpath, f->linkpathsize) != f->st->st_size)
		{
			error(2, "%s: cannot read symbolic link", f->path);
			ftw->status = FTW_SKIP;
			return 0;
		}
		f->linktype = SOFTLINK;
		pathcanon(f->linkpath, 0);
		if (!(state.ftwflags & FTW_PHYSICAL))
			f->linkpath = map(f->linkpath);
		if (streq(f->path, f->linkpath))
		{
			error(2, "%s: symbolic link loops to self", f->path);
			ftw->status = FTW_SKIP;
			return 0;
		}
	}
	else
	{
		f->linktype = NOLINK;
		f->linkpath = 0;
		f->linkpathsize = 0;
	}
	f->ro = ropath(f->name);
	if (!validout(ap, f))
		return 0;
	if (!(state.operation & IN) && f->type != X_IFDIR)
	{
		if (!addlink(ap, f) && !state.header.linkdata)
			f->st->st_size = 0;
		message((-3, "getfile(%s): dev'=%d ino'=%d", f->name, f->st->st_dev, f->st->st_ino));
	}
	ap->entries++;
	f->delta.op = 0;
	f->longname = 0;
	f->longlink = 0;
	f->skip = 0;
	message((-2, "getfile(): path=%s name=%s mode=%s size=%I*d", name, f->name, fmtmode(f->st->st_mode, 1), sizeof(f->st->st_size), f->st->st_size));
	return 1;
}

/*
 * check that f is valid for archive output
 */

int
validout(register Archive_t* ap, register File_t* f)
{
	register char*	s;

	if (f->ro)
		return 0;
	switch (f->type)
	{
	case X_IFCHR:
	case X_IFBLK:
		f->st->st_size = 0;
		break;
	case X_IFREG:
		IDEVICE(f->st, 0);
		break;
	case X_IFDIR:
	case X_IFLNK:
		f->st->st_size = 0;
		IDEVICE(f->st, 0);
		break;
	}
	switch (ap->format)
	{
	case ALAR:
	case IBMAR:
	case SAVESET:
		if (f->type != X_IFREG)
		{
			error(2, "%s: only regular files copied in %s format", f->path, format[ap->format].name);
			return 0;
		}
		if (s = strrchr(f->name, '/'))
		{
			s++;
			error(1, "%s: file name stripped to %s", f->name, s);
		}
		else s = f->name;
		if (strlen(s) > sizeof(ap->id.id) - 1)
		{
			error(2, "%s: file name too long", f->name);
			return 0;
		}
		f->id = strupper(strcpy(ap->id.id, s));
		break;
	case BINARY:
		if (f->namesize > (BINARY_NAMESIZE + 1))
		{
			error(2, "%s: file name too long", f->name);
			return 0;
		}
		break;
	}
	return 1;
}

/*
 * add file which may be a link
 * 0 returned if <dev,ino> already added
 */

int
addlink(register Archive_t* ap, register File_t* f)
{
	register Link_t*	p;
	register char*		s;
	int			n;
	Fileid_t		id;
	unsigned short		us;

	id.dev = f->st->st_dev;
	id.ino = f->st->st_ino;
	if (!ap->delta)
		switch (state.operation)
		{
		case IN:
			us = id.dev;
			if (us > state.devcnt)
			{
				state.devcnt = us;
				state.inocnt = id.ino;
			}
			else if (us == state.devcnt)
			{
				us = id.ino;
				if (us > state.inocnt)
					state.inocnt = us;
			}
			break;
		case IN|OUT:
			if (!state.pass)
				break;
			/*FALLTHROUGH*/
		case OUT:
			if (!++state.inocnt)
			{
				if (!++state.devcnt)
					goto toomany;
				state.inocnt = 1;
			}
			f->st->st_dev = state.devcnt;
			f->st->st_ino = state.inocnt;
			break;
		}
	if (f->type == X_IFDIR)
		return 0;
	switch (ap->format)
	{
	case ALAR:
	case IBMAR:
	case SAVESET:
		if (state.operation == IN || f->st->st_nlink <= 1)
			return 1;
		break;
	case PAX:
	case TAR:
	case USTAR:
		if (state.operation == IN)
		{
			if (f->linktype == NOLINK)
				return 1;
			f->linkpath = map(f->linkpath);
			goto linked;
		}
		/*FALLTHROUGH*/
	default:
		if (f->st->st_nlink <= 1)
			return 1;
		break;
	}
	if (p = (Link_t*)hashget(state.linktab, (char*)&id))
	{
		switch (ap->format)
		{
		case ALAR:
		case IBMAR:
		case SAVESET:
			error(1, "%s: hard link information lost in %s format", f->name, format[ap->format].name);
			return 1;
		}
		f->st->st_dev = p->id.dev;
		f->st->st_ino = p->id.ino;
		f->link = p;
		f->linktype = HARDLINK;
		f->linkpath = p->name;
		if (state.pass && (state.operation & OUT) || !state.pass && state.operation == OUT)
			return 0;
	linked:
		message((-1, "addlink(%s,%s)", f->name, f->linkpath));

		/*
		 * compensate for a pre 951031 pax bug
		 * that added linknamesize to st_size
		 */

		if (ap->format == CPIO && f->st->st_size == f->linkpathsize && bread(ap, state.tmp.buffer, (off_t)0, n = f->st->st_size + 6, 0) > 0)
		{
			bunread(ap, state.tmp.buffer, n);
			state.tmp.buffer[6] = 0;
			state.tmp.buffer[n] = 0;
			if (strtol(state.tmp.buffer, NiL, 8) == CPIO_MAGIC && strtol(state.tmp.buffer + f->st->st_size, NiL, 8) != CPIO_MAGIC)
			{
				f->st->st_size = 0;
				if (!ap->warnlinkhead)
				{
					ap->warnlinkhead = 1;
					error(1, "%s: compensating for invalid %s header hard link sizes", ap->name, format[ap->format].name);
				}
			}
		}
		if (streq(f->name, f->linkpath))
		{
			error(2, "%s: hard link loops to self", f->name);
			return 0;
		}
		if (!state.list)
		{
			s = f->linkpath;
			if (access(s, 0))
			{
				f->skip = 1;
				error(2, "%s must exist for hard link %s", s, f->name);
				return 0;
			}
			remove(f->name);
			if (state.operation == IN && *s != '/')
			{
				strcpy(state.pwd + state.pwdlen, s);
				s = state.pwd;
			}
			if (link(s, f->name))
			{
				if (missdir(ap, f))
				{
					error(ERROR_SYSTEM|2, "%s: cannot create intermediate directories", f->name);
					return 0;
				}
				if (link(s, f->name))
				{
					error(ERROR_SYSTEM|2, "%s: cannot link to %s", f->linkpath, f->name);
					return -1;
				}
			}
			listentry(f);
		}
		return 0;
	}
	n = strlen(f->name) + 1;
	if (!(p = newof(0, Link_t, 1, n)))
		goto toomany;
	f->link = p;
	strcpy(p->name = (char*)p + sizeof(*p), f->name);
	p->namesize = n;
	p->id.dev = f->st->st_dev;
	p->id.ino = f->st->st_ino;
	hashput(state.linktab, NiL, p);
	return -1;
 toomany:
	if (!state.warnlinknum)
	{
		state.warnlinknum = 1;
		error(1, "too many hard links -- some links may become copies");
	}
	return -1;
}

/*
 * get file uid and gid names given numbers
 */

void
getidnames(register File_t* f)
{
	if (!f->uidname) f->uidname = fmtuid(f->st->st_uid);
	if (!f->gidname) f->gidname = fmtgid(f->st->st_gid);
}

/*
 * set file uid and gid numbers given names
 */

void
setidnames(register File_t* f)
{
	register int	id;

	if (f->uidname)
	{
		if ((id = struid(f->uidname)) < 0)
		{
			if (id == -1 && state.owner)
				error(1, "%s: invalid user name", f->uidname);
			f->uidname = 0;
			id = state.uid;
		}
		f->st->st_uid = id;
	}
	if (f->gidname)
	{
		if ((id = strgid(f->gidname)) < 0)
		{
			if (id == -1 && state.owner)
				error(1, "%s: invalid group name", f->gidname);
			f->gidname = 0;
			id = state.gid;
		}
		f->st->st_gid = id;
	}
}

/*
 * allocate and initialize new archive pointer
 */

Archive_t*
initarchive(const char* name, int mode)
{
	Archive_t*	ap;

	if (!(ap = newof(0, Archive_t, 1, 0)))
		nospace();
	initfile(ap, &ap->file, &ap->st, NiL, 0);
	ap->name = (char*)name;
	ap->expected = ap->format = -1;
	ap->section = 0;
	ap->sum = -1;
	ap->mio.mode = ap->tio.mode = mode;
	ap->io = &ap->mio;
	return ap;
}

/*
 * return pointer to archive for op
 */

Archive_t*
getarchive(int op)
{
	Archive_t**	app;

	app = (op & OUT) ? &state.out : &state.in;
	if (!*app) *app = initarchive(NiL, (op & OUT) ? (O_CREAT|O_TRUNC|O_WRONLY) : O_RDONLY);
	return *app;
}

/*
 * initialize file info with name and mode
 */

void
initfile(register Archive_t* ap, register File_t* f, struct stat* st, register char* name, int mode)
{
	memzero(f, sizeof(*f));
	f->st = st;
	memzero(f->st, sizeof(*f->st));
	if (name)
	{
		f->id = f->name = f->path = name;
		f->namesize = strlen(name) + 1;
	}
	f->st->st_mode = modex(mode);
	f->st->st_nlink = 1;		/* system V needs this!!! */
}

/*
 * set copied file info
 */

void
setfile(register Archive_t* ap, register File_t* f)
{
	register Post_t*	p;
	Post_t			post;

	if (f->skip || f->extended)
		return;
	switch (f->type)
	{
	case X_IFLNK:
#if _lib_lchown
		if (state.owner)
		{
			if (state.flags & SETIDS)
			{
				post.uid = state.setuid;
				post.gid = state.setgid;
			}
			else
			{
				post.uid = f->st->st_uid;
				post.gid = f->st->st_gid;
			}
			if (lchown(f->name, post.uid, post.gid) < 0)
				error(1, "%s: cannot chown to (%d,%d)", f->name, post.uid, post.gid);
		}
#endif
#if _lib_lchmod
		if (f->chmod)
		{
			int		m;
			struct stat	st;

			if (lchmod(f->name, f->perm & state.modemask))
				error(1, "%s: cannot chmod to %s", f->name, fmtmode(f->perm & state.modemask, 0) + 1);
			else if (m = f->perm & (S_ISUID|S_ISGID|S_ISVTX))
			{
				if (lstat(f->name, &st))
					error(1, "%s: not found", f->name);
				else if (m ^= (st.st_mode & (S_ISUID|S_ISGID|S_ISVTX)))
					error(1, "%s: mode %s not set", f->name, fmtmode(m, 0) + 1);
			}
		}
#endif
		return;
	case X_IFDIR:
		if (f->chmod || state.acctime || state.modtime || state.owner || (f->perm & S_IRWXU) != S_IRWXU)
		{
			if (!(p = newof(0, Post_t, 1, 0)))
				error(3, "not enough space for file restoration info");
			tvgetstat(f->st, &p->atime, &p->mtime, NiL);
			p->uid = f->st->st_uid;
			p->gid = f->st->st_gid;
			p->mode = f->perm;
			if ((f->perm & S_IRWXU) != S_IRWXU)
			{
				p->chmod = 1;
				if (chmod(f->name, f->perm|S_IRWXU))
					error(1, "%s: cannot chmod to %s", f->name, fmtmode(f->st->st_mode|X_IRWXU, 1) + 1);
			}
			else
				p->chmod = f->chmod;
			hashput(state.restore, f->name, p);
			return;
		}
		break;
	}
	p = &post;
	tvgetstat(f->st, &p->atime, &p->mtime, NiL);
	p->uid = f->st->st_uid;
	p->gid = f->st->st_gid;
	p->mode = f->perm;
	p->chmod = f->chmod;
	restore(f->name, (char*)p, NiL);
}

/*
 * set access and modification times of file
 */

void
settime(const char* name, Tv_t* ap, Tv_t* mp, Tv_t* cp)
{
	if (*name && tvtouch(name, ap, mp, cp, 1))
		error(1, "%s: cannot set times", name);
}

/*
 * restore file status after processing
 */

int
restore(register const char* name, char* ap, void* handle)
{
	register Post_t*	p = (Post_t*)ap;
	int			m;
	struct stat		st;

	NoP(handle);
	if (!*name)
		return 0;
	if (state.owner)
	{
		if (state.flags & SETIDS)
		{
			p->uid = state.setuid;
			p->gid = state.setgid;
		}
		if (chown(name, p->uid, p->gid) < 0)
			error(1, "%s: cannot chown to (%d,%d)", name, p->uid, p->gid);
	}
	if (p->chmod)
	{
		if (chmod(name, p->mode & state.modemask))
			error(1, "%s: cannot chmod to %s", name, fmtmode(p->mode & state.modemask, 0) + 1);
		else if (m = p->mode & (S_ISUID|S_ISGID|S_ISVTX))
		{
			if (stat(name, &st))
				error(1, "%s: not found", name);
			else if (m ^= (st.st_mode & (S_ISUID|S_ISGID|S_ISVTX)))
				error(1, "%s: mode %s not set", name, fmtmode(m, 0) + 1);
		}
	}
	if (state.modtime)
		settime(name, &p->atime, &p->mtime, NiL);
	return 0;
}

/*
 * return 1 if f output can be pruned
 */

int
prune(register Archive_t* ap, register File_t* f, register struct stat* st)
{
	if (st->st_mode == f->st->st_mode && (ap->delta && f->st->st_mtime == st->st_mtime || state.update && (unsigned long)f->st->st_mtime <= (unsigned long)st->st_mtime))
		return 1;
	return 0;
}

/*
 * write siz bytes of buf to fd checking for HOLE_MIN hole chunks
 * we assume siz is rounded nicely until the end
 */

ssize_t
holewrite(int fd, void* buf, size_t siz)
{
	register char*	t = (char*)buf;
	register char*	e = t + siz;
	register char*	b = 0;
	register char*	s;
	ssize_t		i;
	ssize_t		n = 0;

	static char	hole[HOLE_MIN];

#if DEBUG
	if (state.test & 0100)
		b = t;
	else
#endif
	while (t < e)
	{
		s = t;
		if ((t += HOLE_MIN) > e)
			t = e;
		if (!*s && !*(t - 1) && !memcmp(s, hole, t - s))
		{
			if (b)
			{
				if (state.hole)
				{
					if (lseek(fd, state.hole, SEEK_CUR) < state.hole)
						return -1;
					state.hole = 0;
				}
				if ((i = write(fd, b, s - b)) != (s - b))
					return i;
				n += i;
				b = 0;
			}
			state.hole += t - s;
			n += t - s;
		}
		else if (!b)
			b = s;
	}
	if (b)
	{
		if (state.hole)
		{
			if (lseek(fd, state.hole, SEEK_CUR) < state.hole)
				return -1;
			state.hole = 0;
		}
		if ((i = write(fd, b, e - b)) != (e - b))
			return i;
		n += i;
	}
	return n;
}
