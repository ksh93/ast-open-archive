/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1984-2001 AT&T Corp.                *
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
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * make rule binding routines
 */

#include "make.h"

#include <glob.h>

/*
 * embedded spaces in file name wreak havoc
 * we wreak hack to get through
 */

#define HACKSPACE(f,s)	for (s = f; s = strchr(s, ' '); *s++ = (state.test & 4) ? '?' : FILE_SPACE)
#define FIGNORE(s)	((s)[0]=='.'&&((s)[1]==0||(s)[1]=='.'&&(s)[2]==0))

#if _WIN32

/*
 * we have a system in which some directories preserve
 * mixed case entries but ignore case on name lookup
 * maybe they can get a patent on that
 * hey, maybe we can get a patent on this
 */

static int
file_compare(register const char* s, register const char* t)
{
	return ((struct file*)hashgetbucket(s)->value)->dir->ignorecase ? strcasecmp(s, t) : strcmp(s, t);
}

static unsigned int
file_hash(const char* s)
{
	register const unsigned char*	p = (const unsigned char*)s;
	register unsigned int		h = 0;
	register unsigned int		c;

	while (c = *p++)
	{
		if (isupper(c))
			c = tolower(c);
		HASHPART(h, c);
	}
	return h;
}

static int
rule_compare(register const char* s, register const char* t)
{
	register int	c;
	register int	d;
	register int	x;

	x = (*s == '.') ? 0 : -1;
	while (c = *s++)
	{
		if (!x)
		{
			if (c == '%')
				x = (*(s - 2) == '.') ? 1 : -1;
			else if (c != '.' && !isupper(c))
				x = -1;
		}
		if ((d = *t++) != c)
		{
			if (x > 0)
			{
				if (isupper(c))
					c = tolower(c);
				else if (isupper(d))
					d = tolower(d);
				if (c == d)
					continue;
			}
			return c - d;
		}
	}
	return c - *t;
}

static unsigned int
rule_hash(const char* s)
{
	register const unsigned char*	p = (const unsigned char*)s;
	register unsigned int		h = 0;
	register unsigned int		c;
	register int			x;

	x = (*s == '.') ? 0 : -1;
	while (c = *p++)
	{
		if (!x)
		{
			if (c == '%')
				x = (*(p - 2) == '.') ? 1 : -1;
			else if (c != '.' && !isupper(c))
				x = -1;
		}
		if (x > 0 && isupper(c))
			c = tolower(c);
		HASHPART(h, c);
	}
	return h;
}

#endif

/*
 * initialize the hash tables
 */

void
inithash(void)
{
	table.ar	= hashalloc(NiL, HASH_name, "archives", 0);
	table.bound	= hashalloc(table.ar, HASH_name, "bound-directories", 0);
#if _WIN32
	table.file	= hashalloc(NiL, HASH_set, HASH_ALLOCATE, HASH_compare, file_compare, HASH_hash, file_hash, HASH_name, "files", 0);
	table.oldvalue	= hashalloc(NiL, HASH_name, "old-values", 0);
	table.rule	= hashalloc(NiL, HASH_compare, rule_compare, HASH_hash, rule_hash, HASH_name, "atoms", 0);
#else
	table.file	= hashalloc(table.ar, HASH_set, HASH_ALLOCATE, HASH_name, "files", 0);
	table.oldvalue	= hashalloc(table.file, HASH_name, "old-values", 0);
	table.rule	= hashalloc(table.oldvalue, HASH_name, "atoms", 0);
#endif
	table.var	= hashalloc(table.oldvalue, HASH_name, "variables", 0);
	table.dir	= hashalloc(NiL, HASH_set, HASH_ALLOCATE, HASH_namesize, sizeof(struct fileid), HASH_name, "directories", 0);
	optinit();
}

/*
 * determine if a directory (archive) has already been scanned
 */

struct dir*
unique(register struct rule* r)
{
	register struct dir*	d;
	struct rule*		x;
	struct fileid		id;
	struct stat		st;

	if (rstat(r->name, &st, 0))
	{
		r->time = 0;
		return 0;
	}
	id.dev = st.st_dev;
	id.ino = st.st_ino;
	if ((d = getdir(&id)) && state.alias && (!state.mam.statix || S_ISDIR(st.st_mode)))
	{
		/*
		 * internal.unbind causes directory rescan
		 */

		if (r->name == d->name || (r->dynamic & D_alias) && makerule(r->name)->name == d->name || (x = makerule(d->name)) == r)
			return d;

		/*
		 * the physical directory alias can go either way
		 * but we bias the choice towards shorter pathnames
		 */

		if (!x->uname && strlen(r->name) < strlen(x->name))
		{
			struct rule*	t;

			t = r;
			r = x;
			x = t;
			x->time = d->time;
		}
		message((-2, "%s %s is also specified as %s", (r->property & P_archive) ? "archive" : "directory", unbound(r), x->name));
		r->dynamic |= D_alias;
		merge(r, x, MERGE_ALL|MERGE_BOUND);
		r->view = x->view;
		r->time = x->time;
		r->uname = r->name;
		d->name = r->name = x->name;
		return 0;
	}
	d = newof(0, struct dir, 1, 0);
	d->name = r->name;
	d->time = r->time = st.st_mtime;
	d->directory = S_ISDIR(st.st_mode) != 0;
	putdir(0, d);
#if _WIN32
	d->ignorecase = d->directory;
#endif
	return d;
}

/*
 * add a directory (archive) entry to the file hash
 */

struct file*
addfile(struct dir* d, char* name, unsigned long date)
{
	register struct file*	f;
	register struct file*	n;
	register char*		s;

	HACKSPACE(name, s);

	/*
	 * this test avoids duplicate entries for systems that
	 * support viewed or covered directories in the pathname
	 * system calls
	 *
	 * we assume that the cover directories are read in order
	 * from top to bottom
	 *
	 * the entries for a directory and its covered directories
	 * all have the same file.dir value
	 */

	if ((n = getfile(name)) && n->dir == d)
	{
		if (d->archive)
		{
			if (n->time < date)
				n->time = date;
#if DEBUG
			message((-12, "%s: %s %s [duplicate member]", d->name, name, strtime(n->time)));
#endif
		}
		return n;
	}
#if DEBUG
	message((-12, "%s: %s %s%s", d->name, name, strtime(date), d->ignorecase ? " [ignorecase]" : null));
#endif
	f = newof(0, struct file, 1, 0);
	f->next = n;
	f->dir = d;
	f->time = date;
	putfile(0, f);
#if _WIN32
	if (!d->archive && (s = strchr(name, '.')))
	{
		s++;
		if (streq(s, "exe") || streq(s, "EXE"))
		{
			*--s = 0;
			addfile(d, name, date);
			*s = '.';
		}
	}
#endif
	return f;
}

