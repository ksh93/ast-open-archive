/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1985-2000 AT&T Corp.                *
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
*                David Korn <dgk@research.att.com>                 *
*                 Phong Vo <kpv@research.att.com>                  *
*                                                                  *
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * error and message formatter
 *
 *	level is the error level
 *	level >= error_info.core!=0 dumps core
 *	level >= ERROR_FATAL calls error_info.exit
 *	level < 0 is for debug tracing
 *
 * NOTE: id && ERROR_NOID && !ERROR_USAGE implies format=id for errmsg()
 */

#include "loclib.h"

#include <ctype.h>
#include <ccode.h>
#include <namval.h>
#include <sig.h>
#include <stk.h>
#include <times.h>

Error_info_t	_error_info_ =
{
	2, exit, write,
	0,0,0,0,0,0,0,0,
	0,
	0,
	0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,
	0,
	translate,
	0
};

__EXTERN__(Error_info_t, _error_info_);

#undef	ERROR_CATALOG
#define ERROR_CATALOG	(ERROR_LIBRARY<<1)

#define OPT_CATALOG	1
#define OPT_CORE	2
#define OPT_FD		3
#define OPT_LIBRARY	4
#define OPT_MASK	5
#define OPT_SYSTEM	6
#define OPT_TIME	7
#define OPT_TRACE	8

static const Namval_t		options[] =
{
	"catalog",	OPT_CATALOG,
	"core",		OPT_CORE,
	"debug",	OPT_TRACE,
	"fd",		OPT_FD,
	"library",	OPT_LIBRARY,
	"mask",		OPT_MASK,
	"system",	OPT_SYSTEM,
	"time",		OPT_TIME,
	"trace",	OPT_TRACE,
	0,		0
};

/*
 * called by stropt() to set options
 */

static int
setopt(void* a, const void* p, register int n, register const char* v)
{
	NoP(a);
	if (p)
		switch (((Namval_t*)p)->value)
		{
		case OPT_CORE:
			if (n) switch (*v)
			{
			case 'e':
			case 'E':
				error_info.core = ERROR_ERROR;
				break;
			case 'f':
			case 'F':
				error_info.core = ERROR_FATAL;
				break;
			case 'p':
			case 'P':
				error_info.core = ERROR_PANIC;
				break;
			default:
				error_info.core = strtol(v, NiL, 0);
				break;
			}
			else error_info.core = 0;
			break;
		case OPT_CATALOG:
			if (n)
				error_info.set |= ERROR_CATALOG;
			else
				error_info.clear |= ERROR_CATALOG;
			break;
		case OPT_FD:
			error_info.fd = n ? strtol(v, NiL, 0) : -1;
			break;
		case OPT_LIBRARY:
			if (n)
				error_info.set |= ERROR_LIBRARY;
			else
				error_info.clear |= ERROR_LIBRARY;
			break;
		case OPT_MASK:
			if (n)
				error_info.mask = strtol(v, NiL, 0);
			else
				error_info.mask = 0;
			break;
		case OPT_SYSTEM:
			if (n)
				error_info.set |= ERROR_SYSTEM;
			else
				error_info.clear |= ERROR_SYSTEM;
			break;
		case OPT_TIME:
			error_info.time = n ? 1 : 0;
			break;
		case OPT_TRACE:
			if (n)
				error_info.trace = -strtol(v, NiL, 0);
			else
				error_info.trace = 0;
			break;
		}
	return 0;
}

/*
 * print a name with optional delimiter, converting unprintable chars
 */

static void
print(register Sfio_t* sp, register char* name, char* delim)
{
	register int	c;

	while (c = *name++)
	{
		c = ccmapc(c, CC_NATIVE, CC_ASCII);
		if (c & 0200)
		{
			c &= 0177;
			sfputc(sp, '?');
		}
		if (c < ccmapc(' ', CC_NATIVE, CC_ASCII))
		{
			c += ccmapc('A', CC_NATIVE, CC_ASCII) - 1;
			sfputc(sp, '^');
		}
		c = ccmapc(c, CC_ASCII, CC_NATIVE);
		sfputc(sp, c);
	}
	if (delim)
		sfputr(sp, delim, -1);
}

/*
 * print error context FIFO stack
 */

