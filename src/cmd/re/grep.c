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

static const char usage[] =
"[-?\n@(#)$Id: grep (AT&T Labs Research) 1999-08-11 $\n]"
USAGE_LICENSE
"[+NAME?grep - search lines in files for matching patterns]"
"[+DESCRIPTION?The grep commands search the named input files"
"	for lines containing a match for the given \apatterns\a."
"	Matching lines are printed by default. The standard input is searched"
"	if no files are given or when the file \b-\b is specified.]"
"[+?There are five variants of grep, each one using a different form of"
"	\apattern\a, controlled either by option or the command path"
"	base name. Details of each variant may be found in \bregex\b(3).]"
"	{"
"	[+grep?The default basic regular expressions (no alternations.)]"
"	[+egrep?Extended regular expressions (alternations, one or more.)]"
"	[+xgrep?Augmented regular expressions (conjunction, negation.)]"
"	[+fgrep?Fixed string expressions.]"
"	[+agrep?Approximate regular expressions (not implemented.)]"
"	}"
"[G:basic-regexp?\bgrep\b mode (default): basic regular expression \apatterns\a.]"
"[E:extended-regexp?\begrep\b mode: extended regular expression \apatterns\a.]"
"[X:augmented-regexp?\bxgrep\b mode: augmented regular expression \apatterns\a.]"
"[F:fixed-string?\bfgrep\b mode: fixed string \apatterns\a.]"
"[A:approximate-regexp?\bagrep\b mode: approximate regular expression \apatterns\a (not implemented.)]"

"[C:context?Set the matched line context \abefore\a and \aafter\a count."
"	By default only matched lines are printed.]:?"
"		[before[,after]]:=2,2]"
"[c:count?Only print a matching line count for each file.]"
"[e:expression|pattern|regexp?Specify a matching \apattern\a. More than one"
"	\apattern\a implies alternation. If this option is specified"
"	then the command line \apattern\a must be omitted.]:"
"		[pattern]"
"[f:file?Each line in \apattern-file\a is a \apattern\a, placed into a single"
"	alternating expression.]:"
"		[pattern-file]"
"[H:filename|with-filename?Prefix each matched line with the containing file name.]"
"[h:no-filename?Suppress containing file name prefix for each matched line.]"
"[i:ignore-case?Ignore case when matching.]"
"[l:files-with-matches?Only print file names with at least one match.]"
"[L:files-without-matches?Only print file names with no matches.]"
"[v:invert-match|revert-match?Invert the \apattern\a match sense.]"
"[O:lenient?Enable lenient \apattern\a interpretation. This is the default.]"
"[x:line-match|line-regexp?Force \apatterns\a to match complete lines.]"
"[n:number|line-number?Prefix each matched line with its line number.]"
"[q:quiet|silent?Do not print matching lines.]"
"[S:strict?Enable strict \apattern\a interpretation with diagnostics.]"
"[s:suppress|no-messages?Suppress error and warning messages.]"
"[T:test?Enable implementation specific tests.]:"
"		[test]"
"[w:word-match|word-regexp?Force \apatterns\a to match complete words.]"
"\n"
"\n[ pattern ] [ file ... ]\n"
"\n"
"[+DIAGNOSTICS?Exit status 0 if matches were found, 1 if no matches were found,"
"	where \b-v\b invertes the exit status. Exit status 2 for other"
"	errors that are accompanied by a message on the standard error.]"
"[+SEE ALSO?\bed\b(1), \bsed\b(1), \bregex\b(3)]"
"[+CAVEATS?Some expressions of necessity require exponential space"
"	and/or time.]"
"[+BUGS?Some expressions may use sub-optimal algorithms. For example,"
"	don't use this implementation to compute primes.]"
;

#include <ast.h>
#include <ctype.h>
#include <error.h>
#include <regex.h>
#include <sfstr.h>

#ifndef EISDIR
#define EISDIR		(-1)
#endif

/*
 * snarfed from Doug McElroy's C++ version
 *
 * this grep is based on the Posix re package.
 * unfortunately it has to have a nonstandard interface.
 * 1. fgrep does not have usual operators. REG_LITERAL
 * caters for this.
 * 2. grep allows null expressions, hence REG_NULL.
 * 3. it may be possible to combine the multiple 
 * patterns of grep into single patterns.  important
 * special cases are handled by regcomb().
 * 4. anchoring by -x has to be done separately from
 * compilation (remember that fgrep has no ^ or $ operator),
 * hence REG_LEFT|REG_RIGHT.  (An honest, but slow alternative:
 * run regexec with REG_NOSUB off and nmatch=1 and check
 * whether the match is full length)
 */