/*
 * add new file r to the directory hash at dir
 */

void
newfile(register struct rule* r, char* dir, unsigned long date)
{
	register char*		s;
	register char*		t;
	char*			nam;
	struct rule*		z;
	struct dir*		d;
	Hash_position_t*	pos;
	Sfio_t*			tmp;

	tmp = sfstropen();
	sfputc(tmp, '.');
	sfputc(tmp, '/');
	edit(tmp, r->name, dir ? dir : KEEP, KEEP, KEEP);
	s = (nam = sfstruse(tmp)) + 1;
	do
	{
		*s = 0;
		if ((z = getrule(nam)) && (z->dynamic & D_entries))
		{
			if (t = strchr(s + 1, '/')) *t = 0;

			/*
			 * sequential scan is OK since this is uncommon
			 */

			if (pos = hashscan(table.dir, 0))
			{
				while (hashnext(pos))
				{
					d = (struct dir*)pos->bucket->value;
					if (d->name == z->name)
					{
						addfile(d, s + 1, date);
						break;
					}
				}
				hashdone(pos);
			}
			if (t) *t = '/';
		}
		*s++ = '/';
	} while (s = strchr(s, '/'));
	sfstrclose(tmp);
}

/*
 * scan directory r and record all its entries
 */

void
dirscan(struct rule* r)
{
	register DIR*		dirp;
	register struct dir*	d;
	register struct dirent*	entry;
	char*			s;
	int			n;
	struct stat		st;

	if (r->dynamic & D_scanned) return;
	if ((n = strlen(r->name)) > 1) s = canon(r->name) - 1;
	else s = r->name;
	if (s > r->name && *s == '/') *s-- = 0;
	if ((s - r->name + 1) != n) r->name = putrule(r->name, r);
	r->dynamic |= D_scanned;
	r->dynamic &= ~D_entries;
	if (!(r->property & P_state))
	{
		if (d = unique(r))
		{
			s = r->name;
			if (d->directory)
			{
				if (dirp = opendir(s))
				{
#if DEBUG
					message((-5, "scan directory %s", s));
#endif
					while (entry = readdir(dirp))
						if (!FIGNORE(entry->d_name))
							addfile(d, entry->d_name, NOTIME);
					r->dynamic |= D_entries;
					if (!(r->dynamic & D_bound) && !stat(s, &st))
					{
						r->dynamic |= D_bound;
						r->time = st.st_mtime;
						if (!r->view && ((state.questionable & 0x00000800) || !(r->property & P_target)) && *s == '/' && (strncmp(s, internal.pwd, internal.pwdlen) || *(s + internal.pwdlen) != '/'))
							r->dynamic |= D_global;
					}
				}
				if (dirp)
				{
					closedir(dirp);
					return;
				}
			}
#if DEBUG
			message((-5, "dirscan(%s) failed", s));
#endif
		}
		else if (r->time) r->dynamic |= D_entries;
	}
}

/*
 * scan archive r and record all its entries
 */

static void
arscan(struct rule* r)
{
	int		arfd;
	struct dir*	d;

	if (r->dynamic & D_scanned) return;
	r->dynamic |= D_scanned;
	if (r->property & P_state) r->dynamic &= ~D_entries;
	else if (!(d = unique(r))) r->dynamic |= D_entries;
	else if (r->scan >= SCAN_USER)
	{
#if DEBUG
		message((-5, "scan aggregate %s", r->name));
#endif
		d->archive = 1;
		state.archive = d;
		scan(r, NiL);
		state.archive = 0;
		r->dynamic |= D_entries;
	}
	else if ((arfd = openar(r->name, "br")) >= 0)
	{
#if DEBUG
		message((-5, "scan archive %s", r->name));
#endif
		d->archive = 1;
		if (walkar(d, arfd, r->name)) r->dynamic |= D_entries;
		else r->dynamic &= ~D_entries;
		closear(arfd);
	}
#if DEBUG
	else message((-5, "arscan(%s) failed", r->name));
#endif
}

struct globstate
{
	char*		name;
	DIR*		dirp;
	int		view;
	int		root;
	Hash_table_t*	overlay;
};

/*
 * glob() diropen for 2d views
 */

static void*
glob_diropen(glob_t* gp, const char* path)
{
	struct globstate*	gs = (struct globstate*)gp->gl_handle;
	const char*		dir;
	register int		i;
	register int		n;

	if (!(gs->overlay = hashalloc(NiL, HASH_set, HASH_ALLOCATE, 0)))
		return 0;
	gs->view = 0;
	gs->root = 0;
	dir = path;
	if (*path == '/')
		for (i = 0; i <= state.maxview; i++)
		{
			if (!strncmp(path, state.view[i].root, n = state.view[i].rootlen) && (!*(path + n) || *(path + n) == '/'))
			{
				if (!*(path += n + 1))
					path = internal.dot->name;
				gs->view = i;
				gs->root = 1;
				break;
			}
		}
	gs->name = makerule((char*)path)->name;
	if (gs->dirp = opendir(dir))
		return (void*)gs;
	if (*path != '/')
		while (gs->view++ < state.maxview)
		{
			if (gs->root)
				sfprintf(internal.nam, "%s/%s", state.view[gs->view].root, gs->name);
			else
			{
				if (*state.view[gs->view].path != '/')
					sfprintf(internal.nam, "%s/", internal.pwd);
				sfprintf(internal.nam, "%s/%s", state.view[gs->view].path, gs->name);
			}
			if (gs->dirp = opendir(sfstruse(internal.nam)))
				return (void*)gs;
		}
	hashfree(gs->overlay);
	return 0;
}

/*
 * glob() dirnext for 2d views
 */

