/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1984-2000 AT&T Corp.              *
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
 * Glenn Fowler
 * AT&T Research
 *
 * make variable expansion routines
 */

#include "make.h"
#include "expand.h"

#include <regex.h>

#define BREAKARGS	100
#define BREAKLINE	(BREAKARGS*16)
#define EDITCONTEXT	20

/*
 * `$(...)' expansion
 *
 * each <var> is expanded before the value is determined
 * each <op> is applied to blank separated tokens in the variable value
 * $$(...) expands to $(...) -- one $ gobbled each time
 *
 * general syntax:
 *
 *	$(<var>[|<var>][:[<pfx>]<op>[<sep><val>]])
 *
 * top level alternate syntax:
 *
 *	$(<var>[|<var>]`[<del>[<pfx>]<op>[<sep><val>]]<del>)
 *
 * variable name syntax:
 *
 *	$(<var>)		value of <var>
 *	$(<var1>|<var2>)	value of <var1> if non-null else value of <var2>
 *	$("<string>")		<string> is used as the variable value
 *
 * edit operator syntax:
 *
 *	:<op><sep><value>:
 *	:/<old>/<new>/<glb>:
 *	:C<del><old><del><new><del><glb>:
 *	:?<if-non-null>?<if-null>?:
 *	:Y<del><if-non-null><del><if-null><del>:
 *
 *
 * edit operator forms:
 *
 *	$(s)		simple expansion (multiple character name)
 *	$(s:@<op>...)	don't tokenize list elements for <op>
 *	$(s:A=a[|b])	list of rules with attribute a [or b ...]
 *	$(s:F=<fmt>)	format components according to <fmt>
 *	$(s:G=<pat>)	select components that build (generate) <pat> files
 *	$(s:H[>])	sort [hi to lo]
 *	$(s:I=<lst>)	directory intersection of s with list <lst>
 *	$(s:K=<pfx>)	break into `<pfx> s ...' ~ BREAK(ARGS|LINE) line chunks
 *	$(s:L[=<pat>)	list all (viewed) files matching <pat>
 *	$(s:M[!]=<pat>)	select components [not] matching RE pattern <pat>
 *	$(s:N[!]=<pat>)	select components [not] matching shell pattern <pat>
 *	$(s:O<sep><n>)	select components <|<=|==|!=|>=|> <n> starting at 1
 *	$(s:P=<op>)	path name operations (see below)
 *	$(s:Q)		quote shell chars in value
 *	$(s:R)		read value as makefile
 *	$(s:T=<op>[r])	select components matching rule token <op> (see below)
 *	$(s:V)		do not expand value (prefix)
 *	$(s:W?[=<arg>])	ops on entire value
 *	$(s:X=<lst>)	directory cross product of components of s and <lst>
 *	$(s:Z)		expand in parent reference frame (prefix cumulative)
 *	$(s:f)		include or modify file name components (see below)
 *
 * token op components:
 *
 *	A	archive (returns archive symbol table update command)
 *	D	definition of state variable: -Dx[=y]
 *	E	alternate definition of state variable: x=[y]
 *	F	file
 *	G	built (generated) file
 *	I[-]	value is [non]expanded contents of bound file
 *	Q	select defined atoms
 *	QV	select defined variables
 *	P	physically a file (not a symbolic link ...)
 *	R	relative time since epoch
 *	Sc	return staterule name given non-state rule
 *	U	return variable or non-state name given state rule
 *	W	component prefix to not wait for bind
 *	X	component prefix to skip binding
 *	*	any
 *
 * token op forms:
 *
 *	$(s:T=t?ret)	ret if true else null
 *
 * format forms:
 *
 *	L	convert to lower case
 *	U	convert to upper case
 *	%n.nc	printf(3) format
 *
 * path name operations:
 *
 *	A	absolute (rooted) path name
 *	B	physical binding
 *	C	canonicalize path name
 *	D	generate directory where s was bound
 *	I=n	files identical to n
 *	L[=n]	return s if bound in view level 0 [n]
 *	P=l	probe info file for language l processor (in token)
 *	R[=d]	relative dir path to d [pwd]
 *	S	atoms bound in subdirectory of view
 *	U	unbound name
 *	V	generate view directory path where s was bound
 *	X	return s if it is an existing file
 *
 * file name components (any or all may be null):
 *
 *	D	directory	up to and including the last '/'
 *	B	base		after D up to but not including the last '.'
 *	S	suffix		after B
 *
 * a component (DBS) is deleted if the corresponding char is omitted
 * a component (DBS) is retained if the corresponding char is specified
 * a component (DBS) is changed if the corresponding char is followed
 *	by '=' and a replacement value
 *
 * if '=' is specified then a ':' separates the value from the next
 *	file name component specification
 */

/*
 * return edit map for *p delimited by space or del
 * 0 for no match otherwise *p points to del or op arg
 */

static Edit_map_t*
getedit(char** p, int del)
{
	register int		v;
	register unsigned char*	s;
	register unsigned char*	t;
	register Edit_map_t*	mid = (Edit_map_t*)editmap;
	register Edit_map_t*	lo = mid;
	register Edit_map_t*	hi = mid + elementsof(editmap) - 1;

	while (lo <= hi)
	{
		mid = lo + (hi - lo) / 2;
		s = (unsigned char*)*p;
		t = (unsigned char*)mid->name;
		for (;;)
		{
			if (!*t && (*s == del || isspace(*s) || !*s))
			{
				while (isspace(*s)) s++;
				*p = (char*)s;
				return mid;
			}
			if ((v = *s++ - *t++) > 0)
			{
				lo = mid + 1;
				break;
			}
			else if (v < 0)
			{
				hi = mid - 1;
				break;
			}
		}
	}
	return 0;
}

/*
 * expand one instance of v not in w into xp
 * (sep & NOT) for file equality
 */

static void
uniq(Sfio_t* xp, char* v, char* w, int sep)
{
	register char*	s;
	char*		tok;
	Hash_table_t*	tab;
	struct fileid	id;
	struct stat	st;

	tok = tokopen(w, 1);
	if (sep & NOT)
	{
		tab = hashalloc(table.dir, 0);
		while (s = tokread(tok))
			if (!stat(s, &st))
			{
				id.dev = st.st_dev;
				id.ino = st.st_ino;
				hashput(tab, (char*)&id, (char*)tab);
			}
		tokclose(tok);
		sep = 0;
		tok = tokopen(v, 1);
		while (s = tokread(tok))
			if (!stat(s, &st))
			{
				id.dev = st.st_dev;
				id.ino = st.st_ino;
				if (!hashget(tab, (char*)&id))
				{
					hashput(tab, (char*)0, (char*)tab);
					if (sep)
						sfputc(xp, ' ');
					else
						sep = 1;
					sfputr(xp, s, -1);
				}
			}
	}
	else
	{
		tab = hashalloc(table.rule, 0);
		while (s = tokread(tok))
			hashput(tab, s, (char*)tab);
		tokclose(tok);
		sep = 0;
		tok = tokopen(v, 1);
		while (s = tokread(tok))
			if (!hashget(tab, s))
			{
				hashput(tab, (char*)0, (char*)tab);
				if (sep)
					sfputc(xp, ' ');
				else
					sep = 1;
				sfputr(xp, s, -1);
			}
	}
	hashfree(tab);
	tokclose(tok);
}

/*
 * expand the directory cross product of v and w into xp
 *
 * memorize your multiplication table:
 *
 *	.	unit multiplication operand
 *	A	absolute path rooted at /
 *	R	path relative to .
 *
 *	lhs	rhs	cross-product
 *	----	-----	-------------
 *	.	.	.	*note (1)*
 *	.	A	A	*note (2)*
 *	.	R	R
 *	A	.	A
 *	A	R	A/R
 *	A	A	A	*note (2)*
 *	R	.	R
 *	R	A	A	*note (2)*
 *	R	R	R/R
 *
 *	(1) the first . lhs operand produces a . in the product
 *
 *	(2) the first A rhs operand is placed in the product
 */

static void
cross(Sfio_t* xp, char* v, char* w)
{
	register char*	s;
	register char*	t;
	char*		x;
	int		dot;
	int		sep;
	long		pos;
	char*		tok0;
	char*		tok1;
	Sfio_t*		tmp;

	tmp = sfstropen();
	expand(tmp, w);
	w = sfstruse(tmp);
	sep = 0;
	tok0 = tokopen(w, 0);
	while (t = tokread(tok0))
	{
		if (*t == '/')
		{
			if (sep) sfputc(xp, ' ');
			else sep = 1;
			pos = sfstrtell(xp);
			sfputr(xp, t, 0);
			x = sfstrset(xp, pos);
			pos += canon(x) - x;
			sfstrset(xp, pos);
		}
		else
		{
			dot = (*t == '.' && !*(t + 1));
			tok1 = tokopen(v, 1);
			while (s = tokread(tok1))
			{
				if (sep) sfputc(xp, ' ');
				else sep = 1;
				pos = sfstrtell(xp);
				x = t;
				if (dot || *s != '.' || *(s + 1))
				{
					if (!dot) sfprintf(xp, "%s/", s);
					else x = s;
				}
				sfputr(xp, x, 0);
				x = sfstrset(xp, pos);
				pos += canon(x) - x;
				sfstrset(xp, pos);
			}
			tokclose(tok1);
		}
	}
	tokclose(tok0);
	sfstrclose(tmp);
}

/*
 * expand the pathname intersection of v with w into xp
 * sep!=EQ for literal intersection
 */

static void
intersect(Sfio_t* xp, char* v, char* w,int sep)
{
	register struct list*	p;
	register struct rule*	r;
	register char*		s;
	register int		n;
	struct list*		q;
	struct list*		x;
	char*			tok;
	Sfio_t*			tmp;

	tmp = sfstropen();
	p = q = 0;
	tok = tokopen(v, 1);
	while (s = tokread(tok))
	{
		canon(s);
		r = makerule(s);
		if (p) p = p->next = cons(r, NiL);
		else p = q = cons(r, NiL);
		s = r->name;
		if (sep == EQ)
		{
			if ((r->dynamic & D_alias) && (!state.context || !iscontext(r->uname)) && (r = makerule(s)))
				p = p->next = cons(r, NiL);
			if (state.expandview && state.fsview)
			{
				if (s[0] == '.' && !s[1]) sfputr(tmp, "...", -1);
				else sfprintf(tmp, "%s/...", s);
				s = sfstruse(tmp);
				while (!mount(s, s, FS3D_GET|FS3D_VIEW|FS3D_SIZE(MAXNAME), NiL))
				{
					r = makerule(s);
					p = p->next = cons(r, NiL);
					strcpy(s + strlen(s), "/...");
				}
			}
		}
	}
	tokclose(tok);
	expand(tmp, w);
	for (x = q; x; x = x->next)
		x->rule->mark |= M_mark|M_metarule;
	tok = tokopen(sfstruse(tmp), 0);
	while (s = tokread(tok))
	{
		canon(s);
		if (r = getrule(s))
		{
			r->mark &= ~M_mark;
			if (sep == EQ)
			{
				if (!(r->mark & M_metarule) && r->name[0] == '.' && r->name[1] == '.' && (!r->name[2] || r->name[2] == '/'))
				{
					r->mark |= M_metarule;
					if (p) p = p->next = cons(r, NiL);
					else p = q = cons(r, NiL);
					internal.dot->mark &= ~M_mark;
				}
				if ((r->dynamic & D_alias) && (r = makerule(r->name)))
				{
					r->mark &= ~M_mark;
					if (!(r->mark & M_metarule) && r->name[0] == '.' && r->name[1] == '.' && (!r->name[2] || r->name[2] == '/'))
					{
						r->mark |= M_metarule;
						p = p->next = cons(r, NiL);
						internal.dot->mark &= ~M_mark;
					}
				}
			}
		}
	}
	n = 0;
	for (p = q; p; p = p->next)
		if (!(p->rule->mark & M_mark))
		{
			if (n) sfputc(xp, ' ');
			else n = 1;
			sfputr(xp, state.localview ? localview(p->rule) : p->rule->name, -1);
			p->rule->mark |= M_mark;
		}
	tokclose(tok);
	sfstrclose(tmp);
	for (p = q; p; p = p->next)
		p->rule->mark &= ~(M_mark|M_metarule);
	freelist(q);
}


