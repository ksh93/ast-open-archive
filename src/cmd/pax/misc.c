/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1987-2002 AT&T Corp.                *
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
 * pax miscellaneous support
 */

#include "pax.h"
#include "options.h"

#include <sfdisc.h>

/*
 * return format index given format name
 */

int
getformat(register char* name)
{
	register int	i;

	strlower(name);
	for (i = 0; format[i].name; i++)
		if (streq(name, format[i].name))
			break;
	return format[i].name ? i : streq(name, "-") || streq(name, "pax") ? OUT_DEFAULT : -1;
}

/*
 * path name strcmp()
 */

static int
pathcmp(register const char* s, register const char* t)
{
	register int	sc;
	register int	tc;

	for (;;)
	{
		tc = *t++;
		if (!(sc = *s++))
			return tc ? -1 : 0;
		if (sc != tc)
		{
			if (tc == 0 || tc == '/')
				return 1;
			if (sc == '/')
				return -1;
			return strcoll(s - 1, t - 1);
		}
	}
}

/*
 * check base archive ordering
 */

static void
ordered(Archive_t* ap, const char* prv, const char* cur)
{
	if (pathcmp(prv, cur) > 0)
		error(3, "%s: %s: archive member must appear before %s", ap->name, prv, cur);
}

/*
 * check f with patterns given on cmd line
 */

int
selectfile(register Archive_t* ap, register File_t* f)
{
	register Archive_t*	bp;
	register Member_t*	d;
	int			linked = 0;

	if (f->skip || f->namesize <= 1 || f->linkname && !*f->linkname)
		return 0;
	if (state.ordered)
	{
		ordered(ap, ap->path.prev, f->name);
		strcpy(ap->path.prev, f->name);
	}
	if (f->record.format && state.record.pattern)
	{
		static char	fmt[2];

		fmt[0] = f->record.format;
		if (!strmatch(fmt, state.record.pattern))
			return 0;
	}
	if (state.append || ap->parent)
	{
		linked = 1;
		addlink(ap, f);
		if (!ap->parent)
			return 0;
		if (!(d = newof(0, Member_t, 1, 0)))
			error(3, "%s: out of space", f->name);
		d->dev = f->st->st_dev;
		d->ino = f->st->st_ino;
		d->mode = f->st->st_mode;
		d->mtime = f->st->st_mtime;
		d->offset = ap->io->offset + ap->io->count;
		d->size = f->st->st_size;
		d->expand = f->delta.size;
		if (!(d->info = (File_t*)memdup(f, sizeof(File_t))) || !(d->info->st = (struct stat*)memdup(f->st, sizeof(struct stat))))
			error(3, "%s: out of space", f->name);
		d->info->name = d->info->path = hashput(ap->parent->delta->tab, f->name, d);
		if (d->info->linktype != NOLINK)
			d->info->linkname = strdup(d->info->linkname);
		if (d->info->uidname)
			d->info->uidname = strdup(d->info->uidname);
		if (d->info->gidname)
			d->info->gidname = strdup(d->info->gidname);
		d->info->delta.base = d;
		if (!state.ordered)
			return 0;
	}
	if (!match(f->path) || state.verify && f->type != X_IFDIR && !verify(ap, f, NiL))
		return 0;
	ap->selected++;
	if (!linked)
	{
		if (state.list)
			addlink(ap, f);
		if (ap->tab)
		{
			if (!(d = newof(0, Member_t, 1, 0)))
				error(3, "%s: out of space", f->name);
			d->dev = f->st->st_dev;
			d->ino = f->st->st_ino;
			d->mode = f->st->st_mode;
			d->mtime = f->st->st_mtime;
			d->offset = ap->io->offset + ap->io->count;
			d->size = f->st->st_size;
			d->expand = f->delta.size;
			hashput(ap->tab, f->path, d);
		}
	}
	if (state.ordered && ap->delta && ap->delta->format != COMPRESS && (bp = ap->delta->base))
	{
		register int	n;
		register int	m;

		for (;;)
		{
			if (bp->peek) bp->peek = 0;
			else
			{
				if (bp->skip && bp->skip == bp->io->offset + bp->io->count)
					fileskip(bp, &bp->file);
				if (!getheader(bp, &bp->file)) break;
				bp->skip = bp->io->offset + bp->io->count;
			}
			ordered(bp, bp->path.prev, bp->file.name);
			if ((m = pathcmp(bp->file.name, f->name)) > 0)
			{
				bp->peek = 1;
				break;
			}
			n = selectfile(bp, &bp->file);
			if (!m) break;
			if (n && !state.list)
			{
				if (ap->io->mode != O_RDONLY)
				{
					File_t	tmp;

					initfile(ap, &tmp, bp->file.name, X_IFREG);
					tmp.delta.op = DELTA_delete;
					putheader(ap, &tmp);
					puttrailer(ap, &tmp);
				}
				else
				{
					struct stat	st;

					if (!(*state.statf)(f->name, &st))
					{
						if (S_ISDIR(st.st_mode))
						{
							if (!streq(f->name, ".") && !streq(f->name, ".."))
							{
								if (rmdir(f->name)) error(ERROR_SYSTEM|2, "%s: cannot remove directory", f->name);
								else listentry(f);
							}
						}
						else if (remove(f->name)) error(ERROR_SYSTEM|2, "%s: cannot remove file", f->name);
						else listentry(f);
					}
				}
			}
		}
	}
	return 1;
}

