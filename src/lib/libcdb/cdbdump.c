/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1997-2000 AT&T Corp.                *
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
*                                                                  *
*******************************************************************/
#pragma prototyped

/*
 * cdb debug support
 */

#include "cdblib.h"

/*
 * first entry value is a mask, active if not present
 */

static const Namval_t	typenames[] =
{
	"ignore",		CDB_STRING|CDB_INTEGER|CDB_FLOATING,
	"physical",		CDB_PHYSICAL,
	"virtual",		CDB_VIRTUAL,
	"binary",		CDB_BINARY,
	"unsigned",		CDB_UNSIGNED,
	"long",			CDB_LONG,
	"cached",		CDB_CACHED,
	"string",		CDB_STRING,
	"integer",		CDB_INTEGER,
	"floating",		CDB_FLOATING,
	"INVALID",		CDB_INVALID,
	"NEGATIVE",		CDB_NEGATIVE,
	"SPACE",		CDB_SPACE,
	"ZERO",			CDB_ZERO,
	"INITIALIZED",		CDB_INITIALIZED,
	"STACK",		CDB_STACK,
	"TERMINATED",		CDB_TERMINATED,
	0,			0
};

static const Namval_t	flagnames[] =
{
	"",			0,
	"READ",			CDB_READ,
	"WRITE",		CDB_WRITE,
	"HEADER",		CDB_HEADER,
	"PACK",			CDB_PACK,
	"RAW",			CDB_RAW,
	"TERMINATED",		CDB_TERMINATED,
	"DUMP",			CDB_DUMP,
	"TEST1",		CDB_TEST1,
	"TEST2",		CDB_TEST2,
	"VERBOSE",		CDB_VERBOSE,
	0,			0
};

static const Namval_t	mapflagnames[] =
{
	"",			0,
	"IGNORE",		CDB_MAP_IGNORE,
	"PHYSICAL",		CDB_MAP_PHYSICAL,
	"VIRTUAL",		CDB_MAP_VIRTUAL,
	"DELIMIT",		CDB_MAP_DELIMIT,
	"NATIVE",		CDB_MAP_NATIVE,
	"SIZED",		CDB_MAP_SIZED,
	"TYPES",		CDB_MAP_TYPES,
	0,			0
};

/*
 * low level for cdbtypes(), cdbflags(), cdbmapflags()
 */

static char*
names(Cdb_t* cdb, register const Namval_t* tab, char* lab, register int flags)
{
	register char*		prev;
	register Sfio_t*	sp;
	register int		base;

 	static Sfio_t*		tmp;
 
	if (cdb)
		sp = cdb->tmp;
	else if (!(sp = tmp) && !(sp = tmp = sfstropen()))
		return "CDB-INTERNAL-ERROR";
	base = sfstrtell(sp);
	prev = 0;
	if (tab->value)
	{
		if (!(flags & tab->value))
			prev = tab->name;
		tab++;
	}
	for (; tab->name; tab++)
		if (flags & tab->value)
		{
			if (prev)
				sfputr(sp, prev, '.');
			prev = tab->name;
		}
	if (!prev)
		sfprintf(sp, "%s=0x%04x", lab, flags);
	else if (sfstrtell(sp) == base)
		return prev;
	else
		sfputr(sp, prev, -1);
	sfputc(sp, 0);
	return sfstrset(sp, base);
}

/*
 * return type name and attributes in flags
 */

char*
cdbtypes(Cdb_t* cdb, int flags)
{
	return names(cdb, typenames, "TYPE", flags);
}

/*
 * return flag names in flags
 */

char*
cdbflags(Cdb_t* cdb, int flags)
{
	return names(cdb, flagnames, "FLAGS", flags);
}
/*
 * return map flag names in flags
 */

char*
cdbmapflags(Cdb_t* cdb, int flags)
{
	return names(cdb, mapflagnames, "MAPFLAGS", flags);
}

/*
 * dump the record in data
 */