static void
context(register Sfio_t* sp, register Error_context_t* cp)
{
	if (cp->context) context(sp, cp->context);
	if (!(cp->flags & ERROR_SILENT))
	{
		if (cp->id)
			print(sp, cp->id, NiL);
		if (cp->line > ((cp->flags & ERROR_INTERACTIVE) != 0))
		{
			if (cp->file)
				sfprintf(sp, ": \"%s\", %s %d", cp->file, ERROR_translate(NiL, NiL, ast.id, "line"), cp->line);
			else
				sfprintf(sp, "[%d]", cp->line);
		}
		sfputr(sp, ": ", -1);
	}
}

void
error(int level, ...)
{
	va_list	ap;

	va_start(ap, level);
	errorv(NiL, level, ap);
	va_end(ap);
}

void
errorv(const char* id, int level, va_list ap)
{
	register int	n;
	int		fd;
	int		flags;
	char*		s;
	char*		t;
	char*		format;
	char*		library;
	const char*	catalog;

	int		line;
	char*		file;

	unsigned long	d;
	struct tms	us;

	if (!error_info.init)
	{
		error_info.init = 1;
		stropt(getenv("ERROR_OPTIONS"), options, sizeof(*options), setopt, NiL);
	}
	if (level > 0)
	{
		flags = level & ~ERROR_LEVEL;
		level &= ERROR_LEVEL;
	}
	else
		flags = 0;
	if ((flags & (ERROR_USAGE|ERROR_NOID)) == ERROR_NOID)
	{
		format = (char*)id;
		id = 0;
	}
	else
		format = 0;
	if (id)
	{
		catalog = (char*)id;
		if (!*catalog || *catalog == ':')
		{
			catalog = 0;
			library = 0;
		}
		else if ((library = strchr(catalog, ':')) && !*++library)
			library = 0;
	}
	else
	{
		catalog = 0;
		library = 0;
	}
	if (catalog)
		id = 0;
	else
	{
		id = (const char*)error_info.id;
		catalog = error_info.catalog;
	}
	if (level < error_info.trace || (flags & ERROR_LIBRARY) && !(((error_info.set | error_info.flags) ^ error_info.clear) & ERROR_LIBRARY) || level < 0 && error_info.mask && !(error_info.mask & (1<<(-level - 1))))
	{
		if (level >= ERROR_FATAL)
			(*error_info.exit)(level - 1);
		return;
	}
	if (error_info.trace < 0)
		flags |= ERROR_LIBRARY|ERROR_SYSTEM;
	flags |= error_info.set | error_info.flags;
	flags &= ~error_info.clear;
	if (!library)
		flags &= ~ERROR_LIBRARY;
	fd = (flags & ERROR_OUTPUT) ? va_arg(ap, int) : error_info.fd;
	if (error_info.write)
	{
		long	off;
		char*	bas;

		bas = stkptr(stkstd, 0);
		if (off = stktell(stkstd))
			stkfreeze(stkstd, 0);
		file = error_info.id;
		if (flags & ERROR_USAGE)
		{
			if (flags & ERROR_NOID)
				sfprintf(stkstd, "       ");
			else
				sfprintf(stkstd, "%s: ", ERROR_translate(NiL, NiL, ast.id, "Usage"));
			if (file || opt_info.argv && (file = opt_info.argv[0]))
				print(stkstd, file, " ");
		}
		else
		{
			if (level && !(flags & ERROR_NOID))
			{
				if (error_info.context && level > 0)
					context(stkstd, error_info.context);
				if (file)
					print(stkstd, file, (flags & ERROR_LIBRARY) ? " " : ": ");
				if (flags & (ERROR_CATALOG|ERROR_LIBRARY))
				{
					sfprintf(stkstd, "[");
					if (flags & ERROR_CATALOG)
						sfprintf(stkstd, "%s %s%s",
							catalog ? catalog : ERROR_translate(NiL, NiL, ast.id, "DEFAULT"),
							ERROR_translate(NiL, NiL, ast.id, "catalog"),
							(flags & ERROR_LIBRARY) ? ", " : "");
					if (flags & ERROR_LIBRARY)
						sfprintf(stkstd, "%s %s",
							library,
							ERROR_translate(NiL, NiL, ast.id, "library"));
					sfprintf(stkstd, "]: ");
				}
			}
			if (level > 0 && error_info.line > ((flags & ERROR_INTERACTIVE) != 0))
			{
				if (error_info.file && *error_info.file)
					sfprintf(stkstd, "\"%s\", ", error_info.file);
				sfprintf(stkstd, "%s %d: ", ERROR_translate(NiL, NiL, ast.id, "line"), error_info.line);
			}
		}
		if (error_info.time)
		{
			if (error_info.time == 1 || (d = times(&us)) < error_info.time)
				d = error_info.time = times(&us);
			sfprintf(stkstd, " %05lu.%05lu.%05lu ", d - error_info.time, (unsigned long)us.tms_utime, (unsigned long)us.tms_stime);
		}
		switch (level)
		{
		case 0:
			flags &= ~ERROR_SYSTEM;
			break;
		case ERROR_WARNING:
			error_info.warnings++;
			sfprintf(stkstd, "%s: ", ERROR_translate(NiL, NiL, ast.id, "warning"));
			break;
		case ERROR_PANIC:
			error_info.errors++;
			sfprintf(stkstd, "%s: ", ERROR_translate(NiL, NiL, ast.id, "panic"));
			break;
		default:
			if (level < 0)
			{
				s = ERROR_translate(NiL, NiL, ast.id, "debug");
				if (error_info.trace < -1)
					sfprintf(stkstd, "%s%d:%s", s, level, level > -10 ? " " : "");
				else
					sfprintf(stkstd, "%s: ", s);
				for (n = 0; n < error_info.indent; n++)
				{
					sfputc(stkstd, ' ');
					sfputc(stkstd, ' ');
				}
			}
			else
				error_info.errors++;
			break;
		}
		if (flags & ERROR_SOURCE)
		{
			/*
			 * source ([version], file, line) message
			 */

			file = va_arg(ap, char*);
			line = va_arg(ap, int);
			s = ERROR_translate(NiL, NiL, ast.id, "line");
			if (error_info.version)
				sfprintf(stkstd, "(%s: \"%s\", %s %d) ", error_info.version, file, s, line);
			else
				sfprintf(stkstd, "(\"%s\", %s %d) ", file, s, line);
		}
		if (format || (format = va_arg(ap, char*)))
		{
			if (!(flags & ERROR_USAGE))
				format = ERROR_translate(NiL, id, catalog, format);
			sfvprintf(stkstd, format, ap);
		}
		if (!(flags & ERROR_PROMPT))
		{
			/*
			 * level&ERROR_OUTPUT on return means message
			 * already output
			 */

			if (error_info.auxilliary && level >= 0)
				level = (*error_info.auxilliary)(stkstd, level, flags);
			if ((flags & ERROR_SYSTEM) && errno && errno != error_info.last_errno)
			{
				sfprintf(stkstd, " [%s]", fmterror(errno));
				if (error_info.set & ERROR_SYSTEM)
					errno = 0;
				error_info.last_errno = (level >= 0) ? 0 : errno;
			}
			sfputc(stkstd, '\n');
		}
		if (level < 0 || !(level & ERROR_OUTPUT))
		{
			n = stktell(stkstd);
			s = stkptr(stkstd, 0);
			if (t = memchr(s, '\f', n))
			{
				n -= ++t - s;
				s = t;
			}
#if HUH_19980401 /* nasty problems if sfgetr() is in effect! */
			sfsync(sfstdin);
#endif
			sfsync(sfstdout);
			sfsync(sfstderr);
			if (fd == sffileno(sfstderr) && error_info.write == write)
			{
				sfwrite(sfstderr, s, n);
				sfsync(sfstderr);
			}
			else
				(*error_info.write)(fd, s, n);
		}
		else
			level &= ERROR_LEVEL;
		stkset(stkstd, bas, off);
	}
	if (level >= error_info.core && error_info.core)
	{
#ifndef SIGABRT
#ifdef	SIGQUIT
#define SIGABRT	SIGQUIT
#else
#ifdef	SIGIOT
#define SIGABRT	SIGIOT
#endif
#endif
#endif
#ifdef	SIGABRT
		signal(SIGABRT, SIG_DFL);
		kill(getpid(), SIGABRT);
		pause();
#else
		abort();
#endif
	}
	if (level >= ERROR_FATAL)
		(*error_info.exit)(level - ERROR_FATAL + 1);
}
