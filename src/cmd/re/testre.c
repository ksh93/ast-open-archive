/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1995-2001 AT&T Corp.                *
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
 * regex(3) test harness
 * see help() for details
 */

static const char id[] = "\n@(#)$Id: testre (AT&T Research) 2001-06-19 $\0\n";

#if _PACKAGE_ast
#include <ast.h>
#else
#define fmtident(s)	((char*)(s)+10)
#endif

#include <stdio.h>
#include <regex.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <locale.h>

#ifdef	__STDC__
#include <stdlib.h>
#endif

#if !_PACKAGE_ast
#undef	REG_DISCIPLINE
#endif

#if !REG_DELIMITED
#undef	_REG_sub
#endif

#if REG_DISCIPLINE || _REG_sub

#include <sfstr.h>

#if REG_DISCIPLINE

#include <stk.h>

typedef struct Disc_s
{
	regdisc_t	disc;
	int		ordinal;
	Sfio_t*		sp;
} Disc_t;

static void*
compf(const regex_t* re, const char* xstr, size_t xlen, regdisc_t* disc)
{
	Disc_t*		dp = (Disc_t*)disc;

	return (void*)++dp->ordinal;
}

static int
execf(const regex_t* re, void* data, const char* xstr, size_t xlen, const char* sstr, size_t slen, char** snxt, regdisc_t* disc)
{
	Disc_t*		dp = (Disc_t*)disc;

	sfprintf(dp->sp, "{%-.*s}(%d:%d)", xlen, xstr, (int)data, slen);
	return atoi(xstr);
}

static void*
resizef(void* handle, void* data, size_t size)
{
	if (!size)
		return 0;
	return stkalloc((Sfio_t*)handle, size);
}

#endif

#endif

#ifndef NiL
#ifdef	__STDC__
#define NiL		0
#else
#define NiL		(char*)0
#endif
#endif

#define H(x)		fprintf(stderr,x)

static void
help(void)
{
H("NAME\n");
H("  testre - regex(3) test harness\n");
H("\n");
H("SYNOPSIS\n");
H("  testre [ options ] < testre.dat\n");
H("\n");
H("DESCRIPTION\n");
H("  testre reads regex(3) test specifications, one per line, from the\n");
H("  standard input and writes one output line for each failed test. A\n");
H("  summary line is written after all tests are done. Each successful\n");
H("  test is run again with REG_NOSUB. Unsupported features are noted\n");
H("  before the first test, and tests requiring these features are\n");
H("  silently ignored.\n");
H("\n");
H("OPTIONS\n");
H("  -c	catch signals and non-terminating calls\n");
H("  -e	ignore error return mismatches\n");
H("  -h	list help\n");
H("  -p	ignore negative position mismatches\n");
H("  -s	use stack instead of malloc\n");
H("  -v	list each test line\n");
H("\n");
H("INPUT FORMAT\n");
H("  Input lines may be blank, a comment beginning with #, or a test\n");
H("  specification. A specification is five fields separated by one\n");
H("  or more tabs. NULL denotes the empty string and NIL denotes the\n");
H("  0 pointer.\n");
H("\n");
H("  Field 1: the regex(3) flags to apply, one character per REG_feature\n");
H("  flag. The test is skipped if REG_feature is not supported by the\n");
H("  implementation. If the first character is not [BEASKL] then the\n");
H("  specification is a global control line.\n");
H("\n");
H("    B 	basic			BRE	(grep, ed, sed)\n");
H("    E 	REG_EXTENDED		ERE	(egrep)\n");
H("    A	REG_AUGMENTED		ARE	(egrep with negation)\n");
H("    S	REG_SHELL		SRE	(sh glob)\n");
H("    K	REG_SHELL|REG_AUGMENTED	KRE	(ksh glob)\n");
H("    L	REG_LITERAL		LRE	(fgrep)\n");
H("\n");
H("    a	REG_LEFT|REG_RIGHT	implicit ^...$\n");
H("    b	REG_NOTBOL		lhs does not match ^\n");
H("    c	REG_COMMENT		ignore space and #...\\n\n");
H("    d	REG_SHELL_DOT		explicit leading . match\n");
H("    e	REG_NOTEOL		rhs does not match $\n");
H("    f	REG_MULTIPLE		multiple \\n separated patterns\n");
H("    g	FNM_LEADING_DIR		testfnmatch only -- match until /\n");
H("    h	REG_MULTIREF		multiple digit backref\n");
H("    i	REG_ICASE		ignore case\n");
H("    j	REG_SPAN		. matches \\n\n");
H("    k	REG_ESCAPE		\\ to ecape [...] delimiter\n");
H("    l	REG_LEFT		implicit ^...\n");
H("    m	REG_MINIMAL		minimal match\n");
H("    n	REG_NEWLINE		explicit \\n match\n");
H("    p	REG_SHELL_PATH		explicit / match\n");
H("    r	REG_RIGHT		implicit ...$\n");
H("    s	REG_SHELL_ESCAPED	\\ not special\n");
H("    u	standard unspecified behavior -- errors not counted\n");
H("    x	REG_LENIENT		let some errors slide\n");
H("    z	REG_NULL		NULL subexpressions ok\n");
H("    $	                        expand C \\c escapes in fields 2 and 3\n");
H("    /	                        field 2 is a regsub() expression\n");
H("\n");
H("  Field 1 control lines:\n");
H("\n");
H("    C	set LC_COLLATE and LC_CTYPE to locale in field 2\n");
H("\n");
H("    {				silent skip if failed until }\n");
H("    }				end of skip\n");
H("\n");
H("    : comment			comment copied to output\n");
H("\n");
H("    number			use number for nmatch (20 by default)\n");
H("\n");
H("  Field 2: the regular expression pattern; SAME uses the pattern from\n");
H("    the previous specifications.\n");
H("\n");
H("  Field 3: the string to match.\n");
H("\n");
H("  Field 4: the test outcome. This is either one of the posix error\n");
H("    codes (with REG_ omitted) or the match array, a list of (m,n)\n");
H("    entries with m and n being first and last+1 positions in the\n");
H("    field 3 string, or NULL if REG_NOSUB is in effect and success\n");
H("    is expected. BADPAT is acceptable in place of any regcomp(3)\n");
H("    error code. The match[] array is initialized to (-2,-2) before\n");
H("    each test. All array elements not equal to (-2,-2) must be specified\n");
H("    in the outcome. Unspecified endpoints are denoted by -1. {x}(o:n)\n");
H("    denotes a matched (?{...}) expression, where x is the text enclosed\n");
H("    by {...}, o is the expression ordinal counting from 1, and n is the\n");
H("    length of the unmatched portion of the subject string. If x starts\n");
H("    with a number then that is the return value of re_execf(), otherwise\n");
H("    0 is returned.\n");
H("\n");
H("  Field 5: optional comment appended to the report.\n");
H("\n");
H("CONTRIBUTORS\n");
H("  Glenn Fowler <gsf@research.att.com> (ksh strmatch, regex extensions)\n");
H("  David Korn <dgk@research.att.com> (ksh glob matcher)\n");
H("  Doug McIlroy <doug@research.bell-labs.com> (ast regex/testre in C++)\n");
H("  Tom Lord <lord@cygnus.com> (rx tests)\n");
H("  Henry Spencer <henry@zoo.toronto.edu> (original public regex)\n");
H("  Andrew Hume <andrew@research.att.com> (gre tests)\n");
H("  John Maddock <John_Maddock@compuserve.com> (regex++ tests)\n");
H("  Philip Hazel <ph10@cam.ac.uk> (pcre tests)\n");
}

