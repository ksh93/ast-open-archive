#pragma prototyped
/*
 * Mail -- a mail program
 *
 * spam heuristics
 */

#include "mailx.h"

#if _PACKAGE_ast
#include <tm.h>
#endif

#define strcasecmp(s,p)	(!strgrpmatch(s,p,NiL,0,STR_ICASE|STR_MAXIMAL|STR_LEFT|STR_RIGHT))

/*
 * Return 1 if the intersection of the <,><space> separated
 * address strings a and b is not empty.
 */
static int
addrmatch(const char* a, const char* b)
{
	register char*	ap;
	register char*	ae;
	register char*	bp;
	register char*	be;
	register char*	tp;
	register int	many = 0;
	register int	host;

	ap = (char*)a;
	for (;;) {
		while (isspace(*ap))
			ap++;
		if (ae = strchr(ap, ','))
			*ae = 0;
		ap = skin(ap, GDISPLAY|GCOMPARE);
		bp = (char*)b;
		for (;;) {
			while (isspace(*bp))
				bp++;
			if (be = strchr(bp, ',')) {
				*be = 0;
				many = 1;
			}
			bp = skin(bp, GDISPLAY|GCOMPARE);
			if (TRACING('x'))
				note(0, "spam: addr check `%s'  `%s'", ap, bp);
			if (host = *bp == '@' && (tp = strchr(ap, '@')))
				ap = tp + 1;
				bp++;
			for (;;) {
				if (!strcasecmp(ap, bp)) {
					if (ae)
						*ae = ',';
					if (be)
						*be = ',';
					else if (many)
						return 0;
					return 1;
				}
				if (!host || !(tp = strchr(ap, '.')))
					break;
				ap = tp + 1;
			}
			if (!be)
				break;
			*be++ = ',';
			bp = be;
		}
		if (!ae)
			break;
		*ae++ = ',';
		ap = ae;
	}
	return 0;
}

/*
 * Return 1 if hosts in list a are part of the address list in b.
 */
static int
hostmatch(const char* a, const char* b)
{
	register char*	ap;
	register char*	ae;
	register char*	ad;
	register char*	bp;
	register char*	be;
	int		local = 1;

	ap = (char*)a;
	for (;;) {
		if (ae = strchr(ap, ','))
			*ae = 0;
		if ((ad = strchr(ap, '.')) && !strchr(++ad, '.'))
			ad = 0;
		bp = (char*)b;
		for (;;) {
			while (isspace(*bp))
				bp++;
			if (be = strchr(bp, ','))
				*be = 0;
			if (strchr(bp, ' '))
				local = 0;
			else if (bp = strchr(bp, '@')) {
				local = 0;
				bp++;
				if (TRACING('x'))
					note(0, "spam: host check  `%s'  `%s'", ap, bp);
				if (!strcasecmp(ap, bp))
					goto hit;
				if (ad) {
					if (!strcasecmp(ad, bp))
						goto hit;
					while (bp = strchr(bp, '.')) {
						bp++;
						if (!strcasecmp(ad, bp))
							goto hit;
					}
				}
			}
			if (!be)
				break;
			bp = be;
			*bp++ = ',';
		}
		if (!ae)
			return local;
		ap = ae;
		*ap++ = ',';
	}
 hit:
	if (be)
		*be = ',';
	if (ae)
		*ae = ',';
	return 1;
}

/*
 * Return 1 if string a contains any words whose prefixes
 * match the <,><space> separated prefix list in b.
 */
static int
wordmatch(const char* a, const char* b)
{
	register char*	ab;
	register char*	ap;
	register char*	am;
	register char*	bb;
	register char*	be;
	register char*	bm;

	bb = (char*)b;
	for (;;) {
		while (isspace(*bb))
			bb++;
		if (!(be = strchr(bb, ',')))
			be = bb + strlen(bb);
		ab = ap = (char*)a;
		while (ap = strchr(ap, *bb)) {
			if (ap == ab || !isalnum(ap[-1])) {
				am = ap;
				bm = bb;
				do {
					if (bm >= be) {
						if (isalnum(*am))
							break;
						if (TRACING('x'))
							note(0, "spam: word match `%-.*s'", bm - bb, bb);
						return 1;
					}
				} while (*bm++ == *am++);
			}
			ap++;
		}
		if (*be == 0)
			break;
		bb = be + 1;
	}
	return 0;
}

