#pragma prototyped
/*
 * Mail -- a mail program
 *
 * Auxiliary functions.
 *
 * So why isn't this stuff still in the file aux.c?
 * What file base name is special on what system.
 * I mean really special.
 * Any wagers on whether aux is in the POSIX conformance test suite?
 */

#include "mailx.h"

#include <stdarg.h>

/*
 * Note message.
 */
void
note(register int flags, const char* fmt, ...)
{
	register FILE*	fp;
	va_list		ap;

	va_start(ap, fmt);
	if ((flags & DEBUG) && !state.var.debug)
		return;
	if (flags & (ERROR|PANIC)) {
		fp = stderr;
		fflush(stdout);
	}
	else
		fp = stdout;
	if (state.var.coprocess)
		fprintf(fp, "%s ", state.var.coprocess);
	if (flags & IDENTIFY)
		fprintf(fp, "mail: ");
	if (flags & PANIC)
		fprintf(fp, T("panic: "));
	else if (flags & WARNING)
		fprintf(fp, T("warning: "));
	else if (flags & DEBUG)
		fprintf(fp, T("debug: "));
	vfprintf(fp, T(fmt), ap);
	va_end(ap);
	if (flags & SYSTEM)
		fprintf(fp, ": %s", strerror(errno));
	if (!(flags & PROMPT))
		fprintf(fp, "\n");
	fflush(fp);
	if (flags & PANIC)
		abort();
	if (flags & (FATAL|PANIC))
		exit(1);
}

/*
 * Return a pointer to a dynamic copy of the argument.
 */
char*
savestr(char* str)
{
	char*	p;
	int	size = strlen(str) + 1;

	if ((p = salloc(size)))
		memcpy(p, str, size);
	return p;
}

/*
 * Touch the named message by setting its MTOUCH flag.
 * Touched messages have the effect of not being sent
 * back to the system mailbox on exit.
 */
void
touchmsg(register struct msg* mp)
{
	if (mp->m_flag & MREAD)
		msgflags(mp, MTOUCH, 0);
	else
		msgflags(mp, MREAD|MSTATUS, 0);
}

/*
 * Test to see if the passed file name is a directory.
 */
int
isdir(char* name)
{
	struct stat	st;

	if (!name || stat(name, &st) < 0)
		return 0;
	return S_ISDIR(st.st_mode);
}

/*
 * Test to see if the passed file name is a regular file.
 */
int
isreg(char* name)
{
	struct stat	st;

	if (!name || stat(name, &st) < 0)
		return 0;
	return S_ISREG(st.st_mode);
}

/*
 * The following code deals with input stacking to do source
 * commands.  All but the current file pointer are saved on
 * the stack.
 */

/*
 * Pushdown current input file and switch to a new one.
 * Set the global flag "sourcing" so that others will realize
 * that they are no longer reading from a tty (in all probability).
 */
int
source(char** arglist)
{
	FILE*	fp;

	if (!(fp = fileopen(*arglist, "EXr")))
		return 1;
	if (state.source.sp >= NOFILE - 1) {
		note(0, "Too much \"sourcing\" going on");
		fileclose(fp);
		return 1;
	}
	state.source.stack[state.source.sp].input = state.input;
	state.source.stack[state.source.sp].cond = state.cond;
	state.source.stack[state.source.sp].loading = state.loading;
	state.source.sp++;
	state.loading = 0;
	state.cond = 0;
	state.input = fp;
	state.sourcing++;
	return 0;
}

/*
 * Pop the current input back to the previous level.
 * Update the "sourcing" flag as appropriate.
 */
int
unstack(void)
{
	if (state.source.sp <= 0) {
		note(0, "\"Source\" stack over-pop");
		state.sourcing = 0;
		return 1;
	}
	fileclose(state.input);
	if (state.cond)
		note(0, "Unmatched \"if\"");
	state.source.sp--;
	state.cond = state.source.stack[state.source.sp].cond;
	state.loading = state.source.stack[state.source.sp].loading;
	state.input = state.source.stack[state.source.sp].input;
	if (state.source.sp == 0)
		state.sourcing = state.loading;
	return 0;
}

/*
 * Touch the indicated file.
 * This is nifty for the shell.
 */
void
alter(char* name)
{
	touch(name, (time_t)0, (time_t)(-1), 0);
}

/*
 * Examine the passed line buffer and
 * return true if it is all blanks and tabs.
 */
int
blankline(char* linebuf)
{
	register char*	cp;

	for (cp = linebuf; *cp; cp++)
		if (*cp != ' ' && *cp != '\t')
			return 0;
	return 1;
}

/*
 * Start of a "comment".
 * Ignore it.
 */
static char*
skip_comment(register char* cp)
{
	register int	nesting = 1;

	for (; nesting > 0 && *cp; cp++) {
		switch (*cp) {
		case '\\':
			if (cp[1])
				cp++;
			break;
		case '(':
			nesting++;
			break;
		case ')':
			nesting--;
			break;
		}
	}
	return cp;
}