static char*
glob_dirnext(glob_t* gp, void* handle)
{
	struct globstate*	gs = (struct globstate*)handle;
	struct dirent*		dp;
	char*			s;

	for (;;)
	{
		if (dp = readdir(gs->dirp))
		{
			if (FIGNORE(dp->d_name))
				continue;
			HACKSPACE(dp->d_name, s);
			if (hashget(gs->overlay, dp->d_name))
				continue;
			hashput(gs->overlay, 0, (char*)gs);
			return dp->d_name;
		}
		closedir(gs->dirp);
		gs->dirp = 0;
		if (*gs->name == '/')
			return 0;
		do
		{
			if (gs->view++ >= state.maxview)
				return 0;
			if (gs->root)
				sfprintf(internal.nam, "%s/%s", state.view[gs->view].root, gs->name);
			else
			{
				if (*state.view[gs->view].path != '/')
					sfprintf(internal.nam, "%s/", internal.pwd);
				sfprintf(internal.nam, "%s/%s", state.view[gs->view].path, gs->name);
			}
		} while (!(gs->dirp = opendir(sfstruse(internal.nam))));
	}
}

/*
 * glob() dirclose for 2d views
 */

static void
glob_dirclose(glob_t* gp, void* handle)
{
	struct globstate*	gs = (struct globstate*)handle;

	if (gs->dirp)
		closedir(gs->dirp);
	if (gs->overlay)
		hashfree(gs->overlay);
}

/*
 * glob() type for 2d views
 */

static int
glob_type(glob_t* gp, const char* path)
{
	register int		i;
	register int		n;
	int			root;
	struct stat		st;

	i = 0;
	if (stat(path, &st))
	{
		root = 0;
		if (*path == '/')
			for (i = 0; i <= state.maxview; i++)
			{
				if (!strncmp(path, state.view[i].root, n = state.view[i].rootlen) && (!*(path + n) || *(path + n) == '/'))
				{
					if (!*(path += n + 1))
						path = internal.dot->name;
					root = 1;
					break;
				}
			}
		for (i = 0; i <= state.maxview; i++)
		{
			if (root)
				sfprintf(internal.nam, "%s/%s", state.view[i].root, path);
			else
			{
				if (*state.view[i].path != '/')
					sfprintf(internal.nam, "%s/", internal.pwd);
				sfprintf(internal.nam, "%s/%s", state.view[i].path, path);
			}
			if (!stat(sfstruse(internal.nam), &st))
				break;
		}
	}
	if (i > state.maxview)
		i = 0;
	else if (S_ISDIR(st.st_mode))
		i = GLOB_DIR;
	else if (!S_ISREG(st.st_mode))
		i = GLOB_DEV;
	else if (st.st_mode & (S_IXUSR|S_IXGRP|S_IXOTH))
		i = GLOB_EXE;
	else
		i = GLOB_REG;
	return i;
}

/*
 * return a vector of the top view of files in all views matching pattern s
 * the vector is placed on the current stack
 */

char**
globv(char* s)
{
	register char**		q;
	register char**		p;
	register char**		x;
	int			i;
	glob_t			gl;
	struct globstate	gs;

	static char*		nope[1];

	memset(&gl, 0, sizeof(gl));
	gl.gl_intr = &state.caught;
	if (state.maxview && !state.fsview)
	{
		gl.gl_handle = (void*)&gs;
		gl.gl_diropen = glob_diropen;
		gl.gl_dirnext = glob_dirnext;
		gl.gl_dirclose = glob_dirclose;
		gl.gl_type = glob_type;
	}
	if (i = glob(s, GLOB_AUGMENTED|GLOB_DISC|GLOB_NOCHECK|GLOB_STACK, 0, &gl))
	{
		if (!trap())
			error(2, "glob() internal error %d", i);
		return nope;
	}
	if (state.maxview && !state.fsview)
	{
		for (i = 0, p = 0, x = q = gl.gl_pathv; *q; q++)
			if (!p || !streq(*q, *p))
			{
				*x++ = *q;
				p = q;
			}
		*x = 0;
	}
	return gl.gl_pathv;
}

/*
 * enter r as an alias for x
 * path is the canonical path name for x
 * d is the bind directory for path
 * a pointer to r merged with x is returned
 */

static struct rule*
bindalias(register struct rule* r, register struct rule* x, char* path, struct rule* d)
{
	char*		s;
	int		i;
	int		n;
	int		na = 0;
	struct rule*	z;
	struct rule*	a[3];

	if (x->dynamic & D_alias)
	{
		a[na++] = x;
		x = makerule(x->name);
		if (x == r || (x->dynamic & D_alias))
			return r;
	}
	message((-2, "%s is also specified as %s", unbound(r), unbound(x)));
#if DEBUG
	if (state.test & 0x00000040) error(2, "bindalias: path=%s r=%s%s%s x=%s%s%s", path, r->name, r->uname ? "==" : null, r->uname ? r->uname : null, x->name, x->uname ? "==" : null, x->uname ? x->uname : null);
#endif
	r->dynamic |= (D_alias|D_bound);
	merge(r, x, MERGE_ALL|MERGE_BOUND);
	if (!(state.questionable & 0x00001000))
		x->attribute |= r->attribute;
	if (x->uname && !streq(x->name, path) && !streq(x->uname, path))
	{
		putrule(x->name, 0);
		z = makerule(path);
		if (z->dynamic & D_alias)
		{
			a[na++] = z;
			z = makerule(z->name);
#if DEBUG
			if (z->dynamic & D_alias)
				error(PANIC, "multiple alias from %s to %s", z->name, x->name);
#endif
		}
		if (z != x && z != r)
		{
#if DEBUG
			if (state.test & 0x00000040) error(2, "           z=%s%s%s", z->name, z->uname ? "==" : null, z->uname ? z->uname : null);
#endif
			x->dynamic |= (D_alias|D_bound);
			merge(x, z, MERGE_ALL|MERGE_BOUND);
			a[na++] = x;
			x = z;
		}
	}
	if (x->dynamic & D_bound)
	{
		r->time = x->time;
		r->view = x->view;
	}
	else
	{
		x->dynamic |= D_bound;
		x->dynamic &= ~D_member;
		x->time = r->time;
		if (!(x->dynamic & D_source))
			x->view = r->view;
	}
	s = r->uname = r->name;
	r->name = x->name;
	if (d)
	{
		if (state.fsview && strchr(s, '/') && strchr(r->name, '/') && !streq(s, r->name))
		{
			message((-5, "%s and %s are bound in %s", s, r->name, d->name));
			putbound(s, d->name);
			putbound(r->name, d->name);
		}
		else if (s[0] == '.' && s[1] == '.' && s[2] == '/' && ((n = strlen(r->name)) < (i = strlen(s)) || r->name[n - i - 1] != '/' || !streq(s, r->name + n - i)))
			putbound(s, d->name);
	}
	if (!(state.questionable & 0x00002000))
	{
		if ((s = getbound(x->name)) || x->uname && (s = getbound(x->uname)))
			putbound(r->name, s);
		else if ((s = getbound(r->name)) || r->uname && (s = getbound(r->uname)))
			putbound(x->name, s);
		for (i = 0; i < na; i++)
		{
			if (s) putbound(a[i]->name, s);
			x->attribute |= a[i]->attribute;
		}
		for (i = 0; i < na; i++)
			a[i]->attribute |= x->attribute;
		r->attribute |= x->attribute;
	}
	return x;
}

