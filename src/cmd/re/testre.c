/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1995-2000 AT&T Corp.              *
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
 * regex tester
 *
 * testre [ options ] < testre.dat
 *
 *	-c	catch signals and non-terminating regcomp,regexec
 *	-e	ignore error return mismatches
 *	-p	ignore negative position mismatches
 *	-s	use stack instead of malloc
 *	-v	list each test line
 *
 * see comments in testre.dat for description of format
 */

static const char id[] = "\n@(#)testre (AT&T Research) 1999-03-01\0\n";

#include <stdio.h>
#include <regex.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>

#ifdef	__STDC__
#include <stdlib.h>
#endif

#ifndef NiL
#ifdef	__STDC__
#define NiL		0
#else
#define NiL		(char*)0
#endif
#endif

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

static const char* unsupported[] = {
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
#if REG_NOSUB & REG_TEST_DEFAULT
	"SUBMATCH",
#endif
	0
};

#ifndef REG_ICASE
#define REG_ICASE	NOTEST
#endif
#ifndef REG_LEFT
#define REG_LEFT	NOTEST
#endif
#ifndef REG_LENIENT
#define REG_LENIENT	NOTEST
#endif
#ifndef REG_MINIMAL
#define REG_MINIMAL	NOTEST
#endif
#ifndef REG_MULTIPLE
#define REG_MULTIPLE	NOTEST
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

static const struct { int code; char* name; } codes[] = {
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
	struct {
	int		count;
	int		error;
	int		position;
	}		ignore;
	int		lineno;
	int		ret;
	int		signals;
	int		warnings;
	char*		stack;
	char*		which;
	jmp_buf		gotcha;
} state = { { -2, -2 } };

#ifdef REG_AUGMENTED

#include <stk.h>

static void*
alloc(void* handle, void* pointer, size_t size)
{
	return stkalloc((Stk_t*)handle, size);
}

#endif

static void
normal(char* s)
{
	unsigned char*	u = (unsigned char*)s;
	int		c;

	if (!u)
		printf("NIL");
	else if (!*u)
		printf("NULL");
	else for (;;)
		switch (c = *u++) {

		case 0:
			return;
		case '\n':
			printf("\\n");
			break;
		case '\r':
			printf("\\r");
			break;
		case '\t':
			printf("\\t");
			break;
		default:
			if (!iscntrl(c) && isprint(c))
				putchar(c);
			else
				printf("\\x%02x", c);
			break;
		}
}