#ifndef elementsof
#define elementsof(x)	(sizeof(x)/sizeof(x[0]))
#endif

#ifndef streq
#define streq(a,b)	(*(a)==*(b)&&!strcmp(a,b))
#endif

#define LOOPED		2
#define NOTEST		(~0)

#ifndef REG_TEST_DEFAULT
#define REG_TEST_DEFAULT	0
#endif

#ifndef REG_EXEC_DEFAULT
#define REG_EXEC_DEFAULT	0
#endif

static const char* unsupported[] =
{

#if REG_TEST_DEFAULT & (REG_AUGMENTED|REG_EXTENDED|REG_SHELL)
	"BASIC",
#endif
#ifndef REG_EXTENDED
	"EXTENDED",
#endif
#ifndef REG_AUGMENTED
	"AUGMENTED",
#endif
#ifndef REG_SHELL
	"SHELL",
#endif

#ifndef REG_COMMENT
	"COMMENT",
#endif
#ifndef REG_DELIMITED
	"DELIMITED",
#endif
#ifndef REG_DISCIPLINE
	"DISCIPLINE",
#endif
#ifndef REG_ESCAPE
	"ESCAPE",
#endif
#ifndef REG_ICASE
	"ICASE",
#endif
#ifndef REG_LEFT
	"LEFT",
#endif
#ifndef REG_LENIENT
	"LENIENT",
#endif
#ifndef REG_LITERAL
	"LITERAL",
#endif
#ifndef REG_MINIMAL
	"MINIMAL",
#endif
#ifndef REG_MULTIPLE
	"MULTIPLE",
#endif
#ifndef REG_MULTIREF
	"MULTIREF",
#endif
#ifndef REG_NEWLINE
	"NEWLINE",
#endif
#ifndef REG_NOTBOL
	"NOTBOL",
#endif
#ifndef REG_NOTEOL
	"NOTEOL",
#endif
#ifndef REG_NULL
	"NULL",
#endif
#ifndef REG_RIGHT
	"RIGHT",
#endif
#ifndef REG_SHELL_DOT
	"SHELL_DOT",
#endif
#ifndef REG_SHELL_ESCAPED
	"SHELL_ESCAPED",
#endif
#ifndef REG_SHELL_PATH
	"SHELL_PATH",
#endif
#ifndef REG_SPAN
	"SPAN",
#endif
#if REG_NOSUB & REG_TEST_DEFAULT
	"SUBMATCH",
#endif
#if !_REG_nexec
	"regnexec",
#endif
#if !_REG_sub
	"regsub",
#endif
	0
};