/*
 * verify action on file
 *
 *	EOF	exit
 *	NULL	skip file
 *	.	keep file
 *	<else>	rename file
 */

int
verify(Archive_t* ap, register File_t* f, register char* prompt)
{
	register char*	name;

	NoP(ap);
	if (!prompt)
	{
		if (state.yesno)
			switch (state.operation)
			{
			case IN:
				prompt = "Read";
				break;
			case OUT:
				prompt = "Write";
				break;
			default:
				prompt = "Pass";
				break;
			}
		else
			prompt = "Rename";
	}
	sfprintf(state.wtty, "%s %s: " , prompt, f->name);
	if (!(name = sfgetr(state.rtty, '\n', 1)))
	{
		sfputc(state.wtty, '\n');
		finish(2);
	}
	if (state.yesno)
		return *name == 'y';
	switch (*name)
	{
	case 0:
		return 0;
	case '.':
		if (!*(name + 1)) break;
		/*FALLTHROUGH*/
	default:
		f->namesize = pathcanon(f->name = name, 0) - name + 1;
		break;
	}
	return 1;
}

/*
 * check for file name mapping
 * static data possibly returned
 * two simultaneous calls supported
 */

char*
map(register char* name)
{
	register Map_t*	mp;
	char*		to;
	char*		from;
	File_t		f;
	int		n;
	regmatch_t	match[10];

	if (state.filter.line > 1)
	{
		state.filter.line = 1;
		name = state.filter.name;
	}
	from = to = name;
	for (mp = state.maps; mp; mp = mp->next)
		if (!(n = regexec(&mp->re, from, elementsof(match), match, 0)))
		{
			if (n = regsubexec(&mp->re, from, elementsof(match), match))
				regfatal(&mp->re, 3, n);
			n = strlen(mp->re.re_sub->re_buf) + 1;
			to = fmtbuf(n);
			memcpy(to, mp->re.re_sub->re_buf, n);
			if (mp->re.re_sub->re_flags & REG_SUB_PRINT)
				sfprintf(sfstderr, "%s >> %s\n", from, to);
			if (mp->re.re_sub->re_flags & REG_SUB_STOP)
				break;
			from = to;
		}
		else if (n != REG_NOMATCH)
			regfatal(&mp->re, 3, n);
	if (state.local && (*to == '/' || *to == '.' && *(to + 1) == '.' && (!*(to + 2) || *(to + 2) == '/')))
	{
		if (state.verify)
		{
			f.name = to;
			if (verify(NiL, &f, "Retain non-local file"))
				return f.name;
		}
		error(1, "%s: non-local path rejected", to);
		to = "";
	}
	return to;
}

typedef struct
{
	Archive_t*	archive;
	File_t*		file;
} List_handle_t;

/*
 * sfkeyprintf() lookup
 */