/*
 * expand the rules in v that have any of the prereqs in w
 */

static void
hasprereq(Sfio_t* xp, char* v, char* w)
{
	register struct list*	p;
	register struct list*	q;
	register struct rule*	r;
	register char*		s;
	int			sep;
	struct list*		x;
	char*			tok;

	x = 0;
	tok = tokopen(w, 1);
	while (s = tokread(tok))
		x = cons(makerule(s), x);
	tokclose(tok);
	sep = 0;
	tok = tokopen(v, 1);
	while (s = tokread(tok))
	{
		if (r = getrule(s))
			for (p = r->prereqs; p; p = p->next)
				for (q = x; q; q = q->next)
					if (p->rule == q->rule)
					{
						if (sep) sfputc(xp, ' ');
						else sep = 1;
						sfputr(xp, r->name, -1);
						goto next;
					}
	next:;
	}
	tokclose(tok);
	freelist(x);
}

/*
 * break into ~ BREAK(ARGS|LINE) `<pfx> s ...' line chunks
 */

static void
linebreak(Sfio_t* xp, register char* s, char* pfx)
{
	register int	a;
	char*		tok;
	long		rew;
	long		pre;
	long		pos;
	long		brk;

	rew = sfstrtell(xp);
	sfputr(xp, pfx, -1);
	pre = pos = sfstrtell(xp);
	brk = pos + BREAKLINE;
	a = state.mam.out ? 30 : BREAKARGS;
	tok = tokopen(s, 1);
	while (s = tokread(tok))
	{
		if (pos >= brk || !a--)
		{
			pos += sfprintf(xp, "\n%s", pfx);
			brk = pos + BREAKLINE;
			a = BREAKARGS;
		}
		pos += sfprintf(xp, " %s", s);
	}
	tokclose(tok);
	if (pos == pre) sfstrset(xp, rew);
}

/*
 * inverted strcoll(3)
 */

static int
istrcoll(const char* a, const char* b)
{
	return strcoll(b, a);
}

/*
 * numeric strcmp(3)
 */

static int
numcmp(const char* a, const char* b)
{
	long	na = strton(a, NiL, NiL, 0);
	long	nb = strton(b, NiL, NiL, 0);

	if (na < nb) return -1;
	if (na > nb) return 1;
	return 0;
}

/*
 * inverted numeric strcmp(3)
 */

static int
inumcmp(const char* a, const char* b)
{
	return numcmp(b, a);
}

/*
 * generate list of file base names from all dirs in s matching pat
 * sep&GT sorts hi to lo
 * sep&LT sorts lo to hi
 * sep&EQ lists all base names -- otherwise only first in order
 */

static void
list(Sfio_t* xp, register char* s, char* pat, int sep)
{
	register struct rule*	r;
	register char**		v;
	char**			w;
	struct file*		f;
	Hash_position_t*	pos;
	char*			tok;
	int			n;
	int			ignorecase;
	Sfio_t*			vec;
	Sfio_t*			hit;
	regex_t*		re;
	regex_t			sre;
	regex_t			ire;

	if (*pat && (n = regcomp(&sre, pat, REG_SHELL|REG_AUGMENTED|REG_LENIENT|REG_LEFT|REG_RIGHT)))
	{
		regfatalpat(&sre, 2, n, pat);
		return;
	}
	ignorecase = 0;
	vec = sfstropen();
	hit = sfstropen();

	/*
	 * generate and bind (scan) the ordered dir list
	 */

	tok = tokopen(s, 1);
	while (s = tokread(tok))
	{
		r = makerule(s);
		if (!(r->mark & M_mark))
		{
			r->mark |= M_mark;
			if (!(r->dynamic & D_scanned))
				dirscan(r);
			putptr(vec, r);
		}
	}
	putptr(vec, 0);
	tokclose(tok);
	for (v = (char**)sfstrbase(vec); r = (struct rule*)*v;)
	{
		r->mark &= ~M_mark;
		*v++ = r->name;
	}

	/*
	 * scan the file hash for pattern and dir matches
	 */

	if (pos = hashscan(table.file, 0))
	{
		while (hashnext(pos))
		{
			if ((s = pos->bucket->name)[0] != '.' || s[1] && (s[1] != '.' || s[2]))
			{
				f = (struct file*)pos->bucket->value;
				if (*pat)
				{
					if (f->dir->ignorecase)
					{
						re = &ire;
						if (!ignorecase)
						{
							if (n = regcomp(re, pat, REG_SHELL|REG_AUGMENTED|REG_LENIENT|REG_ICASE|REG_LEFT|REG_RIGHT))
							{
								regfatalpat(re, 2, n, pat);
								break;
							}
							ignorecase = 1;
						}
					}
					else
						re = &sre;
					if (n = regexec(re, s, 0, NiL, 0))
					{
						if (n != REG_NOMATCH)
						{
							regfatal(re, 2, n);
							break;
						}
						continue;
					}
				}
				for (; f; f = f->next)
					for (v = (char**)sfstrbase(vec); s = *v++;)
						if (f->dir->name == s)
						{
							putptr(hit, pos->bucket->name);
							goto next;
						}
			}
		next:	;
		}
		hashdone(pos);
	}

	/*
	 * sort and fill the output buffer
	 */

	if (sfstrtell(hit) > 0)
	{
		n = sfstrtell(hit);
		putptr(hit, 0);
		v = (char**)sfstrbase(hit);
		if (sep & (LT|GT))
			strsort(v, n / sizeof(v), (sep & GT) ? istrcoll : strcoll);
		if (sep & EQ)
		{
			sfputr(xp, *v, -1);
			while (*++v)
				sfprintf(xp, " %s", *v);
		}
		else
		{
			for (w = (char**)sfstrbase(vec); s = *w++;)
				for (v = (char**)sfstrbase(hit); *v; v++)
					for (f = getfile(*v); f; f = f->next)
						if (f->dir->name == s)
						{
							sfputr(xp, *v, -1);
							goto first;
						}
		first:	;
		}
	}
	sfstrclose(vec);
	sfstrclose(hit);
	if (*pat)
	{
		regfree(&sre);
		if (ignorecase)
			regfree(&ire);
	}
}

/*
 * sort list s into xp
 * sep&GT sorts hi to lo
 * otherwise sorts lo to hi
 * sep&EQ does numeric sort
 * otherwise string sort
 * sep&NOT uniques after sort
 *
 * NOTE: s modified in place and not restored
 */

static void
sort(Sfio_t* xp, register char* s, int sep)
{
	register char**	p;
	char*		tok;
	long		n;
	Sfio_t*		vec;

	vec = sfstropen();
	tok = tokopen(s, 0);
	while (s = tokread(tok))
		putptr(vec, s);
	tokclose(tok);
	if (n = sfstrtell(vec))
	{
		putptr(vec, 0);
		if (!(sep & (GT|LT)))
			sep &= NOT;
		strsort((char**)sfstrbase(vec), n / sizeof(s), (sep & GT) ? ((sep & EQ) ? inumcmp : istrcoll) : ((sep & EQ) ? numcmp : strcoll));
		p = (char**)sfstrbase(vec);
		sfputr(xp, *p, -1);
		sep &= NOT;
		while (s = *++p)
			if (!sep || !streq(s, *(p - 1)))
				sfprintf(xp, " %s", s);
	}
	sfstrclose(vec);
}

/*
 * construct relative path from dir s to t in xp
 */

static void
relative(Sfio_t* xp, register char* s, register char* t)
{
	register char*	u;
	register char*	v;
	long		pos;
	Sfio_t*		tmp;

	tmp = sfstropen();
	if (*s != '/') sfprintf(tmp, "%s/", internal.pwd);
	sfprintf(tmp, "%s/", s);
	s = sfstruse(tmp);
	pos = pathcanon(s, 0) - s + 1;
	sfstrset(tmp, pos);
	if (*t != '/') sfprintf(tmp, "%s/", internal.pwd);
	sfprintf(tmp, "%s/%c", t, 0);
	pathcanon(v = t = sfstrset(tmp, pos), 0);
	u = s = sfstrbase(tmp);
	while (*s && *s == *t)
		if (*s++ == '/')
		{
			u = s;
			v = ++t;
		}
		else t++;
	pos = sfstrtell(xp);
	while (*u)
		if (*u++ == '/')
			sfputr(xp, "../", -1);
	sfputr(xp, v, -1);
	if (sfstrtell(xp) > pos + 1) sfstrrel(xp, -1);
	else sfputc(xp, '.');
	sfstrclose(tmp);
}

/*
 * apply binary separator operator to a and b
 */

static int
sepcmp(int sep, unsigned long a, unsigned long b)
{
	switch (sep)
	{
	case LT:
		return a < b;
	case LE:
		return a <= b;
	case EQ:
		return a == b;
	case NE:
	case NOT:
		return a != b;
	case GE:
		return a >= b;
	case GT:
		return a > b;
	default:
#if DEBUG
		error(PANIC, "invalid separator operator %d", sep);
#endif
		return 0;
	}
}

/*
 * convert path to native representation with '..' quotes if needed
 */

#if _UWIN

extern int	uwin_path(const char*, char*, int);

static const void
native(Sfio_t* xp, const char* s)
{
	int	m;
	int	n;

	if (*s)
	{
		sfputc(xp, '\'');
		n = PATH_MAX;
		do
		{
			m = n;
			n = uwin_path(s, sfstrrsrv(xp, m), m);
		} while (n > m);
		sfstrrel(xp, n);
		sfputc(xp, '\'');
	}
}

#else

#define native(xp,s)	sfputr(xp,s,-1)

#endif

/*
 * low level for order()
 */

static void
descend(Sfio_t* xp, struct rule* r, int all)
{
	register struct list*	p;
	int			w;

	w = 0;
	r->mark &= ~M_mark;
	for (p = r->prereqs; p; p = p->next)
		if (p->rule->mark & M_mark)
		{
			w = 1;
			descend(xp, p->rule, all);
		}
	if (all)
	{
		if (w)
			sfputr(xp, "-", ' ');
		sfputr(xp, r->name, ' ');
	}
}

#define ORDER_INIT	"INIT"		/* no prereq dir prefix		*/
#define ORDER_PACKAGE	"PACKAGE"	/* package assertion operator	*/

