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
 * strmatch tester
 * using testre input data format
 *
 * testre [ options ] < testre.dat
 *
 *	-v	list each test line
 *
 * see comments in testre.dat for description of format
 */

static const char id[] = "\n@(#)testmatch (AT&T Research) 1998-11-11\0\n";

#include <ast.h>
#include <ctype.h>

#define _DEBUG_MATCH	1

#include "strmatch.c"

#define LOOPED		2
#define NOTEST		(~0)

static struct
{
	int		errors;
	struct
	{
	int		count;
	int		error;
	int		position;
	}		ignore;
	int		lineno;
	int		warnings;
	char*		which;
} state;

static void
normal(char* s)
{
	unsigned char*	u = (unsigned char*)s;
	int		c;

	if (!u)
		sfprintf(sfstdout, "NIL");
	else if (!*u)
		sfprintf(sfstdout, "NULL");
	else for (;;)
		switch (c = *u++) {

		case 0:
			return;
		case '\n':
			sfprintf(sfstdout, "\\n");
			break;
		case '\r':
			sfprintf(sfstdout, "\\r");
			break;
		case '\t':
			sfprintf(sfstdout, "\\t");
			break;
		default:
			if (!iscntrl(c) && isprint(c))
				sfputc(sfstdout, c);
			else
				sfprintf(sfstdout, "\\x%02x", c);
			break;
		}
}

static void
report(char* comment, char* fun, char* re, char* s, char* msg, int flags)
{
	state.errors++;
	sfprintf(sfstdout, "%d:", state.lineno);
	if (re) {
		normal(re);
		if (s) {
			sfprintf(sfstdout, " versus ");
			normal(s);
		}
	}
	sfprintf(sfstdout, " %s", state.which);
	if (fun)
		sfprintf(sfstdout, " %s", fun);
	sfprintf(sfstdout, " %s", comment);
	if (msg && comment[strlen(comment)-1] != '\n')
		sfprintf(sfstdout, "%s: ", msg);
}

static int
note(int skip, char* msg)
{
	if (!skip++) {
		sfprintf(sfstdout, "NOTE\t");
		if (msg)
			sfprintf(sfstdout, "%s: ", msg);
		sfprintf(sfstdout, "skipping lines %d", state.lineno);
	}
	return skip;
}