static int
listlookup(void* handle, register Sffmt_t* fmt, const char* arg, char** ps, Sflong_t* pn)
{
	List_handle_t*		gp = (List_handle_t*)handle;
	register File_t*	f = gp->file;
	register struct stat*	st = f->st;
	register char*		s = 0;
	register Sflong_t	n = 0;
	Option_t*		op;

	static Sfio_t*		mp;
	static const char	fmt_time[] = "time=%?%l";
	static const char	fmt_mode[] = "mode";

	static char		buf[PATH_MAX];
	static char		huh[8];

	if (!(op = (Option_t*)hashget(state.options, fmt->t_str)))
	{
		if (*fmt->t_str != '$')
			return 0;
		if (!(op = newof(0, Option_t, 1, 0)))
			error(3, "out of space [option]");
		op->name = hashput(state.options, 0, op);
		op->macro = getenv(fmt->t_str + 1);
		op->index = OPT_environ;
		op->flags |= OPT_DISABLE;
	}
	if (op->macro && !(op->flags & OPT_DISABLE))
	{
		op->flags |= OPT_DISABLE;
		if (!mp && !(mp = sfstropen()))
			error(3, "out of space [macro]");
		sfkeyprintf(mp, handle, op->macro, listlookup, NiL);
		s = sfstruse(mp);
		op->flags &= ~OPT_DISABLE;
	}
	else switch (op->index)
	{
	case OPT_atime:
		n = st->st_atime;
		if (!arg)
			arg = fmt_time;
		break;
	case OPT_charset:
		s = "ASCII";
		break;
	case OPT_chksum:
	case OPT_magic:
	case OPT_typeflag:
	case OPT_version:
		if (!gp->archive || gp->archive->format != PAX && gp->archive->format != TAR && gp->archive->format != USTAR)
			return 0;
		switch (op->index)
		{
		case OPT_chksum:
			s = tar_header.chksum;
			break;
		case OPT_magic:
			s = tar_header.magic;
			break;
		case OPT_typeflag:
			n = tar_header.typeflag;
			break;
		case OPT_version:
			s = tar_header.version;
			break;
		}
		break;
	case OPT_ctime:
		n = st->st_ctime;
		if (!arg)
			arg = fmt_time;
		break;
	case OPT_delta:
		switch (f->delta.op)
		{
		case 0:
		case DELTA_pass:
		case DELTA_zip:
			return 0;
		case DELTA_create:
			s = "create";
			break;
		case DELTA_delete:
			s = "delete";
			break;
		case DELTA_update:
			s = "update";
			break;
		case DELTA_verify:
			s = "verify";
			break;
		default:
			sfsprintf(s = huh, sizeof(huh), "[op=%c]", f->delta.op);
			break;
		}
		break;
	case OPT_device:
		if (f->type == X_IFBLK || f->type == X_IFCHR)
			s = fmtdev(st);
		else return 0;
		break;
	case OPT_devmajor:
		n = major(st->st_dev);
		break;
	case OPT_devminor:
		n = minor(st->st_dev);
		break;
	case OPT_environ:
		if (!(s = op->macro))
			return 0;
		break;
	case OPT_gname:
		if (f->gidname)
		{
			if (fmt->fmt == 's') s = f->gidname;
			else n = strgid(f->gidname);
		}
		else if (fmt->fmt == 's') s = fmtgid(st->st_gid);
		else n = st->st_gid;
		break;
	case OPT_ino:
		n = st->st_ino;
		break;
	case OPT_linkop:
		switch (f->linktype)
		{
		case HARDLINK:
			s = "==";
			break;
		case SOFTLINK:
			s = "->";
			break;
		default:
			return 0;
		}
		break;
	case OPT_linkpath:
		if (f->linktype == NOLINK)
			return 0;
		s = f->linkname;
		break;
	case OPT_mark:
		if (f->linktype == HARDLINK)
			s = "=";
		else if (f->linktype == SOFTLINK)
			s = "@";
		else if (f->type == X_IFDIR)
			s = "/";
		else if (f->type == X_IFIFO)
			s = "|";
		else if (f->type == X_IFSOCK)
			s = "=";
		else if (f->type == X_IFBLK || f->type == X_IFCHR)
			s = "$";
		else if (st->st_mode & (X_IXUSR|X_IXGRP|X_IXOTH))
			s = "*";
		else return 0;
		break;
	case OPT_mode:
		n = st->st_mode;
		if (!arg)
			arg = fmt_mode;
		break;
	case OPT_mtime:
		n = st->st_mtime;
		if (!arg)
			arg = fmt_time;
		break;
	case OPT_name:
		if (s = strrchr(f->name, '/')) s++;
		else s = f->name;
		break;
	case OPT_nlink:
		n = st->st_nlink;
		break;
	case OPT_path:
		s = f->name;
		break;
	case OPT_sequence:
		sfsprintf(s = buf, sizeof(buf), "%d-%d", gp->archive->volume, gp->archive->entry);
		break;
	case OPT_size:
		if (f->linktype == SOFTLINK) n = f->linknamesize - 1;
		else if (f->delta.size != -1) n = f->delta.size;
		else n = st->st_size;
		break;
	case OPT_uname:
		if (f->uidname)
		{
			if (fmt->fmt == 's') s = f->uidname;
			else n = strgid(f->uidname);
		}
		else if (fmt->fmt == 's') s = fmtuid(st->st_uid);
		else n = st->st_uid;
		break;
	default:
		return 0;
	}
	if (s) *ps = s;
	else if (fmt->fmt == 's' && arg)
	{
		if (strneq(arg, fmt_mode, 4))
			*ps = fmtmode(n, 1);
		else if (strneq(arg, fmt_time, 4))
			*ps = fmttime((*(arg + 4) == '=' ? arg : fmt_time) + 5, n);
	}
	else *pn = n;
	return 1;
}