/*
 * shorten host if it is part of the local domain
 */

char*
localize(char* host)
{
	register char*	lp;
	register char*	le;
	register char*	hx;

	hx = strchr(host, '.');
	lp = state.var.local;
	for (;;) {
		if (le = strchr(lp, ','))
			*le = 0;
		if (!strcasecmp(lp, host)) {
			if (le)
				*le = ',';
			return 0;
		}
		if (hx && !strcasecmp(lp, hx + 1)) {
			*hx = 0;
			if (le)
				*le = ',';
			return host;
		}
		if (!(lp = le))
			break;
		*lp++ = ',';
	}
	return host;
}

/*
 * apply GCOMPARE, GDISPLAY, state.var.allnet, state.var.local
 */

char*
normalize(char* addr, char* buf, unsigned long type)
{
	register char*	p;
	register int	n;
	char*		uucp;
	char*		arpa;
	char*		user;
	char*		inet;
	int		hadarpa;
	int		hadinet;
	char		temp[LINESIZE];
	char		norm[LINESIZE];

	if (!(type & GFROM) && (p = strrchr(addr, ':')))
		addr = p + 1;
	if ((type & GCOMPARE) && state.var.allnet) {
		if (p = strrchr(addr, '!'))
			addr = p + 1;
		if ((p = strchr(addr, '%')) || (p = strchr(addr, '@'))) {
			if (buf) {
				n = p - addr;
				addr = (char*)memcpy(buf, addr, n);
				p = addr + n;
			}
			*p = 0;
			return addr;
		}
	}
	else {
		user = strcpy(temp, addr);
		uucp = arpa = inet = 0;
		hadarpa = hadinet = 0;
		if (p = strrchr(user, '!')) {
			uucp = user;
			*p++ = 0;
			user = p;
			if (p = strrchr(uucp, '!'))
				p++;
			if (p && (type & GDISPLAY)) {
				uucp = p;
				p = 0;
			}
			if (p && state.var.local)
				uucp = localize(p);
		}
		if (p = strchr(user, '@')) {
			hadinet = 1;
			*p++ = 0;
			inet = state.var.local ? localize(p) : p;
		}
		if (p = strchr(user, '%')) {
			hadarpa = 1;
			*p++ = 0;
			if (!(type & (GCOMPARE|GDISPLAY)))
				arpa = state.var.local ? localize(p) : p;
		}
		if (uucp &&
		    (hadinet || (inet && streq(uucp, inet)) ||
		    (hadarpa || arpa && streq(uucp, arpa))))
			uucp = 0;
		if (arpa && (hadinet || inet && streq(arpa, inet)))
			arpa = 0;
		if (type & GDISPLAY) {
			if (inet)
				uucp = 0;
			else if (uucp) {
				inet = uucp;
				uucp = 0;
			}
		}
		p = norm;
		if (uucp) {
			p = strcopy(p, uucp);
			p = strcopy(p, "!");
		}
		p = strcopy(p, user);
		if (arpa) {
			p = strcopy(p, "%");
			p = strcopy(p, arpa);
		}
		if (inet) {
			p = strcopy(p, "@");
			p = strcopy(p, inet);
		}
		if (!streq(addr, norm))
			return savestr(norm);
	}
	return buf ? (char*)0 : (type & GSTACK) ? savestr(addr) : addr;
}

/*
 * Skin an arpa net address according to the RFC 822 interpretation
 * of "host-phrase."
 */
char*
skin(char* name, unsigned long type)
{
	register int	c;
	register char*	cp;
	register char*	cp2;
	char*		bufend;
	int		gotlt;
	int		lastsp;
	char		buf[LINESIZE];

	if (!name)
		return 0;
	if (type & (GMESSAGEID|GREFERENCES))
		return savestr(name);
	if (!strchr(name, '(') && !strchr(name, '<') && !strchr(name, ' '))
		return normalize(name, NiL, type);
	gotlt = 0;
	lastsp = 0;
	bufend = buf;
	for (cp = name, cp2 = bufend; c = *cp++; ) {
		switch (c) {
		case '(':
			cp = skip_comment(cp);
			lastsp = 0;
			break;

		case '"':
			/*
			 * Start of a "quoted-string".
			 * Copy it in its entirety.
			 */
			while (c = *cp) {
				cp++;
				if (c == '"')
					break;
				if (c != '\\')
					*cp2++ = c;
				else if (c = *cp) {
					*cp2++ = c;
					cp++;
				}
			}
			lastsp = 0;
			break;

		case ' ':
			if (cp[0] == 'a' && cp[1] == 't' && cp[2] == ' ')
				cp += 3, *cp2++ = '@';
			else if (cp[0] == '@' && cp[1] == ' ')
				cp += 2, *cp2++ = '@';
			else
				lastsp = 1;
			break;

		case '<':
			cp2 = bufend;
			gotlt++;
			lastsp = 0;
			break;

		case '>':
			if (gotlt) {
				gotlt = 0;
				while ((c = *cp) && c != ',') {
					cp++;
					if (c == '(')
						cp = skip_comment(cp);
					else if (c == '"')
						while (c = *cp) {
							cp++;
							if (c == '"')
								break;
							if (c == '\\' && *cp)
								cp++;
						}
				}
				lastsp = 0;
				break;
			}
			/* Fall into . . . */

		default:
			if (lastsp) {
				lastsp = 0;
				*cp2++ = ' ';
			}
			*cp2++ = c;
			if (c == ',' && !gotlt) {
				*cp2++ = ' ';
				for (; *cp == ' '; cp++)
					;
				lastsp = 0;
				bufend = cp2;
			}
		}
	}
	*cp2 = 0;
	return normalize(buf, NiL, type|GSTACK);
}