/*
 * Return 1 if user a is part of the <,><space> separated address strings in b.
 */
static int
usermatch(const char* a, const char* b, int to)
{
	register char*	ap;
	register char*	ae;
	register char*	ad;
	register char*	bp;
	register char*	be;
	register char*	td;

	ap = (char*)a;
	for (;;) {
		while (isspace(*ap))
			ap++;
		if (ae = strchr(ap, ','))
			*ae = 0;
		if (strchr(ap, ' ')) {
			if (ae)
				*ae = ',';
			return 1;
		}
		ad = strchr(ap, '@');
		bp = (char*)b;
		for (;;) {
			while (isspace(*bp))
				bp++;
			if (be = strchr(bp, ','))
				*be = 0;
			if (TRACING('x'))
				note(0, "spam: user match  `%s'  `%s'", ap, bp);
			if (*bp == '@') {
				bp++;
				for (td = ad; td; td = strchr(td, '.'))
					if (!strcasecmp(++td, bp))
						goto hit;
			}
			else if (!strcasecmp(ap, bp))
				goto hit;
			else if (ad) {
				*ad = 0;
				if (to && !strcasecmp(ap, state.var.user)) {
					*ad = '@';
					if (TRACING('x'))
						note(0, "user addr check `%s' suspect domain", ap);
					goto hit;
				}
				if (!strcasecmp(ap, bp) || strchr(ap, '!')) {
					*ad = '@';
					goto hit;
				}
				*ad = '@';
			}
			if (!be) {
				if (ae)
					*ae = ',';
				break;
			}
			*be++ = ',';
			bp = be;
		}
		if (!ae)
			return 0;
		*ae++ = ',';
	}
 hit:
	if (ae)
		*ae = ',';
	if (be)
		*be = ',';
	return 1;
}

/*
 * Return 1 if it looks like we've been spammed.
 */