#ifndef REG_COMMENT
#define REG_COMMENT	NOTEST
#endif
#ifndef REG_ESCAPE
#define REG_ESCAPE	NOTEST
#endif
#ifndef REG_ICASE
#define REG_ICASE	NOTEST
#endif
#ifndef REG_LEFT
#define REG_LEFT	NOTEST
#endif
#ifndef REG_LENIENT
#define REG_LENIENT	0
#endif
#ifndef REG_MINIMAL
#define REG_MINIMAL	NOTEST
#endif
#ifndef REG_MULTIPLE
#define REG_MULTIPLE	NOTEST
#endif
#ifndef REG_MULTIREF
#define REG_MULTIREF	NOTEST
#endif
#ifndef REG_NEWLINE
#define REG_NEWLINE	NOTEST
#endif
#ifndef REG_NOTBOL
#define REG_NOTBOL	NOTEST
#endif
#ifndef REG_NOTEOL
#define REG_NOTEOL	NOTEST
#endif
#ifndef REG_NULL
#define REG_NULL	NOTEST
#endif
#ifndef REG_RIGHT
#define REG_RIGHT	NOTEST
#endif
#ifndef REG_SHELL_DOT
#define REG_SHELL_DOT	NOTEST
#endif
#ifndef REG_SHELL_ESCAPED
#define REG_SHELL_ESCAPED	NOTEST
#endif
#ifndef REG_SHELL_PATH
#define REG_SHELL_PATH	NOTEST
#endif
#ifndef REG_SPAN
#define REG_SPAN	NOTEST
#endif

#define REG_UNKNOWN	(-1)

#ifndef REG_ENEWLINE
#define REG_ENEWLINE	(REG_UNKNOWN-1)
#endif
#ifndef REG_ENULL
#ifndef REG_EMPTY
#define REG_ENULL	(REG_UNKNOWN-2)
#else
#define REG_ENULL	REG_EMPTY
#endif
#endif
#ifndef REG_ECOUNT
#define REG_ECOUNT	(REG_UNKNOWN-3)
#endif
#ifndef REG_BADESC
#define REG_BADESC	(REG_UNKNOWN-4)
#endif
#ifndef REG_EMEM
#define REG_EMEM	(REG_UNKNOWN-5)
#endif
#ifndef REG_ELOOP
#define REG_ELOOP	(REG_UNKNOWN-6)
#endif
#ifndef REG_EBUS
#define REG_EBUS	(REG_UNKNOWN-7)
#endif
#ifndef REG_EFAULT
#define REG_EFAULT	(REG_UNKNOWN-8)
#endif

static const struct { int code; char* name; } codes[] =
{
	REG_UNKNOWN,	"UNKNOWN",
	REG_NOMATCH,	"NOMATCH",
	REG_BADPAT,	"BADPAT",
	REG_ECOLLATE,	"ECOLLATE",
	REG_ECTYPE,	"ECTYPE",
	REG_EESCAPE,	"EESCAPE",
	REG_ESUBREG,	"ESUBREG",
	REG_EBRACK,	"EBRACK",
	REG_EPAREN,	"EPAREN",
	REG_EBRACE,	"EBRACE",
	REG_BADBR,	"BADBR",
	REG_ERANGE,	"ERANGE",
	REG_ESPACE,	"ESPACE",
	REG_BADRPT,	"BADRPT",
	REG_ENEWLINE,	"ENEWLINE",
	REG_ENULL,	"ENULL",
	REG_ECOUNT,	"ECOUNT",
	REG_BADESC,	"BADESC",
	REG_EMEM,	"EMEM",
	REG_ELOOP,	"ELOOP",
	REG_EBUS,	"EBUS",
	REG_EFAULT,	"EFAULT",
};

static struct
{
	regmatch_t	NOMATCH;
	int		errors;
	struct
	{
	int		count;
	int		error;
	int		position;
	}		ignore;
	int		lineno;
	int		signals;
	int		unspecified;
	int		warnings;
	char*		stack;
	char*		which;
	jmp_buf		gotcha;
#if REG_DISCIPLINE
	Disc_t		disc;
#endif
} state = { { -2, -2 } };

static void
quote(char* s, int len, int expand)
{
	unsigned char*	u = (unsigned char*)s;
	unsigned char*	e;
	int		c;

	if (!u)
		printf("NIL");
	else if (!*u && len <= 1)
		printf("NULL");
	else if (expand)
	{
		if (len < 0)
			len = strlen((char*)u);
		e = u + len;
		printf("\"");
		while (u < e)
			switch (c = *u++)
			{
			case '\\':
				printf("\\\\");
				break;
			case '"':
				printf("\\\"");
				break;
			case '\a':
				printf("\\a");
				break;
			case '\b':
				printf("\\b");
				break;
			case 033:
				printf("\\e");
				break;
			case '\f':
				printf("\\f");
				break;
			case '\n':
				printf("\\n");
				break;
			case '\r':
				printf("\\r");
				break;
			case '\t':
				printf("\\t");
				break;
			case '\v':
				printf("\\v");
				break;
			default:
				if (!iscntrl(c) && isprint(c))
					putchar(c);
				else
					printf("\\x%02x", c);
				break;
			}
		printf("\"");
	}
	else
		printf("%s", s);
}

static void
report(char* comment, char* fun, char* re, char* s, int len, char* msg, int flags, int unspecified, int expand)
{
	printf("%d:", state.lineno);
	if (re)
	{
		printf(" ");
		quote(re, -1, expand);
		if (s)
		{
			printf(" versus ");
			quote(s, len, expand);
		}
	}
	if (unspecified)
	{
		state.unspecified++;
		printf(" unspecified behavior");
	}
	else
		state.errors++;
	if (state.which)
		printf(" %s", state.which);
	if (flags & REG_NOSUB)
		printf(" NOSUB");
	if (fun)
		printf(" %s", fun);
	printf(" %s", comment);
	if (msg && comment[strlen(comment)-1] != '\n')
		printf("%s: ", msg);
}

