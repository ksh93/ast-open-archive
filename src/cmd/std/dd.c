/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1989-2001 AT&T Corp.                *
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
 * posix dd
 */

static const char usage1[] =
"[-1p0?\n@(#)$Id: dd (AT&T Labs Research) 2000-06-27 $\n]"
USAGE_LICENSE
"[+NAME?dd - convert and copy a file]"
"[+DESCRIPTION?\bdd\b copies an input file to an output file with optional"
"	conversions. The standard input and output are used by default."
"	Input and output block sizes can be specified to take advantage of"
"	physical io limitations. Options are of the form \aname=value\a"
"	and may be specified with 0, 1, or 2 leading `-' characters.]"
"[+?A \anumber\a argument may be a scaled number with optional trailing"
" multipliers of the form `* \anumber\a', `x \anumber\a' or `X \anumber\a'."
" Scale suffixes may be one of:]{"
"	[+b|B?512]"
"	[+c|C?1]"
"	[+g|G?1073741824]"
"	[+k|K?1024]"
"	[+l|L?4]"
"	[+m|M?1048576]"
"	[+q|Q?8]"
"	[+w|W?2]"
"}"
;

static const char usage2[] =
"[+SEE ALSO?\bcp\b(1), \biconv\b(1), \bpax\b(1), \btr\b(1), \bseek\b(2)]"
;

#include <ast.h>
#include <ctype.h>
#include <ccode.h>
#include <error.h>
#include <iconv.h>
#include <ls.h>
#include <sig.h>
#include <sfstr.h>
#include <swap.h>

#define CODE		0
#define CONV		1
#define FLAG		2
#define NUMBER		3
#define STRING		4

#define A2E		(1<<0)
#define A2I		(1<<1)
#define A2N		(1<<2)
#define A2O		(1<<3)
#define BLOCK		(1<<4)
#define E2A		(1<<5)
#define IGNERROR	(1<<6)
#define I2A		(1<<7)
#define ISPECIAL	(1<<8)
#define LCASE		(1<<9)
#define N2A		(1<<10)
#define NOERROR		(1<<11)
#define NOTRUNC		(1<<12)
#define O2A		(1<<13)
#define OSPECIAL	(1<<14)
#define SILENT		(1L<<15)
#define SWAP		(1L<<16)
#define SYNC		(1L<<17)
#define UCASE		(1L<<18)
#define UNBLOCK		(1L<<19)

#define BS		512

#define operand_begin	bs
#define operand_end	to

#define conv_begin	a2e
#define conv_end	unblock

typedef struct
{
	const char*	name;
	const char*	help;
} Desc_t;

typedef struct
{
	char*		string;
	Sflong_t	number;
} Value_t;

typedef struct
{
	const char*	name;
	long		type;
	const char*	help;
	Value_t		value;
} Operand_t;

typedef struct
{
	Sfio_t*		fp;
	Sfulong_t	complete;
	Sfulong_t	partial;
	Sfulong_t	truncated;
	Sfulong_t	remains;
	int		special;
} Io_t;

typedef struct
{
	Operand_t	bs;
	Operand_t	cbs;
	Operand_t	conv;
	Operand_t	count;
	Operand_t	from;
	Operand_t	ibs;
	Operand_t	ifn;
	Operand_t	iseek;
	Operand_t	obs;
	Operand_t	ofn;
	Operand_t	oseek;
	Operand_t	silent;
	Operand_t	skip;
	Operand_t	swap;
	Operand_t	to;

	Operand_t	a2e;
	Operand_t	a2i;
	Operand_t	a2n;
	Operand_t	a2o;
	Operand_t	ascii;
	Operand_t	block;
	Operand_t	e2a;
	Operand_t	ebcdic;
	Operand_t	i2a;
	Operand_t	ibm;
	Operand_t	ifix;
	Operand_t	ignerror;
	Operand_t	lcase;
	Operand_t	n2a;
	Operand_t	noerror;
	Operand_t	notrunc;
	Operand_t	o2a;
	Operand_t	ofix;
	Operand_t	swab;
	Operand_t	sync;
	Operand_t	ucase;
	Operand_t	unblock;

	Io_t		in;
	Io_t		out;
	char*		buffer;
	int		pad;
	iconv_t		cvt;
	Sfio_t*		tmp;
} State_t;

