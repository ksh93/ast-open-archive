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
 * make support routines
 */

#include "make.h"

#include <int.h>
#include <tm.h>

/*
 * stat() that checks for read access
 * if res!=0 then resolve(name,fd,mode) must be called
 */

int
rstat(char* name, struct stat* st, int res)
{
	if (internal.openfile)
	{
		close(internal.openfd);
		internal.openfile = 0;
	}
	while ((internal.openfd = open(name, O_RDONLY)) < 0)
	{
		if (errno != EINTR)
		{
			if (errno == EACCES)
			{
				if (state.maxview && !state.fsview)
				{
					int		oerrno = errno;
					struct stat	rm;

					if (!stat(name, st) && st->st_nlink > 1 && !st->st_size && !(st->st_mode & S_IPERM))
					{
						edit(internal.tmp, name, KEEP, ".../...", DELETE);
						if (!stat(sfstruse(internal.tmp), &rm) && rm.st_ino == st->st_ino)
						{
							/*
							 * not in this or any lower view
							 */

							errno = ENODEV;
							return -1;
						}
					}
					errno = oerrno;
				}
				if (!stat(name, st) && (st->st_mode & (S_IXUSR|S_IXGRP|S_IXOTH)))
					goto found;
				error(1, "%s ignored -- not readable", name);
			}
			return -1;
		}
	}
	internal.openfile = name;
	if (res && resolve(internal.openfile, internal.openfd, O_RDONLY))
		return -1;
	if (fstat(internal.openfd, st))
		return -1;
 found:
	if (!st->st_mtime)
	{
		error(1, "%s modify time must be after the epoch", name);
		st->st_mtime = OLDTIME;
	}
	return 0;
}

/*
 * allocate a chunk of units
 * free units linked onto head
 */

void*
newchunk(char** head, register size_t unit)
{
#if __hppa || __hppa__ || hppa /* cc botches e arithmetic -- off by 8! */
	NoP(head);
	return newof(0, char, unit, 0);
#else
	register char*	p;
	register char*	e;
	register char**	x;
	int		n;
	void*		v;

	n = (4096 / unit) * unit;
	v = p = newof(0, char, n, 0);
	e = p + n - unit;
	x = head;
	while (((char*)(x = (char**)(*x = p += unit))) < e);
	return v;
#endif
}

/*
 * append a list q onto the end of list p
 * list p is surgically modified
 */

struct list*
append(struct list* p, struct list* q)
{
	register struct list*	t;

	if (t = p)
	{
		if (q)
		{
			while (t->next)
				t = t->next;
			t->next = q;
		}
		return p;
	}
	else return q;
}

/*
 * add rule r onto the front of list p
 * list p is not modified
 */

struct list*
cons(struct rule* r, struct list* p)
{
	register struct list*	q;

	newlist(q);
	q->next = p;
	q->rule = r;
	return q;
}

/*
 * construct and return a copy of list p
 * the items in the list are not copied
 */

struct list*
listcopy(register struct list* p)
{
	register struct list*	q;
	register struct list*	r;
	register struct list*	t;

	if (!p)
		return 0;
	newlist(r);
	q = r;
	while (p)
	{
		q->rule = p->rule;
		if (p = p->next)
		{
			newlist(t);
			q = q->next = t;
		}
	}
	q->next = 0;
	return r;
}

/*
 * convert time t to a string for tracing
 */

char*
strtime(unsigned long t)
{
	if (!t)
		return "not found";
	else if (t == NOTIME)
		return "not checked";
	else if (t == OLDTIME)
		return "really old";
	else
	{
		time_t		tm = t;

		static char	tms[3][64];
		static int	tmx;

		if (++tmx >= elementsof(tms))
			tmx = 0;
		tmform(tms[tmx], "%?%h %d %H:%M:%S %Y", &tm);
		return tms[tmx];
	}
}

/*
 * printext() value types
 */

#ifndef int_8
#define int_8		int_4
#undef	strtoll
#define strtoll		strtol
#undef	strtoull
#define strtoull	strtoul
#endif

typedef union
{
	char**		p;
	char*		s;
	int_8		q;
	unsigned long	u;
	long		l;
	int		i;
	short		h;
	char		c;
	double		d;
} Value_t;

typedef struct
{
	Sffmt_t		fmt;
	char*		arg;
	int		all;
	Sfio_t*		tmp;
} Fmt_t;

/*
 * printf %! extension function
 */