/*
 * return local view rule for r if defined
 * force forces the rule to be allocated
 * 0 always returned if !state && == r or if not in local view
 */

static struct rule*
localrule(register struct rule* r, int force)
{
	register char*		s;
	register struct rule*	x;
	char*			p;
	char*			v;
	Sfio_t*			tmp;

	if (r->property & P_state) return force ? 0 : r;
	if (r->dynamic & D_alias) r = makerule(r->name);
	if (!r->view) return 0;
	if (!strncmp(r->name, state.view[r->view].path, state.view[r->view].pathlen))
	{
		s = r->name + state.view[r->view].pathlen;
		switch (*s++)
		{
		case 0:
			return internal.dot;
		case '/':
			if (!(x = getrule(s)) && force) x = makerule(s);
			if (x && (x->dynamic & D_alias)) x = makerule(x->name);
			if (x && !x->view && (x != r || force))
			{
				merge(r, x, MERGE_ATTR|MERGE_FORCE);
				x->uname = r->uname;
				x->time = r->time;
				x->status = r->status;
				return x;
			}
			return 0;
		}
	}
	p = 0;
	s = r->name;
	v = state.view[r->view].path;
	while (*s && *s == *v++)
		if (*s++ == '/')
			p = s;
	if (p)
	{
		s = internal.pwd;
		v--;
		while (s = strchr(s, '/'))
			if (!strcmp(++s, v))
			{
				tmp = sfstropen();
				*--s = 0;
				sfprintf(tmp, "%s/%s", internal.pwd, p);
				v = sfstruse(tmp);
				*s = '/';
				if (!(x = getrule(v)) && force) x = makerule(v);
				if (x && (x->dynamic & D_alias)) x = makerule(x->name);
				sfstrclose(tmp);
				if (x && !x->view && (force || x != r))
				{
					merge(r, x, MERGE_ATTR);
					if (!x->uname) x->uname = r->uname;
					if (!x->time) x->time = r->time;
					return x;
				}
				return 0;
			}
	}
	return 0;
}

/*
 * bind a rule to a file
 */

