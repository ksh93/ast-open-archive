/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*           Copyright (c) 1992-2006 AT&T Knowledge Ventures            *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                      by AT&T Knowledge Ventures                      *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * David Korn
 * AT&T Research
 *
 * od
 */

static const char usage[] =
"[-?\n@(#)$Id: od (AT&T Research) 2006-01-30 $\n]"
USAGE_LICENSE
"[+NAME?od - dump files in octal or other formats]"
"[+DESCRIPTION?\bod\b dumps the contents of the input files"
"	in various formats on the standard output. The standard input"
"	is read if \b-\b or no files are specified. Each output line contains"
"	the file offset of the data in the leftmost column, followed"
"	by one or more columns in the specified format. If more than"
"	one format is specified then the subsequent lines are listed"
"	with the offset column blank. Second and subsequent occurrences"
"	of a repeated output line are replaced by a single line with `*'"
"	in the first data column.]"
"[+?If the output format is specified by one of the obsolete forms (not \b-t\b)"
"	then the last file argument is interpreted as an offset expression"
"	if it matches the extended regular expression"
"	\b+?[0-9]]+\\.?[bkm]]?(ll|LL)?\b . In this case the first \aoffset\a"
"	bytes of the file are skipped. The optional \bb\b means bytes, \bk\b"
"	means Kb, and \bm\b means Mb. \bll\b and \bLL\b are ignored for"
"	compatibility with some systems.]"

"[A:address-radix?The file offset radix.]:[radix:=o]{"
"	[+d?decimal]"
"	[+o?octal]"
"	[+x?hexadecimal]"
"	[+n?none - do not print offset]"
"}"
"[B:swap?Swap input bytes according to the bit mask \aop\a, which is the"
"	inclusive or of:]#[op]{"
"		[+01?swap 8-bit bytes]"
"		[+02?swap 16-bit words]"
"		[+04?swap 32-bit longs]"
"		[+0?swap for big endian testing]"
"}"
"[j:skip-bytes?Skip \bbytes\b bytes into the data before formatting.]#[bytes]"
"[N:count|read-bytes?Output only \bbytes\b bytes of data.]#[bytes]"
"[m:map?\b--printable\b and \b--format=m\b bytes are converted from"
"	\acodeset\a to the native codeset. The codesets"
"	are:]:[codeset]{\fcodesets\f}"
"[p:printable?Output the printable bytes (after \b--map\b if specified), in"
"	the last data column. Non-printable byte values are printed as `.'.]"
"[z:strings?Output NUL terminated strings of at least \alength\a bytes.]#?"
"	[length:=3]"
"[t:format|type?The data item output \aformat\a and \asize\a. A decimal byte" 
"	count or size code may follow all but the \ba\b, \bc\b and \bm\b"
"	formats.]:[format[size]]:=o2]{\ftypes\f}"
"[T:test?Enable internal implementation specific tests.]:[test]{"
"	[+b\an\a?Allocate a fixed input buffer of size \an\a.]"
"	[+m\an\a?Set the mapped input buffer size to \an\a.]"
"	[+n?Turn off the \bSF_SHARE\b input buffer flag.]"
"}"
"[v:all|output-duplicates?Output all data.]"
"[w:per-line|width?The number of items to format per output line."
"	\aper-line\a must be a multiple of the least common multiple"
"	of the sizes of the format types.]#[per-line]"
"	[a?Equivalent to \b-ta\b.]"
"	[b?Equivalent to \b-toC\b.]"
"	[c?Equivalent to \b-tc\b.]"
"	[d?Equivalent to \b-tuS\b.]"
"	[D?Equivalent to \b-tuL\b.]"
"	[f?Equivalent to \b-tfF\b.]"
"	[F?Equivalent to \b-tfD\b.]"
"	[h?Equivalent to \b-txS\b.]"
"	[i?Equivalent to \b-tdS\b.]"
"	[l?Equivalent to \b-tdL\b.]"
"	[o?Equivalent to \b-toS\b.]"
"	[O?Equivalent to \b-toL\b.]"
"	[s?Equivalent to \b-tdS\b.]"
"	[S?Equivalent to \b-tdL\b.]"
"	[u?Equivalent to \b-tuS\b.]"
"	[U?Equivalent to \b-tuL\b.]"
"	[x?Equivalent to \b-txS\b.]"
"	[X?Equivalent to \b-txL\b.]"