static void
error(regex_t* preg, int code, int lineno)
{
	char*	msg;
	char	buf[256];

	switch (code)
	{
	case REG_EBUS:
		msg = "bus error";
		break;
	case REG_EFAULT:
		msg = "memory fault";
		break;
	case REG_ELOOP:
		msg = "did not terminate";
		break;
	default:
		regerror(code, preg, msg = buf, sizeof buf);
		break;
	}
	printf("%s\n", msg);
}

static int
note(int level, int skip, char* msg)
{
	if (!skip)
	{
		printf("NOTE\t");
		if (msg)
			printf("%s: ", msg);
		printf("skipping lines %d", state.lineno);
	}
	return skip | level;
}

static void
bad(char* comment, char* re, char* s, int len, int expand)
{
	printf("bad test case ");
	report(comment, NiL, re, s, len, NiL, 0, 0, expand);
	exit(1);
}

static int
escape(char* s)
{
	char*	b;
	char*	t;
	char*	q;
	char*	e;
	int	c;

	for (b = t = s; *t = *s; s++, t++)
		if (*s == '\\')
			switch (*++s)
			{
			case '\\':
				break;
			case 'a':
				*t = '\a';
				break;
			case 'b':
				*t = '\b';
				break;
			case 'c':
				if (*t = *++s)
					*t &= 037;
				else
					s--;
				break;
			case 'e':
			case 'E':
				*t = 033;
				break;
			case 'f':
				*t = '\f';
				break;
			case 'n':
				*t = '\n';
				break;
			case 'r':
				*t = '\r';
				break;
			case 's':
				*t = ' ';
				break;
			case 't':
				*t = '\t';
				break;
			case 'v':
				*t = '\v';
				break;
			case 'x':
				c = 0;
				q = ++s + 2;
				e = s;
				for (;;)
				{
					if (e && s >= q)
					{
						s--;
						break;
					}
					switch (*s)
					{
					case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
						c = (c << 4) + *s++ - 'a' + 10;
						continue;
					case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
						c = (c << 4) + *s++ - 'A' + 10;
						continue;
					case '0': case '1': case '2': case '3': case '4':
					case '5': case '6': case '7': case '8': case '9':
						c = (c << 4) + *s++ - '0';
						continue;
					case '{':
						if (s != e)
							break;
						e = 0;
						s++;
						continue;
					case '}':
						if (e)
							s--;
						break;
					default:
						s--;
						break;
					}
					break;
				}
				*t = c;
				break;
			case '0': case '1': case '2': case '3':
			case '4': case '5': case '6': case '7':
				c = *s - '0';
				q = s + 2;
				while (s < q)
				{
					switch (*++s)
					{
					case '0': case '1': case '2': case '3':
					case '4': case '5': case '6': case '7':
						c = (c << 3) + *s - '0';
						break;
					default:
						q = --s;
						break;
					}
				}
				*t = c;
				break;
			default:
				*(s + 1) = 0;
				bad("invalid C \\ escape\n", s - 1, NiL, 0, 0);
			}
	return t - b;
}

static void
matchprint(regmatch_t* match, int nmatch)
{
	int	i;

	for (; nmatch > 0; nmatch--)
		if (match[nmatch-1].rm_so != -1 && (!state.ignore.position || match[nmatch-1].rm_so >= 0 && match[nmatch-1].rm_eo >= 0))
			break;
	for (i = 0; i < nmatch; i++)
	{
		printf("(");
		if (match[i].rm_so == -1)
			printf("?");
		else
			printf("%d", (int)match[i].rm_so);
		printf(",");
		if (match[i].rm_eo == -1)
			printf("?");
		else
			printf("%d", (int)match[i].rm_eo);
		printf(")");
	}
	printf("\n");
}

static int
matchcheck(int nmatch, regmatch_t* match, char* ans, char* re, char* s, int len, int flags, int query, int unspecified, int expand)
{
	char*	p;
	char*	x;
	int	i;
	int	m;
	int	n;

	if (streq(ans, "OK"))
		return 0;
	for (i = 0, p = ans; i < nmatch && *p; i++)
	{
		if (*p == '{')
		{
#if REG_DISCIPLINE
			x = sfstruse(state.disc.sp);
			if (strcmp(p, x))
			{
				report("callout error: ", NiL, re, s, len, NiL, flags, unspecified, expand);
				quote(p, -1, expand);
				printf(" expected, ");
				quote(x, -1, expand);
				printf(" returned\n");
			}
#endif
			break;
		}
		if (*p++ != '(')
			bad("improper answer\n", re, s, -1, expand);
		if (*p == '?')
		{
			m = -1;
			p++;
		}
		else
			m = strtol(p, &p, 10);
		if (*p++ != ',')
			bad("improper answer\n", re, s, -1, expand);
		if (*p == '?')
		{
			n = -1;
			p++;
		}
		else
			n = strtol(p, &p, 10);
		if (*p++ != ')')
			bad("improper answer\n", re, s, -1, expand);
		if (m!=match[i].rm_so || n!=match[i].rm_eo)
		{
			if (!query)
			{
				report("match was: ", NiL, re, s, len, NiL, flags, unspecified, expand);
				matchprint(match, nmatch);
			}
			return 0;
		}
	}
	for (; i < nmatch; i++)
	{
		if (match[i].rm_so!=-1 || match[i].rm_eo!=-1)
		{
			if (!query)
			{
				if (state.ignore.position && (match[i].rm_so<0 || match[i].rm_eo<0))
				{
					state.ignore.count++;
					return 0;
				}
				report("match was: ", NiL, re, s, len, NiL, flags, unspecified, expand);
				matchprint(match, nmatch);
			}
			return 0;
		}
	}
	if (match[nmatch].rm_so != state.NOMATCH.rm_so)
	{
		report("overran match array: ", NiL, re, s, len, NiL, flags, unspecified, expand);
		matchprint(match, nmatch + 1);
	}
	return 1;
}