/*
 * set up lookup() handle and call sfkeyprintf()
 */

int
listprintf(Sfio_t* sp, Archive_t* ap, File_t* f, const char* format)
{
	List_handle_t	list;

	NoP(ap);
	list.archive = state.in;
	list.file = f;
	return sfkeyprintf(sp, &list, format, listlookup, NiL);
}

/*
 * list entry information based on state.drop, state.list and state.verbose
 */

void
listentry(register File_t* f)
{
	int	n;
	int	p;
	int	i;
	int	j;
	int	k;
	char*	s;
	char	bar[METER_parts + 1];

	if (!f->extended && !f->skip && (state.drop || state.list || state.meter.on || state.verbose))
	{
		if (state.meter.on)
		{
			for (s = f->name; *s; s++)
				if (s[0] == ' ' && s[1] == '-' && s[2] == '-' && s[3] == ' ')
					break;
			if (*s)
			{
				if (state.meter.last)
				{
					sfprintf(sfstderr, "%*s", state.meter.last, "\r");
					state.meter.last = 0;
				}
				sfprintf(sfstderr, "\n");
				listprintf(sfstderr, state.in, f, state.listformat);
				sfprintf(sfstderr, "\n\n");
				return;
			}
			if ((p = (state.in->io->count * 100) / state.meter.size) > 100)
				p = 100;
			n = listprintf(state.tmp.str, state.in, f, state.listformat);
			s = sfstruse(state.tmp.str);
			if (state.meter.fancy)
			{
				if (n > (state.meter.width - METER_parts - 1))
				{
					s += n - (state.meter.width - METER_parts - 1);
					n = state.meter.width - METER_parts - 1;
				}
				i = (p / (100 / METER_parts));
				j = n + METER_parts + 2;
				if (!state.meter.last)
					state.meter.last = j + 5;
				if ((k = state.meter.last - j - 5) < 0)
					k = 0;
				n = 0;
				while (n < i)
					bar[n++] = '*';
				while (n < elementsof(bar) - 1)
					bar[n++] = ' ';
				bar[n] = 0;
				state.meter.last = sfprintf(sfstderr, "%02d%% |%s| %s%*s", p, bar, s, k, "\r");
			}
			else
				sfprintf(sfstderr, "%02d%% %s\n", p, s);
			sfsync(sfstderr);
			if (state.test & 1) sleep(1);
		}
		else if (state.drop)
		{
			if (++state.dropcount >= 50)
			{
				state.dropcount = 0;
				sfprintf(sfstderr, ".\n");
			}
			else
			{
				sfprintf(sfstderr, ".");
				sfsync(sfstderr);
			}
		}
		else
			listprintf(state.list ? sfstdout : sfstderr, state.in, f, state.listformat);
	}
}

/*
 * prepare patterns for match()
 */

char**
initmatch(char** p)
{
	register char**	a;

	a = p;
	while (*a)
		pathcanon(*a++, 0);
	return p;
}

/*
 * determine if file s matches input patterns
 */

int
match(register char* s)
{
	register char**	p;
	register char*	t;
	int		n;

	if (!(p = state.patterns)) return state.matchsense;
	if (state.exact)
	{
		n = 0;
		while (t = *p++)
			if (*t)
			{
				if (streq(s, t))
				{
					*--p = "";
					return 1;
				}
				n = 1;
			}
		if (!n) finish(0);
	}
	else while (t = *p++)
	{
		if (state.descend && dirprefix(t, s) || strmatch(s, t))
			return state.matchsense;
	}
	return !state.matchsense;
}

/*
 * return 1 if p is a directory prefix of s
 */

int
dirprefix(register char* p, register char* s)
{
	if (*p == '.' && !*(p + 1) && *s != '/' && (*s != '.' || *(s + 1) != '.' || *(s + 2) && *(s + 2) != '/'))
		return 1;
	if (*p == '/' && !*(p + 1))
		return *s == '/';
	while (*p)
		if (*p++ != *s++)
			return 0;
	return !*s || *s == '/';
}

/*
 * return 1 if s is a portable string
 */

int
portable(const char* s)
{
	register unsigned char*	u = (unsigned char*)s;
	register int		c;

	while (c = *u++)
		if (c > 0177)
			return 0;
	return 1;
}