typedef struct Item			/* list item			*/
{
	struct Item*	next;		/* next in list			*/
	union
	{
	regex_t		re;		/* re value - sue me for waste	*/
	char*		string;		/* string value			*/
	}		value;
} Item_t;

typedef struct				/* generic list			*/
{
	Item_t*		head;		/* list head			*/
	Item_t*		tail;		/* list tail			*/
} List_t;

static struct				/* program state		*/
{
	struct
	{
	char*		base;		/* sfsetbuf buffer		*/
	size_t		size;		/* sfsetbuf size		*/
	int		noshare;	/* turn off SF_SHARE		*/
	}		buffer;

	List_t		file;		/* pattern file list		*/
	List_t		pattern;	/* pattern list			*/
	List_t		re;		/* re list			*/

	int		hits;		/* if any patterns hit		*/
	int		list;		/* list files with hits		*/
	int		notfound;	/* some input file not found	*/
	int		options;	/* regex options		*/

	unsigned char	count;		/* count number of hits		*/
	unsigned char	match;		/* match sense			*/
	unsigned char	query;		/* return status but no output	*/
	unsigned char	number;		/* line numbers			*/
	unsigned char	prefix;		/* print file prefix		*/
	unsigned char	suppress;	/* no unopenable file messages	*/
	unsigned char	words;		/* word matches only		*/
} state;

static void
addre(List_t* p, char* s)
{
	int	result;
	Item_t*	x;
	Sfio_t*	t;

error(-1, "AHA addre(\"%s\")", s);
	if (!(x = newof(0, Item_t, 1, 0)))
		error(ERROR_SYSTEM|3, "out of space (pattern `%s')", s);
	if (state.words)
	{
		if (!(t = sfstropen()))
			error(ERROR_SYSTEM|3, "out of space (word pattern `%s')", s);
		if (!(state.options & REG_AUGMENTED))
			sfputc(t, '\\');
		sfputc(t, '<');
		sfputr(t, s, -1);
		if (!(state.options & REG_AUGMENTED))
			sfputc(t, '\\');
		sfputc(t, '>');
		s = sfstruse(t);
	}
	else t = 0;
	if (result = regcomp(&x->value.re, s, state.options|REG_MULTIPLE))
		regfatal(&x->value.re, 3, result);
	if (t)
		sfstrclose(t);
	if (!p->head)
		p->head = p->tail = x;
	else if (regcomb(&p->tail->value.re, &x->value.re))
		p->tail = p->tail->next = x;
	else free(x);
}

static void
addstring(List_t* p, char* s)
{
	Item_t*	x;

	if (!(x = newof(0, Item_t, 1, 0)))
		error(ERROR_SYSTEM|3, "out of space (string `%s')", s);
	x->value.string = s;
	if (p->head) p->tail->next = x;
	else p->head = x;
	p->tail = x;
}

static void
compile(void)
{
	int	line;
	size_t	n;
	char*	s;
	char*	t;
	char*	file;
	Item_t*	x;
	Sfio_t*	f;

	for (x = state.pattern.head; x; x = x->next)
		addre(&state.re, x->value.string);
	for (x = state.file.head; x; x = x->next)
	{
		s = x->value.string;
		if (!(f = sfopen(NiL, s, "r")))
			error(ERROR_SYSTEM|4, "%s: cannot open", s);
		else
		{
			file = error_info.file;
			error_info.file = s;
			line = error_info.line;
			error_info.line = 0;
			while (s = (char*)sfreserve(f, SF_UNBOUND, 1))
			{
				if (!(n = sfvalue(f)))
					break;
				if (s[n - 1] != '\n')
				{
					for (t = s + n; t > s && *--t != '\n'; t--);
					if (t == s)
					{
						sfread(f, s, 0);
						break;
					}
					n = t - s + 1;
				}
				s[n - 1] = 0;
				addre(&state.re, s);
				s[n - 1] = '\n';
				sfread(f, s, n);
			}
			while ((s = sfgetr(f, '\n', 1)) || (s = sfgetr(f, '\n', -1)))
			{
				error_info.line++;
				addre(&state.re, s);
			}
			error_info.file = file;
			error_info.line = line;
			sfclose(f);
		}
	}
	if (!state.re.head)
		error(3, "no pattern");
}

static int
record(void* handle, const char* s, size_t len)
{
	unsigned long*	hits = (unsigned long*)handle;

	(*hits)++;
	if (state.query || state.list)
		return -1;
	if (!state.count)
	{
		if (state.prefix)
			sfprintf(sfstdout, "%s:", error_info.file);
		sfwrite(sfstdout, s, len + 1);
	}
	return 0;
}