static void
sigunblock(int s)
{
#ifdef SIG_SETMASK
	int		op;
	sigset_t	mask;

	sigemptyset(&mask);
	if (s)
	{
		sigaddset(&mask, s);
		op = SIG_UNBLOCK;
	}
	else op = SIG_SETMASK;
	sigprocmask(op, &mask, NiL);
#else
#ifdef sigmask
	sigsetmask(s ? (sigsetmask(0L) & ~sigmask(s)) : 0L);
#endif
#endif
}

static void
gotcha(int sig)
{
	int	ret;

	signal(sig, gotcha);
	alarm(0);
	state.signals++;
	switch (sig)
	{
	case SIGALRM:
		ret = REG_ELOOP;
		break;
	case SIGBUS:
		ret = REG_EBUS;
		break;
	default:
		ret = REG_EFAULT;
		break;
	}
	sigunblock(sig);
	longjmp(state.gotcha, ret);
}

static char*
getline(void)
{
	static char	buf[32 * 1024];

	register char*	s = buf;
	register char*	e = &buf[sizeof(buf)];
	register char*	b;

	for (;;)
	{
		if (!(b = fgets(s, e - s, stdin)))
			return 0;
		state.lineno++;
		s += strlen(s) - 1;
		if (*s != '\n')
			break;
		if (s == b || *(s - 1) != '\\')
		{
			*s = 0;
			break;
		}
		s--;
	}
	return buf;
}