struct rule*
bindfile(register struct rule* r, char* name, int flags)
{
	register struct rule*	d;
	register struct file*	f;
	register char*		s;
	struct list*		p;
	struct file*		files;
	struct file*		ofiles;
	char*			dir;
	char*			base;
	char*			b;
	int			found;
	int			i;
	int			n;
	int			c;
	int			ndirs;
	int			allocated = 0;
	int			aliased = 0;
	unsigned int		view;
	unsigned long		tm;
	struct stat		st;
	struct rule*		a;
	struct rule*		od;
	struct rule*		x;
	struct rule*		z;
	struct list*		dirs[5];
	struct list		dot;
	Sfio_t*			buf;
	Sfio_t*			tmp;

	static struct dir	tmp_dir;
	static struct file	tmp_file = { 0, &tmp_dir, NOTIME };

	if (r || (r = getrule(name)))
	{
		if ((r->property & P_state) || (r->property & P_virtual) && !(flags & BIND_FORCE)) return 0;
		if (r->dynamic & D_alias) r = makerule(r->name);
		if (r->dynamic & D_bound) return r;
		if (!name) name = r->name;
	}
	buf = sfstropen();
	tmp = sfstropen();
	for (;;)
	{
		found = 0;
		view = 0;
		od = 0;

		/*
		 * at this point name!=r->name is possible
		 */

#if _WIN32
		if (*name == '/' || isalpha(*name) && *(name + 1) == ':' && (*(name + 2) == '/' || *(name + 2) == '\\'))
#else
		if (*name == '/')
#endif
		{
			sfputr(buf, name, 0);
			s = sfstrset(buf, 0);
			canon(s);
			if (!rstat(s, &st, 0))
			{
				tm = st.st_mtime;
				f = 0;
				found = 1;
			}
			else if (state.maxview && !state.fsview)
				for (i = 0; i <= state.maxview; i++)
				{
					if (!strncmp(s, state.view[i].root, n = state.view[i].rootlen) && (!*(s + n) || *(s + n) == '/'))
					{
						if (!*(s += n + 1))
							s = internal.dot->name;
						for (i = 0; i <= state.maxview; i++)
						{
							sfprintf(internal.nam, "%s/%s", state.view[i].root, s);
							if (!rstat(b = sfstruse(internal.nam), &st, 0))
							{
								sfputr(buf, b, 0);
								tm = st.st_mtime;
								f = 0;
								found = 1;
#if 0 /* not sure about this */
								view = i;
#endif
								break;
							}
						}
						break;
					}
				}
			break;
		}

		/*
		 * dir contains the directory path name component (usually 0)
		 * base contains the base path name component
		 */

		edit(tmp, name, KEEP, DELETE, DELETE);
		dir = sfstruse(tmp);
		if (*dir) base = name + strlen(dir) + 1;
		else
		{
			dir = 0;
			base = name;
		}

		/*
		 * gather the directories to search in reverse order
		 */

#if DEBUG
		message((-11, "bindfile(%s): dir=%s base=%s", name, dir ? dir : internal.dot->name, base));
#endif
		ndirs = 0;
		a = 0;
		if (!(flags & BIND_DOT))
		{
			if (!r) a = associate(internal.source_p, NiL, name, NiL);
			else if (name == r->name) a = associate(internal.source_p, r, NiL, NiL);
			if (!a || !(a->property & P_force))
			{
				x = internal.source;
				if (!(x->dynamic & D_cached)) x = source(x);
				dirs[ndirs++] = x->prereqs;
				if ((flags & BIND_MAKEFILE) && (z = catrule(internal.source->name, external.source, NiL, 0)))
				{
					if (!(z->dynamic & D_cached)) z = source(z);
					dirs[ndirs++] = z->prereqs;
				}
				edit(buf, name, DELETE, internal.source->name, KEEP);
				if ((z = getrule(sfstruse(buf))) && z != x && z != internal.source_p)
				{
					if (!(z->dynamic & D_cached)) z = source(z);
					dirs[ndirs++] = z->prereqs;
				}
			}
			if (a)
			{
#if DEBUG
				message((-5, "%s directory order from %s", (a->property & P_force) ? "override" : "insert", a->name));
#endif
				if (!(a->dynamic & D_cached)) a = source(a);
				dirs[ndirs++] = a->prereqs;
			}
		}
		dot.rule = internal.dot;
		dot.next = 0;
		dirs[ndirs++] = &dot;

		/*
		 * the nested loops preserve the directory search order
		 *
		 * .			current directory
		 * <source>.<pattern>	alternate directories (may pre-empt)
		 * <source>.x		suffix association directories
		 * <source>		default source directories
		 */

		files = getfile(base);
		ofiles = 0;

		/*
		 * first check if r is an archive member
		 */

		if (r && r->active && (r->active->parent->target->property & P_archive) && !(r->dynamic & D_membertoo))
		{
			struct dir*	ar;

			i = 0;
			for (f = files; f; f = f->next)
				if (r->active->parent->target->name == f->dir->name && f->dir->archive)
				{
					i = 1;
					if (r->dynamic & D_member)
						error(1, "%s is duplicated within %s", r->name, r->active->parent->target->name);
					if (f->time >= r->time)
					{
						r->dynamic |= D_member;
						r->time = f->time;
					}
				}
			if (!i && (ar = getar(r->active->parent->target->name)) && (i = ar->truncate) && strlen(base) > i)
			{
				c = base[i];
				base[i] = 0;
				for (f = getfile(base); f; f = f->next)
				{
					if (r->active->parent->target->name == f->dir->name && f->dir->archive)
					{
						if (r->dynamic & D_member)
							error(1, "%s is duplicated within %s", r->name, r->active->parent->target->name);
						if (f->time >= r->time)
						{
							r->dynamic |= D_member;
							r->time = f->time;
						}
					}
				}
				base[i] = c;
			}
		}

		/*
		 * now check the directories
		 */

		view = state.maxview;
		for (i = ndirs; i-- > 0;)
		{
			for (p = dirs[i]; p; p = p->next)
			{
				d = p->rule;
				if (!(d->mark & M_directory))
				{
					/*UNDENT*/
	d->mark |= M_directory;
	c = (s = dir) && (s[0] != '.' || s[1] != '.' || s[2] != '/' && s[2] != 0);
	if (c && (d->property & P_terminal))
	{
		if (state.test & 0x00000008)
			error(2, "prune: %s + %s TERM", d->name, s);
		continue;
	}
	if (!(d->dynamic & D_scanned))
	{
		dirscan(d);
		files = getfile(base);
	}
	od = d;
	if (s)
	{
		if (c)
		{
			if (s = strchr(s, '/')) *s = 0;
			z = catrule(d->name, "/", dir, -1);
			if (z->dynamic & (D_entries|D_scanned))
			{
				if (s) *s = '/';
				if (!(z->dynamic & D_entries)) continue;
			}
			else
			{
				for (f = getfile(dir), b = d->name; f; f = f->next)
					if (f->dir->name == b) break;
				if (!f)
				{
					z->dynamic |= D_scanned;
					z->dynamic &= ~D_entries;
					if (s) *s = '/';
					if (state.test & 0x00000008) error(2, "prune: %s + %s HASH", d->name, dir);
					continue;
				}
				if (s)
				{
					*s = '/';
					z->dynamic |= D_entries;
				}
			}
			if (s) d = catrule(d->name, "/", dir, -1);
			else d = z;
		}
		else if (!(state.questionable & 0x00000080)) d = catrule(d->name, "/", dir, -1);
		else for (;;)
		{
			if (*s++ != '.' || *s++ != '.' || *s++ != '/')
			{
				d = *(s - 1) ? catrule(d->name, "/", dir, -1) : makerule(dir);
				break;
			}
		}
		if (!(d->dynamic & D_scanned))
		{
			d->view = od->view;
			dirscan(d);
			files = getfile(base);
		}
	}
	if (!(d->dynamic & D_entries))
		continue;
	else if (!files && name[0] == '.' && (name[1] == 0 || name[1] == '/' || name[1] == '.' && (name[2] == 0 || name[2] == '/' || state.fsview && name[2] == '.' && (name[3] == 0 || name[3] == '/'))))
	{
		tmp_dir.name = d->name;
		ofiles = files;
		files = &tmp_file;
	}
#if DEBUG
	message((-11, "bindfile(%s): dir=%s", name, d->name));
#endif
	if (d->view <= view)
	{
		for (f = files; f; f = f->next)
		{
			s = f->dir->name;
			if (s == d->name && !f->dir->archive)
			{
				if (s == internal.dot->name) sfputr(buf, base, -1);
				else sfprintf(buf, "%s/%s", s, base);
				tm = f->time;
				s = sfstruse(buf);
				canon(s);
				st.st_mode = 0;
				if (tm == NOTIME)
				{
					x = 0;
					if (state.believe && d->view >= (state.believe - 1))
					{
						if (!r) r = makerule(name);
						r->view = d->view;
						r->mark |= M_bind;
						x = staterule(RULE, r, NiL, 0);
						r->mark &= ~M_bind;
					}
					if (x)
					{
						view = d->view;
						tm = x->time;
						message((-3, "%s: believe time [%s] from view %d", r->name, strtime(tm), view));
					}
					else if (rstat(s, &st, 0)) tm = 0;
					else
					{
						tm = st.st_mtime;
						view = state.fsview ? iview(&st) : d->view;
					}
				}
				else view = d->view;
				if (tm)
				{
					if (!(flags & BIND_DOT) || !view)
					{
						if (view && state.fsview && state.expandview)
							mount(s, s, FS3D_GET|FS3D_VIEW|FS3D_SIZE(sfstrsize(buf)), NiL);
						found = 1;
						goto clear;
					}
					if (d->view > view) break;
				}
				else if (errno == ENODEV) view = d->view;
			}
		}
	}
	if (ofiles)
	{
		files = ofiles;
		ofiles = 0;
	}
					/*INDENT*/
				}
			}
		}
	clear:

		/*
		 * clear the visit marks
		 */

		for (i = ndirs; i-- > 0;)
			for (p = dirs[i]; p; p = p->next)
				p->rule->mark &= ~M_directory;
		if (!found && r && name == r->name && (a = associate(internal.bind_p, r, NiL, NiL)) && (b = call(a, name)) && (s = getarg(&b, NiL)))
		{
			char*	t;

			/*
			 * [+|-] value [time]
			 *
			 *	-	name = name, time = time(value)
			 *	+	name = value, time = OLDTIME
			 *		name = value, time = time(value)
			 */

			if (streq(s, "-") || streq(s, "+"))
			{
				n = *s;
				s = getarg(&b, NiL);
			}
			else n = 0;
			if (s && !streq(s, name))
			{
				t = getarg(&b, NiL);
				a = getrule(s);
				if (n == '+')
				{
					st.st_mode = 0;
					st.st_mtime = OLDTIME;
					sfputr(buf, s, 0);
				}
				else
				{
					if (t) st.st_mode = 0;
					else if (rstat(s, &st, 0))
					{
						if (!a || !(a->dynamic & D_bound) && !(a->property & P_target))
						{
							if (allocated) free(name);
							else allocated = 1;
							name = strdup(s);
							continue;
						}
						if (a->dynamic & D_bound)
						{
							st.st_mode = !(a->dynamic & D_regular);
							st.st_mtime = a->time;
						}
						else
						{
							st.st_mode = 0;
							st.st_mtime = 0;
						}
					}
					if (n == '-')
					{
						if (*s) putbound(name, makerule(s)->name);
						sfputr(buf, name, 0);
					}
					else
					{
						aliased = 1;
						sfputr(buf, s, 0);
						s = sfstrset(buf, 0);
						canon(s);
						if (state.fsview && state.expandview && st.st_mtime && iview(&st))
							mount(s, s, FS3D_GET|FS3D_VIEW|FS3D_SIZE(sfstrsize(buf)), NiL);
					}
				}
				tm = t ? strtol(t, NiL, 0) : st.st_mtime;
				view = a ? a->view : state.maxview + 1;
				od = 0;
				found = 1;
			}
		}
		break;
	}
	if (!found && state.targetcontext && r && name != r->name && (x = getrule(base)) && (x->dynamic & D_context))
	{
		for (i = ndirs; i-- > 0;)
		{
			for (p = dirs[i]; p; p = p->next)
			{
				d = p->rule;
				if (!(d->mark & M_directory))
				{
					d->mark |= M_directory;
					if (d->name != internal.dot->name)
					{
						sfprintf(buf, "%s/%s", d->name, base);
						s = sfstruse(buf);
						canon(s);
						if ((x = getrule(s)) && (x->property & P_target))
						{
							found = 1;
							st.st_mode = 0;
							view = 0;
							tm = (x->time || (x = staterule(RULE, x, NiL, 0)) && x->time) ? x->time : CURTIME;
							goto context;
						}
					}
				}
			}
		}
	context:
		for (i = ndirs; i-- > 0;)
			for (p = dirs[i]; p; p = p->next)
				p->rule->mark &= ~M_directory;
	}
	if (found)
	{
		/*
		 * the file exists with:
		 *
		 *	buf	canonical file path name
		 *	od	original directory pointer
		 *	st	file stat() info
		 *	tm	file time
		 *	view	view index of dir containing file
		 */

		if (view > state.maxview) view = 0;
		b = sfstrset(buf, 0);
#if DEBUG
		message((-11, "bindfile(%s): path=%s rule=%s alias=%s view=%d time=%s", name, b, r ? r->name : (char*)0, (x = getrule(b)) ? x->name : (char*)0, view, strtime(tm)));
#endif
		if (!r) r = makerule(name);
		if (!(r->dynamic & D_member) || tm > r->time)
		{
			if (r->dynamic & D_member)
			{
				r->dynamic &= ~D_member;
				r->dynamic |= D_membertoo;
			}
			r->time = tm;
			if (!(r->dynamic & D_entries))
			{
				if (S_ISREG(st.st_mode) || !st.st_mode) r->dynamic |= D_regular;
				if (!(r->dynamic & D_source)) r->view = view;
			}
			if (!r->view && *b == '/')
			{
				if (strncmp(b, internal.pwd, internal.pwdlen) || *(b + internal.pwdlen) != '/')
				{
					if ((state.questionable & 0x00000800) || !(r->property & P_target))
						r->dynamic |= D_global;
				}
				else if ((r->dynamic & D_regular) && (x = getrule(b + internal.pwdlen + 1)) && x != r)
					r = bindalias(r, x, b + internal.pwdlen + 1, od);
			}
			if (!(r->dynamic & D_global))
				r->preview = r->view;
			if ((x = getrule(b)) && (x->dynamic & D_alias))
				x = makerule(x->name);
			if (!(state.questionable & 0x00001000) && aliased && !x && !streq(name, r->name))
				x = makerule(name);
			if (x && x != r)
			{
				if (st.st_mode) internal.openfile = x->name;
				if (r->property & x->property & P_target)
				{
					message((-2, "%s not aliased to %s", unbound(r), unbound(x)));
					if (state.questionable & 0x00000040) found = 0;
				}
				else r = bindalias(r, x, b, od);
			}
			else
			{
				/*
				 * bind the rule to file name in b
				 * saving the unbound name
				 */

				s = r->name;
				r->name = putrule(b, r);
				if (st.st_mode) internal.openfile = r->name;
				if (r->name != s)
				{
					r->uname = s;
					if (od && (s != name || state.mam.statix || (n = strlen(r->name)) < (i = strlen(s)) || r->name[n - i - 1] != '/' || !streq(s, r->name + n - i)))
						putbound(s, od->name);
				}
			}
			if ((r->dynamic & D_source) && r->uname)
				r->view = r->preview = view;
			if (r->view && (x = localrule(r, 0)))
				merge(x, r, MERGE_ALL|MERGE_BOUND);
		}
		else if (!state.accept && !view)
			r->view = r->active ? r->active->parent->target->view : state.maxview;
	}
	else if (!r)
	{
		if (!(flags & BIND_RULE))
			goto done;
		r = makerule(name);
	}

	/*
	 * propagate pattern association attributes
	 */

	bindattribute(r);

	/*
	 * archive binding and final checks
	 */

	if (found)
	{
		if (r->scan == SCAN_IGNORE)
			r->dynamic |= D_scanned;
		else if ((r->property & (P_archive|P_target)) == (P_archive|P_target))
			arscan(r);

		/*
		 * if name is partially qualified then check for
		 * alias bindings in the path suffixes of name
		 */

		if (!(state.questionable & 0x00010000) && dir && (r->dynamic & D_regular) && *(s = name) != '/')
			while (s = strchr(s, '/'))
				if ((a = getrule(++s)) && !(a->dynamic & D_bound))
					bindfile(a, NiL, 0);
	}
	else if (!(r->dynamic & D_member))
	{
		r->time = ((r->property & P_dontcare) || !(flags & BIND_FORCE)) ? 0 : CURTIME;
		if (r->property & P_dontcare)
			r->view = state.maxview;
	}
 done:
	if (allocated)
		free(name);
	sfstrclose(buf);
	sfstrclose(tmp);
	return r;
}