"\n"
"\n[ file ... ] [ [+]offset[.|b|k|m|ll|LL] ]\n"
"\n"
"[+SEE ALSO?\bsed\b(1), \bstrings\b(1), \bswap\b(3), \bascii\b(5)]"
;

#include <cmd.h>
#include <int.h>
#include <sig.h>
#include <swap.h>
#include <ccode.h>
#include <ctype.h>
#include <iconv.h>

#define NEW		(1<<0)
#define OLD		(1<<1)

#define BASE_WIDTH	7
#define LINE_LENGTH	78

#define WIDTHINDEX(n)	((n>1)+(n>2)+(n>4)+(n>8))

#ifdef int_8
#define QUAL		"ll"
#else
#define QUAL		"l"
#endif

#if _typ_long_double
#define double_max	long double
#else
#define double_max	double
#endif

typedef char* (*Format_f)(int);

typedef struct Size_s
{
	char*		desc;
	char		name;
	char		map;
	char		dflt;
	char		prec;
	const char*	qual;
	char		size;
} Size_t;

typedef struct Type_s
{
	char*		desc;
	char		name;
	const char*	fill;
	Format_f	fun;
	const Size_t*	size;
	char		width[5];
} Type_t;

typedef struct Format_s
{
	struct Format_s*next;
	const Type_t*	type;
	Format_f	fun;
	char		form[16];
	struct
	{
	short		external;
	short		internal;
	}		size;
	short		fp;
	short		width;
	short		us;
} Format_t;

static struct State_s
{
	int		addr;
	char		base[8];
	int		block;
	char		buf[256];
	struct
	{
	char*		base;
	size_t		size;
	int		noshare;
	}		buffer;
	size_t		count;
	struct
	{
	char		buf[1024];
	char*		data;
	int		mark;
	size_t		size;
	}		dup;
	char*		file;
	Format_t*	form;
	Format_t*	last;
	unsigned char*	map;
	int_max		offset;
	struct
	{
	char*		data;
	size_t		size;
	}		peek;
	int		printable;
	int		size;
	size_t		skip;
	int		strings;
	int		style;
	int		swap;
	size_t		total;
	int		verbose;
	int		width;
} state;

static const Size_t	csize[] =
{
"char",		'C',	0,	1,	0,	0,	1,
0
};

static const Size_t	isize[] =
{
"char",		'C',	0,	0,	0,	0,	1,
"short",	'S',	0,	0,	0,	0,	sizeof(short),
"int",		'I',	0,	1,	0,	0,	sizeof(int),
"long",		'L',	0,	0,	0,	"l",	sizeof(long),
"long long",	'D',	0,	0,	0,
#ifdef int_8
						"ll",	sizeof(int_8),
#else
						0,	sizeof(long),
#endif
	0
};

static const Size_t	fsize[] =
{
"float",	'F',	'e',	0,	8,	0,	sizeof(float),
"double",	'D',	'e',	1,	16,	0,	sizeof(double),
"long double",	'L',	'e',	0,
#if _typ_long_double
					34,	"L",	sizeof(long double),
#else
					16,	0,	sizeof(double),
#endif
0
};

static char*
aform(int c)
{
	static const char	anames[] = "nulsohstxetxeoteqnackbel bs ht nl vt ff cr so sidledc1dc2dc3dc4naksynetbcan emsubesc fs gs rs us sp";
	static char		buf[4];

	if ((c &= 0177) == 0177)
		return "del";
	if (c <= 040)
		sfsprintf(buf, sizeof(buf), "%3.3s", anames + 3 * c);
	else
	{
		buf[0] = c;
		buf[1] = 0;
	}
	return buf;
}

static char*
bform(int c)
{
	static char	buf[9];

	sfsprintf(buf, sizeof(buf), "%08..2u", c);
	return buf;
}

static char*
cform(int c)
{
	register char*	s;

	static char	buf[2];

	switch (buf[0] = c)
	{
	case 0:
		return "\\0";
	case '\\':
		return "\\";
	}
	s = fmtesc(buf);
	if (*s == '\\' && !isalpha(*(s + 1)))
		s++;
	return s;
}