static void
execute(Sfio_t* input, char* name)
{
	register char*	s;
	char*		file;
	Item_t*		x;
	size_t		len;
	int		result;
	int		line;
	unsigned long	hits = 0;
	
	if (state.buffer.noshare)
		sfset(input, SF_SHARE, 0);
	if (state.buffer.size)
		sfsetbuf(input, state.buffer.base, state.buffer.size);
	if (!name)
		name = "/dev/stdin";
	file = error_info.file;
	error_info.file = name;
	line = error_info.line;
	error_info.line = 0;
	if (state.number || state.re.head->next || !regrecord(&state.re.head->value.re))
	{
		for (;;)
		{
			error_info.line++;
			if (s = sfgetr(input, '\n', 0))
				len = sfvalue(input) - 1;
			else if (s = sfgetr(input, '\n', -1))
			{
				len = sfvalue(input);
				s[len] = '\n';
#if _you_like_the_noise
				error(1, "newline appended");
#endif
			}
			else
			{
				if (sferror(input) && errno != EISDIR)
					error(ERROR_SYSTEM|2, "read error");
				break;
			}
			x = state.re.head;
			do
			{
				if (!(result = regnexec(&x->value.re, s, len, 0, 0, 0)))
					break;
				if (result != REG_NOMATCH)
					regfatal(&x->value.re, 3, result);
			} while (x = x->next);
			if ((x != 0) == state.match)
			{
				hits++;
				if (state.query || state.list)
					break;
				if (!state.count)
				{
					if (state.prefix)
						sfprintf(sfstdout, "%s:", name);
					if (state.number)
						sfprintf(sfstdout, "%d:", error_info.line);
					sfwrite(sfstdout, s, len + 1);
				}
			}
		}
	}
	else
	{
		register char*	e;
		register char*	t;
		char*		x;
		regex_t*	re = &state.re.head->value.re;

		static char*	span = 0;
		static size_t	spansize = 0;

		s = e = 0;
		for (;;)
		{
			if (s < e)
			{
				t = span;
				for (;;)
				{
					len = 2 * (e - s) + t - span + 1;
					len = roundof(len, SF_BUFSIZE);
					if (spansize < len)
					{
						spansize = len;
						len = t - span;
						if (!(span = newof(span, char, spansize, 0)))
							error(ERROR_SYSTEM|3, "%s: line longer than %lu characters", name, len + e - s);
						t = span + len;
					}
					len = e - s;
					memcpy(t, s, len);
					t += len;
					if (!(s = sfreserve(input, SF_UNBOUND, 0)) || (len = sfvalue(input)) <= 0)
					{
						if ((sfvalue(input) || sferror(input)) && errno != EISDIR)
							error(ERROR_SYSTEM|2, "%s: read error", name);
						break;
					}
					else if (!(e = memchr(s, '\n', len)))
						e = s + len;
					else
					{
						x = s + len;
						len = (e - s) + t - span;
						len = roundof(len, SF_BUFSIZE);
						if (spansize < len)
						{
							spansize = len;
							len = t - span;
							if (!(span = newof(span, char, spansize, 0)))
								error(ERROR_SYSTEM|3, "%s: line longer than %lu characters", name, len + e - s);
							t = span + len;
						}
						len = e - s;
						memcpy(t, s, len);
						t += len;
						s += len + 1;
						e = x;
						break;
					}
				}
				*t = '\n';
				if ((result = regrexec(re, span, t - span, 0, 0, state.options, '\n', (void*)&hits, record)) < 0)
					break;
				if (result && result != REG_NOMATCH)
					regfatal(re, 3, result);
				if (!s)
					break;
			}
			else
			{
				if (!(s = sfreserve(input, SF_UNBOUND, 0)))
				{
					if ((sfvalue(input) || sferror(input)) && errno != EISDIR)
						error(ERROR_SYSTEM|2, "%s: read error", name);
					break;
				}
				if ((len = sfvalue(input)) <= 0)
					break;
				e = s + len;
			}
			t = e;
			while (t > s)
				if (*--t == '\n')
				{
					if ((result = regrexec(re, s, t - s, 0, 0, state.options, '\n', (void*)&hits, record)) < 0)
						goto done;
					if (result && result != REG_NOMATCH)
						regfatal(re, 3, result);
					s = t + 1;
					break;
				}
		}
	}
 done:
	error_info.file = file;
	error_info.line = line;
	if (hits && state.list >= 0)
		state.hits = 1;
	if (!state.query)
	{
		if (!state.list)
		{
			if (state.count)
			{
				if (state.prefix)
					sfprintf(sfstdout, "%s:", name);
				sfprintf(sfstdout, "%d\n", hits);
			}
		}
		else if ((hits != 0) == (state.list > 0))
		{
			if (state.list < 0)
				state.hits = 1;
			sfprintf(sfstdout, "%s\n", name);
		}
	}
}