/*
 * scan the makefiles in s and put the ordered list in xp
 */

static void
order(Sfio_t* xp, register char* s, char* val)
{
	register struct rule**	v;
	register struct rule*	r;
	register struct rule*	q;
	char*			tok;
	char*			t;
	char*			u;
	int			i;
	int			j;
	int			k;
	int			p;
	Sfio_t*			vec;
	Sfio_t*			sp;
	Hash_table_t*		tab;

	tab = hashalloc(table.rule, 0);
	vec = sfstropen();
	tok = tokopen(s, 1);
	while (s = tokread(tok))
	{
		r = makerule(s);
		putptr(vec, r);
		if (s = strrchr(r->name, '/'))
		{
			*s = 0;
			r = makerule(r->name);
			*s = '/';
			if (s = strrchr(r->name, '/'))
				s++;
			else
				s = r->name;
			if (strneq(s, ORDER_INIT, sizeof(ORDER_INIT) - 1))
			{
				sfputr(xp, r->name, ' ');
				sfputr(xp, "-", ' ');
			}
			else
				r->mark |= M_mark;
			hashput(tab, s, r);
		}
		else
		{
			r = internal.dot;
			r->mark |= M_mark;
		}
		putptr(vec, r);
	}
	putptr(vec, 0);
	tokclose(tok);
	v = (struct rule**)sfstrbase(vec);
	while (q = *v++)
	{
		r = *v++;
		if ((r->mark & M_mark) && (sp = sfopen(NiL, bind(q)->name, "r")))
		{
			while (s = sfgetr(sp, '\n', 1))
			{
				j = p = 0;
				while (*s)
				{
					for (k = 1; (i = *s) == ' ' || i == '\t' || i == '"' || i == '\''; s++);
					for (t = s; (i = *s) && i != ' ' && i != '\t' && i != '"' && i != '\'' && i != '\\' && i != ':'; s++)
						if (i == '/')
							t = s + 1;
						else if (i == '.' && t[0] == 'l' && t[1] == 'i' && t[2] == 'b')
							*s = 0;
					if (*s)
						*s++ = 0;
					if (!t[0])
						k = 0;
					else if ((t[0] == '-' || t[0] == '+') && t[1] == 'l' && t[2])
					{
						sfprintf(internal.nam, "lib%s", t + 2);
						t = sfstruse(internal.nam);
					}
					else if (p)
					{
						if (t[0] == '+' && !t[1])
							p = 2;
						else if (p == 1)
						{
							sfprintf(internal.nam, "lib%s", t);
							t = sfstruse(internal.nam);
						}
					}
					else if (i == ':')
					{
						if (j != ':' || !isupper(*t))
							k = 0;
						else if (streq(t, ORDER_PACKAGE))
						{
							p = 1;
							k = 0;
						}
						else
							for (u = t; *u; u++)
								if (isupper(*u))
									*u = tolower(*u);
								else if (!isalnum(*u))
								{
									k = 0;
									break;
								}
					}
					else if (t[0] != 'l' || t[1] != 'i' || t[2] != 'b')
						k = 0;
					else
						for (u = t + 3; *u; u++)
							if (!isalnum(*u))
							{
								k = 0;
								break;
							}
					if (k && (q = (struct rule*)hashget(tab, t)) && (q->mark & M_mark) && q != r)
						addprereq(r, q, PREREQ_APPEND);
					j = i;
				}
			}
			sfclose(sp);
		}
	}
	if (val)
	{
		tok = tokopen(val, 1);
		while (s = tokread(tok))
			if ((r = (struct rule*)hashget(tab, s)) && (r->mark & M_mark))
				descend(xp, r, 1);
		tokclose(tok);
		k = 0;
	}
	else
		k = 1;
	v = (struct rule**)sfstrbase(vec);
	while (*v++)
	{
		r = *v++;
		if (r->mark & M_mark)
			descend(xp, r, k);
	}
	sfstrclose(vec);
	hashfree(tab);
}

/*
 * path name operations from (rule) s into xp using op
 */

static void
pathop(Sfio_t* xp, register char* s, char* op, int sep)
{
	register char*		t;
	register int		n;
	register struct rule*	r;
	register char**		p;
	char*			e;
	struct rule*		x;
	int			c;
	int			i;
	int			root;
	struct stat		st;
	long			pos;

	n = islower(*op) ? toupper(*op) : *op;
	if (r = getrule(s))
	{
		if (r->dynamic & D_alias)
			switch (n)
			{
			case 'B':
			case 'D':
			case 'P':
			case 'Z':
				break;
			default:
				r = makerule(r->name);
				break;
			}
		s = r->name;
	}
	switch (n)
	{
	case 'A':
	absolute:
		/*
		 * construct absolute pathname for s
		 */

		if (*s)
		{
			pos = sfstrtell(xp);
			if (*s == '/')
				sfputr(xp, s, 0);
			else
				sfprintf(xp, "%s/%s%c", internal.pwd, s, 0);
			s = sfstrset(xp, pos);
			pos += canon(s) - s;
			sfstrset(xp, pos);
		}
		return;
	case 'B':
		/*
		 * return physical binding for name
		 */

		if (r && (r->dynamic & D_bound))
		{
			if ((t = getbound(s)) && (r = getrule(t)) && (r->dynamic & D_regular))
				s = state.localview ? localview(makerule(t)) : t;
			sfputr(xp, s, -1);
		}
		return;
	case 'C':
		/*
		 * canonicalize path name
		 */

		if (*s)
		{
			pos = sfstrtell(xp);
			sfputr(xp, s, 0);
			s = sfstrset(xp, pos);
			pos += canon(s) - s;
			sfstrset(xp, pos);
		}
		return;
	case 'D':
		/*
		 * generate directory where s was bound
		 */

		sep = 0;
		if (!r || !r->time || (r->property & P_state) || r->status == IGNORE) break;
		if ((t = getbound(r->name)) || (s = r->uname) && (t = getbound(s)))
		{
			if ((x = getrule(t)) && (x->dynamic & (D_entries|D_scanned)) == (D_entries|D_scanned) || *t == '/' && !*(t + 1))
				s = 0;
			else if (s = strrchr(t, '/'))
				*s = 0;
			else
				t = ".";
			x = makerule(t);
			if (s)
				*s = '/';
			s = (!(state.questionable & 0x00008000) && *r->name == '/') ? r->uname : (char*)0;
			sfputr(xp, state.localview ? localview(x) : x->name, -1);
			sep = 1;
			if (!s)
				return;
		}
		if (s)
		{
			if ((n = strlen(r->name)) > (c = strlen(s))) for (;;)
			{
				if (*(t = r->name + n - c - 1) == '/' && streq(s, t + 1))
				{
					*t = 0;
					r = makerule(r->name);
					*t = '/';
					if (sep)
						sfputc(xp, ' ');
					sfputr(xp, state.localview ? localview(r) : r->name, -1);
					return;
				}
				if (!(t = strchr(s, '/')))
				{
#if DEBUG
					message((-2, "pathop('%c',%s==%s): cannot find '/'", *op, r->name, r->uname));
#endif
					break;
				}
				c -= ++t - s;
				s = t;
			}
#if DEBUG
			else message((-2, "pathop('%c',%s==%s): bound shorter than unbound", *op, r->name, r->uname));
#endif
		}
		if (*r->name != '/')
		{
			if (sep)
				sfputc(xp, ' ');
			sfputc(xp, '.');
		}
		return;
	case 'G':
		sep = 0;
		for (p = globv(s); *p; p++)
		{
			if (sep)
				sfputc(xp, ' ');
			else
				sep = 1;
			sfputr(xp, *p, -1);
		}
		return;
	case 'H':
		/*
		 * generate 14 char hash of s with op suffix
		 */

		 sfprintf(xp, "M%08lX", strhash(s));
		 if (*++op == '=')
			op++;
		 if ((n = strlen(op)) > 5)
			n = 5;
		 while (n++ < 5 && (c = *s++))
			if (istype(n, C_VARIABLE1|C_VARIABLE2))
				sfputc(xp, c);
		 n = 0;
		 while (n++ < 5 && (c = *op++))
			sfputc(xp, c);
		 return;
	case 'I':
		/*
		 * return bound name if identical to op
		 * or return inode number
		 */

		if (*s)
		{
			struct stat	st1;

			if (*++op == '=')
				op++;
			if (!*op)
				sfprintf(xp, "%lu", stat(s, &st) ? 0L : st.st_ino);
			else if ((!stat(s, &st) && !stat(op, &st1) && st.st_dev == st1.st_dev && st.st_ino == st1.st_ino) == (sep == EQ))
				sfputr(xp, s, -1);
		}
		return;
	case 'L':
		if (!r || !(r->dynamic & D_bound) || !r->time)
			/* ignore */;
		else if (*++op == '*' || *op == '!' || *op == '=' && (*(op + 1) == '*' || *(op + 1) == '!'))
		{
			/*
			 * if bound then return top and covered view names
			 */

			if (*op == '=')
				op++;
			sep = 0;
			if (!state.maxview)
				goto absolute;
			else if (state.fsview)
			{
				sfstrrsrv(internal.nam, MAXNAME + 5);
				t = sfstruse(internal.nam);
				strcpy(t, r->name);
				for (n = 0;;)
				{
					if (mount(t, t, FS3D_GET|FS3D_VIEW|FS3D_SIZE(MAXNAME), NiL))
						break;
					if (sep)
						sfputc(xp, ' ');
					else
						sep = 1;
					sfputr(xp, t + ((!n++ && !strncmp(t, internal.pwd, internal.pwdlen) && t[internal.pwdlen] == '/') ? (internal.pwdlen + 1) : 0), -1);
					if (*op != '*')
						break;
					if (n > 64)
					{
						error(1, "%s: view loop", t);
						break;
					}
					strcpy(t + strlen(t), "/...");
				}
			}
			else
			{
				root = 0;
				s = r->name;
				if (n = r->view)
				{
					c = state.view[n].rootlen;
					if (!strncmp(s, state.view[n].root, c) && (!s[c] || s[c] == '/') && *(s += c))
					{
						s++;
						root = 1;
					}
				}
				else if (*s == '/')
				{
					for (i = 0;; i++)
					{
						if (i > state.maxview)
							break;
						if (!strncmp(s, state.view[i].root, n = state.view[i].rootlen) && (!*(s + n) || *(s + n) == '/'))
						{
							if (!*(s += n) || !*++s)
								s = internal.dot->name;
							n = i;
							root = 1;
							break;
						}
					}
				}
				if (*s == '/')
					goto absolute;
				for (; n <= state.maxview; n++)
				{
					if (root)
						sfprintf(internal.nam, "%s/%s", state.view[n].root, s);
					else
					{
						if (*state.view[n].path != '/')
							sfprintf(internal.nam, "%s/", internal.pwd);
						sfprintf(internal.nam, "%s", state.view[n].path);
						if (*s)
							sfprintf(internal.nam, "/%s", s);
					}
					t = sfstruse(internal.nam);
					pathcanon(t, 0);
					if (!access(t, 0))
					{
						if (sep)
							sfputc(xp, ' ');
						else
							sep = 1;
						sfputr(xp, t, -1);
						if (*op != '*')
							break;
					}
				}
			}
		}
		else
		{
			/*
			 * return bound name if bound in view level 0 [n]
			 */

			n = (*op == '=') ? (int)strtol(op + 1, NiL, 0) : 0;
			if (sepcmp(sep, (unsigned long)((r->dynamic & D_global) ? state.maxview : r->view), (unsigned long)n))
				sfputr(xp, s, -1);
		}
		return;
	case 'N':
		native(xp, s);
		return;
	case 'P':
		if (*++op == '=')
			op++;
		if ((t = strchr(op, ',')) || (t = strchr(op, ' ')))
			*t++ = 0;
		if (s = pathprobe(sfstrrsrv(xp, MAXNAME), NiL, op, t ? t : idname, s, 0))
		{
			sfstrrel(xp, strlen(s));
			makerule(s)->dynamic |= D_built|D_global;
		}
		if (t)
			*--t = 0;
		return;
	case 'R':
		if (*++op == '=')
			op++;
		relative(xp, s, op);
		return;
	case 'S':
		/*
		 * return bound name if bound in subdirectory in view
		 */

		if (r && (r->dynamic & D_bound))
		{
			if (*++op == '=')
				op++;
			if (!r->view || state.fsview && !state.expandview)
			{
				if (r->dynamic & D_global)
					return;
				if (*op)
				{
					sfprintf(internal.nam, "%s/%s", op, s);
					s = sfstruse(internal.nam);
					pathcanon(s, 0);
				}
				if (*s++ == '.' && *s++ == '.' && (*s == '/' || !*s))
					return;
			}
			else
			{
				n = state.view[r->view].pathlen;
				if (*op)
				{
					for (n = 1; op = strchr(op, '/'); n++, op++);
					for (t = state.view[r->view].path + n; t > state.view[r->view].path && (*t != '/' || --n > 0); t--);
					n = t - state.view[r->view].path;
				}
				if (strncmp(s, state.view[r->view].path, n) || *(s + n) != '/')
					return;
			}
			sfputr(xp, r->name, -1);
		}
		return;
	case 'U':
		/*
		 * return unbound name
		 */

		sfputr(xp, (r && !(r->property & P_state) && !(r->dynamic & D_alias)) ? unbound(r) : s, -1);
		return;
	case 'V':
		if (!r || !(r->dynamic & D_bound) || !r->time)
			return;
		if (*++op)
		{
			/*
			 * return the top view logical name of s
			 */

			s = r->name;
			if (*op == '=')
				op++;
			if (strtol(op, &e, 0) || *e)
			{
				error(2, "%s: view %s not supported", s, op);
				return;
			}
			else if (state.maxview && !state.fsview && r->view)
			{
				c = state.view[r->view].pathlen;
				if (!strncmp(s, state.view[r->view].path, c) && (!s[c] || s[c] == '/') && *(s += c))
					s++;
			}
		}
		else
		{
			/*
			 * return view directory path of s
			 */

			s = state.view[r->view].path;
		}
		sfputr(xp, s, -1);
		return;
	case 'W':
		/*
		 * return license info
		 */

		if (*++op == '=')
			op++;
		n = 8 * 1024;
		if ((n = astlicense(sfstrrsrv(xp, n), n, s, op, '/', '*', '/')) < 0)
			error(2, "license: %s", sfstrrel(xp, 0));
		else if (n > 0 && *(sfstrrel(xp, n) - 1) == '\n')
			sfstrrel(xp, -1);
		return;
	case 'X':
		/*
		 * return s if it is an existing file
		 * op[1] == 'P' does physical test
		 */

		if ((*(op + 1) == 'P' ? !lstat(s, &st) : !access(s, 0)) == (sep == EQ))
			sfputr(xp, s, -1);
		return;
	case 'Z':
		/*
		 * return the longer of the bound name and the alias name
		 */

		 if (r)
			sfputr(xp, !(r->dynamic & D_alias) || !r->uname || strlen(r->name) >= strlen(r->uname) ? r->name : r->uname, -1);
		 return;
	default:
		error(1, "invalid path name operator `%c'", *op);
		return;
	}
}