static char*
mform(int c)
{
	register char*	s;

	static char	buf[2];

	switch (buf[0] = ccmapchr(state.map, c))
	{
	case 0:
		return "00";
	case '\\':
		return "\\";
	}
	s = fmtesc(buf);
	if (*s == '\\' && isdigit(*(s + 1)))
		sfsprintf(s, 4, "%02lx", strtol(s + 1, NiL, 8));
	return s;
}

static const Type_t	type[] =
{
{
	"named character (ASCII 3-characters)",
	'a',	0,	aform,	csize,	 3,  0,  0,  0,  0
},
{
	"binary character",
	'b',	0,	bform,	csize,	 8, 0,  0,  0,  0
},
{
	"ASCII character or backslash escape",
	'c',	0,	cform,	csize,	 3,  0,  0,  0,  0
},
{
	"signed decimal",
	'd',	0,	0,	isize,	 4,  6, 11, 21, 31
},
{
	"floating point",
	'f',	0,	0,	fsize,	 9,  9, 15, 24, 42
},
{
	"\b--map\b mapped character or hexadecimal value if not printable",
	'm',	0,	mform,	csize,	 2,  0,  0,  0,  0
},
{
	"octal",
	'o',	"0",	0,	isize,	 3,  6, 11, 22, 33
},
{
	"unsigned decimal",
	'u',	0,	0,	isize,	 3,  5, 10, 20, 30
},
{
	"hexadecimal",
	'x',	"0",	0,	isize,	 2,  4,  8, 16, 32
},
{
	"printable bytes",
	'z',	0,	0,	0,	 1,  0,  0, 0, 0
},
};

/*
 * add format type t to the format list
 */

static void
format(register char* t)
{
	register int		c;
	register const Type_t*	tp;
	register const Size_t*	zp;
	register Format_t*	fp;
	const Size_t*		xp;
	int			n;
	int			i;
	char*			e;
	char*			s;

	while (c = *t++)
	{
		if (isspace(c) || c == ',')
			continue;
		tp = type;
		while (c != tp->name)
			if (++tp >= &type[elementsof(type)])
			{
				error(3, "%c: invalid type name", c);
				return;
			}
		if (!(zp = tp->size) && !tp->fun)
		{
			switch (tp->width[0])
			{
			case 1:
				state.printable = 1;
				break;
			}
			continue;
		}
		xp = 0;
		if (isdigit(*t))
		{
			c = 0;
			n = (int)strton(t, &e, NiL, 1);
			t = e;
		}
		else
		{
			c = isupper(*t) ? *t++ : 0;
			n = 0;
		}
		for (;;)
		{
			if (!zp->name)
			{
				if (c)
					error(3, "%c: invalid size for type %c", c, tp->name);
				else if (!(zp = xp) || tp->size == fsize)
					error(3, "%d: invalid size for type %c", n, tp->name);
				break;
			}
			if (n)
			{
				if (n == zp->size)
					break;
				zp++;
				if (n > (zp-1)->size && n < zp->size)
					xp = zp;
			}
			else if (c == zp->name)
			{
				if (c != *t)
					break;
				t++;
				zp++;
				if (zp->name)
					break;
			}
			else if (!c && !n && zp->dflt)
				break;
			else zp++;
		}
		i = zp - tp->size;
		if (!(fp = newof(0, Format_t, 1, 0)))
		{
			error(ERROR_system(1), "out of space [format]");
			return;
		}
		if (state.last) state.last = state.last->next = fp;
		else state.form = state.last = fp;
		fp->type = tp;
		fp->fp = tp->size == fsize;
		fp->us = tp->name != 'd';
		fp->size.internal = zp->size;
		fp->size.external = n ? n : zp->size;
		if (fp->size.external > state.size)
			state.size = fp->size.external;
		fp->width = tp->width[i = WIDTHINDEX(fp->size.internal)];
		if (n > 1 && (n & (n - 1)))
		{
			c = (1 << i) - n;
			n = (1 << i) - (1 << (i - 1));
			fp->width -= ((fp->width - tp->width[i - 1] + n - 1) / n) * c;
		}
		if (!(fp->fun = tp->fun))
		{
			e = fp->form;
			*e++ = '%';
			if (s = (char*)tp->fill)
				while (*e = *s++)
					e++;
			e += sfsprintf(e, sizeof(fp->form) - (e - fp->form), "%d", fp->width);
			if (c = zp->prec)
				e += sfsprintf(e, sizeof(fp->form) - (e - fp->form), ".%d", c);
			if (s = (char*)zp->qual)
				while (*e = *s++)
					e++;
			*e = zp->map ? zp->map : tp->name;
		}
	}
}