void
cdbdump(register Cdb_t* cdb, Sfio_t* op, const char* msg, register Cdbrecord_t* rp)
{
	register Cdbschema_t*	sp;
	register Cdbdata_t*	dp;
	register Cdbformat_t*	fp;
	register int		n;
	register char*		s;
	Cdbschema_t**		tp;
	Cdbrecord_t*		mp;
	char*			e;
	int			w;

	if (!(cdb->flags & CDB_DUMPED) || !rp)
	{
		cdb->flags |= CDB_DUMPED;
		s = "================================================================================";
		sfprintf(op, "%s\n%s%s%s format=%s flags=%s", s, msg ? msg : "", msg ? " " : "", cdb->path, cdb->meth.name, cdbflags(cdb, cdb->flags));
		if (cdb->sized)
			sfprintf(op, " sized=%c", cdb->sized);
		if (cdb->disc->details)
			sfprintf(op, " details='%s'", cdb->disc->details);
		sfprintf(op, "\n   schema=%s\n", cdb->schema);
		for (tp = cdb->table; sp = *tp; tp++)
		{
			sfprintf(op, "   partition=%s fields=%d permanent=%d strings=%d referenced=%d%s", sp->name, sp->fields, sp->permanent, sp->strings, sp->referenced, sp->delimterm ? " delimterm" : "");
			if (sp->fixed)
				sfprintf(op, " fixed=%d", sp->fixed);
			if (sp->terminator.str)
				sfprintf(op, " '%s'", fmtesc(sp->terminator.str));
			sfprintf(op, "\n");
			if ((cdb->flags & CDB_VERBOSE) || !rp)
			{
				n = 0;
				w = 0;
				for (fp = sp->format; fp < sp->format + sp->fields; fp++)
				{
					if (n > 0)
						n--;
					else
					{
						if (fp->virtuals)
							n = fp->virtuals;
						w += fp->width;
					}
					sfprintf(op, "   [%04d]", fp - sp->format);
					if (fp->name)
						sfprintf(op, " %10s", fp->name);
					sfprintf(op, " width %2d/%-4d base %-4d %-6s  ", fp->width, w, fp->base, ccmapname(fp->code));
					if (fp->external)
						sfprintf(op, "%s.", fp->external->name);
					sfprintf(op, "%s", cdbtypes(cdb, fp->flags));
					if (fp->quotebegin.str)
					{
						sfprintf(op, " %s", fmtesc(fp->quotebegin.str));
						sfprintf(op, "%s", fmtesc(fp->quoteend.str ? fp->quoteend.str : fp->quotebegin.str));
					}
					if (fp->delimiter.str)
						sfprintf(op, " %c'%s' ", fp->delimiter.dir > 0 ? '+' : fp->delimiter.dir < 0 ? '-' : ' ', fmtesc(fp->delimiter.str));
					sfputc(op, '\n');
				}
			}
		}
		sfprintf(op, "%s\n", s);
	}
	if (rp)
	{
		for (mp = rp; mp; mp = mp->next)
		{
			cdb->count--;
			mp->schema->count--;
		}
		sp = rp->schema;
		cdb->count++;
		sp->count++;
		sfprintf(op, "[%07d/%07d] %s%s%s record %s offset (%07llo) %lld\n", cdb->count + !(cdb->flags & CDB_READ), sp->count + !(cdb->flags & CDB_READ), msg ? msg : "", msg ? " " : "", sp->name, cdb->path, rp->offset, rp->offset);
		for (;;)
		{
			for (dp = rp->data, fp = sp->format; dp < rp->data + sp->fields; dp++, fp++)
				if ((cdb->flags & CDB_VERBOSE) || (dp->flags & fp->type))
				{
					switch (fp->ultype)
					{
					case CDB_FLOATING:
						if (!(cdb->flags & CDB_VERBOSE))
						{
							if ((fp->flags & CDB_NEGATIVE) && dp->number.floating < 0)
								continue;
							if ((fp->flags & CDB_ZERO) && dp->number.floating == 0)
								continue;
						}
						n = sfprintf(op, "   [%04d] %g", dp - rp->data, dp->number.floating);
						break;
					case CDB_INTEGER:
						if (!(cdb->flags & CDB_VERBOSE))
						{
							if ((fp->flags & CDB_NEGATIVE) && dp->number.integer < 0)
								continue;
							if ((fp->flags & CDB_ZERO) && dp->number.integer == 0)
								continue;
						}
						n = sfprintf(op, "   [%04d] %d", dp - rp->data, dp->number.integer);
						break;
					case CDB_UNSIGNED|CDB_INTEGER:
						if (!(cdb->flags & CDB_VERBOSE))
						{
							if ((fp->flags & CDB_ZERO) && dp->number.uinteger == 0)
								continue;
						}
						n = sfprintf(op, "   [%04d] %u", dp - rp->data, dp->number.uinteger);
						break;
					case CDB_LONG|CDB_INTEGER:
						if (!(cdb->flags & CDB_VERBOSE))
						{
							if ((fp->flags & CDB_NEGATIVE) && dp->number.linteger < 0)
								continue;
							if ((fp->flags & CDB_ZERO) && dp->number.linteger == 0)
								continue;
						}
						n = sfprintf(op, "   [%04d] %lld", dp - rp->data, dp->number.linteger);
						break;
					case CDB_UNSIGNED|CDB_LONG|CDB_INTEGER:
						if (!(cdb->flags & CDB_VERBOSE))
						{
							if ((fp->flags & CDB_ZERO) && dp->number.winteger == 0)
								continue;
						}
						n = sfprintf(op, "   [%04d] %llu", dp - rp->data, dp->number.winteger);
						break;
					case CDB_STRING:
						if (!(cdb->flags & CDB_VERBOSE) && dp->string.length == 0)
							continue;
						n = sfprintf(op, "   [%04d] %d \"", dp - rp->data, dp->string.length);
						if (!(fp->flags & CDB_BINARY))
							n += sfprintf(op, "%-.*s\"", dp->string.length, dp->string.base);
						else
						{
							for (e = (s = dp->string.base) + dp->string.length; s < e; s++)
								if (isprint(*s) && !iscntrl(*s))
								{
									sfputc(op, *s);
									n++;
								}
								else
									n += sfprintf(op, "\\%03o", *(unsigned char*)s);
							sfputc(op, '"');
							n++;
						}
						break;
					default:
						if (!(cdb->flags & CDB_VERBOSE))
							continue;
						n = sfprintf(op, "   [%04d] %d", dp - rp->data, fp->width);
						break;
					}
					while (n++ < 24)
						sfputc(op, ' ');
					sfputc(op, ' ');
					if (fp->external)
						sfprintf(op, "%s.", fp->external->name);
					sfprintf(op, "%s", cdbtypes(cdb, (fp->flags&(dp->flags|CDB_PHYSICAL))|(fp->flags&~fp->type)|(fp->flags&CDB_OPT)|dp->flags));
					if (fp->code != CC_NATIVE)
						sfprintf(op, ".%s", ccmapname(fp->code));
					sfputc(op, '\n');
				}
			if (!(rp = rp->next))
				break;
			sp = rp->schema;
			cdb->count++;
			sp->count++;
			sfprintf(op, "[%07d] %s%s%s partition %s\n", sp->count + !(cdb->flags & CDB_READ), msg ? msg : "", msg ? " " : "", sp->name, cdb->path);
		}
	}
}