static void
bad(char* comment, char* re, char* s)
{
	sfprintf(sfstdout, "bad test case ");
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
matchprint(int* match, int nmatch)
{
	int	i;

	for (; nmatch > 0; nmatch -= 2)
		if (match[nmatch-2] != -2 && (!state.ignore.position || match[nmatch-2] >= 0 && match[nmatch-2] >= 0))
			break;
	for (i = 0; i < nmatch; i += 2) {
		sfprintf(sfstdout, "(");
		if (match[i] == -1)
			sfprintf(sfstdout, "?");
		else
			sfprintf(sfstdout, "%d", match[i]);
		sfprintf(sfstdout, ",");
		if (match[i+1] == -1)
			sfprintf(sfstdout, "?");
		else
			sfprintf(sfstdout, "%d", match[i+1]);
		sfprintf(sfstdout, ")");
	}
	sfprintf(sfstdout, "\n");
}

static int
matchcheck(int nmatch, int* match, char* ans, char* re, char* s, int flags, int query)
{
	char*	p;
	int	i;
	int	m;
	int	n;

	for (i = 0, p = ans; i < nmatch && *p; i += 2) {
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
		if (m!=match[i] || n!=match[i+1]) {
			if (!query) {
				report("match was: ", NiL, re, s, NiL, flags);
				matchprint(match, nmatch);
			}
			return 0;
		}
	}
	for (; i < nmatch; i += 2) {
		if (match[i]!=-2 || match[i+1]!=-2) {
			if (!query) {
				if (state.ignore.position && (match[i]<0 || match[i+1]<0)) {
					state.ignore.count++;
					return 0;
				}
				report("match was: ", NiL, re, s, NiL, flags);
				matchprint(match, nmatch);
			}
			return 0;
		}
	}
	if (match[nmatch] != -2) {
		report("overran match array: ", NiL, re, s, NiL, flags);
		matchprint(match, nmatch + 1);
	}
	return 1;
}

int
main(int argc, char** argv)
{
	int		cflags;
	int		eflags;
	int		query;
	int		kre;
	int		sre;
	int		nmatch;
	int		eret;
	int		i;
	int		sub;
	char*		p;
	char*		spec;
	char*		re;
	char*		s;
	char*		ans;
	char*		msg;
	char*		fun;
	char*		field[6];
	char		unit[64];
	int		match[200];

	int		skip = 0;
	int		testno = 0;
	int		verbose = 0;

	sfprintf(sfstdout, "TEST	");
	sfprintf(sfstdout, "%s", id + 5);
	s = "strmatch";
	p = unit;
	while (p < &unit[sizeof(unit)-1] && (*p = *s++) && !isspace(*p))
		p++;
	*p = 0;
	while ((p = *++argv) && *p == '-')
		for (;;) {
			switch (*++p) {

			case 0:
				break;
			case 'v':
				verbose = 1;
				sfprintf(sfstdout, ", verbose");
				continue;
			default:
				sfprintf(sfstdout, ", invalid option %c", *p);
				continue;
			}
			break;
		}
	if (p)
		sfprintf(sfstdout, ", argument(s) ignored");
	sfprintf(sfstdout, "\n");
	while (p = sfgetr(sfstdin, '\n', 1)) {
		state.lineno++;

	/* parse: */

		if (*p == 0 || *p == '#')
			continue;
		if (*p == ':') {
			while (*++p == ' ');
			sfprintf(sfstdout, "NOTE	%s\n", p);
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

		cflags = 0;
		eflags = STR_MAXIMAL;
		query = kre = sre = 0;
		nmatch = 20;
		for (p = spec; *p; p++) {
			if (isdigit(*p)) {
				nmatch = strtol(p, &p, 10);
				p--;
				continue;
			}
			switch (*p) {

			case 'A':
				continue;
			case 'B':
				continue;
			case 'E':
				continue;
			case 'K':
				kre = 1;
				continue;
			case 'L':
				continue;
			case 'S':
				sre = 1;
				continue;

			case 'a':
				eflags |= STR_LEFT|STR_RIGHT;
				continue;
			case 'b':
				cflags = 1;
				continue;
			case 'd':
				cflags = 1;
				continue;
			case 'e':
				cflags = 1;
				continue;
			case 'i':
				eflags |= STR_ICASE;
				continue;
			case 'l':
				eflags |= STR_LEFT;
				continue;
			case 'm':
				eflags &= ~STR_MAXIMAL;
				continue;
			case 'n':
				cflags = 1;
				continue;
			case 'p':
				cflags = 1;
				continue;
			case 'r':
				eflags |= STR_RIGHT;
				continue;
			case 's':
				cflags = 1;
				continue;
			case 'x':
				cflags = 1;
				continue;
			case 'z':
				cflags = 1;
				continue;

			case '{':
				if (skip) {
					skip++;
					cflags = 1;
				} else
					query = 1;
				continue;
			case '}':
				if (skip && !--skip)
					sfprintf(sfstdout, "-%d\n", state.lineno);
				cflags = 1;
				continue;

			default:
				bad("bad spec\n", spec, NiL);
				break;

			}
			break;
		}
		if (cflags || !sre && !kre)
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
		sfsync(sfstdout);

	compile:

		if (skip)
			continue;
		if (sre) {
			state.which = "SRE";
			sre = 0;
		} else if (kre) {
			state.which = "KRE";
			kre = 0;
		} else
			continue;
		if (!query && verbose)
			sfprintf(sfstdout, "test %-3d %s \"%s\" \"%s\"\n", state.lineno, state.which, re, s);
		sub = 1;

	nosub:

		if (skip)
			continue;
		if (!query)
			testno++;
		for (i = 0; i < elementsof(match); i++)
			match[i] = -2;
		if (sub) {
			fun = "strgrpmatch";
			eret = strgrpmatch(s, re, match, nmatch, eflags) == 0;
			if (verbose)
				sfprintf(sfstdout, "[%s]", fun);
		} else {
			fun = "strmatch";
			eret = strmatch(s, re) == 0;
		}
		if (!sub) {
			if (eret) {
				if (!streq(ans, "NOMATCH")) {
					if (query)
						skip = note(skip, msg);
					else
						report("failed: ", fun, re, s, msg, nmatch);
						sfprintf(sfstdout, "\n");
				}
			} else if (streq(ans, "NOMATCH")) {
				if (query)
					skip = note(skip, msg);
				else {
					report("should fail and didn't: ", fun, re, s, msg, nmatch);
					sfprintf(sfstdout, "\n");
				}
			}
		} else if (eret) {
			if (!streq(ans, "NOMATCH")) {
				if (query)
					skip = note(skip, msg);
				else {
					report("failed", fun, re, s, msg, nmatch);
					sfprintf(sfstdout, "\n");
				}
			}
		} else if (streq(ans, "NOMATCH")) {
			if (query)
				skip = note(skip, msg);
			else {
				report("should fail and didn't: ", fun, re, s, msg, nmatch);
				matchprint(match, nmatch);
			}
		} else if (!*ans) {
			if (match[0] != -2) {
				if (query)
					skip = note(skip, msg);
				else {
					report("no match but match array assigned: ", NiL, re, s, msg, nmatch);
					matchprint(match, nmatch);
				}
			}
		} else if (!matchcheck(nmatch, match, ans, re, s, nmatch, query)) {
			if (eflags ^ (STR_LEFT|STR_RIGHT))
				continue;
			sub = 0;
			goto nosub;
		} else if (query)
			skip = note(skip, msg);
		goto compile;
	}
	sfprintf(sfstdout, "TEST	%s, %d test%s", unit, testno, testno == 1 ? "" : "s");
	if (state.ignore.count)
		sfprintf(sfstdout, ", %d ignored mismatche%s", state.ignore.count, state.ignore.count == 1 ? "" : "s");
	if (state.warnings)
		sfprintf(sfstdout, ", %d warning%s", state.warnings, state.warnings == 1 ? "" : "s");
	sfprintf(sfstdout, ", %d error%s\n", state.errors, state.errors == 1 ? "" : "s");
	return 0;
}