static Sfio_t*
init(char*** p)
{
	Sfio_t*	ip;
	int_max	offset;

	for (;;)
	{
		if (**p)
			state.file = *((*p)++);
		else if (state.file)
			return 0;
		if (!state.file || streq(state.file, "-"))
		{
			state.file = "/dev/stdin";
			ip = sfstdin;
		}
		else if (!(ip = sfopen(NiL, state.file, "r")))
		{
			error(ERROR_system(0), "%s: cannot open", state.file);
			error_info.errors = 1;
			continue;
		}
		if (state.buffer.noshare)
			sfset(ip, SF_SHARE, 0);
		if (state.buffer.size)
			sfsetbuf(ip, state.buffer.base, state.buffer.size);
		if (state.skip)
		{
			if ((offset = sfseek(ip, (off_t)0, SEEK_END)) > 0)
			{
				if (offset <= state.skip)
				{
					state.skip -= offset;
					state.offset += offset;
					goto next;
				}
				if (sfseek(ip, state.skip, SEEK_SET) != state.skip)
				{
					error(ERROR_system(2), "%s: seek error", state.file);
					goto next;
				}
				state.offset += state.skip;
				state.skip = 0;
			}
			else
			{
				for (;;)
				{
					if (!(state.peek.data = sfreserve(ip, SF_UNBOUND, 0)))
					{
						if (sfvalue(ip))
							error(ERROR_system(2), "%s: read error", state.file);
						goto next;
					}
					state.peek.size = sfvalue(ip);
					if (state.peek.size < state.skip)
						state.skip -= state.peek.size;
					else
					{
						state.peek.data += state.skip;
						state.peek.size -= state.skip;
						state.skip = 0;
						break;
					}
				}
			}
		}
		return ip;
	next:
		if (ip != sfstdin)
			sfclose(ip);
	}
}

static int
block(Sfio_t* op, char* bp, char* ep, int_max base)
{
	register Format_t*	fp;
	register unsigned char*	u;
	register char*		f;
	unsigned long		n;
	int_max			x;

	static char		buf[256];
	static union
	{
	char			m_char[sizeof(int_max) + sizeof(double_max)];
	float			m_float;
	double			m_double;
#if _typ_long_double
	long double		m_long_double;
#endif
	}			mem;

	if (!state.verbose)
	{
		if (state.dup.size == (n = ep - bp) && !memcmp(state.dup.data, bp, n))
		{
			if (!state.dup.mark)
			{
				state.dup.mark = 1;
				sfprintf(op, "*\n");
			}
			return 0;
		}
		state.dup.mark = 0;
	}
	for (fp = state.form; fp; fp = fp->next)
	{
		if (*state.base)
		{
			if (fp == state.form)
				sfprintf(op, state.base, base);
			else
				sfprintf(op, "%-*.*s ", BASE_WIDTH, BASE_WIDTH, "");
		}
		u = (unsigned char*)bp;
		for (;;)
		{
			if (fp->fun)
				f = (*fp->fun)(*u);
			else
			{
				f = buf;
				if (fp->fp)
				{
					swapmem(state.swap ^ int_swap, u, mem.m_char, fp->size.internal);
#if _typ_long_double
					if (fp->size.internal == sizeof(long double))
						sfsprintf(f, sizeof(buf), fp->form, mem.m_long_double);
					else
#endif
					if (fp->size.internal == sizeof(double))
						sfsprintf(f, sizeof(buf), fp->form, mem.m_double);
					else
						sfsprintf(f, sizeof(buf), fp->form, mem.m_float);
				}
				else
				{
					x = swapget(state.swap, u, fp->size.external);
					if (fp->us)
						switch (fp->size.internal)
						{
						case 1:
							sfsprintf(f, sizeof(buf), fp->form, (unsigned int_1)x);
							break;
						case 2:
							sfsprintf(f, sizeof(buf), fp->form, (unsigned int_2)x);
							break;
						case 4:
							sfsprintf(f, sizeof(buf), fp->form, (unsigned int_4)x);
							break;
						default:
							sfsprintf(f, sizeof(buf), fp->form, (unsigned int_max)x);
							break;
						}
					else
						switch (fp->size.internal)
						{
						case 1:
							sfsprintf(f, sizeof(buf), fp->form, (int_1)x);
							break;
						case 2:
							sfsprintf(f, sizeof(buf), fp->form, (int_2)x);
							break;
						case 4:
							sfsprintf(f, sizeof(buf), fp->form, (int_4)x);
							break;
						default:
							sfsprintf(f, sizeof(buf), fp->form, (int_max)x);
							break;
						}
				}
			}
			sfprintf(op, "%*s", state.width * fp->size.external / state.size, f);
			if ((u += fp->size.external) < (unsigned char*)ep)
				sfputc(op, ' ');
			else
			{
				if (state.printable && fp == state.form)
				{
					register int	c;

					if (c = (state.block - (ep - bp)) / state.size * (state.width + 1))
						sfprintf(op, "%*s", c, "");
					sfputc(op, ' ');
					for (u = (unsigned char*)bp; u < (unsigned char*)ep;)
					{
						if ((c = ccmapchr(state.map, *u++)) < 040 || c >= 0177)
							c = '.';
						sfputc(op, c);
					}
				}
				sfputc(op, '\n');
				break;
			}
			if (sferror(op))
				return -1;
		}
		if (sferror(op))
			return -1;
	}
	return 0;
}