static int
printext(Sfio_t* sp, void* vp, Sffmt_t* dp)
{
	register Fmt_t*	fp = (Fmt_t*)dp;
	Value_t*	value = (Value_t*)vp;
	register char*	s;
	char*		txt;
	time_t		tm;

	if (fp->all)
	{
		s = fp->arg;
		fp->arg += strlen(s);
	}
	else if (!(s = getarg(&fp->arg, NiL)))
		return -1;
	if (dp->n_str > 0)
	{
		if (!fp->tmp)
			fp->tmp = sfstropen();
		sfprintf(fp->tmp, "%.*s", dp->n_str, dp->t_str);
		txt = sfstruse(fp->tmp);
	}
	else
		txt = 0;
	dp->flags |= SFFMT_VALUE;
	switch (dp->fmt)
	{
	case 'c':
		value->c = *s;
		break;
	case 'd':
		dp->size = sizeof(value->q);
		value->q = strtoll(s, NiL, 0);
		break;
	case 'F':
		dp->fmt = 'f';
		/*FALLTHROUGH*/
	case 'e':
	case 'f':
	case 'g':
	case 'E':
	case 'G':
		dp->size = sizeof(value->d);
		value->d = strtod(s, NiL);
		break;
	case 'p':
		value->p = (char**)strtol(s, NiL, 0);
		break;
	case 's':
		value->s = s;
		break;
	case 'u':
	case 'x':
	case 'X':
		dp->size = sizeof(value->q);
		value->q = strtoull(s, NiL, 0);
		break;
	case 'S':
		value->s = s;
		if (txt)
		{
			if (streq(txt, "identifier"))
			{
				if (*s && !istype(*s, C_ID1))
					*s++ = '_';
				for (; *s; s++)
					if (!istype(*s, C_ID1|C_ID2))
						*s = '_';
			}
			else if (streq(txt, "invert"))
			{
				for (; *s; s++)
					if (isupper(*s))
						*s = tolower(*s);
					else if (islower(*s))
						*s = toupper(*s);
			}
			else if (streq(txt, "lower"))
			{
				for (; *s; s++)
					if (isupper(*s))
						*s = tolower(*s);
			}
			else if (streq(txt, "upper"))
			{
				for (; *s; s++)
					if (islower(*s))
						*s = toupper(*s);
			}
			else if (streq(txt, "variable"))
			{
				for (; *s; s++)
					if (!istype(*s, C_VARIABLE1|C_VARIABLE2))
						*s = '.';
			}
		}
		dp->fmt = 's';
		dp->size = -1;
		break;
	case 'T':
		tm = strtol(s, NiL, 0);
		if (txt)
			tmform(value->s = tmpname, txt, tm == -1 ? (time_t*)0 : &tm);
		else
			value->s = strtime(tm);
		dp->fmt = 's';
		dp->size = -1;
		break;
	case 'Z':
		dp->fmt = 'c';
		value->c = 0;
		break;
	case '.':
		value->i = (int)strtol(s, NiL, 0);
		break;
	default:
		dp->flags &= ~SFFMT_VALUE;
		error(2, "%%%c: unknown format", dp->fmt);
		break;
	}
	return 0;
}

/*
 * printf from args in argp into sp
 * all!=0 if %s gets all of argp
 * term is an sfputr terminator
 */

int
strprintf(Sfio_t* sp, const char* format, char* argp, int all, int term)
{
	int	n;
	int	i;
	Sfio_t*	tp;
	Fmt_t	fmt;

	memset(&fmt, 0, sizeof(fmt));
	fmt.fmt.version = SFIO_VERSION;
	tp = sfstropen();
	sfprintf(tp, "%s", format);
	stresc(fmt.fmt.form = sfstruse(tp));
	fmt.fmt.extf = printext;
	fmt.arg = argp;
	fmt.all = all;
	n = 0;
	while ((i = sfprintf(sp, "%!", &fmt)) >= 0)
	{
		n += i;
		if (fmt.arg <= argp || !*(argp = fmt.arg))
			break;
	}
	if (term != -1)
	{
		sfputc(sp, term);
		n++;
	}
	sfstrclose(tp);
	if (fmt.tmp)
		sfstrclose(fmt.tmp);
	return n;
}

/*
 * time comparison with tolerance
 * same return value as strcmp()
 */

int
cmptime(unsigned long a, unsigned long b)
{
	long	diff;

	diff = a - b;
	if (diff < -state.tolerance)
		return -1;
	if (diff > state.tolerance)
		return 1;
	return 0;
}

/*
 * return next (possibly quoted) space-separated arg in *buf
 * *buf points past end of arg on return
 * the contents of buf are modified
 * if flags!=0 then it is set with metarule specific A_* flags
 */

char*
getarg(char** buf, register int* flags)
{
	register char*	s;
	register char*	t;
	register int	c;
	char*		a;
	char*		q;
	int		paren;

	if (flags)
		*flags &= ~(A_group|A_metarule|A_scope);
	for (s = *buf; isspace(*s); s++);
	if (!*(a = t = s))
		return 0;
	paren = 0;
	for (;;)
	{
		switch (c = *s++)
		{
		case '\\':
			if (*s)
			{
				*t++ = c;
				c = *s++;
			}
			break;
		case '(':
			paren++;
			if (flags)
				*flags |= A_group;
			break;
		case ')':
			paren--;
			break;
		case '"':
		case '\'':
			if (!paren)
			{
				for (q = t; *s && *s != c; *t++ = *s++)
					if (*s == '\\' && *(s + 1))
						*t++ = *s++;
				if (*s)
					s++;
				*t = 0;
				t = q + stresc(q);
				continue;
			}
			break;
		case '%':
			if (flags && !(*flags & A_scope))
				*flags |= A_metarule;
			break;
		case '=':
			if (flags && !(*flags & (A_group|A_metarule)))
				*flags |= A_scope;
			break;
		default:
			if (paren || !isspace(c))
				break;
			/*FALLTHROUGH*/
		case 0:
			*t = 0;
			if (!c)
				s--;
			*buf = s;
			return a;
		}
		*t++ = c;
	}
}

/*
 * list explanation
 */

void
explain(int level, ...)
{
	va_list	ap;

	va_start(ap, level);
	errorv(NiL, state.explain ? 0 : EXPTRACE, ap);
	va_end(ap);
}

/*
 * return # outstanding jobs
 */

char*
b_outstanding(char** args)
{
	sfprintf(internal.val, "%d", state.coshell ? state.coshell->outstanding : 0);
	return sfstruse(internal.val);
}

/*
 * getconf() builtin
 */

char*
b_getconf(char** args)
{
	char*	name;
	char*	path;
	char*	value;

	if (name = *args)
		args++;
	if (path = *args)
		args++;
	value = *args;
	return astconf(name, path, value);
}