/*
 * edit a single (expanded) file name s into xp
 *
 * each file component (described above) is modified as follows:
 *
 *	KEEP		component is kept unchanged
 *	DELETE		component is deleted
 *	<string>	component is changed to <string>
 */

void
edit(Sfio_t* xp, register char* s, char* dir, char* bas, char* suf)
{
	register char*	p;
	register char*	q;
	long		pos;

	if (!*s) return;
	pos = sfstrtell(xp);

	/*
	 * directory
	 */

	q = dir;
	if (q != DELETE && q != KEEP && (!*q || *q == '.' && !*(q + 1))) q = DELETE;
	if (p = strrchr(s, '/'))
	{
		if (q == KEEP) while (s <= p) sfputc(xp, *s++);
		else s = ++p;
	}
	if (q != DELETE && q != KEEP)
	{
		sfputr(xp, q, -1);
		if (*q && *(sfstrrel(xp, 0) - 1) != '/') sfputc(xp, '/');
	}

	/*
	 * base
	 */

	q = bas;
	if (!(p = strrchr(s, '.')) || p == s) p = s + strlen(s);
	else while (p > s && *(p - 1) == '.') p--;
	if (q == KEEP) while (s < p) sfputc(xp, *s++);
	else s = p;
	if (q != DELETE && q != KEEP) sfputr(xp, q, -1);

	/*
	 * suffix
	 */

	q = suf;
	if (*p && q == KEEP) sfputr(xp, s, -1);
	else if (q != DELETE && q != KEEP) sfputr(xp, q, -1);

	/*
	 * cleanup
	 */

	p = sfstrbase(xp) + pos;
	q = sfstrrel(xp, 0);
	while (q > p && *(q - 1) == '/') q--;
	pos = q - sfstrbase(xp);
	sfstrset(xp, pos);
}

/*
 * substitute a single (expanded) name s into xp
 */

static void
substitute(Sfio_t* xp, regex_t* re, register char* s, char* newp, int glob)
{
	int		n;
	regmatch_t	match[10];

	if (*s)
	{
		if (!(n = regexec(re, s, elementsof(match), match, 0)))
			n = regsub(re, xp, s, newp, elementsof(match), match, glob);
		if (n)
		{
			if (n != REG_NOMATCH)
				regfatal(re, 2, n);
			sfputr(xp, s, -1);
		}
	}
}

/*
 * apply token op p on (possibly bound) s with result in xp
 *
 * NOTE: this and :D:B:S: were the first edit operators
 */