int
spammed(register struct msg* mp)
{
	char*		s;
	char*		t;
	char*		e;
	char*		to;
	char*		cc;
	char*		local;
	unsigned long	q;
	unsigned long	x;
	unsigned long	d;
	int		proper;
	int		ours;
	long		test;
	struct parse	pp;

	if (!(to = grab(mp, GTO|GCOMPARE|GDISPLAY|GLAST|GUSER, NiL)) || !*to) {
		if (TRACING('x'))
			note(0, "spam: To: header missing");
		return 1;
	}
	if ((cc = grab(mp, GCC|GCOMPARE|GDISPLAY, NiL)) && addrmatch(state.var.user, cc))
		return 0;
	if (state.var.spamsub && (s = grab(mp, GSUB|GCOMPARE|GDISPLAY, NiL)) && wordmatch(strlower(s), state.var.spamsub))
		return 1;
	if (headset(&pp, mp, NiL, NiL, NiL, GFROM)) {
		d = state.var.spamdelay;
		q = 0;
		proper = 0;
		ours = state.var.domain ? strlen(state.var.domain) : 0;
		test = 0;
		while (headget(&pp)) {
			t = pp.name;
			if ((*t == 'X' || *t == 'x') && *(t + 1) == '-')
				t += 2;
			if (*t == 'A' || *t == 'a') {
				if (!strcasecmp(t, "Ad") || !strcasecmp(t, "Advertisement")) {
					if (TRACING('x'))
						note(0, "spam: advertisement header");
					return 1;
				}
			}
			else if (*t == 'C' || *t == 'c') {
				if (!strcasecmp(t, "Content-Type")) {
					t = skin(pp.data, GDISPLAY|GCOMPARE|GFROM);
					if (TRACING('x'))
						note(0, "spam: test 0x0001: content-type `%s'", t);
					if (!strncasecmp(t, "text/html", 9))
						test |= 0x0001;
				}
			}
			else if (*t == 'F' || *t == 'f') {
				if (!strcasecmp(t, "From")) {
					t = skin(pp.data, GDISPLAY|GCOMPARE|GFROM);
					if (s = strchr(t, ' '))
						*s = 0;
					if (TRACING('x'))
						note(0, "spam: from `%s'", t);
					if (addrmatch(t, state.var.user))
						return 0;
					if (state.var.spamfromok && usermatch(t, state.var.spamfromok, 0))
						return 0;
					if (state.var.spamfrom && usermatch(t, state.var.spamfrom, 0))
						return 1;
					if (addrmatch(t, to))
						return 1;
				}
			}
			else if (*t == 'M' || *t == 'm') {
				if (!strcasecmp(t, "Message-Id")) {
					t = skin(pp.data, GDISPLAY|GCOMPARE|GFROM);
					if (TRACING('x'))
						note(0, "spam: message-id `%s'", t);
					if (!*t)
						return 1;
				}
			}
			else if (*t == 'R' || *t == 'r') {
				if (!strcasecmp(t, "Received")) {
					for (t = pp.data; *t; t++) {
						if (*t == 'u') {
							if ((t == pp.data || *(t - 1) == '(' || *(t - 1) == ' ') && strneq(t, "unknown ", 8)) {
								if (TRACING('x'))
									note(0, "spam: unknown host name");
								return 1;
							}
						}
						else if (*t == 'f' && (t == pp.data || *(t - 1) == ' ')) {
							if (strneq(t, "forged", 6)) {
								if (TRACING('x'))
									note(0, "spam: forged");
								return 1;
							}
							else if (ours && strneq(t, "from ", 6) && (s = strchr(t, '.')) && (e = strchr(++s, ' ')) && ((e - s) != ours || !strneq(s, state.var.domain, ours))) {
								ours = 0;
								if (TRACING('x'))
									note(0, "spam: %*.*s: not ours", e - s, e - s, s);
							}
						}
					}
#if _PACKAGE_ast
					if (!ours && d && (s = strrchr(pp.data, ';'))) {
						while (*++s && isspace(*s));
						if (*s) {
							x = tmdate(s, NiL, NiL);
							if (q == 0)
								q = x;
							else if (((q > x) ? (q - x) : (x - q)) > d) {
								if (TRACING('x'))
									note(0, "spam: delay %ld", (q > x) ? (q - x) : (x - q));
								return 1;
							}
							q = x;
						}
					}
#endif
				}
			}
			else if (*t == 'T' || *t == 't') {
				if (!strcasecmp(t, "To")) {
					for (t = pp.data; t = strchr(t, ':'); *t = ',');
					s = pp.data;
					do {
						if (e = strchr(s, ','))
							*e++ = 0;
						t = skin(s, GDISPLAY|GCOMPARE);
						if (TRACING('x'))
							note(0, "spam: to `%s'", t);
						if (*t == 0)
							return 1;
						if (addrmatch(state.var.user, t))
							proper = 1;
						if (state.var.spamtook && usermatch(t, state.var.spamtook, state.var.local != 0))
							return 0;
						if (state.var.spamto && usermatch(t, state.var.spamto, state.var.local != 0))
							return 1;
					} while (s = e);
				}
			}
		}
		if (proper)
			return 0;
		if (test & state.var.spamtest)
			return 1;
	}
	if (state.var.local) {
		local = state.var.local;
		state.var.local = 0;
		if (!(s = grab(mp, GTO|GCOMPARE|GDISPLAY|GLAST|GUSER, NiL))) {
			if (TRACING('x'))
				note(0, "spam: To: header missing");
			state.var.local = local;
			return 1;
		}
		if (hostmatch(local, s)) {
			if (TRACING('y'))
				note(0, "spam: host ok#%d `%s' `%s'", __LINE__, local, s);
			state.var.local = local;
			return 0;
		}
		if ((s = grab(mp, GCC|GCOMPARE|GDISPLAY, NiL)) && hostmatch(local, s)) {
			if (TRACING('y'))
				note(0, "spam: host ok#%d `%s' `%s'", __LINE__, local, s);
			state.var.local = local;
			return 0;
		}
		state.var.local = local;
		return 1;
	}
	return 0;
}