/*
 * propagate pattern association attributes
 */

void
bindattribute(register struct rule* r)
{
	register struct rule*	x;
	register struct rule*	z;

	r->dynamic |= D_bound;
	if (x = associate(internal.attribute_p, r, NiL, NiL))
	{
		merge(x, r, MERGE_ATTR);
		*x->name = ATTRCLEAR;
		if (z = getrule(x->name))
			negate(z, r);
		*x->name = ATTRNAME;
	}
}

/*
 * bind a rule, possibly changing the rule name
 */

struct rule*
bind(register struct rule* r)
{
	register struct rule*	b;

	if (!r)
		return 0;
	if (r->dynamic & D_alias)
		r = makerule(r->name);
	if (r->dynamic & D_bound)
		return r;
	switch (r->property & (P_state|P_virtual))
	{
	case 0:
		if (b = bindfile(r, NiL, 0))
			return b;
		break;
	case P_state:
		if (b = bindstate(r, NiL))
		{
			if (state.mam.regress && (r->property & P_statevar))
				dumpregress(state.mam.out, "bind", r->name, r->statedata);
			return b;
		}
		break;
	case P_virtual:
		r->time = staterule(RULE, r, NiL, 1)->time;
		break;
	}
	bindattribute(r);
	return r;
}

/*
 * rebind rule r
 * op > 0 skips bindfile()
 * op < 0 skips statetime()
 */