static void
token(Sfio_t* xp, char* s, register char* p, int sep)
{
	register struct rule*	r;
	register struct rule*	x;
	register struct var*	v;
	register int		op;
	char*			ops;
	int			dobind;
	int			dowait;
	int			force;
	int			matched;
	int			tst;
	unsigned long		tm;
	struct list*		q;
	struct list*		z;
	struct stat		st;
	Sfio_t*			sp;
	Sfio_t*			tmp = 0;

	dobind = 1;
	dowait = 1;
	while (op = *p)
	{
		p++;
		if (islower(op))
			op = toupper(op);
		if (op == 'W')
			dowait = 0;
		else if (op == 'X')
			dobind = 0;
		else
			break;
	}
	ops = p;
	while (*p && *p++ != '?');
	switch (op)
	{
	case 'N':
	case 'V':
		if ((*s == 0) == ((op == 'V') == (sep == EQ)))
			/*NOP*/;
		else if (*p)
			expand(xp, p);
		else
			sfputc(xp, '1');
		return;
	case 'Q':
		if ((*ops == 'V' ? (getvar(s) != 0) : (getrule(s) != 0)) == (sep == EQ))
			sfputr(xp, s, -1);
		return;
	}
	r = makerule(s);
	if (dobind)
	{
		tst = state.mam.regress && !(r->dynamic & D_bound);
		r = bind(r);
		if (tst && !(r->dynamic & D_built))
			sfprintf(state.mam.out, "%sbind %s\n", state.mam.label, mamname(r));
		if (op == 'F' && r->status == MAKING && dowait)
		{
			struct frame*	fp;

			/*
			 * don't wait for targets in the active frames
			 */

			for (fp = state.frame; fp->target != r; fp = fp->parent)
				if (fp == fp->parent)
				{
					complete(r, NiL, NiL, 0);
					break;
				}
		}
	}
	if (!*r->name)
	{
		switch (op)
		{
		case 'R':
			if (*ops)
				sfputr(xp, fmttime(ops, CURTIME), -1);
			else
				sfprintf(xp, "%lu", CURTIME);
			break;
		}
		return;
	}
	tst = (notfile(r) || !r->time || r->status == IGNORE || state.exec && r->status != NOTYET && (x = staterule(RULE, r, NiL, 0)) && !x->time || (r->dynamic & (D_member|D_membertoo)) == D_member) ? 0 : 'F';
	switch (op)
	{
	case 0:
	case '*':
		matched = 1;
		break;
	case 'A':
		if (r->scan != SCAN_IGNORE)
		{
			closear(openar(r->name, "r"));
			if (internal.arupdate)
			{
				struct frame*	oframe;
				struct frame	frame;

				oframe = state.frame;
				if (!(state.frame = r->active))
				{
					zero(frame);
					frame.target = r;
					state.frame = frame.parent = &frame;
				}
				expand(xp, internal.arupdate);
				state.frame = oframe;
			}
		}
		return;
	case 'D':
	case 'E':
		if ((r->property & (P_parameter|P_statevar)) == P_statevar && (v = varstate(r, 0)))
		{
			if (*(p = v->value))
			{
				tmp = sfstropen();
				expand(tmp, p);
				p = sfstruse(tmp);
			}
			if (state.localview > 1)
				localvar(xp, v, p, op == 'D' ? V_local_D : V_local_E);
			else if (*p)
			{
				if (op == 'D')
				{
					if (*p != '-' || isdigit(*(p + 1)))
						sfprintf(xp, "-D%s", v->name);
					else
						op = 0;
					if (*p != '1' || *(p + 1))
					{
						if (!op)
							sfputr(xp, p, -1);
						else
						{
							sfputc(xp, '=');
							shquote(xp, p);
						}
					}
				}
				else
				{
					sfprintf(xp, "%s=", v->name);
					if (*p)
						shquote(xp, p);
				}
			}
			if (tmp)
				sfstrclose(tmp);
		}
		return;
	case 'G':
		if (tst != 'F' || (r->property & (P_target|P_terminal)) == P_terminal)
		{
			matched = 0;
			break;
		}
		matched = 1;
		if ((r->dynamic & D_built) || (x = staterule(RULE, r, NiL, 0)) && (x->dynamic & D_built))
			break;

		/*
		 * the remaining checks are necessary because of
		 * state.accept | state.ignorestate
		 * the tests are conservative, allowing some built
		 * files to go undetected
		 */

		if (r->property & P_target)
		{
			if (r->action || (r->property & (P_archive|P_command)) || !(state.questionable & 0x00004000) && (r->dynamic & D_dynamic))
				break;
			for (z = r->prereqs; z; z = z->next)
				if (z->rule->property & P_use) break;
			if (z)
				break;
		}
		tmp = sfstropen();
		matched = 0;
		for (z = internal.metarule->prereqs; z; z = z->next)
		{
			char	stem[MAXNAME];

			if (metamatch(stem, unbound(r), z->rule->name) && (x = metainfo('I', z->rule->name, NiL, 0)))
				for (q = x->prereqs; q; q = q->next)
				{
					metaexpand(tmp, stem, q->rule->name);
					if ((x = bindfile(NiL, sfstruse(tmp), 0)) && (x->time || (x->property & P_target)))
					{
						matched = 1;
						break;
					}
				}
		}
		sfstrclose(tmp);
		break;
	case 'I':
		if (!(sp = sfopen(NiL, r->name, "r")))
			error(2, "%s: cannot read", r->name);
		else
		{
			switch (*ops)
			{
			case '-':
			case 'X':
			case 'x':
				tmp = xp;
				break;
			default:
				tmp = sfstropen();
				break;
			}
			sfmove(sp, tmp, SF_UNBOUND, -1);
			sfclose(sp);
			s = sfstrbase(tmp);
			p = s + sfstrtell(tmp);
			while (p > s && *(p - 1) == '\n')
				p--;
			sfstrset(tmp, p - s);
			if (tmp != xp)
			{
				expand(xp, sfstruse(tmp));
				sfstrclose(tmp);
			}
		}
		return;
	case 'M':
		if (!*ops)
			ops = " ";
		else if (*ops == '=' && !*++ops)
			ops = " : ";
		parentage(xp, r, ops);
		return;
	case 'O':
		p = "w";
		sep = '\n';
		tst = 1;
		for (;; ops++)
		{
			switch (*ops)
			{
			case '+':
			case 'A':
			case 'a':
				p = "a";
				continue;
			case '-':
			case 'N':
			case 'n':
				sep = -1;
				continue;
			case 'X':
			case 'x':
				tst = 0;
				continue;
			}
			break;
		}
		if (*ops == '=')
			ops++;
		if (!(sp = sfopen(NiL, r->name, p)))
			error(2, "%s: cannot write", r->name);
		else
		{
			if (tst)
				sfputr(sp, ops, sep);
			else
			{
				tmp = sfstropen();
				expand(tmp, ops);
				if (sep != -1) sfputc(tmp, sep);
				sep = sfstrtell(tmp);
				sfwrite(sp, sfstrbase(tmp), sep);
				sfstrclose(tmp);
			}
			if (sferror(sp))
				error(ERROR_SYSTEM|2, "%s: write error", r->name);
			sfclose(sp);
		}
		return;
	case 'P':
		matched = (tst == 'F' && (lstat(r->name, &st) || !S_ISLNK(st.st_mode)));
		break;
	case 'R':
		if (*ops)
			sfputr(xp, fmttime(ops, r->time), -1);
		else
			sfprintf(xp, "%lu", r->time);
		return;
	case 'S':
		op = *ops++;
		if (islower(op))
			op = toupper(op);
		if (force = op == 'F')
		{
			op = *ops++;
			if (islower(op))
				op = toupper(op);
		}
		if (!op)
		{
			matched = (r->property & P_state) != 0;
			break;
		}
		x = 0;
		switch (op)
		{
		case 'A':
			if (r->property & P_staterule)
				x = rulestate(r, force);
			break;
		case 'M':
			x = metarule(r->name, ops, force);
			break;
		case 'P':
			x = staterule(PREREQS, r, NiL, force);
			break;
		case 'R':
			x = staterule(RULE, r, NiL, force);
			break;
		case 'V':
			if (!(r->property & P_state))
				x = staterule(VAR, NiL, r->name, force);
			break;
		default:
			error(2, "%c: unknown state op", op);
			break;
		}
		if (x)
			sfputr(xp, x->name, -1);
		return;
	case 'T':
		if (!*ops)
			tm = state.frame->target->time;
		else
		{
			tm = strtoul(ops, &s, 10);
			if (*s)
			{
				if (x = getrule(ops))
				{
					if (dobind)
						x = bind(x);
					tm = x->time;
				}
				else tm = 0;
			}
		}
		if (sepcmp(sep, (unsigned long)r->time, tm))
			sfputr(xp, r->name, -1);
		return;
	case 'U':
		sfputr(xp, (r->property & P_staterule) ? rulestate(r, 1)->name : (r->property & P_statevar) ? varstate(r, 1)->name : r->name, -1);
		return;
	case 'Z':
		switch (*ops++)
		{
		case 'C':
			tm = (x = staterule(PREREQS, r, NiL, 0)) ? x->time : 0;
			break;
		case 'E':
			tm = (x = staterule(RULE, r, NiL, 0)) ? x->event : 0;
			break;
		case 'R':
			tm = r->time;
			break;
		case 0:
			ops--;
			/*FALLTHROUGH*/
		case 'S':
			tm = (x = staterule(RULE, r, NiL, 0)) ? x->time : 0;
			break;
		case 'W':
			tm = CURTIME;
			break;
		default:
			tm = 0;
			error(1, "%s: unknown time component", ops);
			ops = null;
			break;
		}
		if (*ops)
			sfputr(xp, fmttime(ops, tm), -1);
		else
			sfprintf(xp, "%lu", tm);
		return;
	default:
		matched = (op == tst);
		break;
	}
	if (matched == (sep == EQ))
	{
		if (*p)
			expand(xp, p);
		else
			sfputr(xp, r->name, -1);
	}
}

/*
 * construct the parentage of r in xp, starting with r
 * sep placed between names
 */

void
parentage(Sfio_t* xp, register struct rule* r, char* sep)
{
	if (r->active && r->active->parent && !(r->active->parent->target->mark & M_mark) && r->active->parent->parent != r->active->parent)
	{
		r->mark |= M_mark;
		parentage(xp, r->active->parent->target, sep);
		r->mark &= ~M_mark;
		sfputr(xp, sep, -1);
	}
	sfputr(xp, (r->property & P_operator) && r->statedata ? r->statedata : r->name, -1);
}

/*
 * copy s into xp if rule s has any attribute in att or
 * if att is 0 then copy the named attributes of rule s into xp
 * attribute pattern propagation is taken into account
 */

static void
attribute(Sfio_t* xp, char* s, register char* att, int sep)
{
	register char*		t;
	register struct rule*	r;
	register struct rule*	a;
	register struct list*	p;
	long			n;
	int			c;
	int			i;
	struct rule*		x;
	struct rule*		y;
	struct rule*		z;

	i = 0;
	r = getrule(s);
	do
	{
		if (!r) z = 0;
		else if (!(r->dynamic & D_alias))
		{
			s = r->name;
			z = 0;
		}
		else if (!(z = getrule(r->name))) break;
		x = associate(internal.attribute_p, r, s, NiL);
		if (r || (x || (sep & LT)) && (r = makerule(s)))
		{
			n = r->attribute;
			if (x) n |= x->attribute;
			if (att)
			{
				for (;;)
				{
					while (isspace(*att)) att++;
					if ((t = strchr(att, c = '|')) || (t = strchr(att, c = ' '))) *t = 0;
					if (sep & GT)
					{
						for (p = r->prereqs; p; p = p->next)
							if (strmatch(p->rule->name, att))
							{
								if (t) *t = c;
								if (i) sfputc(xp, ' ');
								else i = 1;
								sfputr(xp, s, -1);
								goto next;
							}
					}
					else if (a = getrule(att))
					{
						if (sep & LT)
						{
							if ((y = associate(a, r, s, NiL)) || x && (y = associate(a, x, NiL, NiL)))
							{
								if (t) *t = c;
								if (i) sfputc(xp, ' ');
								else i = 1;
								sfputr(xp, y->name, -1);
								goto next;
							}
						}
						else if (hasattribute(r, a, x))
						{
							if (t) *t = c;
							if (sep == EQ)
							{
								if (i) sfputc(xp, ' ');
								else i = 1;
								sfputr(xp, s, -1);
							}
							goto next;
						}
					}
					if (!t) break;
					*t++ = c;
					att = t;
				}
				if (sep & NOT)
				{
					if (i) sfputc(xp, ' ');
					else i = 1;
					sfputr(xp, s, -1);
				}
			}
			else
			{
				if (n && r != internal.attribute)
					for (p = internal.attribute->prereqs; p; p = p->next)
						if (n & p->rule->attribute)
						{
							if (i) sfputc(xp, ' ');
							else i = 1;
							sfputr(xp, p->rule->name, -1);
						}
				if (r->scan && r != internal.scan)
					for (p = internal.scan->prereqs; p; p = p->next)
						if (p->rule->scan == r->scan)
						{
							if (i) sfputc(xp, ' ');
							else i = 1;
							sfputr(xp, p->rule->name, -1);
							break;
						}
			}
		}
		else if (att && (sep & NOT))
		{
			if (i) sfputc(xp, ' ');
			else i = 1;
			sfputr(xp, s, -1);
		}
	next:	;
	} while (r = z);
}

/*
 * check if name generates a target matching the metarule pattern pat
 * (sep&LT) lists the primary and secondary targets for name
 */

static void
generate(Sfio_t* xp, char* name, char* pat, int sep)
{
	register struct rule*	x;
	register struct list*	p;
	register struct list*	q;
	char*			b;
	char			stem[MAXNAME];

	if (pat[0] != '%' || pat[1])
	{
		if (metamatch(NiL, name, pat))
		{
			if (!(sep & NOT))
				sfputr(xp, name, -1);
			return;
		}
		if (!strchr(pat, '%'))
		{
			for (p = internal.metarule->prereqs; p; p = p->next)
				if (metamatch(stem, pat, p->rule->name) && (x = metainfo('I', p->rule->name, NiL, 0)))
					for (q = x->prereqs; q; q = q->next)
						if (metamatch(tmpname, name, q->rule->name) && streq(stem, tmpname))
						{
							if (!(sep & NOT))
								sfputr(xp, name, -1);
							return;
						}
			if (x = metainfo('N', NiL, NiL, 0))
				for (p = x->prereqs; p; p = p->next)
					if (metamatch(tmpname, name, p->rule->name) && (streq(tmpname, pat) || (b = strrchr(pat, '/')) && streq(tmpname, b + 1)))
					{
						if (!(sep & NOT))
							sfputr(xp, name, -1);
						return;
					}
		}
		else
		{
			Sfio_t*	tp;
			long	n;

			tp = sfstropen();
			for (p = internal.metarule->prereqs; p; p = p->next)
				if (metamatch(NiL, p->rule->name, pat) && (x = metainfo('I', p->rule->name, NiL, 0)))
				{
					for (q = x->prereqs; q; q = q->next)
						if (metamatch(stem, name, q->rule->name))
						{
							if (!(sep & NOT))
							{
								/*UNDENT...*/

	struct rule*		y;
	struct rule*		z;
	struct list*		u;
	long			b;

	if (z = metarule(q->rule->name, p->rule->name, 0))
	{
		if (!z->uname)
		{
			if (y = metainfo('S', z->name, NiL, 0))
			{
				b = sfstrtell(xp);
				for (u = y->prereqs; u; u = u->next)
				{
					n = sfstrtell(xp);
					metaexpand(tp, stem, u->rule->name);
					generate(xp, sfstruse(tp), pat, sep);
					if (sfstrtell(xp) != n)
						sfputc(xp, ' ');
					if ((state.questionable & 0x00040000) && !(sep & LT))
						break;
				}
				if (sfstrtell(xp) != b)
					sfstrrel(xp, -1);
				sfstrclose(tp);
				return;
			}
		}
		else if (y = metainfo('O', q->rule->name, NiL, 0))
		{
			for (u = y->prereqs; u; u = u->next)
				if (metamatch(NiL, u->rule->name, z->uname) && (y = metainfo('S', q->rule->name, u->rule->name, 0)))
				{
					b = sfstrtell(xp);
					for (u = y->prereqs; u; u = u->next)
					{
						n = sfstrtell(xp);
						metaexpand(tp, stem, u->rule->name);
						generate(xp, sfstruse(tp), pat, sep);
						if (sfstrtell(xp) != n)
							sfputc(xp, ' ');
					}
					if (sfstrtell(xp) != b)
						sfstrrel(xp, -1);
					sfstrclose(tp);
					return;
				}
		}
	}
	metaexpand(xp, stem, z && z->uname && (y = metarule(z->uname, p->rule->name, 0)) && y->action && !*y->action ? z->uname : p->rule->name);

								/*...INDENT*/
							}
							sfstrclose(tp);
							return;
						}
					if (!(state.questionable & 0x00000200))
					{
						char*	t;

						n = sfstrtell(xp);
						for (q = x->prereqs; q; q = q->next)
							if (q->rule->name != pat)
							{
								generate(tp, name, q->rule->name, sep);
								if (sfstrtell(tp))
								{
									t = sfstruse(tp);
									if (!streq(t, name))
									{
										if (sfstrtell(xp) != n)
											sfputc(xp, ' ');
										generate(xp, sfstruse(tp), pat, sep);
									}
								}
							}
						if (sfstrtell(xp) != n)
						{
							sfstrclose(tp);
							return;
						}
					}
				}
			sfstrclose(tp);
		}
	}
	else if (x = metainfo('N', NiL, NiL, 0))
	{
		for (p = x->prereqs; p; p = p->next)
			if (metamatch(tmpname, name, p->rule->name))
			{
				if (!(sep & NOT))
					metaexpand(xp, tmpname, p->rule->name);
				return;
			}
	}
	if (sep & NOT) sfputr(xp, name, -1);
}