/*
 * Are any of the characters in the two strings the same?
 */
int
anyof(register char* s1, register char* s2)
{

	while (*s1)
		if (strchr(s2, *s1++))
			return 1;
	return 0;
}

/*
 * Convert c to lower case
 */
int
lower(register int c)
{
	return isupper(c) ? tolower(c) : c;
}

/*
 * Convert c to upper case
 */
int
upper(register int c)
{
	return islower(c) ? toupper(c) : c;
}

/*
 * Convert s to lower case
 */
char*
strlower(register char* s)
{
	register char*	b = s;
	register int	c;

	while (c = *s)
		*s++ = isupper(c) ? tolower(c) : c;
	return b;
}

/*
 * See if the given header field is supposed to be ignored.
 */
int
ignored(Dt_t** ignore, const char* field)
{
	struct name*	tp;

	if (ignore == &state.ignoreall)
		return 1;
	tp = dictsearch(ignore, field, LOOKUP);
	if (*ignore && (dictflags(ignore) & RETAIN))
		return !tp || !(tp->flags & RETAIN);
	return tp && (tp->flags & IGNORE);
}

/*
 * Allocate size more bytes of space and return the address of the
 * first byte to the caller.  An even number of bytes are always
 * allocated so that the space will always be on a word boundary.
 * The string spaces are of exponentially increasing size, to satisfy
 * the occasional user with enormous string size requests.
 *
 * Strings handed out here are reclaimed at the top of the command
 * loop each time, so they need not be freed.
 */

char*
salloc(register int size)
{
	register char*			t;
	register struct strings*	sp;
	int				index;

	if (state.onstack <= 0) {
		if (!(t = newof(0, char, size, 0)))
			note(PANIC, "Out of space");
		return t;
	}
	size += 7;
	size &= ~7;
	index = 0;
	for (sp = &state.stringdope[0]; sp < &state.stringdope[elementsof(state.stringdope)]; sp++) {
		if (!sp->s_topfree && (STRINGSIZE << index) >= size)
			break;
		if (sp->s_nleft >= size)
			break;
		index++;
	}
	if (sp >= &state.stringdope[elementsof(state.stringdope)])
		note(PANIC, "String too large");
	if (!sp->s_topfree) {
		index = sp - &state.stringdope[0];
		sp->s_topfree = (char*)malloc(STRINGSIZE << index);
		if (!sp->s_topfree)
			note(PANIC, "No room for dynamic string space %d", index);
		sp->s_nextfree = sp->s_topfree;
		sp->s_nleft = STRINGSIZE << index;
	}
	sp->s_nleft -= size;
	t = sp->s_nextfree;
	sp->s_nextfree += size;
	return t;
}

/*
 * Reset the string area to be empty.
 * Called to free all strings allocated
 * since last reset.
 */
void
sreset(void)
{
	register struct strings*	sp;
	register int			index;

	if (state.noreset)
		return;
	index = 0;
	for (sp = &state.stringdope[0]; sp < &state.stringdope[elementsof(state.stringdope)]; sp++) {
		if (!sp->s_topfree)
			continue;
		sp->s_nextfree = sp->s_topfree;
		sp->s_nleft = STRINGSIZE << index;
		index++;
	}
	dictreset();
}

/*
 * Return lines/chars for display.
 */
char*
counts(int wide, off_t lines, off_t chars)
{
	sprintf(state.counts, wide ? "%5ld/%-7ld" : "%3ld/%-5ld", (long)lines, (long)chars);
	return state.counts;
}

/*
 * Check if s matches `all'.
 */
int
isall(register const char* s)
{
	return s && (streq(s, "all") || streq(s, "*"));
}

/*
 * Check if name is a pipe command.
 */
char*
iscmd(register char* s)
{
	if (!s)
		return 0;
	while (isspace(*s))
		s++;
	if (*s != '!' && *s != '|')
		return 0;
	do {
		if (!*++s)
			return 0;
	} while (isspace(*s));
	return s;
}

/*
 * Set/Clear message flags
 */

void
msgflags(register struct msg* mp, int set, int clr)
{
	if (state.folder == FIMAP)
		imap_msgflags(mp, set, clr);
	else {
		if (clr)
			mp->m_flag &= ~clr;
		if (set)
			mp->m_flag |= set;
	}
}
