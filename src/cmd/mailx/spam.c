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
	for (;;)
	{
		while (isspace(*ap))
			ap++;
		if (ae = strchr(ap, ','))
			*ae = 0;
		ap = skin(ap, GDISPLAY|GCOMPARE);
		bp = (char*)b;
		for (;;)
		{
			while (isspace(*bp))
				bp++;
			if (be = strchr(bp, ','))
			{
				*be = 0;
				many = 1;
			}
			bp = skin(bp, GDISPLAY|GCOMPARE);
			if (TRACING('x'))
				note(0, "spam: addr check `%s'  `%s'", ap, bp);
			if (host = *bp == '@' && (tp = strchr(ap, '@')))
				ap = tp + 1;
				bp++;
			for (;;)
			{
				if (!strcasecmp(ap, bp))
				{
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
	for (;;)
	{
		if (ae = strchr(ap, ','))
			*ae = 0;
		if ((ad = strchr(ap, '.')) && !strchr(++ad, '.'))
			ad = 0;
		bp = (char*)b;
		for (;;)
		{
			while (isspace(*bp))
				bp++;
			if (be = strchr(bp, ','))
				*be = 0;
			if (strchr(bp, ' '))
				local = 0;
			else if (bp = strchr(bp, '@'))
			{
				local = 0;
				bp++;
				if (TRACING('x'))
					note(0, "spam: host check  `%s'  `%s'", ap, bp);
				if (!strcasecmp(ap, bp))
					goto hit;
				if (ad)
				{
					if (!strcasecmp(ad, bp))
						goto hit;
					while (bp = strchr(bp, '.'))
					{
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
	for (;;)
	{
		while (isspace(*bb))
			bb++;
		if (!(be = strchr(bb, ',')))
			be = bb + strlen(bb);
		ab = ap = (char*)a;
		while (ap = strchr(ap, *bb))
		{
			if (ap == ab || !isalnum(ap[-1]))
			{
				am = ap;
				bm = bb;
				do
				{
					if (bm >= be)
					{
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
	for (;;)
	{
		while (isspace(*ap))
			ap++;
		if (ae = strchr(ap, ','))
			*ae = 0;
		if (strchr(ap, ' '))
		{
			if (ae)
				*ae = ',';
			return 1;
		}
		ad = strchr(ap, '@');
		bp = (char*)b;
		for (;;)
		{
			while (isspace(*bp))
				bp++;
			if (be = strchr(bp, ','))
				*be = 0;
			if (TRACING('x'))
				note(0, "spam: user match  `%s'  `%s'", ap, bp);
			if (*bp == 0)
				/* skip */;
			else if (*bp == '@')
			{
				bp++;
				for (td = ad; td; td = strchr(td, '.'))
					if (!strcasecmp(++td, bp))
						goto hit;
			}
			else if (!strcasecmp(ap, bp))
				goto hit;
			else if (ad)
			{
				*ad = 0;
				if (to && !strcasecmp(ap, state.var.user))
				{
					*ad = '@';
					if (TRACING('x'))
						note(0, "spam: user addr check `%s' suspect domain", ap);
					goto hit;
				}
				if (!strcasecmp(ap, bp) || strchr(ap, '!'))
				{
					*ad = '@';
					goto hit;
				}
				*ad = '@';
			}
			if (!be)
			{
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
 * check if s cam from inside the domain
 */

static int
insider(register char* s, register char* e, int f, char* d1, int n1, char* d2, int n2)
{
	register int	n;

	if (*s != '.' && !(s = strchr(s, '.')))
		return f;
	if (!e)
		e = s + strlen(s);
	do
	{
		n = e - ++s;
		if (n == n1 && strneq(s, d1, n1) || n == n2 && strneq(s, d2, n2))
			return 1;
	} while (s = strchr(s, '.'));
	return -1;
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
	char*		domain2;
	unsigned long	q;
	unsigned long	x;
	unsigned long	d;
	int		n;
	int		ok;
	int		no;
	int		proper;
	int		ours;
	int		ours2;
	int		fromours;
	long		test;
	struct parse	pp;

	if (!(to = grab(mp, GTO|GCOMPARE|GDISPLAY|GLAST|GUSER, NiL)) || !*to)
	{
		if (TRACING('x'))
			note(0, "spam: To: header missing");
		return 1;
	}
	if ((cc = grab(mp, GCC|GCOMPARE|GDISPLAY, NiL)) && addrmatch(state.var.user, cc))
		return 0;
	if (state.var.spamsub && (s = grab(mp, GSUB|GCOMPARE|GDISPLAY, NiL)) && wordmatch(strlower(s), state.var.spamsub))
		return 1;
	if (headset(&pp, mp, NiL, NiL, NiL, GFROM))
	{
		d = state.var.spamdelay;
		q = 0;
		ok = no = proper = fromours = 0;
		if (state.var.domain)
		{
			ours = strlen(state.var.domain);
			if ((domain2 = strchr(state.var.domain, '.')) && strchr(domain2 + 1, '.'))
				ours2 = strlen(++domain2);
			else
			{
				domain2 = 0;
				ours2 = 0;
			}
		}
		else
		{
			ours = ours2 = 0;
			domain2 = 0;
		}
		test = 0;
		while (headget(&pp))
		{
			t = pp.name;
			if (TRACING('h'))
				note(0, "spam: head: %s: %s", t, pp.data);
			if ((*t == 'X' || *t == 'x') && *(t + 1) == '-')
				t += 2;
			if (*t == 'A' || *t == 'a')
			{
				if (!strcasecmp(t, "Ad") || !strcasecmp(t, "Advertisement"))
				{
					if (TRACING('x'))
						note(0, "spam: advertisement header");
					return 1;
				}
				else if ((TRACING('t') || (state.var.spamtest & 0x0060)) && !strcasecmp(t, "Authentication-Warning"))
				{
					test |= 0x0004;
					if (t = strrchr(pp.data, ' '))
					{
						*t++ = 0;
						if (streq(t, "-f"))
						{
							if ((t = strrchr(pp.data, ' ')) && streq(t + 1, "using") && !(*t = 0) && (t = strrchr(pp.data, ' ')) && insider(t + 1, NiL, 0, state.var.domain, ours, domain2, ours2))
								test |= 0x0040;
						}
						else if (streq(t, "protocol"))
							test |= 0x0020;
					}
				}
			}
			else if (*t == 'C' || *t == 'c')
			{
				if (!strcasecmp(t, "Content-Type"))
				{
					t = skin(pp.data, GDISPLAY|GCOMPARE|GFROM);
					if (TRACING('x'))
						note(0, "spam: content-type `%s'", t);
					if (!strncasecmp(t, "text/html", 9))
						test |= 0x0001;
					if (!strncasecmp(t, "multipart/related", 17))
						test |= 0x0002;
				}
			}
			else if (*t == 'F' || *t == 'f')
			{
				if (!strcasecmp(t, "From"))
				{
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
					if (fromours >= 0)
						fromours = insider(t, NiL, fromours, state.var.domain, ours, domain2, ours2);
				}
			}
			else if (*t == 'M' || *t == 'm')
			{
				if (!strcasecmp(t, "Message-Id"))
				{
					t = skin(pp.data, GDISPLAY|GCOMPARE|GFROM);
					if (TRACING('x'))
						note(0, "spam: message-id `%s'", t);
					if (!*t)
						return 1;
				}
				else if (!strcasecmp(t, "Mime-Autoconverted"))
				{
					if (TRACING('x'))
						note(0, "spam: mime autoconverted");
					test |= 0x0008;
				}
			}
			else if (*t == 'R' || *t == 'r')
			{
				if (!strcasecmp(t, "Received"))
				{
					for (t = pp.data; *t; t++)
					{
						if (*t == 'u')
						{
							if ((t == pp.data || *(t - 1) == '(' || *(t - 1) == ' ') && strneq(t, "unknown ", 8))
							{
								if (TRACING('x'))
									note(0, "spam: unknown host name");
								return 1;
							}
						}
						else if (*t == 'f' && (t == pp.data || *(t - 1) == ' '))
						{
							if (strneq(t, "forged", 6))
							{
								if (TRACING('x'))
									note(0, "spam: forged");
								return 1;
							}
							else if (ours && strneq(t, "from ", 4))
							{
								n = 0;
								e = t;
								while ((s = strchr(e + 1, '.')) && (e = strchr(s, ' ')))
									if (insider(s, e, 0, state.var.domain, ours, domain2, ours2))
									{
										n = 1;
										break;
									}
								if (!n)
								{
									ours = 0;
									if (TRACING('x'))
										note(0, "spam: outsider: %s", pp.data);
								}
							}
						}
					}
#if _PACKAGE_ast
					if (!ours && d && (s = strrchr(pp.data, ';')))
					{
						while (*++s && isspace(*s));
						if (*s)
						{
							x = tmdate(s, NiL, NiL);
							if (q == 0)
								q = x;
							else if (((q > x) ? (q - x) : (x - q)) > d)
							{
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
			else if (*t == 'T' || *t == 't')
			{
				if (!strcasecmp(t, "To"))
				{
					for (t = pp.data; t = strchr(t, ':'); *t = ',');
					s = pp.data;
					do
					{
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
						{
							if (TRACING('x'))
								note(0, "spam: spamtook `%s'", t);
							ok++;
						}
						if (state.var.spamto && usermatch(t, state.var.spamto, state.var.local != 0))
						{
							if (TRACING('x'))
								note(0, "spam: spamto `%s'", t);
							no++;
						}
					} while (s = e);
				}
			}
		}
		if (fromours > 0 && !ours)
			test |= 0x0010;
		if (TRACING('t') || TRACING('x'))
			note(0, "spam: proper=%d ok=%d no=%d test=0x%04x", proper, ok, no, test);
		if (proper)
			return 0;
		if (test & state.var.spamtest)
			return 1;
		if (no > ok)
			return 1;
		if (ok > no)
			return 0;
	}
	if (state.var.local)
	{
		local = state.var.local;
		state.var.local = 0;
		if (!(s = grab(mp, GTO|GCOMPARE|GDISPLAY|GLAST|GUSER, NiL)))
		{
			if (TRACING('x'))
				note(0, "spam: To: header missing");
			state.var.local = local;
			return 1;
		}
		if (hostmatch(local, s))
		{
			if (TRACING('y'))
				note(0, "spam: host ok#%d `%s' `%s'", __LINE__, local, s);
			state.var.local = local;
			return 0;
		}
		if ((s = grab(mp, GCC|GCOMPARE|GDISPLAY, NiL)) && hostmatch(local, s))
		{
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