/*
 * quote s into xp according to sh syntax
 */

void
shquote(register Sfio_t* xp, char* s)
{
	register char*	t;
	register char*	b;
	register int	c;
	register int	q;

	if (*s == '"' && *(s + strlen(s) - 1) == '"')
	{
		sfprintf(xp, "\\\"%s\\\"", s);
		return;
	}
	q = 0;
	b = 0;
	for (t = s;;)
	{
		switch (c = *t++)
		{
		case 0:
			break;
		case '\n':
		case ';':
		case '&':
		case '|':
		case '<':
		case '>':
		case '(':
		case ')':
		case '[':
		case ']':
		case '{':
		case '}':
		case '*':
		case '?':
		case ' ':
		case '\t':
		case '\\':
			q |= 4;
			continue;
		case '\'':
			q |= 1;
			if (q & 2)
				break;
			continue;
		case '"':
		case '$':
			q |= 2;
			if (q & 1)
				break;
			continue;
		case '=':
			if (!b)
				b = t;
			continue;
		default:
			continue;
		}
		break;
	}
	if (!q)
		sfputr(xp, s, -1);
	else if (!(q & 1))
	{
		if (b)
			sfprintf(xp, "%-.*s'%s'", b - s, s, b);
		else
			sfprintf(xp, "'%s'", s);
	}
	else if (!(q & 2))
	{
		if (b)
			sfprintf(xp, "%-.*s\"%s\"", b - s, s, b);
		else
			sfprintf(xp, "\"%s\"", s);
	}
	else
		for (t = s;;)
			switch (c = *t++)
			{
			case 0:
				return;
			case '\n':
				sfputc(xp, '"');
				sfputc(xp, c);
				sfputc(xp, '"');
				break;
			case ';':
			case '&':
			case '|':
			case '<':
			case '>':
			case '(':
			case ')':
			case '[':
			case ']':
			case '{':
			case '}':
			case '$':
			case '*':
			case '?':
			case ' ':
			case '\t':
			case '\\':
			case '\'':
			case '"':
				sfputc(xp, '\\');
				/*FALLTHROUGH*/
			default:
				sfputc(xp, c);
				break;
			}
}

/*
 * generate edit context in xp at cur from beg
 */

static char*
editcontext(register char* beg, register char* cur)
{
	register int	n;

	sfstrset(internal.tmp, 0);
	if ((n = cur - beg) > (EDITCONTEXT / 2)) beg = cur - (n = (EDITCONTEXT / 2));
	if (n > 0) sfprintf(internal.tmp, "%-*.*s", n, n, beg);
	if (*cur) sfprintf(internal.tmp, ">>>%c", *cur);
	sfprintf(internal.tmp, "<<<");
	if (*cur && *(cur + 1))
	{
		n = EDITCONTEXT - n;
		if (n > 0) sfprintf(internal.tmp, "%-*.*s", n, n, cur + 1);
	}
	return sfstruse(internal.tmp);
}

/*
 * expand rules selected by $(...) into xp
 */

static void
expandall(register Sfio_t* xp, register unsigned long all)
{
	register int		sep;
	register struct rule*	r;
	Hash_position_t*	pos;

	sep = 0;
	if (pos = hashscan(table.rule, 0))
	{
		while (hashnext(pos))
		{
			r = (struct rule*)pos->bucket->value;
			if (pos->bucket->name == r->name && !(r->dynamic & D_alias) && (!all || (r->dynamic & all)))
			{
				if (sep) sfputc(xp, ' ');
				else sep = 1;
				sfputr(xp, r->name, -1);
			}
		}
		hashdone(pos);
	}
}

/*
 * apply edit operators ed on value v into xp
 *
 * :C:, :/:, :Y: and :?: have a different syntax from the other ops
 * :D:, :B: and :S:, when contiguous, are collected and applied as a group
 * a single `:' must separate each op, the trailing `:' is optional
 * =, !, !=, <>, <, <=, > and >= may separate an op from its value
 */