static void
report(char* comment, char* fun, char* re, char* s, char* msg, int flags)
{
	state.errors++;
	printf("%d:", state.lineno);
	if (re) {
		normal(re);
		if (s) {
			printf(" versus ");
			normal(s);
		}
	}
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

	switch (code) {

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
note(int skip, char* msg)
{
	if (!skip++) {
		printf("NOTE\t");
		if (msg)
			printf("%s: ", msg);
		printf("skipping lines %d", state.lineno);
	}
	return skip;
}

static void
bad(char* comment, char* re, char* s)
{
	printf("bad test case ");
	report(comment, NiL, re, s, NiL, 0);
	exit(1);
}

static int
hex(int c)
{
	return isdigit(c) ? (c - '0') : (c - (isupper(c) ? 'A' : 'a') + 10);
}

static void
escape(char* s)
{
	char*	t;

	for (t = s; *t = *s; s++, t++) {
		if (*s != '\\')
			continue;
		switch (*++s) {

		case 0:
			*++t = 0;
			break;
		case 'n':
			*t = '\n';
			break;
		case 'r':
			*t = '\r';
			break;
		case 't':
			*t = '\t';
			break;
		case 'x':
			if (!isxdigit(s[1]) || !isxdigit(s[2]))
				bad("bad \\x\n", NiL, NiL);
			*t = hex(*++s) << 4;
			*t |= hex(*++s);
			break;
		default:
			s--;
			break;
		}
	}
}

static void
matchprint(regmatch_t* match, int nmatch)
{
	int	i;

	for (; nmatch > 0; nmatch--)
		if (match[nmatch-1].rm_so != -1 && (!state.ignore.position || match[nmatch-1].rm_so >= 0 && match[nmatch-1].rm_eo >= 0))
			break;
	for (i = 0; i < nmatch; i++) {
		printf("(");
		if (match[i].rm_so == -1)
			printf("?");
		else
			printf("%d", match[i].rm_so);
		printf(",");
		if (match[i].rm_eo == -1)
			printf("?");
		else
			printf("%d", match[i].rm_eo);
		printf(")");
	}
	printf("\n");
}

static int
matchcheck(int nmatch, regmatch_t* match, char* ans, char* re, char* s, int flags, int query)
{
	char*	p;
	int	i;
	int	m;
	int	n;

	for (i = 0, p = ans; i < nmatch && *p; i++) {
		if (*p++ != '(')
			bad("improper answer\n", re, s);
		if (*p == '?') {
			m = -1;
			p++;
		} else
			m = strtol(p, &p, 10);
		if (*p++ != ',')
			bad("improper answer\n", re, s);
		if (*p == '?') {
			n = -1;
			p++;
		} else
			n = strtol(p, &p, 10);
		if (*p++ != ')')
			bad("improper answer\n", re, s);
		if (m!=match[i].rm_so || n!=match[i].rm_eo) {
			if (!query) {
				report("match was: ", NiL, re, s, NiL, flags);
				matchprint(match, nmatch);
			}
			return 0;
		}
	}
	for (; i < nmatch; i++) {
		if (match[i].rm_so!=-1 || match[i].rm_eo!=-1) {
			if (!query) {
				if (state.ignore.position && (match[i].rm_so<0 || match[i].rm_eo<0)) {
					state.ignore.count++;
					return 0;
				}
				report("match was: ", NiL, re, s, NiL, flags);
				matchprint(match, nmatch);
			}
			return 0;
		}
	}
	if (match[nmatch].rm_so != state.NOMATCH.rm_so) {
		report("overran match array: ", NiL, re, s, NiL, flags);
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
	if (s) {
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
	signal(sig, gotcha);
	alarm(0);
	state.signals++;
	switch (sig) {

	case SIGALRM:
		state.ret = REG_ELOOP;
		break;
	case SIGBUS:
		state.ret = REG_EBUS;
		break;
	case SIGSEGV:
		state.ret = REG_EFAULT;
		break;
	}
	sigunblock(sig);
	longjmp(state.gotcha, 1);
}

int
main(int argc, char** argv)
{
	int		flags;
	int		cflags;
	int		eflags;
	int		query;
	int		are;
	int		bre;
	int		ere;
	int		kre;
	int		lre;
	int		sre;
	int		nmatch;
	int		nexec;
	int		cret;
	int		eret;
	int		i;
	int		expected;
	int		got;
	char*		p;
	char*		spec;
	char*		re;
	char*		s;
	char*		ans;
	char*		msg;
	char*		fun;
	char*		field[6];
	char		unit[64];
	char		buf[16 * 1024];
	regmatch_t	match[100];
	regex_t		preg;

	int		catch = 0;
	int		skip = 0;
	int		testno = 0;
	int		verbose = 0;

	printf("TEST	");
#ifdef REG_TEST_VERSION
	s = REG_TEST_VERSION;
#else
#ifdef REG_VERSION
	if (regerror(REG_VERSION, NiL, buf, sizeof(buf)) > 0)
		s = buf;
	else
#endif
	s = "regex";
#endif
	printf("%s", s);
	p = unit;
	while (p < &unit[sizeof(unit)-1] && (*p = *s++) && !isspace(*p))
		p++;
	*p = 0;
	while ((p = *++argv) && *p == '-')
		for (;;) {
			switch (*++p) {

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
			case 'p':
				state.ignore.position = 1;
				printf(", ignore negative position mismatches");
				continue;
			case 's':
#ifdef REG_AUGMENTED
				printf(", stack");
				if (state.stack = stkalloc(stkstd, 0))
					regalloc(stkstd, alloc, REG_NOFREE);
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
	if (elementsof(unsupported) > 1) {
		printf("NOTE\tunsupported:");
		got = ' ';
		for (i = 0; i < elementsof(unsupported) - 1; i++) {
			printf("%c%s", got, unsupported[i]);
			got = ',';
		}
		printf("\n");
	}
	if (catch) {
		signal(SIGALRM, gotcha);
		signal(SIGBUS, gotcha);
		signal(SIGSEGV, gotcha);
	}
	while (p = gets(buf)) {
		state.lineno++;

	/* parse: */

		if (*p == 0 || *p == '#')
			continue;
		if (*p == ':') {
			while (*++p == ' ');
			printf("NOTE	%s\n", p);
			continue;
		}
		i = 0;
		field[i++] = p;
		for (;;) {
			switch (*p++) {

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
					bad("too many fields\n", NiL, NiL);
				field[i++] = p;
				/*FALLTHROUGH*/
			default:
				continue;
			}
			break;
		}
		if (!(spec = field[0]))
			bad("NIL spec\n", NiL, NiL);

	/* interpret: */

		cflags = REG_TEST_DEFAULT;
		eflags = REG_EXEC_DEFAULT;
		query = are = bre = ere = kre = lre = sre = 0;
		nmatch = 20;
		for (p = spec; *p; p++) {
			if (isdigit(*p)) {
				nmatch = strtol(p, &p, 10);
				p--;
				continue;
			}
			switch (*p) {

			case 'A':
				are = 1;
				continue;
			case 'B':
				bre = 1;
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
			case 'd':
				cflags |= REG_SHELL_DOT;
				continue;
			case 'e':
				eflags |= REG_NOTEOL;
				continue;
			case 'f':
				cflags |= REG_MULTIPLE;
				continue;
			case 'i':
				cflags |= REG_ICASE;
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
			case 'x':
				cflags |= REG_LENIENT;
				continue;
			case 'z':
				cflags |= REG_NULL;
				continue;

			case '{':
				if (skip) {
					skip++;
					cflags |= NOTEST;
				} else
					query = 1;
				continue;
			case '}':
				if (skip && !--skip)
					printf("-%d\n", state.lineno);
				cflags |= NOTEST;
				continue;

			default:
				bad("bad spec\n", spec, NiL);
				break;

			}
			break;
		}
		if ((cflags|eflags) == NOTEST)
			continue;
		if (i < 4)
			bad("too few fields\n", NiL, NiL);
		while (i < elementsof(field))
			field[i++] = 0;
		if (re = field[1])
			escape(re);
		if (s = field[2])
			escape(s);
		if (!(ans = field[3]))
			bad("NIL answer\n", NiL, NiL);
		msg = field[4];
		fflush(stdout);

	compile:

		if (skip)
			continue;
#if !(REG_TEST_DEFAULT & (REG_AUGMENTED|REG_EXTENDED|REG_SHELL))
		if (bre) {
			state.which = "BRE";
			bre = 0;
			flags = cflags;
		} else
#endif
#ifdef REG_EXTENDED
		if (ere) {
			state.which = "ERE";
			ere = 0;
			flags = cflags | REG_EXTENDED;
		} else
#endif
#ifdef REG_AUGMENTED
		if (are) {
			state.which = "ARE";
			are = 0;
			flags = cflags | REG_AUGMENTED;
		} else
#endif
#ifdef REG_LITERAL
		if (lre) {
			state.which = "LRE";
			lre = 0;
			flags = cflags | REG_LITERAL;
		} else
#endif
#ifdef REG_SHELL
		if (sre) {
			state.which = "SRE";
			sre = 0;
			flags = cflags | REG_SHELL;
		} else
#ifdef REG_AUGMENTED
		if (kre) {
			state.which = "KRE";
			kre = 0;
			flags = cflags | REG_SHELL | REG_AUGMENTED;
		} else
#endif
#endif
			continue;
		if (!query && verbose)
			printf("test %-3d %s \"%s\" \"%s\"\n", state.lineno, state.which, re, s);

	nosub:

		fun = "regcomp";
		nexec = -1;
		if (skip) {
			regfree(&preg);
			continue;
		}
		if (!query)
			testno++;
#if REG_AUGMENTED
		if (state.stack)
			stkset(stkstd, state.stack, 0);
#endif
		if (catch) {
			if (setjmp(state.gotcha))
				cret = state.ret;
			else {
				alarm(LOOPED);
				cret = regcomp(&preg, re, flags);
				alarm(0);
			}
		} else
			cret = regcomp(&preg, re, flags);

		if (cret == 0) {
			if (*ans && *ans != '(' && !streq(ans, "NOMATCH")) {
				if (query)
					skip = note(skip, msg);
				else
					report("should fail and didn't\n", fun, re, NiL, msg, flags);
				regfree(&preg);
				continue;
			}
		} else {
			if (!*ans || ans[0]=='(' || cret == REG_BADPAT && streq(ans, "NOMATCH")) {
				if (query)
					skip = note(skip, msg);
				else {
					report("failed: ", fun, re, NiL, msg, flags);
					error(&preg, cret, state.lineno);
				}
			} else {
				expected = got = 0;
				for (i = 1; i < elementsof(codes); i++) {
					if (streq(ans, codes[i].name))
						expected = i;
					if (cret==codes[i].code)
						got = i;
				}
				if (!expected) {
					if (query)
						skip = note(skip, msg);
					else {
						report("invalid error code: ", NiL, re, NiL, msg, flags);
						printf("%s expected, %s returned\n", ans, codes[got].name);
					}
				}
				else if (cret != codes[expected].code && cret != REG_BADPAT) {
					if (query)
						skip = note(skip, msg);
					else if (state.ignore.error)
						state.ignore.count++;
					else {
						report("should fail and did: ", fun, re, NiL, msg, flags);
						printf("%s expected, %s returned: ", ans, codes[got].name);
						state.errors--;
						state.warnings++;
						error(&preg, cret, state.lineno);
					}
				}
			}
			goto compile;
		}
		fun = "regexec";

#ifdef REG_AUGMENTED
	execute:
#endif
		
		for (i = 0; i < elementsof(match); i++)
			match[i] = state.NOMATCH;

#ifdef REG_AUGMENTED
		if (nexec >= 0) {
			eret = regnexec(&preg, s, nexec, nmatch, match, eflags);
			s[nexec] = 0;
		}
		else
#endif
		if (catch) {
			if (setjmp(state.gotcha))
				eret = state.ret;
			else {
				alarm(LOOPED);
				eret = regexec(&preg, s, nmatch, match, eflags);
				alarm(0);
			}
		} else
			eret = regexec(&preg, s, nmatch, match, eflags);

		if (flags & REG_NOSUB) {
			if (eret) {
				if (!streq(ans, "NOMATCH")) {
					if (query)
						skip = note(skip, msg);
					else {
						report("REG_NOSUB failed: ", fun, re, s, msg, flags);
						error(&preg, eret, state.lineno);
					}
				}
			} else if (streq(ans, "NOMATCH")) {
				if (query)
					skip = note(skip, msg);
				else {
					report("should fail and didn't: ", fun, re, s, msg, flags);
					error(&preg, eret, state.lineno);
				}
			}
		} else if (eret) {
			if (!streq(ans, "NOMATCH")) {
				if (query)
					skip = note(skip, msg);
				else {
					report("failed", fun, re, s, msg, flags);
					if (eret != REG_NOMATCH) {
						printf(": ");
						error(&preg, eret, state.lineno);
					} else
						printf("\n");
				}
			}
		} else if (streq(ans, "NOMATCH")) {
			if (query)
				skip = note(skip, msg);
			else {
				report("should fail and didn't: ", fun, re, s, msg, flags);
				matchprint(match, nmatch);
			}
		} else if (!*ans) {
			if (match[0].rm_so != state.NOMATCH.rm_so) {
				if (query)
					skip = note(skip, msg);
				else {
					report("no match but match array assigned: ", NiL, re, s, msg, flags);
					matchprint(match, nmatch);
				}
			}
		} else if (matchcheck(nmatch, match, ans, re, s, flags, query)) {
#ifdef REG_AUGMENTED
			if (nexec < 0) {
				nexec = strlen(s);
				s[nexec] = '\n';
				testno++;
				fun = "regnexec";
				goto execute;
			}
#endif
			regfree(&preg);
			flags |= REG_NOSUB;
			goto nosub;
		} else if (query)
			skip = note(skip, msg);
		regfree(&preg);
		goto compile;
	}
	printf("TEST	%s, %d test%s", unit, testno, testno == 1 ? "" : "s");
	if (state.ignore.count)
		printf(", %d ignored mismatche%s", state.ignore.count, state.ignore.count == 1 ? "" : "s");
	if (state.warnings)
		printf(", %d warning%s", state.warnings, state.warnings == 1 ? "" : "s");
	if (state.signals)
		printf(", %d signal%s", state.signals, state.signals == 1 ? "" : "s");
	printf(", %d error%s\n", state.errors, state.errors == 1 ? "" : "s");
	return 0;
}