void
rebind(register struct rule* r, register int op)
{
	char*		t;
	struct rule*	x;

	if (!(r->property & P_state))
	{
		if (r->uname) oldname(r);
		r->dynamic &= ~(D_bound|D_entries|D_member|D_scanned);
		if (op > 0) r->dynamic |= D_bound;
		else
		{
			newfile(r, NiL, NOTIME);
			if ((t = strchr(r->name, '/')) && (x = getrule(t + 1)) && (x = bindfile(x, NiL, 0))) r = x;
			else bindfile(r, NiL, 0);
		}
		if (op >= 0) r->dynamic |= D_triggered;
		if (!state.exec)
		{
			r->time = CURTIME;
			r->status = EXISTS;
		}
		else if (op >= 0)
		{
			statetime(r, !op);
			r->status = r->time ? EXISTS : (op || (r->property & P_dontcare)) ? IGNORE : FAILED;
		}
	}
	else if (r->property & P_statevar)
	{
		if (op <= 0)
			r->dynamic &= ~D_bound;
		if (!(r->dynamic & D_bound) && !(r = bindstate(r, NiL)))
			return;
		if (op > 0)
			r->time = OLDTIME;
	}
	message((-2, "%s(%s) = %s", op > 0 ? "accept" : "rebind", r->name, strtime(r->time)));
}

/*
 * remove binding on r
 * candidates have s==0 or r->mark==1
 * h!=0 for alias reference
 */

int
unbind(const char* s, char* v, void* h)
{
	register struct rule*	r = (struct rule*)v;

	if (!s || !h && (r->mark & M_mark) || h && (r->dynamic & D_alias) && (makerule(r->name)->mark & M_mark))
	{
		message((-2, "unbind(%s)%s%s", r->name, h ? " check-alias" : null, s && streq(s, r->name) ? null : " diff-hash"));
		r->mark &= ~M_mark;
		if (r->property & P_metarule) r->uname = 0;
		else
		{
			int	u = 0;

			if (!(r->property & P_state))
			{
				newfile(r, NiL, NOTIME);
				if (r->uname)
				{
					oldname(r);
					u = 1;
				}
			}
			else if (r->property & P_staterule)
			{
				error(2, "%s: cannot unbind staterules", r->name);
				return 0;
			}
			r->dynamic &= ~(D_bound|D_entries|D_global|D_member|D_membertoo|D_regular|D_scanned);
			r->must = 0;
			r->scan = 0;
			r->status = NOTYET;
			r->time = 0;
			if (u || !(r->dynamic & D_source))
			{
				r->preview = 0;
				r->view = 0;
			}
		}
	}
	return 0;
}

/*
 * fix up .SOURCE prereqs after user assertion
 */