static int
od(char** files)
{
	register char*	s;
	register char*	e;
	register char*	x;
	register char*	split = 0;
	register int	c;
	Sfio_t*		ip;
	unsigned long	n;
	unsigned long	m;
	unsigned long	r;

	static char*	buf;
	size_t		bufsize;

	if (!(ip = init(&files)))
		return 0;
	for (;;)
	{
		if (s = state.peek.data)
		{
			state.peek.data = 0;
			n = state.peek.size;
		}
		else for (;;)
		{
			s = sfreserve(ip, SF_UNBOUND, 0);
			n = sfvalue(ip);
			if (s)
				break;
			if (n)
				error(ERROR_system(2), "%s: read error", state.file);
			if (ip != sfstdin)
				sfclose(ip);
			if (!(ip = init(&files)))
			{
				s = 0;
				n = 0;
				break;
			}
		}
		if (state.count)
		{
			if (state.total >= state.count)
			{
				s = 0;
				n = 0;
			}
			else if ((state.total += n) > state.count)
				n -= state.total - state.count;
		}
		if (split)
		{
			if (s)
			{
				m = state.block - (split - buf);
				r = (m > n) ? m : n;
				if (bufsize < (r += (split - buf)))
				{
					bufsize = roundof(r, 1024);
					r = split - buf;
					if (!(buf = newof(buf, char, bufsize, 0)))
						error(ERROR_SYSTEM|3, "out of space");
					split = buf + r;
				}
				if (m > n)
				{
					memcpy(split, s, n);
					split += n;
					continue;
				}
				else
				{
					memcpy(split, s, m);
					split += m;
					s += m;
					n -= m;
				}
			}
			r = split - buf;
			if (m = (split - buf) % state.size)
			{
				m = state.size - m;
				while (m--)
					*split++ = 0;
			}
			if (block(sfstdout, buf, buf + r, state.offset))
				goto bad;
			split = 0;
			if (!state.verbose)
				memcpy(state.dup.data = state.dup.buf, buf, state.dup.size = r);
			state.offset += r;
			if (s && !n)
				continue;
		}
		if (!s)
			break;
		x = s + n;
		if (state.strings)
		{
			state.offset += n;
			n = 0;
			for (;;)
			{
				if (s >= x || (c = *s++) == 0 || c == '\n' || !isprint(c))
				{
					if (n >= state.strings)
					{
						if (*state.base && sfprintf(sfstdout, state.base, state.offset - (x - s) - n - 1) < 0)
							break;
						if (sfprintf(sfstdout, "%.*s\n", n, s - n - 1) < 0)
							break;
					}
					if (s >= x)
						break;
					n = 0;
				}
				else
					n++;
			}
		}
		else
		{
			e = s + (n / state.block) * state.block;
			if (s < e)
			{
				do
				{
					if (block(sfstdout, s, s + state.block, state.offset))
						goto bad;
					state.dup.data = s;
					state.dup.size = state.block;
					state.offset += state.block;
				} while ((s += state.block) < e);
				if (!state.verbose)
				{
					memcpy(state.dup.buf, state.dup.data, state.dup.size = state.block);
					state.dup.data = state.dup.buf;
				}
			}
			if (n = x - s)
			{
				if (bufsize < 2 * n)
				{
					if ((bufsize = 2 * n) < LINE_LENGTH * sizeof(int_max))
						bufsize = LINE_LENGTH * sizeof(int_max);
					bufsize = roundof(bufsize, 1024);
					if (!(buf = newof(buf, char, bufsize, 0)))
						error(ERROR_SYSTEM|3, "out of space");
				}
				memcpy(buf, s, n);
				split = buf + n;
			}
		}
	}
	return 0;
 bad:
	if (ip != sfstdin)
		sfclose(ip);
	return -1;
}