static void
expandops(Sfio_t* xp, char* v, char* ed, int del, int exp)
{
	register char*		s;
	register int		op;
	char*			dir;
	char*			bas;
	char*			suf;
	char*			val;
	char*			oldp;
	char*			newp;
	char*			eb;
	int			glob;
	int			qual;
	int			cnt;
	int			cntlim;
	int			ctx;
	int			n;
	int			m;
	int			sep;
	int			tokenize;
	long			beg;
	long			ctx_beg;
	long			cur;
	long			arg;
	unsigned long		all;
	char*			ctx_end;
	char*			tok;
	char*			x;
	struct rule*		r;
	Edit_map_t*		map;
	Hash_position_t*	pos;
	int			out;
	regex_t			re;
	char			flags[8];
	long			top[2];
	Sfio_t*			buf[2];

	static unsigned long	lla = D_select0;

	/*
	 * apply the operators from left to right
	 */

	out = 0;
	buf[0] = xp;
	top[0] = beg = sfstrtell(xp);
	buf[1] = 0;
	top[1] = 0;
	dir = bas = suf = DELETE;
	if (exp < 0)
		all = lla == D_select0 ? D_select1 : D_select0;
	else
	{
		all = 0;
		if (exp)
			expand(xp, v);
		else
		{
			out = !out;
			xp = buf[out];
			beg = top[out];
		}
	}
	eb = ed;
	qual = 0;
	while (op = *ed++)
	{
		if (op == del || isspace(op))
			continue;
		if (op == '!')
		{
			while (isspace(*ed))
				ed++;
			if ((op = *ed++) == del)
				continue;
			qual |= NE;
		}
		if (islower(op))
		{
			qual |= ED_LONG;
			ed--;
#if DEBUG
			if (state.test & 0x00000010)
				error(2, "edit +++ %-.32s", ed);
#endif
			if (map = getedit(&ed, del))
			{
				switch (map->cmd.type)
				{
				case ED_COPY:
					goto copy;
				case ED_EDIT:
					qual |= ED_PARTS;
					if (!map->cmd.arg || *ed && *ed != del)
					{
						dir = bas = suf = KEEP;
						*--ed = '=';
					}
					goto copy;
				case ED_OP:
					if (map->options)
					{
						const Edit_opt_t*	opt;

						/*UNDENT...*/
	val = flags;
	if (!*(opt = map->options)->name)
	{
		if (opt->cmd.type == ED_QUAL)
			qual |= opt->cmd.op;
		else if (*ed != '-' || *(ed + 1) == '-' && (!*(ed + 2) || isspace(*(ed + 2))))
		{
			if (opt->cmd.op)
				op = opt->cmd.op;
			if (opt->cmd.arg && val < &flags[sizeof(flags)])
			{
				*val++ = opt->cmd.arg;
				if (opt->cmd.aux && val < &flags[sizeof(flags)])
					*val++ = opt->cmd.aux;
			}
		}
	}
	while (*ed == '-')
	{
		if ((n = *++ed) == '-' || n == 'o')
		{
			if (!*++ed || *ed == del)
				break;
			if (isspace(*ed))
			{
				if (n == '-')
					break;
				while (isspace(*++ed));
			}
			for (s = ed; *ed && *ed != del && !isspace(*ed); ed++);
			m = *ed;
			*ed = 0;
			for (opt = map->options;; opt++)
			{
				if (!opt->name)
				{
					error(1, "%s: --%s: unknown edit operator option", map->name, s);
					break;
				}
				if (streq(s, opt->name))
				{
					if (opt->cmd.type == ED_QUAL)
						qual ^= opt->cmd.op;
					else
					{
						if (opt->cmd.op)
							op = opt->cmd.op;
						if (opt->cmd.arg && val < &flags[sizeof(flags)])
						{
							*val++ = opt->cmd.arg;
							if (opt->cmd.aux && val < &flags[sizeof(flags)])
								*val++ = opt->cmd.aux;
						}
					}
					break;
				}
			}
			*ed = m;
		}
		else
		{
			while (n && n != del)
			{
				if (isspace(n))
					break;
				for (opt = map->options;; opt++)
				{
					if (!opt->name)
					{
						error(1, "%s: -%c: unknown edit operator option", map->name, n);
						break;
					}
					if (*opt->name == n)
					{
						if (opt->cmd.type == ED_QUAL)
							qual ^= opt->cmd.op;
						else
						{
							if (opt->cmd.op)
								op = opt->cmd.op;
							if (opt->cmd.arg && val < &flags[sizeof(flags)])
							{
								*val++ = opt->cmd.arg;
								if (opt->cmd.aux && val < &flags[sizeof(flags)])
									*val++ = opt->cmd.aux;
							}
						}
						break;
					}
				}
				n = *++ed;
			}
		}
		while (isspace(*ed))
			ed++;
	}
	while (val > flags)
		*--ed = *--val;
						/*..INDENT*/
					}
					break;
				case ED_QUAL:
					qual |= map->cmd.op;
					continue;
				}
				s = (*ed && *ed != del || (qual & (NOT|EQ|LT|GT))) ? null : ed;
				if (op != 'T')
					qual &= ~(ED_NOBIND|ED_NOWAIT);
				else if (map->cmd.arg != 'S')
					qual &= ~(ED_FORCE);
				if (map->cmd.arg)
				{
					if (map->cmd.aux)
						*--ed = map->cmd.aux;
					if (qual & ED_FORCE)
						*--ed = 'F';
					*--ed = map->cmd.arg;
				}
				if (qual & ED_NOBIND)
					*--ed = 'X';
				if (qual & ED_NOWAIT)
					*--ed = 'W';
				if (ed != s)
				{
					if (qual & (NOT|EQ|LT|GT))
					{
						if (qual & EQ)
							*--ed = '=';
						if (qual & GT)
							*--ed = '>';
						if (qual & LT)
							*--ed = '<';
						if ((qual & (NOT|GT|LT)) == NOT)
							*--ed = '!';
					}
					else
						*--ed = '=';
				}
			copy:
				*--ed = map->cmd.op;
			}
			else
			{
				for (eb = ed; islower(*ed); ed++);
				error(3, "unknown edit operator: %-.*s", ed - eb, eb);
			}
			if (qual & ED_JOIN)
				*--ed = '@';
#if DEBUG
			if (state.test & 0x00000010)
				error(2, "edit --- %-.32s", ed);
#endif
			op = *ed++;
		}
		if (all && (!strchr("ABCDFGMNPQSTY/?", op) || op == 'O' && (!*ed || *ed == del)))
		{
			expandall(xp, exp < 0 ? 0 : all);
			all = 0;
		}

		/*
		 * check for tokenization
		 */

		if (op == '@')
		{
			tokenize = 0;
			if (!(op = *ed++) || op == del)
				error(3, "prefix edit operator only: %s", editcontext(eb, ed));
			if (islower(op))
				op = toupper(op);
		}
		else
			tokenize = 1;
		sep = EQ;

		/*
		 * collect the operands
		 */

		if (op == 'C' || op == '/' || op == 'Y' || op == '?')
		{
			/*
			 * substitute: <delim><old><delim><new><delim>[g]
			 * conditional: <delim><non-null><delim><null><delim>
			 */

			n = op;
			switch (op)
			{
			case 'C':
			case 'Y':
				if (n = *ed)
					ed++;
				break;
			case '/':
				op = 'C';
				break;
			case '?':
				op = 'Y';
				break;
			}
			oldp = ed;
			while (*ed && *ed != n)
				if (*ed++ == '\\' && !*ed++)
					error(3, "unterminated lhs of %s: %s", op == 'C' ? "substitution" : "conditional", editcontext(eb, ed));
			s = ed;
			if (*ed == n)
				ed++;
			*s = 0;
			newp = ed;
			while (*ed && *ed != n)
				if (*ed++ == '\\' && !*ed++)
					error(3, "unterminated rhs of %s: %s", op == 'C' ? "substitution" : "conditional", editcontext(eb, ed));
			s = ed;
			if (*ed)
				ed++;
			*s = 0;
			if (op == 'C' && *oldp == ' ')
				tokenize = 0;
			glob = 0;
			while (*ed && *ed != del && !isspace(*ed))
				switch (*ed++)
				{
				case 'G':
				case 'g':
				case 'O':
				case 'o':
					glob |= REG_SUB_ALL;
					break;
				case 'L':
				case 'l':
					glob |= REG_SUB_LOWER;
					break;
				case 'U':
				case 'u':
					glob |= REG_SUB_UPPER;
					break;
				default:
					error(1, "invalid character `%c' after %s", *(ed - 1), op == 'C' ? "substitution" : "conditional");
					break;
				}
			if (*ed)
				ed++;
			*s = 0;
		}
		else if (*ed == del || (qual & ED_LONG) && isspace(*ed))
		{
			ed++;
			val = KEEP;
		}
		else if (*ed)
		{
			/*
			 * value: [!<>=][=][<val>]
			 */

			switch (*ed++)
			{
			case '!':
				if (*ed == '=')
				{
					ed++;
					sep = NE;
				}
				else
					sep = NOT;
				break;
			case '=':
				if (*ed == '=')
					ed++;
				sep = EQ;
				break;
			case '<':
				if (*ed == '=')
				{
					ed++;
					sep = LE;
				}
				else if (*ed == '>')
				{
					ed++;
					sep = LT|GT;
				}
				else
					sep = LT;
				break;
			case '>':
				if (*ed == '=')
				{
					ed++;
					sep = GE;
				}
				else
					sep = GT;
				break;
			default:
				error(3, "edit operator delimiter omitted: %s", editcontext(eb, ed));
				break;
			}
			val = ed;
			for (cnt = n = 0; *ed; ed++)
			{
				if (cnt)
				{
					if (*ed == cnt)
						n++;
					else if (*ed == cntlim && !--n)
						cnt = 0;
				}
				else if (*ed == '(')
				{
					cnt = '(';
					cntlim = ')';
					n++;
				}
				else if (*ed == '[')
				{
					cnt = '[';
					cntlim = ']';
					n++;
				}
				else if (n <= 0)
				{
					if (*ed == del)
						break;
					if ((qual & ED_LONG) && isspace(*ed))
					{
						s = ed;
						while (isspace(*++s));
						if (*s == del)
							break;
					}
				}
			}
			if (*ed)
				*ed++ = 0;
			if (!*val)
				val = DELETE;
		}
		else
			val = KEEP;
		switch (op)
		{
		case 'B':
			bas = val;
		parts:
			if (!(qual & ED_PARTS))
			{
				/*
				 * B, D and S are grouped before application
				 */

				switch (*ed)
				{
				case 'B':
					if (bas == DELETE)
						continue;
					break;
				case 'D':
					if (dir == DELETE)
						continue;
					break;
				case 'S':
					if (suf == DELETE)
						continue;
					break;
				}
			}
			break;
		case 'D':
			dir = val;
			goto parts;
		case 'S':
			suf = val;
			goto parts;
		}
		qual = 0;

		/*
		 * validate the operator and apply non-tokenizing operators
		 */

		if (!all)
		{
			if (xp)
			{
				sfputc(xp, 0);
				x = sfstrset(xp, beg);
			}
			else x = v;
			out = !out;
			if (!(xp = buf[out])) xp = buf[out] = sfstropen();
			beg = top[out];
		}
		ctx = !!state.context;
		switch (op)
		{
		case 'A':
		case 'Q':
			if (val == KEEP || val == DELETE)
				val = 0;
			break;
		case 'B':
		case 'D':
		case 'S':
			if (!dir)
				ctx = 0;
			break;
		case 'C':
			if (n = regcomp(&re, oldp, REG_LENIENT|REG_NULL))
			{
				regfatalpat(&re, 2, n, oldp);
				continue;
			}
			/*FALLTHROUGH*/
		case 'Y':
			ctx = 0;
			break;
		case 'E':
			sfprintf(xp, "%ld", expr(xp, x));
			continue;
		case 'F':
		case 'X':
			ctx = 0;
			/*FALLTHROUGH*/
		case 'M':
		case 'P':
		case 'T':
			if (val == KEEP || val == DELETE)
				error(3, "edit operator value omitted: %s", editcontext(eb, ed));
			switch (op)
			{
			case 'M':
				if (n = regcomp(&re, val, REG_AUGMENTED|REG_LENIENT|REG_NOSUB|REG_NULL))
				{
					regfatalpat(&re, 2, n, val);
					continue;
				}
				break;
			case 'X':
				cross(xp, x, val);
				continue;
			}
			break;
		case 'G':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
			ctx = 0;
			/*FALLTHROUGH*/
		case 'N':
		case 'O':
		case 'U':
			if (val == KEEP || val == DELETE)
				val = null;
			switch (op)
			{
			case 'I':
				intersect(xp, x, val, sep);
				continue;
			case 'J':
				hasprereq(xp, x, val);
				continue;
			case 'K':
				linebreak(xp, x, val);
				continue;
			case 'L':
				list(xp, x, val, sep);
				continue;
			case 'O':
				cntlim = (*val == 'N' || *val == 'n' || *val == '*') ? -1 : (int)strtol(val, NiL, 0);
				break;
			case 'U':
				uniq(xp, x, val, sep);
				continue;
			}
			break;
		case 'H':
			if (x == v)
			{
				buf[1] = sfstropen();
				sfputr(buf[1], x, 0);
				x = sfstrset(buf[1], 0);
			}
			if (val != DELETE && val != KEEP)
				for (;;)
				{
					switch (*val++)
					{
					case 0:
						break;
					case 'N':
					case 'n':
					case '=':
						sep |= EQ;
						continue;
					case 'U':
					case 'u':
					case '!':
						sep |= NOT;
						continue;
					}
					break;
				}
			sort(xp, x, sep);
			continue;
		case 'R':
			parse(NiL, x, "expand", 0);
			continue;
		case 'V':
			error(1, "edit operator `%c' must appear first", op);
			continue;
		case 'W':
			ctx = 0;
			if (val == KEEP || val == DELETE)
				val = 0;
			if (val)
			{
				if (!(op = *val++))
				{
					op = 'O';
					val = 0;
				}
				else if (!*val || *val == '=' && !*++val)
					val = 0;
			}
			else
				op = 'O';
			switch (op)
			{
			case 'O':
				order(xp, x, val);
				break;
			default:
				error(1, "unknown edit operator `W=%c'", op);
				break;
			}
			continue;
		default:
			error(1, "unknown edit operator `%c'", op);
			continue;
		}

		/*
		 * the operator is applied to each token if tokenize!=0
		 */

		arg = beg;
		if (!all)
			tok = tokopen(x, 1);
		else if (!(pos = hashscan(table.rule, 0)))
			goto breakloop;
		for (cnt = 1, ctx_end = 0;; cnt++, ctx_end = 0)
		{
			if (!tokenize)
			{
				if (cnt > 1)
					break;
				s = x;
			}
			else if (all)
			{
				for (;;)
				{
					if (!hashnext(pos))
						goto breakloop;
					r = (struct rule*)pos->bucket->value;
					if (pos->bucket->name == r->name && !(r->dynamic & D_alias) && (exp < 0 || (r->dynamic & all)))
					{
						r->dynamic &= ~all;
						break;
					}
				}
				s = r->name;
			}
			else
			{
				if (!(s = tokread(tok)))
				{
					if (cnt > 1)
						break;
					s = null;
				}
				if (*s != '\n' && (cur = sfstrtell(xp)) > arg)
				{
					if (!isspace(*(sfstrbase(xp) + cur - 1)))
						sfputc(xp, ' ');
					arg = sfstrtell(xp);
				}
			}
			if (ctx && iscontextp(s, ctx_end))
			{
				s++;
				*ctx_end = 0;
				sfputc(xp, MARK_CONTEXT);
				ctx_beg = sfstrtell(xp);
			}
			switch (op)
			{
			case 'A':
				attribute(xp, s, val, sep);
				break;
			case 'B':
			case 'D':
			case 'S':
				if (*s)
				{
					if (dir == KEEP)
						cur = sfstrtell(xp);
					edit(xp, s, dir, bas, suf);
					if (dir == KEEP && cur == sfstrtell(xp))
						sfputc(xp, '.');
				}
				break;
			case 'C':
				substitute(xp, &re, s, newp, glob);
				break;
			case 'F':
#if !_drop_this_in_3_2
				switch (*val)
				{
				case 'L':
					val = "%(lower)S";
					message((-3, ":F=L: is obsolete -- use :F=%s: instead", val));
					break;
				case 'U':
					val = "%(upper)S";
					message((-3, ":F=U: is obsolete -- use :F=%s: instead", val));
					break;
				case 'V':
					val = "%(variable)S";
					message((-3, ":F=V: is obsolete -- use :F=%s: instead", val));
					break;
				}
#endif
				strprintf(xp, val, s, 1, -1);
				break;
			case 'G':
				if (*val)
				{
					char*	t;
					char*	v;

					t = tokopen(val, 1);
					while (v = tokread(t))
						generate(xp, s, v, sep);
					tokclose(t);
				}
				else
					generate(xp, s, val, sep);
				break;
			case 'M':
				if ((n = regexec(&re, s, 0, NiL, 0)) && n != REG_NOMATCH)
					regfatal(&re, 2, n);
				else if ((n == 0) == (sep == EQ))
					sfputr(xp, s, -1);
				break;
			case 'N':
				if (strmatch(s, val) == (sep == EQ))
					sfputr(xp, s, -1);
				break;
			case 'O':
				if (cntlim < 0)
					sfstrset(xp, beg);
				else
					switch (sep)
					{
					case EQ:
						if (!cntlim)
						{
							if (s == null)
								goto breakloop;
							goto nextloop;
						}
						else if (cnt < cntlim)
							goto nextloop;
						else if (cnt > cntlim)
							goto breakloop;
						break;
					case NOT:
						if (!cntlim)
						{
							sfprintf(xp, "%d", strlen(s));
							goto nextloop;
						}
						/*FALLTHROUGH*/
					case NE:
						if (cnt == cntlim)
							goto nextloop;
						break;
					case LT:
						if (cnt >= cntlim)
							goto breakloop;
						break;
					case LE:
						if (cnt > cntlim)
							goto breakloop;
						break;
					case GE:
						if (cnt < cntlim)
							goto nextloop;
						break;
					case GT:
						if (cnt <= cntlim)
							goto nextloop;
						break;
					}
				sfputr(xp, s, -1);
				break;
			case 'P':
				pathop(xp, s, val, sep);
				break;
			case 'Q':
				shquote(xp, s);
				break;
			case 'T':
				token(xp, s, val, sep);
				break;
			case 'Y':
				expand(xp, *s ? (glob ? s : oldp) : newp);
				break;
#if DEBUG
			default:
				error(PANIC, "edit operator `%c' not applied to each token", op);
#endif
			}
		nextloop:
			if (ctx_end)
			{
				*ctx_end = MARK_CONTEXT;
				n = sfstrtell(xp) - ctx_beg;
				if (!n)
					sfstrrel(xp, -1);
				else if (n > 1 && *(s = sfstrbase(xp) + ctx_beg) == MARK_CONTEXT)
					*(s - 1) = ' ';
				else
				{
					sfputc(xp, MARK_CONTEXT);
					s = sfstrbase(xp) + ctx_beg;
					x = s + n + 1;
					m = 0;
					while (s < x)
						if (*s++ == ' ')
							for (m++; s < x && *s == ' '; s++);
					if (m)
					{
						sfprintf(xp, "%*s", m * 2, null);
						x = sfstrbase(xp) + ctx_beg + n + 1;
						s = x + m * 2;
						for (;;)
						{
							if ((*--s = *--x) == ' ')
							{
								*s = MARK_CONTEXT;
								for (x++; *(x - 1) == ' '; *--s = *--x);
								*--s = MARK_CONTEXT;
								if (--m <= 0)
									break;
							}
						}
					}
				}
			}
			if (all && sfstrtell(xp) > beg)
			{
				sfputc(xp, 0);
				makerule(sfstrset(xp, beg))->dynamic |= lla;
			}
		}
	breakloop:
		if (ctx_end)
		{
			*ctx_end = MARK_CONTEXT;
			if (sfstrtell(xp) == ctx_beg)
				sfstrrel(xp, -1);
		}
		if (all)
		{
			if (pos)
			{
				hashdone(pos);
				if (pos = hashscan(table.rule, 0))
				{
					/*
					 * a poorly understood interaction
					 * between binding/aliasing lets
					 * some unwanted rules slip through
					 * this loop catches them
					 */

					while (hashnext(pos))
					{
						r = (struct rule*)pos->bucket->value;
						r->dynamic &= ~all;
					}
					hashdone(pos);
				}
			}
			n = all;
			all = lla;
			lla = n;
			exp++;
		}
		else
		{
			tokclose(tok);
			if (sfstrtell(xp) == arg)
			{
				if (op == 'O' && !cntlim)
					sfprintf(xp, "%d", cnt - 1);
				else if (arg > beg)
					sfstrrel(xp, -1);
			}
		}

		/*
		 * operator cleanup
		 */

		switch (op)
		{
		case 'B':
		case 'D':
		case 'S':
			dir = bas = suf = DELETE;
			break;
		case 'C':
		case 'M':
			regfree(&re);
			break;
		}
	}
	if (all)
		expandall(buf[0], exp < 0 ? 0 : all);
	else
	{
		if (out)
			sfputr(buf[0], xp ? sfstruse(xp) : v, -1);
		if (buf[1])
			sfstrclose(buf[1]);
	}
}

