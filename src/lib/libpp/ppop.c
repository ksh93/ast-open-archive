/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1986-2002 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*    If you have copied or used this software without agreeing     *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * preprocessor library control interface
 */

#include "pplib.h"
#include "pptab.h"

#include <ls.h>
#include <sfstr.h>

#define REFONE	(pp.truncate?(Hash_table_t*)0:pp.symtab)
#define REFALL	(pp.truncate?pp.dirtab:pp.symtab)

#define ppiskey(t,v,p)	(p=t,v>=p->value&&value<=(p+elementsof(t)-2)->value)

/*
 * set option value
 * initialization files have lowest precedence
 */

static void
set(register long* p, register long op, int val)
{
	long*	r;

	r = p == &pp.state ? &pp.ro_state : p == &pp.mode ? &pp.ro_mode : &pp.ro_option;
	if (!(pp.mode & INIT) || !(pp.in->type == IN_FILE) || !(*r & op))
	{
		if (!pp.initialized && !(pp.mode & INIT))
			*r |= op;
		if (val) *p |= op;
		else *p &= ~op;
	}
	debug((-7, "set(%s)=%s", p == &pp.state ? "state" : p == &pp.mode ? "mode" : "option", p == &pp.state ? ppstatestr(*p) : p == &pp.mode ? ppmodestr(*p) : ppoptionstr(*p)));
}

/*
 * initialize hash table with keywords from key
 */

static void
inithash(register Hash_table_t* tab, register struct ppkeyword* key)
{
	register char*	s;

	for (; s = key->name; key++)
	{
		if (!ppisid(*s)) s++;
		hashput(tab, s, key->value);
	}
}

/*
 * return ppkeyword table name given value
 */

char*
ppkeyname(register int value, int dir)
{
	register char*			s;
	register struct ppkeyword*	p;

	if (dir && ppiskey(directives, value, p) || !dir && (ppiskey(options, value, p) || ppiskey(predicates, value, p) || ppiskey(variables, value, p)))
	{
		s = (p + (value - p->value))->name;
		return(s + !ppisid(*s));
	}
#if DEBUG
	error(PANIC, "no keyword table name for value=%d", value);
#endif
	return("UNKNOWN");
}

/*
 * initialize the include ignores
 */

static void
ignore(char* file)
{
	register char*		s;
	register int		c;
	register struct ppdirs*	dp;
	int			fd;
	char*			old_file;
	long			old_state;
	struct ppfile*		fp;
	struct ppfile*		mp;

	if (*file == '-')
	{
		if (!error_info.file)
		{
			error(1, "%s: input file name required for %s ignore", file, dirname(INCLUDE));
			return;
		}
		old_file = s = strcopy(pp.path, error_info.file);
		c = *++file;
		for (;;)
		{
			if (s <= pp.path || *s == '/')
			{
				s = old_file;
				break;
			}
			else if (*s == c) break;
			s--;
		}
		strcpy(s, file);
		fd = open(file = pp.path, O_RDONLY);
	}
	else fd = open(file, O_RDONLY);
	if (fd < 0)
	{
		error(1, "%s: cannot open %s ignore list", file, dirname(INCLUDE));
		return;
	}
	old_file = error_info.file;
	old_state = pp.state;
	PUSH_FILE(file, fd);
#if CATSTRINGS
	pp.state |= (COMPILE|FILEPOP|HEADER|JOINING|STRIP);
#else
	pp.state |= (COMPILE|FILEPOP|HEADER|STRIP);
#endif
	pp.level++;
	fp = mp = 0;
	for (;;)
	{
		switch (pplex())
		{
		case 0:
			break;
		case T_STRING:
		case T_HEADER:
			pathcanon(s = pp.token, 0);
			fp = ppsetfile(s);
			if (mp)
			{
				mp->flags |= INC_MAPPED;
				mp->bound[INC_MAP] = fp;
				fp = mp = 0;
			}
			else
			{
				fp->guard = INC_IGNORE;
				for (dp = pp.firstdir->next; dp; dp = dp->next)
					if (dp->name && (c = strlen(dp->name)) && !strncmp(dp->name, s, c) && s[c] == '/')
					{
						ppsetfile(s + c + 1)->guard = INC_IGNORE;
						break;
					}
			}
			continue;
		case '=':
			if (!(mp = fp))
				error(3, "%s: \"name\" = \"binding\" expected");
			fp = 0;
			continue;
		case '\n':
			continue;
		default:
			error(3, "%s: %s unexpected in %s ignore list", file, pptokstr(pp.token, 0), dirname(INCLUDE));
			break;
		}
		break;
	}
	pp.level--;
	error_info.file = old_file;
	pp.state = old_state;
}

/*
 * return non-0 if file is identical to fd
 */

static int
identical(char* file, int fd)
{
	struct stat	a;
	struct stat	b;

	return(!stat(file, &a) && !fstat(fd, &b) && a.st_dev == b.st_dev && a.st_ino == b.st_ino);
}

/*
 * compare up to pp.truncate chars
 *
 * NOTE: __STD* and symbols containing ' ' are not truncated
 */