/*
 * optinfo() size description
 */

static int
optsize(Sfio_t* sp, const Size_t* zp)
{
	register int	n;

	for (n = 0; zp->name; zp++)
		if (zp->qual && (*zp->qual != zp->name || *(zp->qual + 1)))
			n += sfprintf(sp, "[%c|%s?sizeof(%s)]", zp->name, zp->qual, zp->desc);
		else
			n += sfprintf(sp, "[%c?sizeof(%s)]", zp->name, zp->desc);
	return n;
}

/*
 * optget() info discipline function
 */

static int
optinfo(Opt_t* op, Sfio_t* sp, const char* s, Optdisc_t* dp)
{
	register iconv_list_t*	ic;
	register int		i;
	register int		n;

	n = 0;
	switch (*s)
	{
	case 'c':
		for (ic = iconv_list(NiL); ic; ic = iconv_list(ic))
			if (ic->ccode >= 0)
				n += sfprintf(sp, "[%c:%s?%s]", ic->match[ic->match[0] == '('], ic->name, ic->desc);
		break;
	case 't':
		for (i = 0; i < elementsof(type); i++)
			n += sfprintf(sp, "[%c?%s]", type[i].name, type[i].desc);
		n += sfprintf(sp, "[+----- sizes -----?]");
		n += optsize(sp, isize);
		n += optsize(sp, fsize);
		break;
	}
	return n;
}