int
main(int argc, char** argv)
{
	int		flags;
	int		cflags;
	int		eflags;
	int		sflags;
	int		expand;
	int		query;
	int		are;
	int		bre;
	int		ere;
	int		kre;
	int		lre;
	int		sre;
	int		sub;
	int		nmatch;
	int		nexec;
	int		nstr;
	int		cret;
	int		eret;
	int		i;
	int		expected;
	int		got;
	int		unspecified;
	char*		p;
	char*		spec;
	char*		re;
	char*		s;
	char*		ans;
	char*		msg;
	char*		fun;
	char*		into;
	char*		qual;
	char*		ppat;
	char*		field[6];
	char		unit[64];
	char		buf[256];
	regmatch_t	match[100];
	regex_t		preg;
#if REG_SUB_ALL
	Sfio_t*		sp;
#endif

	static char	pat[32 * 1024];

	int		catch = 0;
	int		level = 1;
	int		locale = 0;
	int		skip = 0;
	int		testno = 0;
	int		verbose = 0;

	printf("TEST\t%s", s = fmtident(id));
	p = unit;
	while (p < &unit[sizeof(unit)-1] && (*p = *s++) && !isspace(*p))
		p++;
	*p = 0;
	while ((p = *++argv) && *p == '-')
		for (;;)
		{
			switch (*++p)
			{
			case 0:
				break;
			case 'c':
				catch = 1;
				printf(", catch");
				continue;
			case 'e':
				state.ignore.error = 1;
				printf(", ignore error code mismatches");
				continue;
			case 'h':
			case '?':
			case '-':
				printf(", help\n\n");
				help();
				return 2;
			case 'p':
				state.ignore.position = 1;
				printf(", ignore negative position mismatches");
				continue;
			case 's':
#if REG_DISCIPLINE
				printf(", stack");
				if (state.stack = stkalloc(stkstd, 0))
				{
					state.disc.disc.re_resizef = resizef;
					state.disc.disc.re_resizehandle = (void*)stkstd;
				}
				else
					printf(" error");
#endif
				continue;
			case 'v':
				verbose = 1;
				printf(", verbose");
				continue;
			default:
				printf(", invalid option %c", *p);
				continue;
			}
			break;
		}
	if (p)
		printf(", argument(s) ignored");
	printf("\n");
#ifdef REG_TEST_VERSION
	s = REG_TEST_VERSION;
#else
#ifdef REG_VERSIONID
	if (regerror(REG_VERSIONID, NiL, buf, sizeof(buf)) > 0)
		s = buf;
	else
#endif
	s = "regex";
#endif
	printf("NOTE\t%s\n", s);
	if (elementsof(unsupported) > 1)
	{
		printf("NOTE\tunsupported:");
		got = ' ';
		for (i = 0; i < elementsof(unsupported) - 1; i++)
		{
			printf("%c%s", got, unsupported[i]);
			got = ',';
		}
		printf("\n");
	}
#if REG_SUB_ALL
	if (!(sp = sfstropen()))
		bad("out of space [string stream]\n", NiL, NiL, 0, 0);
#endif
#if REG_DISCIPLINE
	state.disc.disc.re_version = REG_VERSION;
	state.disc.disc.re_compf = compf;
	state.disc.disc.re_execf = execf;
	if (!(state.disc.sp = sfstropen()))
		bad("out of space [discipline string stream]\n", NiL, NiL, 0, 0);
	preg.re_disc = &state.disc.disc;
#endif
	if (catch)
	{
		signal(SIGALRM, gotcha);
		signal(SIGBUS, gotcha);
		signal(SIGSEGV, gotcha);
	}
	while (p = getline())
	{

	/* parse: */

		if (*p == 0 || *p == '#')
			continue;
		if (*p == ':')
		{
			while (*++p == ' ');
			printf("NOTE	%s\n", p);
			continue;
		}
		i = 0;
		field[i++] = p;
		for (;;)
		{
			switch (*p++)
			{
			case 0:
				p--;
				goto checkfield;
			case '\t':
				*(p - 1) = 0;
			checkfield:
				s = field[i - 1];
				if (streq(s, "NIL"))
					field[i - 1] = 0;
				else if (streq(s, "NULL"))
					*s = 0;
				while (*p == '\t')
					p++;
				if (!*p)
					break;
				if (i >= elementsof(field))
					bad("too many fields\n", NiL, NiL, 0, 0);
				field[i++] = p;
				/*FALLTHROUGH*/
			default:
				continue;
			}
			break;
		}
		if (!(spec = field[0]))
			bad("NIL spec\n", NiL, NiL, 0, 0);

	/* interpret: */

		cflags = REG_TEST_DEFAULT;
		eflags = REG_EXEC_DEFAULT;
		expand = query = unspecified = are = bre = ere = kre = lre = sre = sub = 0;
		nmatch = 20;
		for (p = spec; *p; p++)
		{
			if (isdigit(*p))
			{
				nmatch = strtol(p, &p, 10);
				p--;
				continue;
			}
			switch (*p)
			{
			case 'A':
				are = 1;
				continue;
			case 'B':
				bre = 1;
				continue;
			case 'C':
				if (!query && !(skip & level))
					bad("locale must be nested\n", NiL, NiL, 0, 0);
				query = 0;
				if (locale)
					bad("locale nesting not supported\n", NiL, NiL, 0, 0);
				if (i != 2)
					bad("locale field expected\n", NiL, NiL, 0, 0);
				if (!(skip & level))
				{
#if defined(LC_COLLATE) && defined(LC_CTYPE)
					s = field[1];
					if (!s || streq(s, "POSIX"))
						s = "C";
					if (!(ans = setlocale(LC_COLLATE, s)) || streq(ans, "C") || streq(ans, "POSIX") || !(ans = setlocale(LC_CTYPE, s)) || streq(ans, "C") || streq(ans, "POSIX"))
						skip = note(level, skip, s);
					else
					{
						printf("NOTE	\"%s\" locale\n", s);
						locale = level;
					}
#else
					skip = note(level, skip, "locales not supported");
#endif
				}
				cflags = NOTEST;
				continue;
			case 'E':
				ere = 1;
				continue;
			case 'K':
				kre = 1;
				continue;
			case 'L':
				lre = 1;
				continue;
			case 'S':
				sre = 1;
				continue;

			case 'a':
				cflags |= REG_LEFT|REG_RIGHT;
				continue;
			case 'b':
				eflags |= REG_NOTBOL;
				continue;
			case 'c':
				cflags |= REG_COMMENT;
				continue;
			case 'd':
				cflags |= REG_SHELL_DOT;
				continue;
			case 'e':
				eflags |= REG_NOTEOL;
				continue;
			case 'f':
				cflags |= REG_MULTIPLE;
				continue;
			case 'g':
				cflags |= NOTEST;
				continue;
			case 'h':
				cflags |= REG_MULTIREF;
				continue;
			case 'i':
				cflags |= REG_ICASE;
				continue;
			case 'j':
				cflags |= REG_SPAN;
				continue;
			case 'k':
				cflags |= REG_ESCAPE;
				continue;
			case 'l':
				cflags |= REG_LEFT;
				continue;
			case 'm':
				cflags |= REG_MINIMAL;
				continue;
			case 'n':
				cflags |= REG_NEWLINE;
				continue;
			case 'p':
				cflags |= REG_SHELL_PATH;
				continue;
			case 'r':
				cflags |= REG_RIGHT;
				continue;
			case 's':
				cflags |= REG_SHELL_ESCAPED;
				continue;
			case 'u':
				unspecified = 1;
				continue;
			case 'x':
				cflags |= REG_LENIENT;
				continue;
			case 'z':
				cflags |= REG_NULL;
				continue;

			case '$':
				expand = 1;
				continue;

			case '/':
				sub = 1;
				continue;

			case '{':
				level <<= 1;
				if (skip & (level >> 1))
				{
					skip |= level;
					cflags = NOTEST;
				}
				else
				{
					skip &= ~level;
					query = 1;
				}
				continue;
			case '}':
				if (level == 1)
					bad("invalid {...} nesting\n", NiL, NiL, 0, 0);
				else
				{
					if ((skip & level) && !(skip & (level>>1)))
						printf("-%d\n", state.lineno);
#if defined(LC_COLLATE) && defined(LC_CTYPE)
					else if (locale & level)
					{
						locale = 0;
						if (!(skip & level))
						{
							s = "C";
							setlocale(LC_COLLATE, s);
							setlocale(LC_CTYPE, s);
							printf("NOTE	\"%s\" locale\n", s);
						}
					}
#endif
					level >>= 1;
				}
				cflags = NOTEST;
				continue;

			default:
				bad("bad spec\n", spec, NiL, 0, 0);
				break;

			}
			break;
		}
		if ((cflags|eflags) == NOTEST)
			continue;
		if (i < 4)
			bad("too few fields\n", NiL, NiL, 0, 0);
		while (i < elementsof(field))
			field[i++] = 0;
		if (re = field[1])
		{
			if (streq(re, "SAME"))
				re = ppat;
			else
			{
				if (expand)
					escape(re);
				strcpy(ppat = pat, re);
			}
		}
		else
			ppat = 0;
		nstr = -1;
		if ((s = field[2]) && expand)
		{
			nstr = escape(s);
#if REG_AUGMENTED
			if (nstr != strlen(s))
				nexec = nstr;
#endif
		}
		if (!(ans = field[3]))
			bad("NIL answer\n", NiL, NiL, 0, 0);
		msg = field[4];
		fflush(stdout);
		if (sub)
#if REG_SUB_ALL
			cflags |= REG_DELIMITED;
#else
			continue;
#endif

	compile:

		if (skip & level)
			continue;
#if !(REG_TEST_DEFAULT & (REG_AUGMENTED|REG_EXTENDED|REG_SHELL))
		if (bre)
		{
			state.which = "BRE";
			bre = 0;
			flags = cflags;
		}
		else
#endif
#if REG_EXTENDED
		if (ere)
		{
			state.which = "ERE";
			ere = 0;
			flags = cflags | REG_EXTENDED;
		}
		else
#endif
#if REG_AUGMENTED
		if (are)
		{
			state.which = "ARE";
			are = 0;
			flags = cflags | REG_AUGMENTED;
		}
		else
#endif
#if REG_LITERAL
		if (lre)
		{
			state.which = "LRE";
			lre = 0;
			flags = cflags | REG_LITERAL;
		}
		else
#endif
#if REG_SHELL
		if (sre)
		{
			state.which = "SRE";
			sre = 0;
			flags = cflags | REG_SHELL;
		}
		else
#if REG_AUGMENTED
		if (kre)
		{
			state.which = "KRE";
			kre = 0;
			flags = cflags | REG_SHELL | REG_AUGMENTED;
		}
		else
#endif
#endif
			continue;
		if (!query && verbose)
		{
			printf("test %-3d %s ", state.lineno, state.which);
			quote(re, -1, expand);
			printf(" ");
			quote(s, nstr, expand);
			printf("\n");
		}

	nosub:

		fun = "regcomp";
#if REG_AUGMENTED
		if (nstr >= 0 && nstr != strlen(s))
			nexec = nstr;

		else
#endif
			nexec = -1;
		if (skip & level)
		{
			regfree(&preg);
			continue;
		}
		if (!query)
			testno++;
#if REG_DISCIPLINE
		if (state.stack)
			stkset(stkstd, state.stack, 0);
		flags |= REG_DISCIPLINE;
		state.disc.ordinal = 0;
		sfstrset(state.disc.sp, 0);
#endif
		if (!catch)
			cret = regcomp(&preg, re, flags);
		else if (!(cret = setjmp(state.gotcha)))
		{
			alarm(LOOPED);
			cret = regcomp(&preg, re, flags);
			alarm(0);
		}

		if (cret == 0)
		{
#if REG_SUB_ALL
			if (sub)
			{
				p = re + preg.re_npat;
				if (i = *p)
					p++;
				into = p;
				while (*p && *p != i)
					if (*p++ == '\\' && !*p++)
					{
						p--;
						break;
					}
				if (*p != i || !i)
				{
					report("failed: ", fun, re, NiL, -1, msg, flags, unspecified, expand);
					printf("missing `%c' delimiter for rhs of substitution\n", i);
					regfree(&preg);
					continue;
				}
				qual = p;
				*p++ = 0;
				sflags = 0;
				for (;;)
				{
					switch (*p++)
					{
					case 0:
						p--;
						break;
					case 'g':
						sflags |= REG_SUB_ALL;
						continue;
					case 'l':
						sflags |= REG_SUB_LOWER;
						continue;
					case 'u':
						sflags |= REG_SUB_UPPER;
						continue;
					default:
						break;
					}
					break;
				}
				if (*p)
				{
					report("failed: ", fun, re, NiL, -1, msg, flags, unspecified, expand);
					printf("invalid characters `%s' after substitution\n", p);
					regfree(&preg);
					continue;
				}
			}
			else
#endif
			if (*ans && *ans != '(' && !streq(ans, "OK") && !streq(ans, "NOMATCH"))
			{
				if (query)
					skip = note(level, skip, msg);
				else
				{
					report("failed: ", fun, re, NiL, -1, msg, flags, unspecified, expand);
					printf("%s expected, OK returned\n", ans);
				}

				regfree(&preg);
				continue;
			}
		}
		else
		{
			if (!*ans || ans[0]=='(' || cret == REG_BADPAT && streq(ans, "NOMATCH"))
			{
				if (query)
					skip = note(level, skip, msg);
				else
				{
					report("failed: ", fun, re, NiL, -1, msg, flags, unspecified, expand);
					got = 0;
					for (i = 1; i < elementsof(codes); i++)
						if (cret==codes[i].code)
							got = i;
					printf("%s returned: ", codes[got].name);
					error(&preg, cret, state.lineno);
				}
			}
			else
			{
				expected = got = 0;
				for (i = 1; i < elementsof(codes); i++)
				{
					if (streq(ans, codes[i].name))
						expected = i;
					if (cret==codes[i].code)
						got = i;
				}
				if (!expected)
				{
					if (query)
						skip = note(level, skip, msg);
					else
					{
						report("invalid error code: ", NiL, re, NiL, -1, msg, flags, unspecified, expand);
						printf("%s expected, %s returned\n", ans, codes[got].name);
					}
				}
				else if (cret != codes[expected].code && cret != REG_BADPAT)
				{
					if (query)
						skip = note(level, skip, msg);
					else if (state.ignore.error)
						state.ignore.count++;
					else
					{
						report("should fail and did: ", fun, re, NiL, -1, msg, flags, unspecified, expand);
						printf("%s expected, %s returned: ", ans, codes[got].name);
						state.errors--;
						state.warnings++;
						error(&preg, cret, state.lineno);
					}
				}
			}
			goto compile;
		}

#if _REG_nexec
	execute:
		if (nexec >= 0)
			fun = "regnexec";
		else
#endif
			fun = "regexec";
		
		for (i = 0; i < elementsof(match); i++)
			match[i] = state.NOMATCH;

#if _REG_nexec
		if (nexec >= 0)
		{
			eret = regnexec(&preg, s, nexec, nmatch, match, eflags);
			s[nexec] = 0;
		}
		else
#endif
		{
			if (!catch)
				eret = regexec(&preg, s, nmatch, match, eflags);
			else if (!(eret = setjmp(state.gotcha)))
			{
				alarm(LOOPED);
				eret = regexec(&preg, s, nmatch, match, eflags);
				alarm(0);
			}
		}
#if _REG_sub
		if (sub && !eret)
			eret = regsub(&preg, sp, s, into, nmatch, match, sflags);
#endif

		if (flags & REG_NOSUB)
		{
			if (eret)
			{
				if (eret != REG_NOMATCH || !streq(ans, "NOMATCH"))
				{
					if (query)
						skip = note(level, skip, msg);
					else
					{
						report("REG_NOSUB failed: ", fun, re, s, nstr, msg, flags, unspecified, expand);
						error(&preg, eret, state.lineno);
					}
				}
			}
			else if (streq(ans, "NOMATCH"))
			{
				if (query)
					skip = note(level, skip, msg);
				else
				{
					report("should fail and didn't: ", fun, re, s, nstr, msg, flags, unspecified, expand);
					error(&preg, eret, state.lineno);
				}
			}
		}
		else if (eret)
		{
			if (eret > REG_NOMATCH || !streq(ans, "NOMATCH"))
			{
				if (query)
					skip = note(level, skip, msg);
				else
				{
					report("failed", fun, re, s, nstr, msg, flags, unspecified, expand);
					if (eret != REG_NOMATCH)
					{
						printf(": ");
						error(&preg, eret, state.lineno);
					}
					else
						printf("\n");
				}
			}
		}
		else if (streq(ans, "NOMATCH"))
		{
			if (query)
				skip = note(level, skip, msg);
			else
			{
				report("should fail and didn't: ", fun, re, s, nstr, msg, flags, unspecified, expand);
				matchprint(match, nmatch);
			}
		}
#if _REG_sub
		else if (sub)
		{
			p = sfstruse(sp);
			if (strcmp(p, ans))
			{
				*qual = *--into = *re;
				report("failed: ", "regsub", re, s, nstr, msg, flags, unspecified, expand);
				quote(ans, -1, expand);
				printf(" expected, ");
				quote(p, -1, expand);
				printf(" returned\n");
			}
		}
#endif
		else if (!*ans)
		{
			if (match[0].rm_so != state.NOMATCH.rm_so)
			{
				if (query)
					skip = note(level, skip, msg);
				else
				{
					report("no match but match array assigned: ", NiL, re, s, nstr, msg, flags, unspecified, expand);
					matchprint(match, nmatch);
				}
			}
		}
		else if (matchcheck(nmatch, match, ans, re, s, nstr, flags, query, unspecified, expand))
		{
#if REG_AUGMENTED
			if (nexec < 0)
			{
				nexec = nstr >= 0 ? nstr : strlen(s);
				s[nexec] = '\n';
				testno++;
				goto execute;
			}
#endif
			if (!sub)
			{
				regfree(&preg);
				flags |= REG_NOSUB;
				goto nosub;
			}
		}
		else if (query)
			skip = note(level, skip, msg);
		regfree(&preg);
		goto compile;
	}
	printf("TEST\t%s, %d test%s", unit, testno, testno == 1 ? "" : "s");
	if (state.ignore.count)
		printf(", %d ignored mismatche%s", state.ignore.count, state.ignore.count == 1 ? "" : "s");
	if (state.warnings)
		printf(", %d warning%s", state.warnings, state.warnings == 1 ? "" : "s");
	if (state.unspecified)
		printf(", %d unspecified difference%s", state.unspecified, state.unspecified == 1 ? "" : "s");
	if (state.signals)
		printf(", %d signal%s", state.signals, state.signals == 1 ? "" : "s");
	printf(", %d error%s\n", state.errors, state.errors == 1 ? "" : "s");
	return 0;
}