static int
trunccomp(register char* a, register char* b)
{
	return(!strchr(b, ' ') && !strneq(b, "__STD", 5) ? strncmp(a, b, pp.truncate) : strcmp(a, b));
}

/*
 * hash up to pp.truncate chars
 *
 * NOTE: __STD* and symbols containing ' ' are not truncated
 */

static unsigned int
trunchash(char* a)
{
	int	n;

	return(memhash(a, (n = strlen(a)) > pp.truncate && !strchr(a, ' ') && !strneq(a, "__STD", 5) ? pp.truncate : n));
}

#if DEBUG & TRACE_debug
/*
 * append context to debug trace
 */

static int
context(Sfio_t* sp, int level, int flags)
{
	static int	state;

	NoP(level);
	NoP(flags);
	if (error_info.trace <= -10 && pp.state != state)
	{
		state = pp.state;
		sfprintf(sp, " %s", ppstatestr(pp.state));
	}
	return(1);
}
#endif

/*
 * pp operations
 *
 * NOTE: PP_INIT must be done before the first pplex() call
 *	 PP_DONE must be done after the last pplex() call
 *	 PP_INIT-PP_DONE must be done for each new PP_INPUT
 */

void
ppop(int op, ...)
{
	va_list				ap;
	register char*			p;
	register struct ppkeyword*	kp;
	register char*			s;
	int				c;
	long				n;
	char*				t;
	struct ppdirs*			dp;
	struct ppdirs*			hp;
	struct ppsymkey*		key;
	struct oplist*			xp;
	Sfio_t*				sp;
	PPCOMMENT			ppcomment;
	PPLINESYNC			pplinesync;

	static int			initialized;

	va_start(ap, op);
	switch (op)
	{
	case PP_ASSERT:
	case PP_DEFINE:
	case PP_DIRECTIVE:
	case PP_OPTION:
	case PP_READ:
	case PP_UNDEF:
		if (pp.initialized) goto before;
		if ((p = va_arg(ap, char*)) && *p)
		{
			if (pp.lastop) pp.lastop = (pp.lastop->next = newof(0, struct oplist, 1, 0));
			else pp.firstop = pp.lastop = newof(0, struct oplist, 1, 0);
			pp.lastop->op = op;
			pp.lastop->value = p;
		}
		break;
	case PP_BUILTIN:
		pp.builtin = va_arg(ap, PPBUILTIN);
		break;
	case PP_CDIR:
		p = va_arg(ap, char*);
		c = va_arg(ap, int);
		if (!p)
		{
			pp.cdir = 0;
			pp.c = c;
		}
		else if (streq(p, "-"))
		{
			pp.cdir = 0;
			pp.c = c;
			for (dp = pp.firstdir; dp; dp = dp->next)
				dp->c = c;
		}
		else if (!pp.c)
		{
			if (!*p)
			{
				pp.cdir = 0;
				pp.c = c;
			}
			else
			{
				pathcanon(p, 0);
				for (dp = pp.firstdir; dp; dp = dp->next)
				{
					if (!pp.c && (dp->c || dp->name && streq(dp->name, p)))
						pp.c = 1;
					dp->c = pp.c == 1;
				}
				pp.cdir = pp.c ? 0 : p;
			}
		}
		break;
	case PP_CHOP:
		if (p = va_arg(ap, char*))
		{
			c = strlen(p);
			xp = newof(0, struct oplist, 1, c + 1);
			xp->value = ((char*)xp) + sizeof(struct oplist);
			s = xp->value;
			c = *p++;
			while (*p && *p != c)
				*s++ = *p++;
			*s++ = '/';
			xp->op = s - xp->value;
			*s++ = 0;
			if (*p && *++p && *p != c)
			{
				while (*p && *p != c)
					*s++ = *p++;
				*s++ = '/';
			}
			*s = 0;
			xp->next = pp.chop;
			pp.chop = xp;
		}
		break;
	case PP_COMMENT:
		if (pp.comment = va_arg(ap, PPCOMMENT)) pp.flags |= PP_comment;
		else pp.flags &= ~PP_comment;
		break;
	case PP_COMPATIBILITY:
		set(&pp.state, COMPATIBILITY, va_arg(ap, int));
#if COMPATIBLE
		ppfsm(FSM_COMPATIBILITY, (pp.state & COMPATIBILITY) ? pp.pass : (char*)0);
#else
		if (pp.state & COMPATIBILITY)
			error(3, "preprocessor not compiled with compatibility dialect enabled [COMPATIBLE]");
#endif
		if (pp.state & COMPATIBILITY) pp.flags |= PP_compatibility;
		else pp.flags &= ~PP_compatibility;
		break;
	case PP_COMPILE:
		if (pp.initialized) goto before;
		pp.state |= COMPILE;
		if (!pp.symtab) pp.symtab = hashalloc(NiL, HASH_name, "symbols", 0);
		if (kp = va_arg(ap, struct ppkeyword*))
			for (; s = kp->name; kp++)
		{
			n = SYM_LEX;
			switch (*s)
			{
			case '-':
				s++;
				break;
			case '+':
				s++;
				if (!(pp.option & PLUSPLUS)) break;
				/*FALLTHROUGH*/
			default:
				n |= SYM_KEYWORD;
				break;
			}
			if (key = ppkeyset(pp.symtab, s))
			{
				key->sym.flags = n;
				key->lex = kp->value;
			}
		}
		break;
	case PP_DEBUG:
		error_info.trace = va_arg(ap, int);
		break;
	case PP_DEFAULT:
		if (p = va_arg(ap, char*)) p = strdup(p);
		if (pp.ppdefault) free(pp.ppdefault);
		pp.ppdefault = p;
		break;
	case PP_DONE:
#if CHECKPOINT
		if (pp.mode & DUMP) ppdump();
#endif
		if (pp.mode & FILEDEPS)
		{
			sfputc(pp.filedeps.sp, '\n');
			if (pp.filedeps.sp == sfstdout) sfsync(pp.filedeps.sp);
			else sfclose(pp.filedeps.sp);
		}
		if (pp.state & STANDALONE) ppflushout();
		error_info.file = 0;
		break;
	case PP_DUMP:
		set(&pp.mode, DUMP, va_arg(ap, int));
#if !CHECKPOINT
		if (pp.mode & DUMP)
			error(3, "preprocessor not compiled with checkpoint enabled [CHECKPOINT]");
#endif
		break;
	case PP_FILEDEPS:
		if (n = va_arg(ap, int))
			pp.filedeps.flags |= n;
		else
			pp.filedeps.flags = 0;
		break;
	case PP_FILENAME:
		error_info.file = va_arg(ap, char*);
		break;
	case PP_HOSTDIR:
		if (!(pp.mode & INIT))
			pp.ro_mode |= HOSTED;
		else if (pp.ro_mode & HOSTED)
			break;
		pp.ro_mode |= INIT;
		p = va_arg(ap, char*);
		c = va_arg(ap, int);
		if (!p)
		{
			pp.hostdir = 0;
			pp.hosted = c;
		}
		else if (streq(p, "-"))
		{
			if (pp.initialized)
				set(&pp.mode, HOSTED, c);
			else
			{
				pp.hostdir = 0;
				pp.hosted = c ? 1 : 2;
				for (dp = pp.firstdir; dp; dp = dp->next)
					dp->hosted = pp.hosted == 1;
			}
		}
		else if (!pp.hosted)
		{
			if (!*p)
			{
				pp.hostdir = 0;
				pp.hosted = 1;
			}
			else
			{
				pathcanon(p, 0);
				for (dp = pp.firstdir; dp; dp = dp->next)
				{
					if (!pp.hosted && (dp->hosted || dp->name && streq(dp->name, p)))
						pp.hosted = 1;
					dp->hosted = pp.hosted == 1;
				}
				pp.hostdir = pp.hosted ? 0 : p;
			}
		}
		break;
	case PP_ID:
		p = va_arg(ap, char*);
		c = va_arg(ap, int);
		if (p) ppfsm(c ? FSM_IDADD : FSM_IDDEL, p);
		break;
	case PP_IGNORE:
		if (p = va_arg(ap, char*))
		{
			pathcanon(p, 0);
			ppsetfile(p)->guard = INC_IGNORE;
			message((-3, "%s: ignore", p));
		}
		break;
	case PP_IGNORELIST:
		if (pp.initialized) goto before;
		pp.ignore = va_arg(ap, char*);
		break;
	case PP_INCLUDE:
		if ((p = va_arg(ap, char*)) && *p)
		{
			pathcanon(p, 0);
			for (dp = pp.stddirs; dp = dp->next;)
				if (dp->name && streq(dp->name, p)) break;
			if (pp.cdir && streq(p, pp.cdir))
			{
				pp.cdir = 0;
				pp.c = 1;
			}
			if (pp.hostdir && streq(p, pp.hostdir))
			{
				pp.hostdir = 0;
				pp.hosted = 1;
			}
			if ((pp.mode & INIT) && !(pp.ro_mode & INIT))
				pp.hosted = 1;
			c = dp && dp->c || pp.c == 1;
			n = dp && dp->hosted || pp.hosted == 1;
			if (!dp || dp == pp.lastdir->next)
			{
				if (dp)
				{
					c = dp->c;
					n = dp->hosted;
				}
				dp = newof(0, struct ppdirs, 1, 0);
				dp->name = p;
				dp->index = INC_LOCAL + pp.ignoresrc != 0;
				dp->next = pp.lastdir->next;
				pp.lastdir = pp.lastdir->next = dp;
			}
			dp->c = c;
			dp->hosted = n;
		}
		break;
	case PP_INCREF:
		pp.incref = va_arg(ap, PPINCREF);
		break;
	case PP_INIT:
		if (pp.initialized)
		{
			error_info.errors = 0;
			error_info.warnings = 0;
		}
		else
		{
			/*
			 * context initialization
			 */

			if (!initialized)
			{
				/*
				 * out of malloc is fatal
				 */

				memfatal();

				/*
				 * initialize the error message interface
				 */

				error_info.version = (char*)pp.version;
#if DEBUG & TRACE_debug
				error_info.auxilliary = context;
				pptrace(0);
#endif

				/*
				 * initialize pplex tables
				 */

				ppfsm(FSM_INIT, NiL);

				/*
				 * fixed macro stack size -- room for improvement
				 */

				pp.macp = newof(0, struct ppmacstk, DEFMACSTACK, 0);
				pp.macp->next = pp.macp + 1;
				pp.maxmac = (char*)pp.macp + DEFMACSTACK;
				initialized = 1;

				/*
				 * initial include/if control stack
				 */

				pp.control = newof(0, long, pp.constack, 0);
				pp.maxcon = pp.control + pp.constack - 1;
			}

			/*
			 * validate modes
			 */

			switch (pp.arg_mode)
			{
			case 'a':
			case 'C':
				ppop(PP_COMPATIBILITY, 0);
				ppop(PP_TRANSITION, 1);
				break;
			case 'A':
			case 'c':
				ppop(PP_COMPATIBILITY, 0);
				ppop(PP_STRICT, 1);
				break;
			case 'f':
				ppop(PP_COMPATIBILITY, 1);
				ppop(PP_PLUSPLUS, 1);
				ppop(PP_TRANSITION, 1);
				break;
			case 'F':
				ppop(PP_COMPATIBILITY, 0);
				ppop(PP_PLUSPLUS, 1);
				break;
			case 'k':
			case 's':
				ppop(PP_COMPATIBILITY, 1);
				ppop(PP_STRICT, 1);
				break;
			case 'o':
			case 'O':
				ppop(PP_COMPATIBILITY, 1);
				ppop(PP_TRANSITION, 0);
				break;
			case 't':
				ppop(PP_COMPATIBILITY, 1);
				ppop(PP_TRANSITION, 1);
				break;
			}
			if (!(pp.arg_style & STYLE_gnu))
				ppop(PP_PEDANTIC, 1);
			if (pp.state & PASSTHROUGH)
			{
				if (pp.state & COMPILE)
				{
					pp.state &= ~PASSTHROUGH;
					error(1, "passthrough ignored for compile");
				}
				else
				{
					ppop(PP_COMPATIBILITY, 1);
					ppop(PP_HOSTDIR, "-", 1);
					ppop(PP_SPACEOUT, 1);
					set(&pp.state, DISABLE, va_arg(ap, int));
				}
			}

			/*
			 * create the hash tables
			 */

			if (!pp.symtab) pp.symtab = hashalloc(NiL, HASH_name, "symbols", 0);
			if (!pp.dirtab)
			{
				pp.dirtab = hashalloc(REFONE, HASH_name, "directives", 0);
				inithash(pp.dirtab, directives);
			}
			if (!pp.filtab) pp.filtab = hashalloc(REFALL, HASH_name, "files", 0);
			if (!pp.prdtab) pp.prdtab = hashalloc(REFALL, HASH_name, "predicates", 0);
			if (!pp.strtab)
			{
				pp.strtab = hashalloc(REFALL, HASH_name, "strings", 0);
				inithash(pp.strtab, options);
				inithash(pp.strtab, predicates);
				inithash(pp.strtab, variables);
			}

			/*
			 * mark macros that are builtin predicates
			 */

			for (kp = predicates; s = kp->name; kp++)
			{
				if (!ppisid(*s)) s++;
				ppassert(DEFINE, s, 0);
			}

			/*
			 * the remaining entry names must be allocated
			 */

			hashset(pp.dirtab, HASH_ALLOCATE);
			hashset(pp.filtab, HASH_ALLOCATE);
			hashset(pp.prdtab, HASH_ALLOCATE);
			hashset(pp.strtab, HASH_ALLOCATE);
			hashset(pp.symtab, HASH_ALLOCATE);
			if (pp.test & TEST_nonoise)
			{
				c = error_info.trace;
				error_info.trace = 0;
			}
#if DEBUG
			if (!(pp.test & TEST_noinit))
			{
#endif

			/*
			 * compose, push and read the builtin initialization script
			 */

			if (!(sp = sfstropen()))
				error(3, "temporary buffer allocation error");
			sfprintf(sp,
"\
#%s %s:%s \"/#<assert> /\" \"/assert /%s #/\"\n\
#%s %s:%s \"/#<unassert> /\" \"/unassert /%s #/\"\n\
",
				dirname(PRAGMA),
				pp.pass,
				keyname(X_MAP),
				dirname(DEFINE),
				dirname(PRAGMA),
				pp.pass,
				keyname(X_MAP),
				dirname(UNDEF));
			if (pp.ppdefault && *pp.ppdefault)
			{
				if (pp.probe)
				{
					s = pp.lastdir->next->name;
					pp.lastdir->next->name = 0;
				}
				if (ppsearch(pp.ppdefault, T_STRING, SEARCH_EXISTS) < 0)
				{
					free(pp.ppdefault);
					if (!(pp.ppdefault = pathprobe(pp.path, NiL, "C", pp.pass, pp.probe ? pp.probe : PPPROBE, 0)))
						error(1, "cannot determine default definitions for %s", pp.probe ? pp.probe : PPPROBE);
				}
				if (pp.ppdefault) sfprintf(sp, "#%s \"%s\"\n", dirname(INCLUDE), pp.ppdefault);
				if (pp.probe)
					pp.lastdir->next->name = s;
			}
			while (pp.firstop)
			{
				switch (pp.firstop->op)
				{
				case PP_ASSERT:
					sfprintf(sp, "#%s #%s\n", dirname(DEFINE), pp.firstop->value);
					break;
				case PP_DEFINE:
					if (*pp.firstop->value == '#') sfprintf(sp, "#%s %s\n", dirname(DEFINE), pp.firstop->value);
					else
					{
						if (s = strchr(pp.firstop->value, '='))
							sfprintf(sp, "#%s %-.*s %s\n", dirname(DEFINE), s - pp.firstop->value, pp.firstop->value, s + 1);
						else
							sfprintf(sp, "#%s %s 1\n", dirname(DEFINE), pp.firstop->value);
					}
					break;
				case PP_DIRECTIVE:
					sfprintf(sp, "#%s\n", pp.firstop->value);
					break;
				case PP_OPTION:
					if (s = strchr(pp.firstop->value, '='))
						sfprintf(sp, "#%s %s:%-.*s %s\n", dirname(PRAGMA), pp.pass, s - pp.firstop->value, pp.firstop->value, s + 1);
					else
						sfprintf(sp, "#%s %s:%s\n", dirname(PRAGMA), pp.pass, pp.firstop->value);
					break;
				case PP_READ:
					sfprintf(sp, "#%s \"%s\"\n", dirname(INCLUDE), pp.firstop->value);
					break;
				case PP_UNDEF:
					sfprintf(sp, "#%s %s\n", dirname(UNDEF), pp.firstop->value);
					break;
				}
				pp.lastop = pp.firstop;
				pp.firstop = pp.firstop->next;
				free(pp.lastop);
			}
			sfprintf(sp,
"\
#%s %s:%s\n\
#%s %s:%s\n\
#%s !#%s(%s)\n\
#%s !#%s(%s) || #%s(%s)\n\
"
				, dirname(PRAGMA)
				, pp.pass
				, keyname(X_BUILTIN)
				, dirname(PRAGMA)
				, pp.pass
				, keyname(X_PREDEFINED)
				, dirname(IF)
				, keyname(X_OPTION)
				, keyname(X_PLUSPLUS)
				, dirname(IF)
				, keyname(X_OPTION)
				, keyname(X_COMPATIBILITY)
				, keyname(X_OPTION)
				, keyname(X_TRANSITION)
				);
			sfprintf(sp,
"\
#%s __STDC__\n\
#%s __STDC__ #(STDC)\n\
#%s\n\
#%s #%s(%s)\n\
#%s %s:%s\n\
#%s %s:%s\n\
#%s __STRICT__ 1\n\
#%s\n\
#%s\n\
"
				, dirname(IFNDEF)
				, dirname(DEFINE)
				, dirname(ENDIF)
				, dirname(IF)
				, keyname(X_OPTION)
				, keyname(X_STRICT)
				, dirname(PRAGMA)
				, pp.pass
				, keyname(X_ALLMULTIPLE)
				, dirname(PRAGMA)
				, pp.pass
				, keyname(X_READONLY)
				, dirname(DEFINE)
				, dirname(ENDIF)
				, dirname(ENDIF)
				);
			for (kp = readonlys; s = kp->name; kp++)
			{
				if (!ppisid(*s)) s++;
				sfprintf(sp, "#%s %s\n", dirname(UNDEF), s);
			}
			sfprintf(sp,
"\
#%s\n\
#%s __STDPP__ 1\n\
#%s %s:no%s\n\
"
				, dirname(ENDIF)
				, dirname(DEFINE)
				, dirname(PRAGMA)
				, pp.pass
				, keyname(X_PREDEFINED)
				);
			if (!pp.truncate) sfprintf(sp,
"\
#%s __STDPP__directive #(%s)\n\
"
				, dirname(DEFINE)
				, keyname(V_DIRECTIVE)
				);
			for (kp = variables; s = kp->name; kp++)
				if (ppisid(*s) || *s++ == '+')
				{
					t = *s == '_' ? "" : "__";
					sfprintf(sp, "#%s %s%s%s #(%s)\n" , dirname(DEFINE), t, s, t, s);
				}
			sfprintf(sp,
"\
#%s %s:no%s\n\
#%s %s:no%s\n\
"
				, dirname(PRAGMA)
				, pp.pass
				, keyname(X_READONLY)
				, dirname(PRAGMA)
				, pp.pass
				, keyname(X_BUILTIN)
				);
			t = sfstruse(sp);
			debug((-9, "\n/* begin initialization */\n%s/* end initialization */", t));
			ppcomment = pp.comment;
			pp.comment = 0;
			pplinesync = pp.linesync;
			pp.linesync = 0;
			PUSH_INIT(pp.pass, t);
			pp.mode |= INIT;
			while (pplex());
			pp.mode &= ~INIT;
			pp.comment = ppcomment;
			pp.linesync = pplinesync;
			pp.prefix = 0;
			sfstrclose(sp);
			if (error_info.trace)
				for (dp = pp.firstdir; dp; dp = dp->next)
					message((-1, "include directory %s%s%s", dp->name, dp->hosted ? " [HOSTED]" : "", dp->c ? " [C]" : ""));
#if DEBUG
			}
			if (pp.test & TEST_nonoise) error_info.trace = c;
#endif
			{
				/*
				 * this is sleazy but at least it's
				 * hidden in the library
				 */
#include <preroot.h>
#if FS_PREROOT
				struct pplist*	preroot;

				if ((preroot = (struct pplist*)hashget(pp.prdtab, "preroot")))
					setpreroot(NiL, preroot->value);
#endif
			}
			if (pp.ignoresrc)
			{
				if (pp.ignoresrc > 1 && pp.stddirs != pp.firstdir)
					error(1, "directories up to and including %s are for \"...\" include files only", pp.stddirs->name);
				pp.lcldirs = pp.lcldirs->next;
			}
			if (pp.ignore)
			{
				if (*pp.ignore) ignore(pp.ignore);
				else pp.ignore = 0;
			}
			if (pp.standalone) pp.state |= STANDALONE;
			ppfsm(FSM_PLUSPLUS, NiL);
			pp.initialized = 1;
		}
#if CHECKPOINT
		if (pp.mode & DUMP)
		{
			if (!pp.pragma)
				error(3, "#%s must be enabled for checkpoints", dirname(PRAGMA));
			(*pp.pragma)(dirname(PRAGMA), pp.pass, keyname(X_CHECKPOINT), pp.checkpoint, 1);
		}
#endif
		if (n = pp.filedeps.flags)
		{
			if (!(n & PP_deps_file))
			{
				pp.state |= NOTEXT;
				pp.option |= KEEPNOTEXT;
				pp.linesync = 0;
			}
			if (n & PP_deps_generated)
				pp.mode |= GENDEPS;
			if (n & PP_deps_local)
				pp.mode &= ~HEADERDEPS;
			else if (!(pp.mode & FILEDEPS))
				pp.mode |= HEADERDEPS;
			pp.mode |= FILEDEPS;
		}

		/*
		 * push the main input file -- special case for hosted mark
		 */

		if (pp.firstdir->hosted) pp.mode |= MARKHOSTED;
		else pp.mode &= ~MARKHOSTED;
#if CHECKPOINT
		if (!(pp.mode & DUMP))
#endif
		{
			if (!(p = error_info.file)) p = "";
			else
			{
				error_info.file = 0;
				if (*p)
				{
					pathcanon(p, 0);
					p = ppsetfile(p)->name;
				}
			}
			PUSH_FILE(p, 0);
		}
		if (pp.mode & FILEDEPS)
		{
			if (s = strrchr(error_info.file, '/'))
				s++;
			else
				s = error_info.file;
			if (!*s)
				s = "-";
			s = strcpy(pp.tmpbuf, s);
			if ((t = p = strrchr(s, '.')) && (*++p == 'c' || *p == 'C'))
			{
				if (c = *++p)
					while (*++p == c);
				if (*p)
					t = 0;
				else
					t++;
			}
			if (!t)
			{
				t = s + strlen(s);
				*t++ = '.';
			}
			*(t + 1) = 0;
			if (pp.state & NOTEXT)
				pp.filedeps.sp = sfstdout;
			else
			{
				*t = 'd';
				if (!(pp.filedeps.sp = sfopen(NiL, s, "w")))
					error(ERROR_SYSTEM|3, "%s: cannot create", s);
			}
			*t = 'o';
			pp.column = sfprintf(pp.filedeps.sp, "%s :", s);
			if (*error_info.file)
				pp.column += sfprintf(pp.filedeps.sp, " %s", error_info.file);
		}
		if (xp = pp.firsttx)
		{
			if (!(sp = sfstropen()))
				error(3, "temporary buffer allocation error");
			while (xp)
			{
				sfprintf(sp, "#%s \"%s\"\n", dirname(INCLUDE), xp->value);
				xp = xp->next;
			}
			t = sfstruse(sp);
			PUSH_BUFFER("options", t, 1);
			sfstrclose(sp);
		}
		break;
	case PP_INPUT:
#if CHECKPOINT && POOL
		if (!(pp.mode & DUMP) || pp.pool.input)
#else
#if CHECKPOINT
		if (!(pp.mode & DUMP))
#else
#if POOL
		if (pp.pool.input)
#endif
#endif
#endif
		{
			p = va_arg(ap, char*);
			if (!error_info.file)
				error_info.file = p;
			close(0);
			if (open(p, O_RDONLY) != 0)
				error(ERROR_SYSTEM|3, "%s: cannot read", p);
			if (strmatch(p, "*.(s|S|as|AS|asm|ASM)"))
			{
				set(&pp.mode, CATLITERAL, 0);
				ppop(PP_SPACEOUT, 1);
			}
			break;
		}
		/*FALLTHROUGH*/
	case PP_TEXT:
		if (pp.initialized) goto before;
		if ((p = va_arg(ap, char*)) && *p)
		{
			if (pp.lasttx) pp.lasttx = pp.lasttx->next = newof(0, struct oplist, 1, 0);
			else pp.firsttx = pp.lasttx = newof(0, struct oplist, 1, 0);
			pp.lasttx->op = op;
			pp.lasttx->value = p;
		}
		break;
	case PP_KEYARGS:
		if (pp.initialized) goto before;
		set(&pp.option, KEYARGS, va_arg(ap, int));
		if (pp.option & KEYARGS)
#if MACKEYARGS
			set(&pp.mode, CATLITERAL, 1);
#else
			error(3, "preprocessor not compiled with macro keyword arguments enabled [MACKEYARGS]");
#endif
		break;
	case PP_LINE:
		pp.linesync = va_arg(ap, PPLINESYNC);
		break;
	case PP_LINEBASE:
		if (va_arg(ap, int)) pp.flags |= PP_linebase;
		else pp.flags &= ~PP_linebase;
		break;
	case PP_LINEFILE:
		if (va_arg(ap, int)) pp.flags |= PP_linefile;
		else pp.flags &= ~PP_linefile;
		break;
	case PP_LINEID:
		if (!(p = va_arg(ap, char*))) pp.lineid = "";
		else if (*p != '-') pp.lineid = strdup(p);
		else pp.option |= IGNORELINE;
		break;
	case PP_LINETYPE:
		if ((n = va_arg(ap, int)) >= 1) pp.flags |= PP_linetype;
		else pp.flags &= ~PP_linetype;
		if (n >= 2) pp.flags |= PP_linehosted;
		else pp.flags &= ~PP_linehosted;
		break;
	case PP_LOCAL:
		if (pp.initialized) goto before;
		pp.ignoresrc++;
		pp.stddirs = pp.lastdir;
		break;
	case PP_MACREF:
		pp.macref = va_arg(ap, PPMACREF);
		break;
	case PP_MULTIPLE:
		set(&pp.mode, ALLMULTIPLE, va_arg(ap, int));
		break;
	case PP_NOHASH:
		set(&pp.option, NOHASH, va_arg(ap, int));
		break;
	case PP_NOISE:
		op = va_arg(ap, int);
		set(&pp.option, NOISE, op);
		set(&pp.option, NOISEFILTER, op < 0);
		break;
	case PP_OPTARG:
		pp.optarg = va_arg(ap, PPOPTARG);
		break;
	case PP_OUTPUT:
		pp.outfile = va_arg(ap, char*);
		if (identical(pp.outfile, 0))
			error(3, "%s: identical to input", pp.outfile);
		close(1);
		if (open(pp.outfile, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH) != 1)
			error(ERROR_SYSTEM|3, "%s: cannot create", pp.outfile);
		break;
	case PP_PASSTHROUGH:
		if (!(pp.state & COMPILE))
			set(&pp.state, PASSTHROUGH, va_arg(ap, int));
		break;
	case PP_PEDANTIC:
		set(&pp.mode, PEDANTIC, va_arg(ap, int));
		break;
	case PP_PLUSCOMMENT:
		set(&pp.option, PLUSCOMMENT, va_arg(ap, int));
		if (pp.initialized) ppfsm(FSM_PLUSPLUS, NiL);
		break;
	case PP_PLUSPLUS:
		set(&pp.option, PLUSPLUS, va_arg(ap, int));
		set(&pp.option, PLUSCOMMENT, va_arg(ap, int));
		if (pp.initialized) ppfsm(FSM_PLUSPLUS, NiL);
		break;
	case PP_POOL:
		if (pp.initialized) goto before;
		if (va_arg(ap, int))
		{
#if POOL
			pp.pool.input = dup(0);
			pp.pool.output = dup(1);
			p = "/dev/null";
			if (!identical(p, 0))
			{
				if (!identical(p, 1))
					ppop(PP_OUTPUT, p);
				ppop(PP_INPUT, p);
			}
#else
			error(3, "preprocessor not compiled with input pool enabled [POOL]");
#endif
		}
		break;
	case PP_PRAGMA:
		pp.pragma = va_arg(ap, PPPRAGMA);
		break;
	case PP_PROBE:
		pp.probe = va_arg(ap, char*);
		break;
	case PP_QUOTE:
		p = va_arg(ap, char*);
		c = va_arg(ap, int);
		if (p) ppfsm(c ? FSM_QUOTADD : FSM_QUOTDEL, p);
		break;
	case PP_REGUARD:
		set(&pp.option, REGUARD, va_arg(ap, int));
		break;
	case PP_RESERVED:
		if ((pp.state & COMPILE) && (p = va_arg(ap, char*)))
		{
			if (!(sp = sfstropen()))
				error(3, "temporary buffer allocation error");
			sfputr(sp, p, -1);
			p = sfstruse(sp);
			if (s = strchr(p, '=')) *s++ = 0;
			else s = p;
			while (*s == '_') s++;
			for (t = s + strlen(s); t > s && *(t - 1) == '_'; t--);
			if (*t == '_') *t = 0;
			else t = 0;
			op = ((key = ppkeyref(pp.symtab, s)) && (key->sym.flags & SYM_LEX)) ? key->lex : T_NOISE;
			if (pp.test & 0x0400) error(1, "reserved#1 `%s' %d", s, op);
			if (t) *t = '_';
			if (!(key = ppkeyget(pp.symtab, p)) || !(key->sym.flags & SYM_LEX))
			{
				if (key) free(key);
				key = ppkeyset(pp.symtab, NiL);
			}
			if (!(key->sym.flags & SYM_KEYWORD))
			{
				key->sym.flags |= SYM_KEYWORD|SYM_LEX;
				key->lex = op;
				if (pp.test & 0x0400) error(1, "reserved#2 `%s' %d", p, op);
			}
			sfstrclose(sp);
		}
		break;
	case PP_SPACEOUT:
		set(&pp.state, SPACEOUT, va_arg(ap, int));
		break;
	case PP_STANDALONE:
		if (pp.initialized) goto before;
		pp.standalone = 1;
		break;
	case PP_STANDARD:
		pp.lastdir->next->name = ((p = va_arg(ap, char*)) && *p) ? p : NiL;
		for (dp = pp.firstdir; dp; dp = dp->next)
			if (dp->name)
				for (hp = pp.firstdir; hp != dp; hp = hp->next)
					if (hp->name && streq(hp->name, dp->name))
					{
						hp->c = dp->c;
						hp->hosted = dp->hosted;
					}
		break;
	case PP_STRICT:
		set(&pp.state, TRANSITION, 0);
		pp.flags &= ~PP_transition;
		set(&pp.state, STRICT, va_arg(ap, int));
		if (pp.state & STRICT) pp.flags |= PP_strict;
		else pp.flags &= ~PP_strict;
		break;
	case PP_TEST:
		if (p = va_arg(ap, char*)) for (;;)
		{
			while (*p == ' ' || *p == '\t') p++;
			for (s = p; n = *s; s++)
				if (n == ',' || n == ' ' || n == '\t')
				{
					*s++ = 0;
					break;
				}
			if (!*p) break;
			n = 0;
			if (*p == 'n' && *(p + 1) == 'o')
			{
				p += 2;
				op = 0;
			}
			else op = 1;
			if (streq(p, "count")) n = TEST_count;
			else if (streq(p, "hashcount")) n = TEST_hashcount;
			else if (streq(p, "hashdump")) n = TEST_hashdump;
			else if (streq(p, "hit")) n = TEST_hit;
			else if (streq(p, "init")) n = TEST_noinit|TEST_INVERT;
			else if (streq(p, "noise")) n = TEST_nonoise|TEST_INVERT;
			else if (streq(p, "proto")) n = TEST_noproto|TEST_INVERT;
			else if (*p >= '0' && *p <= '9') n = strtoul(p, NiL, 0);
			else
			{
				error(1, "%s: unknown test", p);
				break;
			}
			if (n & TEST_INVERT)
			{
				n &= ~TEST_INVERT;
				op = !op;
			}
			if (op) pp.test |= n;
			else pp.test &= ~n;
			p = s;
			debug((-4, "test = 0%o", pp.test));
		}
		break;
	case PP_TRANSITION:
		set(&pp.state, STRICT, 0);
		pp.flags &= ~PP_strict;
		set(&pp.state, TRANSITION, va_arg(ap, int));
		if (pp.state & TRANSITION) pp.flags |= PP_transition;
		else pp.flags &= ~PP_transition;
		break;
	case PP_TRUNCATE:
		if (pp.initialized) goto before;
		if ((op = va_arg(ap, int)) < 0) op = 0;
		set(&pp.option, TRUNCATE, op);
		if (pp.option & TRUNCATE)
		{
			Hash_bucket_t*		b;
			Hash_bucket_t*		p;
			Hash_position_t*	pos;
			Hash_table_t*		tab;

			pp.truncate = op;
			tab = pp.symtab;
			pp.symtab = hashalloc(NiL, HASH_set, tab ? HASH_ALLOCATE : 0, HASH_compare, trunccomp, HASH_hash, trunchash, HASH_name, "truncate", 0);
			if (tab && (pos = hashscan(tab, 0)))
			{
				if (p = hashnext(pos))
					do
					{
						b = hashnext(pos);
						hashlook(pp.symtab, (char*)p, HASH_BUCKET|HASH_INSTALL, NiL);
					} while (p = b);
				hashdone(pos);
			}
		}
		else pp.truncate = 0;
		break;
	case PP_WARN:
		set(&pp.state, WARN, va_arg(ap, int));
		break;
	before:
		error(3, "ppop(%d): preprocessor operation must be done before PP_INIT", op);
		break;
	default: 
		error(3, "ppop(%d): invalid preprocessor operation", op);
		break;
	}
	va_end(ap);
}