/*
 * expand first non-null of nvars variables in s with edit ops ed into xp
 */

static void
expandvars(register Sfio_t* xp, register char* s, char* ed, int del, int nvars)
{
	register char*	v;
	char*		t;
	int		exp;
	int		op;
	int		aux;
	long		pos;
	Edit_map_t*	map;
	Sfio_t*		cvt = 0;
	Sfio_t*		val = 0;
	Sfio_t*		tmp;
#if DEBUG
	long		beg;
	Sfio_t*		msg = 0;
#endif

	static int	level;

	if (level++ > 32)
	{
		level = 0;
		error(3, "%s: recursive variable definition", s);
	}
#if DEBUG
	if (error_info.trace <= -10)
	{
		beg = sfstrtell(xp);
		msg = sfstropen();
	}
#endif

	/*
	 * some operators must appear first
	 */

	exp = 1;
	aux = 0;
	if (ed)
	{
		while (op = *ed)
		{
			if (op == del || isspace(op))
				ed++;
			else if (islower(op))
			{
				t = ed;
				if ((map = getedit(&ed, del)) && map->cmd.type == ED_QUAL)
					switch (map->cmd.op)
					{
					case ED_AUXILLIARY:
						exp = 0;
						aux = -1;
						continue;
					case ED_LITERAL:
						exp = 0;
						continue;
					case ED_PRIMARY:
						exp = 0;
						aux = 1;
						continue;
					}
				ed = t;
				break;
			}
			else
			{
				if (op == 'V')
				{
					exp = 0;
					if (*(ed + 1) == 'A')
					{
						ed++;
						aux = -1;
					}
					else if (*(ed + 1) == 'P')
					{
						ed++;
						aux = 1;
					}
				}
				else break;
				if (*++ed && *ed++ != del)
					error(3, "edit operator `%c' does not take a value", op);
			}
		}
	}
	for (;;)
	{
#if DEBUG
		if (msg)
		{
			sfprintf(msg, "var=%s,ops=%s", s, ed);
			sfstruse(msg);
		}
#endif
		if ((*s == '"' || *s == MARK_QUOTE) && *(s + 1) && *(t = s + strlen(s) - 1) == *s)
		{
			if (!cvt) cvt = sfstropen();
			*t = 0;
			sfputr(cvt, s + 1, 0);
			*t = *s;
			stresc(v = sfstrbase(cvt));
		}
		else
		{
			if (strchr(s, '$'))
			{
				if (!cvt) cvt = sfstropen();
				expand(cvt, s);
				v = sfstruse(cvt);
			}
			else v = s;
			if (nvars == 1 && streq(v, "..."))
			{
				exp = -1;
				if (!ed) ed = null;
			}
			else v = getval(v, aux);
		}
		if (*v || --nvars <= 0)
		{
			if (ed)
			{
				if (!cvt) cvt = sfstropen();
				if (v == sfstrbase(internal.val))
				{
					tmp = internal.val;
					if (!val) val = sfstropen();
					internal.val = val;
				}
				else
				{
					tmp = 0;
					if (v == sfstrbase(cvt)) v = 0;
				}
				pos = sfstrtell(cvt);
				expand(cvt, ed);
				sfputc(cvt, 0);
				expandops(xp, v ? v : sfstrbase(cvt), sfstrbase(cvt) + pos, del, exp);
				if (tmp) internal.val = tmp;
			}
			else if (*v) expand(xp, v);
			break;
		}
		while (*s++);
	}
#if DEBUG
	if (msg)
	{
		pos = sfstrtell(xp);
		sfputc(xp, 0);
		error(-10, "expand(%s,lev=%d): `%s'", sfstrbase(msg), level, sfstrset(xp, beg));
		sfstrset(xp, pos);
		sfstrclose(msg);
	}
#endif
	if (cvt) sfstrclose(cvt);
	if (val) sfstrclose(val);
	level--;
}

/*
 * expand `$(...)' from a into xp
 */

void
expand(register Sfio_t* xp, register char* a)
{
	register int	c;
	register char*	s;
	int		del;
	int		p;
	int		q;
	int		nvars;
	long		ed;
	long		var;
	Sfio_t*		val = 0;

	if (!*a) return;
	if (!(s = strchr(a, '$')))
	{
		sfputr(xp, a, -1);
		return;
	}
	if (a == sfstrbase(internal.val) || state.val)
	{
		val = internal.val;
		internal.val = sfstropen();
	}
	sfwrite(xp, a, s - a);
	a = s;
	while (*a)
	{
		if (*a != '$') sfputc(xp, *a++);
		else if (*++a == '(')
		{
			if (isspace(*++a))
			{
				sfputc(xp, '$');
				sfputc(xp, '(');
				sfputc(xp, *a++);
			}
			else
			{
				var = sfstrtell(xp);
				ed = 0;
				nvars = 1;
				del = ':';
				q = 0;
				p = 1;
				while (c = *a++)
				{
					if (c == '"') q = !q;
					else if (q) /* quoted */;
					else if (c == '(') p++;
					else if (c == ')')
					{
						if (!--p) break;
					}
					else if (!ed && p == 1)
					{
						if (c == '|')
						{
							c = 0;
							nvars++;
						}
						else if (c == del)
						{
							c = 0;
							ed = sfstrtell(xp);
						}
						else if (c == '`')
						{
							c = 0;
							ed = sfstrtell(xp);
							if (!(del = *a++))
							{
								ed--;
								break;
							}
						}
						else if (isspace(c))
						{
							while (isspace(*a)) a++;
							if (*a == del || *a == '`')
								continue;
						}
					}
					sfputc(xp, c);
				}
				sfputc(xp, 0);
				s = sfstrset(xp, var);
				if (q || !c)
				{
					a--;
					error(1, "missing %c in %s variable expansion", q ? '"' : ')', s);
				}
				expandvars(xp, s, ed ? sfstrbase(xp) + ed + 1 : (char*)0, del, nvars);
			}
		}
		else if (*a == '$')
		{
			for (s = a; *s == '$'; s++);
			if (*s != '(') sfputc(xp, '$');
			while (a < s) sfputc(xp, *a++);
		}
		else sfputc(xp, '$');
	}
	if (val)
	{
		sfstrclose(internal.val);
		internal.val = val;
	}
}