int
b_od(int argc, char** argv, void* context)
{
	register int		n;
	register char*		s;
	register Format_t*	fp;
	char*			e;
	int			per;
	char			buf[4];
	Optdisc_t		optdisc;

	NoP(argc);
	cmdinit(argv, context, ERROR_CATALOG, 0);
	optinit(&optdisc, optinfo);
	memset(&state, 0, sizeof(state));
	per = 0;
	state.swap = int_swap;
	state.map = ccmap(CC_ASCII, CC_ASCII);
	for (;;)
	{
		switch (optget(argv, usage))
		{

		case 'A':
			*state.base = *opt_info.arg;
			state.style |= NEW;
			continue;
		case 'B':
			if (opt_info.num <= 0)
				state.swap = -opt_info.num;
			else
				state.swap ^= opt_info.num;
			continue;
		case 'j':
			if (*opt_info.option == '+')
			{
				opt_info.index--;
				break;
			}
			state.skip = opt_info.num;
			state.style |= NEW;
			continue;
		case 'm':
			if ((n = ccmapid(opt_info.arg)) < 0)
				error(3, "%s: unknown character code set", opt_info.arg);
			state.map = ccmap(n, CC_NATIVE);
			continue;
		case 'N':
			state.count = opt_info.num;
			state.style |= NEW;
			continue;
		case 'p':
			state.printable = 1;
			continue;
		case 't':
			format(opt_info.arg);
			state.style |= NEW;
			continue;
		case 'T':
			s = opt_info.arg;
			switch (*s)
			{
			case 'b':
			case 'm':
				n = *s++;
				state.buffer.size = strton(s, &e, NiL, 1);
				if (n == 'b' && !(state.buffer.base = newof(0, char, state.buffer.size, 0)))
					error(ERROR_SYSTEM|3, "out of space [test buffer]");
				if (*e)
					error(3, "%s: invalid characters after test", e);
				break;
			case 'n':
				state.buffer.noshare = 1;
				break;
			default:
				error(3, "%s: unknown test", s);
				break;
			}
			continue;
		case 'v':
			state.verbose = 1;
			state.style |= NEW;
			continue;
		case 'w':
			per = opt_info.num;
			continue;
		case 'z':
			state.strings = opt_info.num;
			continue;

		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'D':
		case 'f':
		case 'F':
		case 'h':
		case 'i':
		case 'l':
		case 'o':
		case 'O':
		case 's':
		case 'S':
		case 'u':
		case 'U':
		case 'x':
		case 'X':
			switch (n = opt_info.option[1])
			{
			case 'D':
				n = 'U';
				break;
			case 'd':
				n = 'u';
				break;
			case 'h':
				n = 'x';
				break;
			case 'i':
			case 's':
				n = 'd';
				break;
			case 'l':
			case 'S':
				n = 'D';
				break;
			}
			s = buf;
			if (isupper(n)) switch (*s++ = tolower(n))
			{
			case 'f':
				*s++ = 'D';
				break;
			default:
				*s++ = 'L';
				break;
			}
			else switch (*s++ = n)
			{
			case 'b':
				*buf = 'o';
				*s++ = 'C';
				break;
			case 'd':
			case 'o':
			case 's':
			case 'u':
			case 'x':
				/* pronounce that! */
				*s++ = 'S';
				break;
			case 'f':
				*s++ = 'F';
				break;
			}
			*s = 0;
			format(buf);
			state.style |= OLD;
			continue;

		case ':':
			error(2, "%s", opt_info.arg);
			continue;
		case '?':
			error(ERROR_usage(2), "%s", opt_info.arg);
			continue;

		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors)
		error(ERROR_usage(2), "%s", optusage(NiL));
	switch (n = *state.base)
	{
	case 'n':
		*state.base = 0;
		break;
	case 0:
		n = 'o';
		/*FALLTHROUGH*/
	case 'o':
		state.addr = 8;
		goto base;
	case 'd':
		state.addr = 10;
		goto base;
	case 'x':
		state.addr = 16;
		/*FALLTHROUGH*/
	base:
		sfsprintf(state.base, sizeof(state.base), "%%0%d%s%c ", BASE_WIDTH, QUAL, n);
		break;
	default:
		error(3, "%c: invalid addr-base type", n);
		break;
	}
	if (!state.form)
		format("o2");
	else if (state.strings)
		error(3, "--strings must be the only format type");
	for (fp = state.form; fp; fp = fp->next)
		if ((n = (state.size / fp->size.external) * (fp->width + 1) - 1) > state.width)
			state.width = n;
	n = LINE_LENGTH - state.printable;
	if (*state.base)
		n -= (BASE_WIDTH + 1);
	if (!(n /= (state.width + state.printable * state.size + 1)))
		n = 1;
	if (state.addr)
	{
		while (n > state.addr)
			state.addr *= 2;
		while (n < state.addr)
			state.addr /= 2;
		if (state.addr)
			n = state.addr;
	}
	if (per)
		n = per;
	state.block = n * state.size;
	if (!(state.style & NEW) && (s = *argv))
	{
		e = "?(+)+([0-9])?(.)?([bkm])?(ll|LL)";
		if (!*(argv + 1))
		{
			if (strmatch(s, e) && *s == '+')
				argv++;
			else s = 0;
		}
		else if (!*(argv + 2))
		{
			s = *(argv + 1);
			if (strmatch(s, e) && (state.style == OLD || *s == '+'))
				*(argv + 1) = 0;
			else s = 0;
		}
		else s = 0;
		if (s)
		{
			state.skip = strtol(s, &e, strchr(s, '.') ? 10 : 8);
			if (*e == '.')
				e++;
			switch (*e)
			{
			case 'b':
				state.skip *= 512;
				break;
			case 'k':
				state.skip *= 1024;
				break;
			case 'm':
				state.skip *= 1024 * 1024;
				break;
			}
		}
	}
#ifdef SIGFPE
	signal(SIGFPE, SIG_IGN);
#endif
	od(argv);
#ifdef SIGFPE
	signal(SIGFPE, SIG_DFL);
#endif
	if (state.skip)
		error(3, "cannot skip past available data");
	if (*state.base && !state.strings)
	{
		*(state.base + strlen(state.base) - 1) = '\n';
		sfprintf(sfstdout, state.base, (int_max)state.offset);
	}
	if (sfsync(sfstdout) && errno != EPIPE)
		error(ERROR_SYSTEM|2, "write error");
	return error_info.errors;
}