static State_t		state =
{
	{
		"bs",
		NUMBER,
		"Input and output block size.",
	},
	{
		"cbs",
		NUMBER,
		"Conversion buffer size (logical record length)."
	},
	{
		"conv",
		CONV,
		"Convert input.",
	},
	{
		"count",
		NUMBER,
		"Copy only \anumber\a input blocks.",
	},
	{
		"from",
		CODE,
		"Convert from \acharset\a to the \bto\b=\acharset\a"
		" or the local default.",
	},
	{
		"ibs",
		NUMBER,
		"Input block size.",
		0, BS,
	},
	{
		"if",
		STRING,
		"The input file name; standard input is the default.",
	},
	{
		"iseek",
		NUMBER,
		"Seek \anumber\a blocks from the beginning of the input"
		" file before copying.",
	},
	{
		"obs",
		NUMBER,
		"Output block size.",
		0, BS,
	},
	{
		"of",
		STRING,
		"The output file name; standard output is the default.",
	},
	{
		"oseek|seek",
		NUMBER,
		"Seek \anumber\a blocks from the beginning of the output"
		" file before copying.",
	},
	{
		"silent",
		FLAG,
		"\bsilent\b does not print the total number of io blocks"
		" on exit.",
	},
	{
		"skip",
		NUMBER,
		"Skip \anumber\a blocks before reading.  Seek is used if"
		" possible, otherwise the blocks are read and discarded.",
	},
	{
		"swap",
		NUMBER,
		"Swap bytes acording to the inclusive or of: 1-byte,"
		" 2-short, 4-long, 8-quad, etc.",
	},
	{
		"to",
		CODE,
		"Convert to \acharset\a from the \bfrom\b=\acharset\a"
		" or the local default. See \bfrom\b for \acharset\a names.",
	},

	{
		"a2e",
		A2E,
		"ascii to ebcdic",
	},
	{
		"a2i",
		A2I,
		"ascii to ibm",
	},
	{
		"a2n",
		A2N,
		"ascii to native",
	},
	{
		"a2o",
		A2O,
		"ascii to old ebcdic",
	},
	{
		"ascii",
		E2A,
		"ebcdic to ascii",
	},
	{
		"block",
		BLOCK,
		"newline-terminated ascii to fixed record length",
	},
	{
		"e2a",
		E2A,
		"ebcdic to ascii",
	},
	{
		"ebcdic",
		A2E,
		"equivalent to \ba2e\b",
	},
	{
		"i2a",
		I2A,
		"ibm to ascii",
	},
	{
		"ibm",
		A2I,
		"ascii to ibm ebcdic",
	},
	{
		"ignerror",
		IGNERROR,
		"continue processing after errors",
	},
	{
		"ispecial",
		ISPECIAL,
		"currently ignored",
	},
	{
		"lcase",
		LCASE,
		"to lower case",
	},
	{
		"n2a",
		N2A,
		"native to ascii",
	},
	{
		"noerror",
		NOERROR,
		"stop processing only after 5 consecutive errors",
	},
	{
		"notrunc",
		NOTRUNC,
		"do not truncate pre-existing output files",
	},
	{
		"o2a",
		O2A,
		"old ibm to ascii",
	},
	{
		"ospecial",
		OSPECIAL,
		"currently ignored"
	},
	{
		"swab",
		SWAP,
		"swap byte pairs",
	},
	{
		"sync",
		SYNC,
		"Pad each input block to \bibs\b. Pad with spaces if"
		" \bconv=block\b or \bconv=unblock\b, otherwise pad"
		" with nulls.",
	},
	{
		"ucase",
		UCASE,
		"to upper case",
	},
	{
		"unblock",
		UNBLOCK,
		"fixed-length records to newline-terminated records",
	},
};

static Desc_t		desc[] =
{
	"charset",	"",
	"conversion",	"",
	0,		0,
	"number",	0,
	"file",		0,
};

static void
fini(int code)
{
	if (state.in.fp != sfstdin)
		sfclose(state.in.fp);
	if (state.out.fp == sfstdout)
		sfsync(state.out.fp);
	else
		sfclose(state.out.fp);
	if (!state.silent.value.number)
	{
		sfprintf(sfstderr, "%lu+%lu records in\n", state.in.complete, state.in.partial);
		sfprintf(sfstderr, "%lu+%lu records out\n", state.out.complete, state.out.partial + (state.out.remains > 0));
		if (state.in.truncated)
			sfprintf(sfstderr, "%lu truncated record%s\n", state.in.truncated, state.in.truncated == 1 ? "" : "s");
	}
	exit(code);
}

