/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1997-2008 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * Glenn Fowler
 * AT&T Research
 *
 * cdb support library
 *
 * Cdbdisc_t.schema:	schema description string
 *
 * {<codeset>}c
 *	string representation codeset (CC_NATIVE)
 * {<delimiter>}[+-]d
 *	field delimiter char (CDB_DELIMITER)
 * {<terminator>}t
 *	record terminator char (CDB_TERMINATOR)
 * {<inz>}o
 *	field options (i:CDB_INVALID n:CDB_NEGATIVE s:CDB_SPACE z:CDB_ZERO)
 * {<library> <type> ...}l
 *	load types support functions from library
 * <permanent>p
 *	number of permanent fields
 * [<count>*][<width>[.<base>]][{<type>}<fFiIsS>
 *	fis	{FLOATING,INTEGER,STRING} field with delimiter
 *	FIS	{FLOATING,INTEGER,STRING} field no delimiter
 *
 * example:	"{;}d3p2*3I4i321*i"
 */

static const char id[] = "\n@(#)$Id: cdb library 2.2 (AT&T Research) 2008-05-11 $\0\n";

#include "cdblib.h"

Cdbstate_t	state =
{
	"libcdb:cdb",	/* id		*/
	0,		/* dll		*/
	0,		/* maps		*/
	&_Cdb,		/* methods	*/
	cdbexternal	/* types	*/
};

/*
 * std pseudo paths
 */

static struct
{
	const char*	name;
	const char*	mode;
	Sfio_t*		io;
}
std[] =
{
	{ "/dev/stdin",  "r" },
	{ "/dev/stdout", "w" }
};

/*
 * return meth corresp[onding to the magic
 *	!<cdb-METHOD[-MAJOR.MINOR]>\n
 * io position is not advanced
 */

static Cdbmeth_t*
magicmeth(register Cdb_t* cdb)
{
	register char*	s;
	register char*	e;
	register char*	m;
	register char*	t;
	register int	c;
	char*		b;
	Cdbmeth_t*	r;
	char		name[64];

	if (!(s = b = (char*)sfreserve(cdb->io, SF_UNBOUND, SF_LOCKR)))
		return 0;
	e = s + sfvalue(cdb->io);
	r = 0;
	m = "!<cdb-";
	while (c = *m++)
	{
		if (s >= e || *s++ != c)
			goto done;
	}
	t = name;
	for (;;)
	{
		if (s >= e)
			goto done;
		if (*s == '-' || *s == '>')
			break;
		if (t >= &name[sizeof(name)-1])
			goto done;
		*t++ = *s++;
	}
	*t = 0;
	if (*s++ == '-')
	{
		for (;;)
		{
			if (s >= e)
				goto done;
			if (*s != '.' && !isdigit(*s))
				break;
			s++;
		}
		if (*s++ != '>')
			goto done;
	}
	if (*s != '\n')
		goto done;
	r = cdbgetmeth(name);
 done:
	sfread(cdb->io, b, 0);
	return r;
}

/*
 * discipline struct changes are handled by recording all previous
 * versions here; cdbopen() checks disc.version to determine which
 * one to use
 */

typedef struct
{
	Cdbuint_t	version;	/* CDB_VERSION			*/
	const char*	schema;		/* schema descriptor		*/
	const char*	comment;	/* format specific comment	*/
	const char*	details;	/* format specific details	*/
	const char*	lib;		/* pathfind() lib		*/
	Error_f		errorf;		/* error function		*/
	Cdbevent_f	eventf;		/* event function		*/
	Cdbindex_f	indexf;		/* key index function		*/
} Disc_pre_19980811_t;

/*
 * open a new cdb stream
 */

Cdb_t*
cdbopen(register Cdb_t* cdb, register Cdbdisc_t* disc, Cdbmeth_t* meth, Sfio_t* io, const char* path, int flags)
{
	register int		i;
	register int		w;
	register Cdbformat_t*	fp;
	register Cdbschema_t*	sp;
	Cdbschema_t**		tp;
	Cdbmeth_t*		mp;
	Vmalloc_t*		vp;
	char*			s;
	int			reopen;

	if (cdb)
	{
		/*
		 * shut down the current io and open a new one
		 */

		reopen = 1;
		if (cdb->meth.eventf)
			(*cdb->meth.eventf)(cdb, CDB_CLOSE + reopen);
		if (cdb->closeio)
		{
			cdb->closeio = 0;
			sfclose(cdb->io);
		}
		cdb->io = 0;
		if (disc && disc != cdb->odisc || meth && cdb->methods || flags && (flags & (CDB_READ|CDB_WRITE)) != (cdb->flags & (CDB_READ|CDB_WRITE)))
		{
			if (disc->errorf)
				(*disc->errorf)(cdb, disc, ERROR_SYSTEM|2, "%s: %s: reopen may not change discpline, method or CDB_READ|CDB_WRITE flags", cdb->path);
			goto bad;
		}
		disc = cdb->disc;
		if (cdb->methods)
			meth = cdb->methods;
	}
	else
	{
		/*
		 * allocate and initialize the cdb handle
		 * format has to wait until the schema parse
		 * which may not be until hdrreadf
		 *
		 * all allocations are done in a separate region
		 */

		reopen = 0;
		if (disc->version < 19970000L || disc->version >= 21000000L)
			return 0;
		if (!(vp = vmopen(Vmdcheap, Vmlast, 0)))
			goto nospace;
		w = 0;
		if (disc->version < 19980811L)
			w += sizeof(Cdbdisc_t);
		if (!(cdb = vmnewof(vp, 0, Cdb_t, 1, w)))
		{
			vmclose(vp);
			goto nospace;
		}
		cdb->odisc = disc;
		if (disc->version < 19980811L)
		{
			cdb->disc = (Cdbdisc_t*)(cdb + 1);
			(*(Disc_pre_19980811_t*)cdb->disc) = (*(Disc_pre_19980811_t*)disc);
			disc = cdb->disc;
		}
		else
			cdb->disc = disc;
		cdb->vm = vp;
		if (!(cdb->buf = sfstropen()) || !(cdb->cvt = sfstropen()) || !(cdb->tmp = sfstropen()) || !(cdb->txt = sfstropen()))
			goto nospace;
		cdb->id = state.id;
		cdb->major = CDB_MAJOR;
		cdb->minor = CDB_MINOR;
		cdb->flags = flags;
		cdb->options = CDB_OPT_DEFAULT;
		cdb->defopts = 1;
	}

	/*
	 * set up the io stream
	 */

	w = (cdb->flags & CDB_WRITE) != 0;
	if (!disc->eventf || !(i = (*disc->eventf)(cdb, CDB_INIT, (void*)path, disc)))
	{
		std[0].io = sfstdin;
		std[1].io = sfstdout;
		if (!path || streq(path , "-") || streq(path, std[w].name))
		{
			path = std[w].name;
			if (!io)
				io = std[w].io;
		}
		else if (!io)
		{
			if (!(io = sfopen(NiL, path, std[w].mode)))
			{
				if (disc->errorf)
					(*disc->errorf)(cdb, disc, ERROR_SYSTEM|2, "%s: cannot open", path);
				goto bad;
			}
			cdb->closeio = 1;
		}
		sfset(io, SF_SHARE, 0);
		cdb->io = io;
		if (!(cdb->path = vmstrdup(cdb->vm, path)))
			goto nospace;

		/*
		 * push any stack methods if recognized
		 */

		if (meth && (meth->flags & CDB_STACK))
		{
			mp = meth;
			meth = 0;
		}
		else if (w)
			mp = 0;
		else
			for (mp = state.methods; mp; mp = mp->next)
				if ((mp->flags & CDB_STACK) && mp->recognizef && (*mp->recognizef)(cdb) >= 0)
					break;
#if 0
		if (mp)
			do
			{
				if ((*mp->eventf)(cdb, CDB_PUSH) < 0)
				{
					if (disc->errorf)
						(*disc->errorf)(cdb, disc, 2, "%s: %s method stack error", path, mp->name);
					goto bad;
				}
				sfputr(cdb->txt, mp->name, '.');
			} while ((mp = mp->next) && (mp->flags & CDB_STACK) && mp->recognizef && (*mp->recognizef)(cdb) >= 0);
#else
		if (mp)
		{
			if ((*mp->eventf)(cdb, CDB_PUSH) < 0)
			{
				if (disc->errorf)
					(*disc->errorf)(cdb, disc, 2, "%s: %s method stack error", path, mp->name);
				goto bad;
			}
			sfputr(cdb->txt, mp->name, '.');
		}
#endif

		/*
		 * if no explicit method then see if the data can self-identify
		 */

		if (!meth)
		{
			meth = state.methods;
			if (!w)
				for (;;)
				{
					if (!(meth->flags & CDB_STACK) && meth->recognizef && (*meth->recognizef)(cdb) >= 0)
						break;
					if (!(meth = meth->next))
					{
						if (meth = magicmeth(cdb))
							break;
						if (disc->eventf && (*disc->eventf)(cdb, CDB_METH, (void*)&meth, disc) >= 0 && meth)
							break;

						if (disc->errorf)
							(*disc->errorf)(cdb, disc, 2, "%s: unknown format", path);
						goto bad;
					}
				}
		}
		cdb->meth = *meth;
		if (sfstrtell(cdb->txt))
		{
			sfputr(cdb->txt, meth->name, -1);
			if (!(s = sfstruse(cdb->txt)))
				goto nospace;
			cdb->meth.name = (const char*)vmstrdup(cdb->vm, s);
		}
	}
	else if (i < 0)
		goto bad;
	if (cdb->meth.eventf && (*cdb->meth.eventf)(cdb, CDB_METH) < 0)
		goto bad;
	if (!reopen)
	{

		/*
		 * read the discipline schema
		 */

		if (cdb->disc->schema && cdbparse(cdb, cdb->disc->schema) < 0)
			goto bad;
	}

	/*
	 * the read/write callout sequences are slightly different
	 * because write requires all info from the discipline
	 * whereas read may get some info from the data file
	 */

	if (!w && cdb->meth.hdrreadf && (*cdb->meth.hdrreadf)(cdb) < 0)
		goto bad;
	if (!cdb->table)
		goto invalid;
	if (!cdb->comment)
	{
		if (!(s = (char*)cdb->disc->comment))
		{
			if (cdb->label)
				sfprintf(cdb->tmp, "%s ", cdb->label);
			sfprintf(cdb->tmp, "data from %s", cdb->path);
			if (!(s = sfstruse(cdb->tmp)))
				goto nospace;
		}
		if (!(cdb->comment = vmstrdup(cdb->vm, s)))
			goto nospace;
	}
	if (w && cdb->meth.hdrwritef && (*cdb->meth.hdrwritef)(cdb) < 0)
		goto bad;
	if (cdb->meth.eventf && (*cdb->meth.eventf)(cdb, CDB_INIT + reopen) < 0)
		goto bad;
	if (cdb->disc->eventf && (*cdb->disc->eventf)(cdb, CDB_OPEN + reopen, NiL, cdb->disc) < 0)
		goto bad;
	if (!reopen)
	{
		for (tp = cdb->table; sp = *tp; tp++)
		{
			i = 0;
			w = sp->terminator.chr;
			for (fp = sp->format; fp < sp->format + sp->fields; fp++)
			{
				if (sp->fixed)
				{
					fp->delimiter.str = 0;
					fp->delimiter.chr = -1;
				}
				else if (fp->delimiter.chr == w)
					sp->delimterm = 1;
				else if (!(i += fp->delimiter.dir))
				{
					sp->delimterm = 0;
					w = -2;
				}
				if (!(fp->flags & (CDB_FLOATING|CDB_INTEGER|CDB_STRING)))
				{
					if ((sp->fixed || fp->delimiter.chr != w) && fp >= sp->format + cdb->common)
						continue;
					fp->flags |= fp->type;
				}
				sp->referenced = fp - sp->format + 1;
				if (fp->flags & CDB_STRING)
				{
					sp->strings = fp - sp->format + 1;
					if (sp->strings > cdb->strings)
						cdb->strings = sp->strings;
				}
			}
		}
		if (cdb->partitions == 1)
		{
			cdb->common = cdb->table[0]->referenced;
			cdb->record->schema = cdb->table[0];
		}
	}
	if (cdb->meth.eventf && (*cdb->meth.eventf)(cdb, CDB_OPEN + reopen) < 0)
		goto bad;
	return cdb;
 nospace:
	cdbnospace(cdb);
	goto bad;
 invalid:
	if (disc->errorf)
		(*disc->errorf)(cdb, disc, 2, "%s: invalid schema", path);
 bad:
	cdbclose(cdb);
	return 0;
}

/*
 * initialize the current record data
 */

int
cdbdata(register Cdb_t* cdb)
{
	if (!(cdb->record = vmnewof(cdb->vm, 0, Cdbrecord_t, 1, cdb->fields * sizeof(Cdbdata_t))) || !(cdb->record->vm = vmopen(Vmdcheap, Vmlast, 0)))
		return -1;
	cdb->record->data = (Cdbdata_t*)(cdb->record + 1);
	return 0;
}

/*
 * initialize cdb by parsing the schema
 */

int
cdbparse(register Cdb_t* cdb, const char* schema)
{
	register char*		s;
	register int		i;
	register int		c;
	register int		m;
	register int		w;
	register int		d;
	register Cdbformat_t*	fp;
	register Cdbschema_t*	sp;
	char*			e;
	char*			t;
	char*			v;
	char*			delimiter;
	char*			escape;
	char*			quotebegin;
	char*			quoteend;
	char*			separator;
	int			code;
	int			comfix;
	int			fields;
	int			total;
	int			level;
	int			options;
	int			partition;
	int			physicals;
	int			virtuals;
	int			end;
	int			nest;
	Cdbtype_t*		type;
	Cdbschema_t*		tail;
	Cdbschema_t*		xp;
	Cdbschema_t*		np;
	Cdbschema_t**		tp;

	/*
	 * copy schema to cdb->cvt for easy string manipulation
	 */

	if (*schema == '<')
	{
		Sfio_t*	io = 0;

		if (!(v = (char*)sfstrrsrv(cdb->tmp, PATH_MAX)))
			goto nospace;
		if (!pathfind(schema + 1, cdb->disc->lib, "schema", v, sfvalue(cdb->tmp)) || !(io = sfopen(NiL, v, "r")) || sfmove(io, cdb->cvt, SF_UNBOUND, -1) < 0 || !sfeof(io) || sferror(cdb->cvt))
		{
			if (io)
				sfclose(io);
			if (cdb->disc->errorf)
				(*cdb->disc->errorf)(cdb, cdb->disc, ERROR_SYSTEM|2, "%s: %s: cannot read", cdb->path, io ? v : (char*)schema + 1);
			return -1;
		}
		sfclose(io);
		sfstrseek(cdb->tmp, 0, SEEK_SET);
		if (!cdb->label && !(cdb->label = vmstrdup(cdb->vm, schema + 1)))
			goto nospace;
	}
	else
		sfputr(cdb->cvt, schema, 0);
	if (!(schema = (const char*)sfstruse(cdb->cvt)))
		goto nospace;

	/*
	 * if we were here already then cdb->schema is the canonical
	 * schema from the application and schema is from the data file
	 * (canonical too) so they better match; if they do its a nop
	 * otherwise its a hard error
	 */

	if (cdb->schema)
	{
		if (cdb->major == 1 && strtol(schema, &e, 10) == cdb->fields && *e == 'p')
			schema = (const char*)e;
		if (!(m = streq(cdb->schema, schema)))
		{
			Cdbmap_t*	map;

			/*
			 * only the physical fields must match
			 */

			if (map = cdbmapopen(cdb, "ipt", NiL))
				m = (s = cdbschema(cdb, map)) && streq(s, schema);
			cdbmapclose(map);
			if (!m)
			{
				if (cdb->disc->errorf)
					(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: data schema `%s' does not match application `%s'", cdb->path, schema, cdb->schema);
				return -1;
			}
		}
		return 0;
	}

	/*
	 * two passes through the schema
	 * the first pass counts the number of fields
	 * and then allocates cdb->table for the second pass
	 */

	tail = 0;
	comfix = 0;
	code = CDB_CCODE;
	options = cdb->options;
	fp = 0;
	m = 1;
	w = 0;
	i = -1;
	v = 0;
	for (;;)
	{
		s = (char*)schema;
		delimiter = CDB_DELIMITER;
		escape = 0;
		fields = total = 0;
		partition = level = 0;
		quotebegin = 0;
		quoteend = 0;
		virtuals = physicals = 0;
		d = 0;
		for (;;)
		{
			switch (c = *s++)
			{
			case ' ':
			case '\t':
			case '\n':
				continue;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				if (i >= 0)
					i = i * 10 + c - '0';
				else
					w = w * 10 + c - '0';
				continue;
			case '.':
				i = 0;
				continue;
			case '*':
				m = w;
				w = 0;
				continue;
			case '-':
				d = -1;
				continue;
			case '+':
				d = 1;
				continue;
			case '{':
				nest = 1;
				v = s;
				for (;;)
				{
					if (!(c = *s++))
					{
						if (cdb->disc->errorf)
							(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: %s: unbalanced {...}", cdb->path, v - 1);
						return -1;
					}
					if (c == '{')
						nest++;
					else if (c == '}' && --nest <= 0)
						break;
				}
				if ((s - v) <= 1)
					v = 0;
				else if (fp)
				{
					*(s - 1) = 0;
					stresc(v);
				}
				continue;
			case '[':
				if (!partition)
				{
					if (!v)
						goto noid;
					cdb->common = fields;
					if (fp)
					{
						if (!(cdb->identify = vmstrdup(cdb->vm, v)))
							goto nospace;
						comfix = sp->fixed;
						tail = sp;
					}
				}
				if (fp)
				{
					sp->fields = fields;
					sp->code = code;
					if (partition)
					{
						np = cdb->table[partition];
						np->index = partition;
						if (level > 0)
						{
							xp = cdb->table[level - 1]->head;
							if (xp->tail)
								xp->tail = xp->tail->next = np;
							else
								xp->partition = xp->tail = np;
						}
						else
							tail = tail->next = np;
						memcpy(np->format = fp, sp->format, cdb->common * sizeof(Cdbformat_t));
						fp += cdb->common;
						if ((np->permanent = sp->permanent) > cdb->common && np->permanent < UINT_MAX)
							np->permanent = 0;
						sp = np;
					}
					else if (!v)
						goto noid;
					else if (!(cdb->identify = vmstrdup(cdb->vm, v)))
						goto nospace;
					else
					{
						comfix = sp->fixed;
						tail = sp;
					}
					xp = cdb->table[level];
					xp->head = sp;
					xp->tail = 0;
					sp->fixed = comfix;
				}
				for (v = s; isalnum(*s); s++);
				i = s - v;
				if (!(c = *s++))
					goto invalid;
				do
				{
					if (!*s)
						goto invalid;
				} while (*s++ != c);
				if (fp)
				{
					sp->sep = c;
					m = s - v;
					if (!(t = vmoldof(cdb->vm, 0, char, m, 0)))
						goto nospace;
					memcpy(t, v, --m);
					t[i++] = t[m] = 0;
					sp->value.string.base = t + i;
					sp->value.string.length = m - i;
					if (!*t)
					{
						sfprintf(cdb->tmp, "PART_%d", partition + 1);
						if (!(e = sfstruse(cdb->tmp)) || !(t = vmstrdup(cdb->vm, e)))
							goto nospace;
					}
					sp->name = (const char*)t;
				}
				if (cdb->fields < fields)
					cdb->fields = fields;
				fields = cdb->common;
				total += cdb->common;
				partition++;
				level++;
				goto reset;
			case ']':
				if (!level--)
					goto unbalanced;
				goto reset;
			case 'c':
				if (fp)
				{
					if (!v)
						code = CC_NATIVE;
					else if ((code = ccmapid(v)) < 0)
					{
						code = CC_NATIVE;
						if (cdb->disc->errorf)
							(*cdb->disc->errorf)(cdb, cdb->disc, 1, "%s: %s: unknown character code set -- assuming native", cdb->path, v);
					}
				}
				goto reset;
			case 'd':
				if (fp)
				{
					if (!v)
						delimiter = 0;
					else if (!(delimiter = vmstrdup(cdb->vm, v)))
						goto nospace;
				}
				goto reset;
			case 'e':
				if (fp)
				{
					if (!v)
						escape = 0;
					else if (!(escape = vmstrdup(cdb->vm, v)))
						goto nospace;
				}
				goto reset;
			case 'o':
				if (!fp || !v)
					options = 0;
				else
					for (options = 0;;)
					{
						switch (*v++)
						{
						case 0:
							break;
						case 'i':
							options |= CDB_INVALID;
							continue;
						case 'n':
							options |= CDB_NEGATIVE;
							continue;
						case 's':
							options |= CDB_SPACE;
							continue;
						case 'z':
							options |= CDB_ZERO;
							continue;
						}
						break;
					}
				goto reset;
			case 'p':
				if (fp)
					sp->permanent = (s - 1) == (char*)schema || !isdigit(*(s - 2)) ? UINT_MAX : m * w;
				goto reset;
			case 'q':
				if (fp)
				{
					if (!v)
						quotebegin = 0;
					else if (!(quotebegin = vmstrdup(cdb->vm, v)))
						goto nospace;
				}
				goto reset;
			case 'Q':
				if (fp)
				{
					if (!v)
						quoteend = 0;
					else if (!(quoteend = vmstrdup(cdb->vm, v)))
						goto nospace;
				}
				goto reset;
			case 't':
				if (fp && v && !(sp->terminator.str = vmstrdup(cdb->vm, v)))
					goto nospace;
				goto reset;
			case 'v':
			case 'V':
				if (virtuals)
				{
					if (cdb->disc->errorf)
						(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: nested virtuals not supported [%d] `%s'", cdb->path, virtuals, schema);
					return -1;
				}
				m *= w;
				w = (c == 'V') + 1;
				if (fp)
				{
					virtuals = m;
					physicals = w;
				}
				goto reset;
			case 'z':
			case 'Z':
				cdb->sized = c;
				goto reset;
			case 'b':
				c = CDB_STRING|CDB_BINARY;
				separator = delimiter;
				goto set;
			case 'B':
				c = CDB_STRING|CDB_BINARY;
				separator = 0;
				goto set;
			case 'f':
				c = CDB_FLOATING;
				separator = delimiter;
				goto set;
			case 'F':
				c = CDB_FLOATING;
				separator = 0;
				goto set;
			case 'i':
				c = CDB_INTEGER;
				separator = delimiter;
				goto set;
			case 'I':
				c = CDB_INTEGER;
				separator = 0;
				goto set;
			case 'l':
				c = CDB_LONG|CDB_INTEGER;
				separator = delimiter;
				goto set;
			case 'L':
				c = CDB_LONG|CDB_INTEGER;
				separator = 0;
				goto set;
			case 's':
				c = CDB_STRING;
				separator = delimiter;
				goto set;
			case 'S':
				c = CDB_STRING;
				separator = 0;
				goto set;
			case 'x':
				c = CDB_IGNORE;
				separator = delimiter;
				goto set;
			case 'X':
				c = CDB_IGNORE;
				separator = 0;
				goto set;
			case 'u':
				c = CDB_UNSIGNED|CDB_INTEGER;
				separator = delimiter;
				goto set;
			case 'U':
				c = CDB_UNSIGNED|CDB_INTEGER;
				separator = 0;
				goto set;
			case 'w':
				c = CDB_UNSIGNED|CDB_LONG|CDB_INTEGER;
				separator = delimiter;
				goto set;
			case 'W':
				c = CDB_UNSIGNED|CDB_LONG|CDB_INTEGER;
				separator = 0;
				goto set;
			set:
				fields += m;
				total += m;
				if (fp)
				{
					if (v)
					{
						/*
						 * external type with
						 * optional data
						 */

						if (e = strchr(v, ':'))
							*e++ = 0;
						for (type = state.types; type; type = type->next)
							if (streq(v, type->name))
								break;
						if (!type)
						{
							if (cdb->disc->errorf)
								(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: %s: unknown type", cdb->path, v);
							return -1;
						}
						if (e && !(e = vmstrdup(cdb->vm, e)))
							goto nospace;
						if (!(type->flags & c))
						{
							if (cdb->disc->errorf)
								(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: %s type not supported by %s", cdb->path, cdbtypes(cdb, c), type->name);
							return -1;
						}
						if (type->initf && !(type->flags & CDB_INITIALIZED))
						{
							type->flags |= CDB_INITIALIZED;
							if ((*type->initf)(cdb, type) < 0)
							{
								if (cdb->disc->errorf)
									(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: %s: external type initialization error", cdb->path, type->name);
								return -1;
							}
						}
					}
					else
						type = 0;
					if (!w)
					{
						if (!type && *s && *s != ']')
							separator = delimiter;
						if (!virtuals || physicals)
							sp->variable = 1;
					}
					else if (!virtuals || physicals)
					{
						sp->fixed += m * w;
						if (separator)
							sp->variable = 1;
					}
					end = separator ? ccmapc(*separator, CC_NATIVE, code) : -1;
					if (i <= ((c & CDB_FLOATING) ? -1 : 0))
						i = 10;
					while (m-- > 0)
					{
						fp->delimiter.str = separator;
						fp->delimiter.chr = end;
						fp->delimiter.dir = d;
						fp->escape.chr = (fp->escape.str = escape) ? ccmapc(*escape, CC_NATIVE, code) : -1;
						fp->quotebegin.chr = (fp->quotebegin.str = quotebegin) ? ccmapc(*quotebegin, CC_NATIVE, code) : -1;
						fp->quoteend.chr = (fp->quoteend.str = quoteend) ? ccmapc(*quoteend, CC_NATIVE, code) : fp->quotebegin.chr;
						d = 0;
						fp->width = w;
						fp->base = i;
						if (virtuals)
						{
							virtuals--;
							if (physicals)
							{
								physicals--;
								c |= CDB_PHYSICAL;
								if (fp > sp->format && (fp-1)->virtuals)
								{
									c |= CDB_VIRTUAL;
									fp->virtuals = 1;
								}
								else
									fp->virtuals = virtuals;
							}
							else
								c |= CDB_VIRTUAL;
							c |= CDB_BINARY;
							fp->code = CC_NATIVE;
						}
						else
							fp->code = code;
						fp->flags = c | options;
						fp->type = c & (CDB_FLOATING|CDB_INTEGER|CDB_STRING);
						fp->ptype = c & (CDB_PHYSICAL|CDB_UNSIGNED|CDB_LONG|CDB_FLOATING|CDB_INTEGER|CDB_STRING);
						fp->ultype = c & (CDB_UNSIGNED|CDB_LONG|CDB_FLOATING|CDB_INTEGER|CDB_STRING);
						if (fp->external = type)
						{
							fp->details = e;
							if (type->flags & CDB_BINARY)
							{
								fp->flags |= CDB_BINARY;
								fp->code = CC_NATIVE;
							}
							if (type->validatef && (*type->validatef)(cdb, fp, type) < 0)
							{
								if (cdb->disc->errorf)
									(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: %s type with width %d not supported by %s", cdb->path, cdbtypes(cdb, fp->type), fp->width, type->name);
								return -1;
							}
						}
						fp++;
					}
				}
			reset:
				m = 1;
				w = 0;
				i = -1;
				v = 0;
				continue;
			case 0:
				break;
			default:
				if (cdb->disc->errorf)
					(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: %s: invalid character in schema descriptor", cdb->path, s - 1);
				return -1;
			}
			break;
		}
		if (cdb->fields < fields)
			cdb->fields = fields;
		if (fp)
		{
			sp->fields = fields;
			sp->code = code;
			break;
		}
		if (level)
			goto unbalanced;
		if (!partition)
			partition = 1;
		cdb->partitions = partition;
		total -= cdb->common;
		if (!(tp = vmnewof(cdb->vm, 0, Cdbschema_t*, partition + 1, partition * sizeof(Cdbschema_t) + total * sizeof(Cdbformat_t))))
			goto nospace;
		cdb->table = tp;
		sp = (Cdbschema_t*)(tp + partition + 1);
		while (tp < cdb->table + partition)
			*tp++ = sp++;
		fp = (Cdbformat_t*)sp;
		sp = cdb->table[0];
		sp->format = fp;
	}
	for (tp = cdb->table; sp = *tp; tp++)
	{
		if (!sp->name)
			sp->name = "DEFAULT";
		if (sp->permanent > sp->fields)
			sp->permanent = sp->fields;
		if (sp->variable)
			sp->fixed = 0;
		if (cdb->sized)
		{
			sp->fixed = 0;
			sp->format->flags |= CDB_PHYSICAL;
			sp->format->ptype |= CDB_PHYSICAL;
		}
		else if (!sp->fixed && !sp->terminator.str)
			sp->terminator.str = CDB_TERMINATOR;
		sp->terminator.chr = sp->terminator.str ? ccmapc(*sp->terminator.str, CC_NATIVE, sp->code) : -1;
	}
	cdb->table[0]->head = cdb->table[0];

	/*
	 * generate the canonical schema
	 */

	if (!(s = cdbschema(cdb, NiL)))
		return -1;
	if (!(cdb->schema = vmstrdup(cdb->vm, s)))
		goto nospace;
	if (cdbdata(cdb) < 0)
		goto nospace;
	return 0;
 invalid:
	if (cdb->disc->errorf)
		(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: invalid schema", cdb->path);
	return -1;
 noid:
	if (cdb->disc->errorf)
		(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: partition identification expression omitted", cdb->path);
	return -1;
 nospace:
	cdbnospace(cdb);
	return -1;
 unbalanced:
	if (cdb->disc->errorf)
		(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: unbalanced [...]", cdb->path);
	return -1;
}

/*
 * output schema string value with op and optional sign
 */

static void
schemastring(Sfio_t* bp, char* s, int c, int d)
{
	register int	x;

	if (s)
	{
		sfputc(bp, '{');
		s = fmtesc(s);
		while (x = *s++)
		{
			if (x == '{' | x == '}')
				sfputc(bp, '\\');
			sfputc(bp, x);
		}
		sfputc(bp, '}');
		if (d > 0)
			sfputc(bp, '+');
		else if (d < 0)
			sfputc(bp, '-');
	}
	sfputc(bp, c);
}

/*
 * generate and return a canoncal schema from cdb.format
 */

char*
cdbschema(register Cdb_t* cdb, Cdbmap_t* map)
{
	register Cdbformat_t*	fp;
	register Cdbformat_t*	ip;
	register Cdbschema_t*	sp;
	register Cdbconvert_t*	cp;
	register char*		s;
	register int		c;
	register int		options;
	register int		i;
	register int		ie;
	register int		im;
	int			x;
	char*			delimiter;
	char*			escape;
	char*			quotebegin;
	char*			quoteend;
	char*			terminator;
	int			code;
	int			flags;
	int			identified;
	int			permanent;
	int			level;
	Sfio_t*			bp;

	static const char	sep[] = ":@%;,/!+=| \001";

	bp = cdb->buf;
	code = CDB_CCODE;
	delimiter = CDB_DELIMITER;
	escape = 0;
	level = 0;
	options = CDB_OPT_DEFAULT;
	permanent = 0;
	quotebegin = 0;
	quoteend = 0;
	terminator = CDB_TERMINATOR;
	i = 0;
	sp = cdb->table[0];
	if (map)
	{
		flags = map->flags;
		cp = map->schema[0].convert;
		ie = ((identified = !cdb->identify) ? map->schema[0].ofields : cdb->common);
		if ((flags & CDB_MAP_SIZED) && cp->input.index == 0)
			sfputc(bp, cdb->sized ? cdb->sized : 'z');
	}
	else
	{
		flags = CDB_MAP_DEFAULT;
		cp = 0;
		ie = ((identified = !cdb->identify) ? sp->fields : cdb->common);
		if (cdb->sized)
			sfputc(bp, cdb->sized);
	}
	for (;;)
	{
		if (sp->terminator.str && (sp->fixed || !streq(sp->terminator.str, terminator)))
			schemastring(bp, terminator = sp->terminator.str, 't', 0);
		if (sp->permanent != permanent && (permanent >= 0 || sp->permanent < sp->fields))
		{
			if (sp->permanent < sp->fields)
				sfprintf(bp, "%d", sp->permanent);
			else
				permanent = -1;
			sfputc(bp, 'p');
		}
		for (; i < ie; i = im)
		{
			im = i + 1;
			if (cp)
			{
				if ((x = cp[i].input.index) < 0)
				{
					sfputc(bp, 'x');
					continue;
				}
				fp = sp->format + x;
			}
			else
				fp = sp->format + i;
			if (fp->code != code && !(fp->flags & CDB_BINARY) && !(flags & CDB_MAP_NATIVE))
				schemastring(bp, ccmapname(code = fp->code), 'c', 0);
			if ((fp->flags & CDB_OPT) != options)
			{
				if (options = fp->flags & CDB_OPT)
				{
					sfputc(bp, '{');
					if (options & CDB_INVALID)
						sfputc(bp, 'i');
					if (options & CDB_NEGATIVE)
						sfputc(bp, 'n');
					if (options & CDB_SPACE)
						sfputc(bp, 's');
					if (options & CDB_ZERO)
						sfputc(bp, 'z');
					sfputc(bp, '}');
				}
				sfputc(bp, 'o');
			}
			if (fp->delimiter.str && !strsame(fp->delimiter.str, delimiter))
				schemastring(bp, delimiter = fp->delimiter.str, 'd', fp->delimiter.dir);
			if (!strsame(fp->escape.str, escape))
				schemastring(bp, escape = fp->escape.str, 'e', 0);
			if (!strsame(fp->quotebegin.str, quotebegin))
				schemastring(bp, quotebegin = fp->quotebegin.str, 'q', 0);
			if (!strsame(fp->quoteend.str, quoteend))
				schemastring(bp, quoteend = fp->quoteend.str, 'Q', 0);
			if ((fp->flags & (CDB_PHYSICAL|CDB_VIRTUAL)) == CDB_VIRTUAL && (!cdb->sized || fp > sp->format) && (flags & (CDB_MAP_PHYSICAL|CDB_MAP_VIRTUAL)) == (CDB_MAP_PHYSICAL|CDB_MAP_VIRTUAL))
				sfprintf(bp, "%d%c", fp->virtuals + 1, ((fp+1)->flags & CDB_PHYSICAL) ? 'V' : 'v');
			for (c = 1; im < ie; im++)
			{
				if (cp)
				{
					if ((x = cp[im].input.index) < 0)
						break;
					ip = sp->format + x;
				}
				else
					ip = sp->format + im;
				if (ip->ptype != fp->ptype && (fp > sp->format || !cdb->sized || ip->type != fp->type))
					break;
				if (flags & CDB_MAP_DELIMIT)
				{
					if (ip->delimiter.str && fp->delimiter.str && !strsame(ip->delimiter.str, fp->delimiter.str))
						break;
				}
				else
				{
					if (ip->width != fp->width)
						break;
					if (!strsame(ip->delimiter.str, fp->delimiter.str))
						break;
					if (!strsame(ip->escape.str, fp->escape.str))
						break;
					if (!strsame(ip->quotebegin.str, fp->quotebegin.str))
						break;
					if (!strsame(ip->quoteend.str, fp->quoteend.str))
						break;
				}
				if (ip->base != fp->base)
					break;
				if ((ip->external != fp->external || !strsame(ip->details, fp->details)) && (flags & CDB_MAP_TYPES))
					break;
				if (ip->code != fp->code && !((fp->flags|ip->flags) & CDB_BINARY) && !(flags & CDB_MAP_NATIVE))
					break;
				c++;
			}
			if (c > 1)
				sfprintf(bp, "%d*", c);
			if (fp->width && !(flags & CDB_MAP_DELIMIT))
				sfprintf(bp, "%d", fp->width);
			if (fp->base != 10)
				sfprintf(bp, ".%d", fp->base);
			if (fp->external && (flags & CDB_MAP_TYPES))
			{
				if (fp->details)
					sfprintf(bp, "{%s:%s}", fp->external->name, fp->details);
				else
					sfprintf(bp, "{%s}", fp->external->name);
			}
			switch (fp->ultype)
			{
			case CDB_FLOATING:
				c = 'f';
				break;
			case CDB_INTEGER:
				c = 'i';
				break;
			case CDB_UNSIGNED|CDB_INTEGER:
				c = 'u';
				break;
			case CDB_LONG|CDB_INTEGER:
				c = 'l';
				break;
			case CDB_UNSIGNED|CDB_LONG|CDB_INTEGER:
				c = 'w';
				break;
			case CDB_STRING:
				c = (fp->flags & CDB_BINARY) ? 'b' : 's';
				break;
			case CDB_IGNORE:
				c = 'x';
				break;
			}
			if (!fp->delimiter.str && !(flags & CDB_MAP_DELIMIT))
				c = toupper(c);
			sfputc(bp, c);
		}
		if (!identified)
		{
			identified = 1;
			sfprintf(bp, "{%s}", cdb->identify);
			cdb->table[level]->head = sp->next;
		}
		else if (sp->partition)
		{
			sp = sp->partition;
			cdb->table[++level]->head = sp->next;
		}
		else if (!(sp = sp->next))
		{
			if (cdb->partitions <= 1)
				break;
			sfputc(bp, ']');
			while (level-- > 0)
			{
				sfputc(bp, ']');
				if (sp = cdb->table[level]->head)
					break;
			}
			if (level < 0)
				break;
			cdb->table[level]->head = sp->next;
		}
		i = cdb->common;
		if (map)
		{
			cp = map->schema[sp->index].convert;
			ie = map->schema[sp->index].ofields;
		}
		else
			ie = sp->fields;
		if (!(c = sp->sep))
			for (s = (char*)sep; (c = *s++) && strchr(sp->value.string.base, c););
		sfprintf(bp, "[%s%c%s%c", sp->name, c, sp->value.string.base, c);
	}
	return sfstruse(bp);
}

/*
 * close an open cdb
 * return <0 on error
 */

int
cdbclose(register Cdb_t* cdb)
{
	register int		i;
	register Cdbrecord_t*	cp;

	if (!cdb)
		return -1;
	if (cdb->disc->eventf && (i = (*cdb->disc->eventf)(cdb, CDB_CLOSE, NiL, cdb->disc)) < 0)
		return i;
	if (cdb->meth.eventf)
		(*cdb->meth.eventf)(cdb, CDB_CLOSE);
	if (cdb->closeio)
		sfclose(cdb->io);
	if (cdb->buf)
		sfstrclose(cdb->buf);
	if (cdb->cvt)
		sfstrclose(cdb->cvt);
	if (cdb->tmp)
		sfstrclose(cdb->tmp);
	if (cdb->txt)
		sfstrclose(cdb->txt);
	for (cp = cdb->cache; cp; cp = cp->next)
		if (cp->vm)
			vmclose(cp->vm);
	for (cp = cdb->free; cp; cp = cp->next)
		if (cp->vm)
			vmclose(cp->vm);
	if (cdb->vm)
		vmclose(cdb->vm);
	return 0;
}