struct rule*
source(register struct rule* r)
{
	register struct rule*	x;

	if (state.compile > COMPILED) return r;
	if (state.compile < COMPILED)
	{
		x = r;
		r = catrule(x->name, internal.internal->name, NiL, 1);
		freelist(r->prereqs);
		r->prereqs = listcopy(x->prereqs);
		r->dynamic |= D_compiled | (x->dynamic & D_dynamic);
		r->property |= P_readonly;
	}
	r->dynamic |= D_cached;
	if (r->dynamic & D_dynamic) dynamic(r);
	if (state.maxview && !state.fsview)
	{
		register char*		s;
		register char*		t;
		register struct list*	p;
		int			dot;
		unsigned int		view;
		struct list*		z;
		struct list		lst;
		Sfio_t*			tmp;

		/*
		 * recompute view cross product
		 *
		 *	.	unit multiplication operand
		 *	A	absolute path rooted at /
		 *	R	path relative to .
		 *
		 *	lhs	rhs	cross-product
		 *	----	-----	-------------
		 *	.	.	.	*note (1)*
		 *	.	A	A	*note (2)*
		 *	.	R	R
		 *	A	.	A
		 *	A	R	A/R
		 *	A	A	A	*note (2)*
		 *	R	.	R
		 *	R	A	A	*note (2)*
		 *	R	R	R/R
		 *
		 *	(1) the first . lhs operand produces a . in the product
		 *
		 *	(2) the first A rhs operand is placed in the product
		 */

#if DEBUG
		message((-5, "%s: recompute view cross product", r->name));
#endif
		tmp = sfstropen();
		z = &lst;
		z->next = 0;
		if (state.strictview)
		{
			/*
			 * this follows 3d fs semantics
			 */

			for (p = r->prereqs; p; p = p->next)
			{
				if (*(t = unbound(p->rule)) == '/')
				{
					sfputr(tmp, t, -1);
					t = sfstruse(tmp);
					pathcanon(t, 0);
					if (!(x = getrule(t))) x = makerule(t);
					if (!(x->mark & M_directory))
					{
						x->mark |= M_directory;
						z = z->next = cons(x, NiL);
					}
				}
				else if (!p->rule->view)
				{
					dot = (*t == '.' && !*(t + 1));
					for (view = 0; view <= state.maxview; view++)
					{
#if BINDINDEX
						s = state.view[view].path->name;
#else
						s = state.view[view].path;
#endif
						if (dot || *s != '.' || *(s + 1))
						{
							if (dot) sfputr(tmp, s, -1);
							else sfprintf(tmp, "%s/%s", s, t);
						}
						else sfputr(tmp, t, -1);
						s = sfstruse(tmp);
						pathcanon(s, 0);
						x = makerule(s);
						if (!(x->dynamic & D_source))
						{
							x->dynamic |= D_source;
							if (x->view < view) x->view = view;
						}
						if (!(x->mark & M_directory))
						{
							x->mark |= M_directory;
							z = z->next = cons(x, NiL);
						}
					}
				}
			}
		}
		else
		{
			struct list*	q;
			int		dotted = 0;

			q = r->prereqs;
			do
			{
				for (view = 0; view <= state.maxview; view++)
				{
#if BINDINDEX
					s = state.view[view].path->name;
#else
					s = state.view[view].path;
#endif
					if ((dot = (*s == '.' && !*(s + 1))) && !dotted)
					{
						dotted = 1;
#if BINDINDEX
						z = z->next = cons(state.view[view].path, NiL);
#else
						z = z->next = cons(makerule(s), NiL);
#endif
					}
					for (p = q; p; p = p->next)
					{
						if (*(t = unbound(p->rule)) == '/') break;
						if (!p->rule->view)
						{
							if (*t == '.' && !*(t + 1))
							{
								if (!dotted)
								{
									dotted = 1;
									sfputr(tmp, t, -1);
									t = sfstruse(tmp);
									pathcanon(t, 0);
									z = z->next = cons(makerule(t), NiL);
								}
								if (dot) continue;
								sfputr(tmp, s, -1);
							}
							else
							{
								if (dot) sfputr(tmp, t, -1);
								else sfprintf(tmp, "%s/%s", s, t);
							}
							t = sfstruse(tmp);
							pathcanon(t, 0);
							x = makerule(t);
							if (!(x->dynamic & D_source))
							{
								x->dynamic |= D_source;
								if (x->view < view) x->view = view;
							}
							if (!(x->mark & M_directory))
							{
								x->mark |= M_directory;
								z = z->next = cons(x, NiL);
							}
						}
					}
				}
				for (; p && *(t = unbound(p->rule)) == '/'; p = p->next)
				{
					sfputr(tmp, t, -1);
					t = sfstruse(tmp);
					pathcanon(t, 0);
					if (!(x = getrule(t))) x = makerule(t);
					if (!(x->mark & M_directory))
					{
						x->mark |= M_directory;
						z = z->next = cons(x, NiL);
					}
				}
			} while (q = p);
		}
		sfstrclose(tmp);
		freelist(r->prereqs);
		for (r->prereqs = p = lst.next; p; p = p->next)
			p->rule->mark &= ~M_directory;
	}
	return r;
}

#if BINDINDEX
/*
 * copy path name of r to s
 * end of s is returned
 */

char*
pathname(register char* s, register struct rule* r)
{
	if ((r->dynamic & D_bound) && !(r->property & (P_state|P_virtual)) && *r->name != '/')
	{
		if (!state.logical && r->view && !(r->dynamic & D_bindindex))
		{
			s = strcopy(s, state.view[r->view].path);
			*s++ = '/';
		}
		if (r->source && !(r->dynamic & D_bindindex))
		{
			s = strcopy(s, state.source[r->source].path);
			*s++ = '/';
		}
	}
	return strcopy(s, r->name);
}
#endif

/*
 * return local view path name for r
 */

char*
localview(register struct rule* r)
{
	register struct rule*	x;

	if (r->dynamic & D_alias) r = makerule(r->name);
	if (state.context && !(r->property & (P_state|P_virtual)))
	{
		register char*	s = r->name;

		if (*s == '/' || iscontext(s)) return s;
		sfprintf(state.context, "%c%s%c", MARK_CONTEXT, s, MARK_CONTEXT);
		x = makerule(sfstruse(state.context));
		if (!(x->dynamic & D_alias))
		{
			x->property |= P_internal;
			x->dynamic |= D_alias;
			x->uname = x->name;
			x->name = s;
		}
		return x->uname;
	}
	if (!state.maxview || state.fsview && !state.expandview) return r->name;
	return (x = localrule(r, 1)) ? x->name : (r->view && r->uname ? r->uname : r->name);
}