static void
interrupt(int sig)
{
	signal(sig, SIG_DFL);
	fini(EXIT_TERM(sig));
}

static ssize_t
output(Sfio_t* sp, const Void_t* buf, size_t n, Sfdisc_t* disc)
{
	register ssize_t	r;
	register size_t		x;

	if ((r = sfwr(sp, buf, n, disc)) > 0)
	{
		state.out.complete += r / state.obs.value.number;
		if (x = r % state.obs.value.number)
		{
			if (state.out.special)
				state.out.partial++;
			else if ((state.out.remains += x) >= state.obs.value.number)
			{
				state.out.remains -= state.obs.value.number;
				state.out.complete++;
			}
		}
	}
	return r;
}

int
main(int argc, char** argv)
{
	register char*		s;
	register char*		v;
	register char*		b;
	register Operand_t*	op;
	register Operand_t*	vp;
	register int		f;
	char*			usage;
	char*			e;
	int			i;
	char*			cb;
	size_t			cc;
	size_t			ce;
	Sfio_t*			sp;
	Sflong_t		c;
	Sflong_t		m;
	Sflong_t		n;
	Sflong_t		z;
	struct stat		st;
	Sfdisc_t		disc;

	setlocale(LC_ALL, "");
	error_info.id = "dd";
	state.from.value.string = state.to.value.string = "";
	if (!(sp = sfstropen()))
		error(ERROR_SYSTEM|3, "out of space [usage]");
	sfputr(sp, usage1, '\n');
	for (op = &state.operand_begin; op <= &state.operand_end; op++)
	{
		sfprintf(sp, "[%d:%s?%s", op - &state.operand_begin + 10, op->name, op->help);
		i = ']';
		if (desc[op->type].name)
		{
			if (desc[op->type].help)
			{
				if (*desc[op->type].help)
					sfprintf(sp, " %s", desc[op->type].help);
				else
					sfprintf(sp, " \a%s\a may be one of:", desc[op->type].name);
				sfprintf(sp, "]:[%s", desc[op->type].name);
				desc[op->type].help = 0;
				if (op->type == CONV)
				{
					i = '}';
					sfprintf(sp, "]{\n");
					for (vp = &state.conv_begin; vp <= &state.conv_end; vp++)
						sfprintf(sp, "\t[+%s?%s]\n", vp->name, vp->help);
				}
				else if (op->type == CODE)
				{
					register iconv_list_t*	ic;

					sfputc(sp, ']');
					sfputc(sp, '{');
					for (ic = iconv_list(NiL); ic; ic = iconv_list(ic))
					{
						sfputc(sp, '[');
						sfputc(sp, '+');
						sfputc(sp, '\b');
						s = (char*)ic->match;
						if (*s == '(')
							s++;
						while (i = *s++)
						{
							if (i == ')' && !*s)
								break;
							if (i == '?' || i == ']')
								sfputc(sp, i);
							sfputc(sp, i);
						}
						sfputc(sp, '?');
						s = (char*)ic->desc;
						while (i = *s++)
						{
							if (i == ']')
								sfputc(sp, i);
							sfputc(sp, i);
						}
						sfputc(sp, ']');
						sfputc(sp, '\n');
					}
					i = '}';
				}
			}
			else
				sfprintf(sp, "]:[%s", desc[op->type].name);
		}
		sfputc(sp, i);
		sfputc(sp, '\n');
	}
	sfputr(sp, usage2, '\n');
	usage = sfstruse(sp);
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, interrupt);
	while (f = optget(argv, usage))
	{
		
		if (f > 0)
		{
			if (f == '?')
				error(ERROR_USAGE|4, "%s", opt_info.arg);
			if (f = ':')
				error(2, "%s", opt_info.arg);
			continue;
		}
		op = &state.operand_begin - (f + 10);
		v = opt_info.arg;
		switch (op->type)
		{
		case CODE:
		case STRING:
			op->value.string = v;
			break;
		case CONV:
			do
			{
				if (e = strchr(v, ','))
					*e = 0;
				vp = (Operand_t*)strsearch(&state.conv_begin, &state.conv_end - &state.conv_begin + 1, sizeof(Operand_t), stracmp, v, NiL);
				if (e)
					*e++ = ',';
				if (!vp)
					error(3, "%s: unknown %s value", v, op->name);
				op->value.number |= vp->type;
			} while (v = e);
			break;
		case FLAG:
			op->value.number = opt_info.num;
			break;
		case NUMBER:
			c = 0;
			for (;;)
			{
				n = strtonll(v, &e, NiL, 0);
				if (n < 0)
					error(3, "%s: %s must be >= 0", v, op->name);
				if (!c)
				{
					c = 1;
					op->value.number = n;
				}
				else
					op->value.number *= n;
				for (v = e; isspace(*v); v++);
				if (*v != 'x' && *v != 'X' && *v != '*')
					break;
				while (isspace(*++v));
				if (!*v)
				{
					v = e;
					break;
				}
			}
			if (*v)
				error(3, "%s: %s: invalid numeric expression", op->name, opt_info.arg);
			break;
		}
	}
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	error_info.exit = fini;
	switch ((long)(state.conv.value.number & (A2E|A2I|A2N|A2O|E2A|I2A|N2A|O2A)))
	{
	case 0:
		break;
	case A2E:
		state.from.value.string = "ascii";
		state.to.value.string = "ebcdic-1";
		break;
	case A2I:
		state.from.value.string = "ascii";
		state.to.value.string = "ebcdic-2";
		break;
	case A2N:
		state.from.value.string = "ascii";
		state.to.value.string = "";
		break;
	case A2O:
		state.from.value.string = "ascii";
		state.to.value.string = "ebcdic-3";
		break;
	case E2A:
		state.from.value.string = "ebcdic-1";
		state.to.value.string = "ascii";
		break;
	case I2A:
		state.from.value.string = "ebcdic-2";
		state.to.value.string = "ascii";
		break;
	case N2A:
		state.from.value.string = "";
		state.to.value.string = "ascii";
		break;
	case O2A:
		state.from.value.string = "ebcdic-3";
		state.to.value.string = "ascii";
		break;
	default:
		error(3, "only one of %s={%s,%s,%s} may be specified", state.conv.name, state.ascii.value.string, state.ebcdic.value.string, state.ibm.value.string);
	}
	if (streq(state.from.value.string, state.to.value.string))
		state.cvt = (iconv_t)(-1);
	else  if ((state.cvt = iconv_open(state.to.value.string, state.from.value.string)) != (iconv_t)(-1))
	{
		if (!(state.tmp = sfstropen()))
			error(ERROR_SYSTEM|3, "out of space [conversion stream]");
	}
	else if (*state.from.value.string && *state.to.value.string)
		error(3, "cannot convert from %s to %s", state.from.value.string, state.to.value.string);
	if ((state.conv.value.number & (BLOCK|UNBLOCK)) == (BLOCK|UNBLOCK))
		error(3, "only one of %s=%s and %s=%s may be specified", state.conv.name, state.block.value.string, state.conv.name, state.unblock.value.string);
	if ((state.conv.value.number & (SYNC|UNBLOCK)) == (SYNC|UNBLOCK))
	{
		state.conv.value.number &= ~SYNC;
		error(1, "%s=%s ignored for %s=%s", state.conv.name, state.sync.value.string, state.conv.name, state.unblock.value.string);
	}
	if (state.conv.value.number & SWAP)
		state.swap.value.number = 1;
	else if (state.swap.value.number)
		state.conv.value.number |= SWAP;
	if (state.oseek.value.number && (state.conv.value.number & NOTRUNC))
	{
		state.oseek.value.number = 0;
		error(1, "%s ignored for %s=%s", state.oseek.name, state.conv.name, state.notrunc.value.string);
	}
	if ((state.conv.value.number & (LCASE|UCASE)) == (LCASE|UCASE))
		error(3, "only one of %s=%s and %s=%s may be specified", state.conv.name, state.lcase.value.string, state.conv.name, state.ucase.value.string);
	if (state.conv.value.number & (BLOCK|UNBLOCK))
	{
		if (!state.cbs.value.number)
			error(3, "%s must be specified for %s=%s", state.cbs.name, state.conv.name, (state.conv.value.number & BLOCK) ? state.block.value.string : state.unblock.value.string);
		state.pad = ' ';
		if (state.conv.value.number & UNBLOCK)
		{
			state.ibs.value.number = state.cbs.value.number;
			if (state.bs.value.number)
			{
				state.obs.value.number = state.bs.value.number;
				state.bs.value.number = 0;
			}
		}
	}
	else if (state.cbs.value.number)
	{
		state.cbs.value.number = 0;
		error(1, "%s ignored", state.cbs.name);
	}
	if (n = state.bs.value.number)
		state.ibs.value.number = state.obs.value.number = n;
	if (n = state.iseek.value.number)
	{
		if (state.skip.value.number)
			error(3, "only on of %s and %s may be specified", state.skip.name, state.iseek.name);
		if (!state.ibs.value.number)
			error(3, "%s requires %s or %s", state.iseek.name, state.bs.name, state.ibs.name);
		state.skip.value.number = n;
	}
	if (state.oseek.value.number && !state.obs.value.number)
		error(3, "%s requires %s or %s", state.oseek.name, state.bs.name, state.obs.name);
	if (!(s = state.ifn.value.string))
	{
		state.ifn.value.string = "/dev/stdin";
		state.in.fp = sfstdin;
	}
	else if (!(state.in.fp = sfopen(NiL, s, "rb")))
		error(ERROR_SYSTEM|3, "%s: cannot read", s);
	if (!(s = state.ofn.value.string))
	{
		state.ofn.value.string = "/dev/stdout";
		state.out.fp = sfstdout;
	}
	else if (!(state.out.fp = sfopen(NiL, s, (state.conv.value.number & NOTRUNC) ? "a+b" : state.oseek.value.number ? "w+b" : "wb")))
		error(ERROR_SYSTEM|3, "%s: cannot write", s);
	if ((state.conv.value.number & ISPECIAL) || fstat(sffileno(state.in.fp), &st) || !S_ISREG(st.st_mode))
		state.in.special = 1;
	if (state.in.special && !(state.conv.value.number & BLOCK) && (n = state.ibs.value.number))
		sfsetbuf(state.in.fp, NiL, n);
	if ((state.conv.value.number & OSPECIAL) || fstat(sffileno(state.out.fp), &st) || !S_ISREG(st.st_mode))
		state.out.special = 1;
	if (state.out.special && !(state.conv.value.number & UNBLOCK) && (n = state.obs.value.number))
		sfsetbuf(state.out.fp, NiL, n);
	state.bs.value.number = state.ibs.value.number;
	if (state.obs.value.number > state.bs.value.number)
		state.bs.value.number = state.obs.value.number;
	if (state.cbs.value.number > state.bs.value.number)
		state.bs.value.number = state.cbs.value.number;
	if (!(state.buffer = newof(0, char, state.bs.value.number, 0)))
		error(ERROR_SYSTEM|3, "out of space [io buffer]");
	if (n = state.skip.value.number)
	{
		if (!state.ibs.value.number)
			error(3, "%s requires %s or %s", state.skip.name, state.bs.name, state.ibs.name);
		n *= state.ibs.value.number;
		if (sfseek(state.in.fp, n, SEEK_SET) != n)
		{
			do
			{
				if (!sfreserve(state.in.fp, state.in.special && state.ibs.value.number < n ? state.ibs.value.number : n, 0))
				{
					if (sfvalue(state.in.fp) > 0)
						error(ERROR_SYSTEM|3, "%s: seek read error", state.ifn.value.string);
					break;
				}
			} while ((n -= sfvalue(state.in.fp)) > 0);
			if (n > 0)
				error(3, "%s: cannot seek past end of file", state.ifn.value.string);
		}
	}
	if (n = state.oseek.value.number)
	{
		n *= state.obs.value.number;
		if (sfseek(state.out.fp, n, SEEK_SET) != n)
		{
			do
			{
				if (!sfreserve(state.out.fp, state.out.special && state.obs.value.number < n ? state.obs.value.number : n, 0))
				{
					if (sfvalue(state.out.fp) > 0)
						error(ERROR_SYSTEM|3, "%s: seek read error", state.ofn.value.string);
					break;
				}
			} while ((n -= sfvalue(state.out.fp)) > 0);
			while (n > 0 && (c = sfwrite(state.out.fp, state.buffer, state.out.special && state.obs.value.number < n ? state.obs.value.number : n)) > 0)
				n -= c;
			if (c < 0)
				error(ERROR_SYSTEM|3, "%s: seek 0 fill write error", state.ofn.value.string);
		}
	}
	memset(&disc, 0, sizeof(disc));
	disc.writef = output;
	sfdisc(state.out.fp, &disc);
	if (!state.obs.value.number)
		state.obs.value.number = BS;
	if (state.conv.value.number & BLOCK)
	{
		c = state.cbs.value.number;
		memset(state.buffer, state.pad, c);
		while ((s = sfgetr(state.in.fp, '\n', 0)) || (s = sfgetr(state.in.fp, '\n', -1)))
		{
			state.in.complete++;
			n = sfvalue(state.in.fp) - 1;
			if (n > c)
			{
				if (sfwrite(state.out.fp, s, c) != c)
					error(ERROR_SYSTEM|3, "%s: write error", state.ofn.value.string);
				state.in.truncated++;
			}
			else
			{
				if (sfwrite(state.out.fp, s, n) != n)
					error(ERROR_SYSTEM|3, "%s: write error", state.ofn.value.string);
				n = c - n;
				if (sfwrite(state.out.fp, state.buffer, n) != n)
					error(ERROR_SYSTEM|3, "%s: write error", state.ofn.value.string);
			}
		}
	}
	else
	{
		if (!(c = state.ibs.value.number))
			c = BS;
		f = state.conv.value.number;
		if (!state.in.special)
			f &= ~NOERROR;
		if (state.count.value.number)
			z = state.count.value.number * c;
		else
			z = 0;
		for (;;)
		{
			b = sfreserve(state.in.fp, SF_UNBOUND, 0);
			m = sfvalue(state.in.fp);
			if (state.count.value.number)
			{
				if (!z)
					goto done;
				if (m > z)
				{
					m = z;
					z = 0;
				}
				else
					z -= m;
					
			}
			if (!b)
			{
				if (!m)
					break;
				error(ERROR_SYSTEM|((f & NOERROR) ? 2 : 3), "%s: read error", state.ifn.value.string);
				memset(b = state.buffer, state.pad, m = c);
			}
			for (;;)
			{
				s = b;
				if (m >= c)
				{
					n = c;
					state.in.complete++;
				}
				else
				{
					n = m;
					if (state.in.special)
						state.in.partial++;
					else if ((state.in.remains += n % c) >= c)
					{
						state.in.remains -= c;
						state.in.complete++;
					}
					if (f & SYNC)
					{
						s = memcpy(state.buffer, s, n);
						memset(s + n, state.pad, c - n);
						n = c;
					}
				}
				if (f & SWAP)
					swapmem(state.swap.value.number, s, s, n);
				if (state.cvt != (iconv_t)(-1))
				{
					cb = s;
					cc = n;
					ce = 0;
					n = iconv_write(state.cvt, state.tmp, &cb, &cc, &ce);
					s = sfstruse(state.tmp);
					if (ce && !state.silent.value.number)
					{
						if (ce == 1)
							error(ERROR_SYSTEM|2, "%s: %d conversion error", state.ofn.value.string, ce);
						else
							error(ERROR_SYSTEM|2, "%s: %d conversion errors", state.ofn.value.string, ce);
					}
				}
				ccmaps(s, n, state.from.value.number, state.to.value.number);
				switch (f & (LCASE|UCASE))
				{
				case LCASE:
					for (v = (e = s) + n; s < v; s++)
						if (isupper(*s))
							*s = tolower(*s);
					s = e;
					break;
				case UCASE:
					for (v = (e = s) + n; s < v; s++)
						if (islower(*s))
							*s = toupper(*s);
					s = e;
					break;
				}
				if (f & UNBLOCK)
				{
					for (v = s + n; v > s && *(v - 1) == ' '; v--);
					if (sfwrite(state.out.fp, s, v - s) != (v - s) || sfputc(state.out.fp, '\n') != '\n')
						error(ERROR_SYSTEM|3, "%s: write error", state.ofn.value.string);
				}
				else if (sfwrite(state.out.fp, s, n) != n)
					error(ERROR_SYSTEM|3, "%s: write error", state.ofn.value.string);
				if ((m -= n) <= 0)
					break;
				b += n;
			}
		}
	done:
		if (sfsync(state.out.fp))
			error(ERROR_SYSTEM|3, "%s: write error", state.ofn.value.string);
	}
	fini(error_info.errors != 0);
}