main(int argc, char** argv)
{
	int	c;
	char*	s;
	Sfio_t*	f;

	NoP(argc);
	state.match = 1;
	state.options = REG_NOSUB|REG_NULL;
	if (strcmp(astconf("CONFORMANCE", NiL, NiL), "standard"))
		state.options |= REG_LENIENT;
	if (s = strrchr(argv[0], '/')) s++;
	else s = argv[0];
	switch (*s)
	{
	case 'e':
	case 'E':
		s = "egrep";
		state.options |= REG_EXTENDED;
		break;
	case 'f':
	case 'F':
		s = "fgrep";
		state.options |= REG_LITERAL;
		break;
	case 'x':
	case 'X':
		s = "xgrep";
		state.options |= REG_AUGMENTED;
		break;
	default:
		s = "grep";
		break;
	}
	error_info.id = s;
	while (c = optget(argv, usage))
		switch (c)
		{
		case 'E':
			state.options |= REG_EXTENDED;
			break;
		case 'F':
			state.options |= REG_LITERAL;
			break;
		case 'G':
			state.options &= ~(REG_AUGMENTED|REG_EXTENDED);
			break;
		case 'H':
			state.prefix = opt_info.num;
			break;
		case 'L':
			state.list = -opt_info.num;
			break;
		case 'O':
			state.options |= REG_LENIENT;
			break;
		case 'S':
			state.options &= ~REG_LENIENT;
			break;
		case 'T':
			s = opt_info.arg;
			switch (*s)
			{
			case 'b':
			case 'm':
				c = *s++;
				state.buffer.size = strton(s, &s, NiL, 1);
				if (c == 'b' && !(state.buffer.base = newof(0, char, state.buffer.size, 0)))
					error(ERROR_SYSTEM|3, "out of space [test buffer]");
				if (*s)
					error(3, "%s: invalid characters after test", s);
				break;
			case 'n':
				state.buffer.noshare = 1;
				break;
			default:
				error(3, "%s: unknown test", s);
				break;
			}
			break;
		case 'X':
			state.options |= REG_AUGMENTED;
			break;
		case 'c':
			state.count = 1;
			break;
		case 'e':
			addstring(&state.pattern, opt_info.arg);
			break;
		case 'f':
			addstring(&state.file, opt_info.arg);
			break;
		case 'h':
			state.prefix = 2;
			break;
		case 'i':
			state.options |= REG_ICASE;
			break;
		case 'l':
			state.list = opt_info.num;
			break;
		case 'n':
			state.number = 1;
			break;
		case 'q':
			state.query = 1;
			break;
		case 's':
			state.suppress = opt_info.num;
			break;
		case 'v':
			if (state.match = !opt_info.num)
				state.options &= ~REG_INVERT;
			else
				state.options |= REG_INVERT;
			break;
		case 'w':
			state.words = 1;
			break;
		case 'x':
			state.options |= REG_LEFT|REG_RIGHT;
			break;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			break;
		case ':':
			error(2, "%s", opt_info.arg);
			break;
		default:
			error(3, "%s: not implemented", opt_info.name);
			break;
		}
	argv += opt_info.index;
	if ((state.options & (REG_AUGMENTED|REG_LITERAL)) == (REG_AUGMENTED|REG_LITERAL))
		error(3, "-F and -X are incompatible");
	if ((state.options & (REG_EXTENDED|REG_LITERAL)) == (REG_EXTENDED|REG_LITERAL))
		error(3, "-E and -F are incompatible");
	if ((state.options & REG_LITERAL) && state.words)
		error(ERROR_SYSTEM|3, "-w and -F are incompatible");
	if (!state.file.head && !state.pattern.head)
	{
		if (!argv[0])
			error(3, "no pattern");
		addstring(&state.pattern, *argv++);
	}
	compile();
	if (!argv[0])
	{
		state.prefix = 0;
		execute(sfstdin, NiL);
	}
	else
	{
		if (state.prefix > 1)
			state.prefix = 0;
		else if (argv[1])
			state.prefix = 1;
		while (s = *argv++)
		{
			if (f = sfopen(NiL, s, "r"))
			{
				execute(f, s);
				sfclose(f);
				if (state.query && state.hits)
					break;
			}
			else
			{
				state.notfound = 1;
				if (!state.suppress)
					error(ERROR_SYSTEM|2, "%s: cannot open", s);
			}
		}
	}
	exit((state.notfound && !state.query) ? 2 : !state.hits);
}
