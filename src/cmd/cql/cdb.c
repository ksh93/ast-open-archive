/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1991-2000 AT&T Corp.              *
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
 * cql db format filter
 */

static const char usage[] =
"[-?\n@(#)cdb (AT&T Labs Research) 1999-05-01\n]"
USAGE_LICENSE
"[+NAME?cdb - display cql data files]"
"[+DESCRIPTION?\bcdb\b displays \bcql\b(1) and \bcdb\b(3) data files."
"	Input data information is optional if the input is self describing."
"	Otherwise the deafult input format is \bflat\b, the default input"
"	schema is best guess, and the default input comment is empty. The"
"	default output format is \bflat\b, the default output schema is the"
"	same as the input schema with external types and virtuals omitted, and"
"	the default output comment is the same as the input comment.]"

"[c:count?Print the total number of records on exit.]"
"[d:debug?Set the debug trace \alevel\a. Higher levels produce more output.]#"
"	[level]"
"[h:identify?Display header identification information and exit.]"
"[i:input?The following options describe the input data. Options describe the"
"	input by default.]:?[options...]"
"[l:library?\alib\a is loaded as a runtime \bdll\b. \alib\a must define"
"	the function \bcdb_init\b. See \bcdb\b(3).]:[lib]"
"[m:methods?List all available methods and exit.]"
"[o:output?The following options describe the output data.]:?[options...]"

"[B:bufsize?Sets the data io buffer size to \asize\a.]#[size]"
"[C:comment?Sets the data comment to \astring\a.]:[string]"
"[D:details?Sets the method details to \astring\a.]:[string]"
"[X:dump?Enables record level trace messages.]"
"[F:format?The data format (method). Formats can be composed using a \b:\b"
"	separated list of these names:]:[name]{"
"	[+cdb?For sparse records. \bpzip\b may perform better.]"
"	[+flat?Delimiter separated or fixed width fields, terminator"
"		separated or fixed length records.]"
"	[+gzip?\bgzip\b(1) compression.]"
"	[+pzip?\bpzip\b(1) compression.]"
"	[+vdelta?\bvdelta\b(1) differencing and/or compression.]"
"}"
"[H:header?Generate and/or read data identification header.]"
"[M:map?Set the record data map to \aname\a.]:[name]"
"[P:pack?Set the \bCDB_PACK\b hint.]"
"[R:raw?Display raw record data.]"
"[S:schema?Set the record data schema to \aname\a composed of:]:[name]{"
"	[+TBD?TBD]"
"}"
"[E:terminated?An otherwise fixed length record also has a record terminator.]"
"[T:test?Add the implementation defined \amask\a to the data/library"
"	test mask.]#[mask]"
"[V:verbose?Enable verbose trace messages.]"
"[W:watch?Enable \b--dump\b and \b--verbose\b after \acount\a records are"
"	processed.]#[count]"

"\n"
"\n[ file ... ]\n"
"\n"

"[+SEE ALSO?\bcql\b(1), \bgzip\b(1), \bod\b(1), \bpzip\b(1),"
"	\bvdelta\b(1), \bcdb\b(3)]"
;

#include <cdb.h>
#include <error.h>
#include <sfstr.h>

typedef struct
{
	Cdbdisc_t	disc;
	Cdbmeth_t*	meth;
	char*		map;
	int		bufsize;
	int		flags;
	Sfoff_t		watch;
} Db_t;

#define COUNT		(1<<0)
#define HEADER		(1<<1)
#define METHODS		(1<<2)

/*
 * event function to capture CDB_OPEN for record count only
 */

static int
eventf(register Cdb_t* cdb, int op, void* arg, Cdbdisc_t* disc)
{
	register Cdbschema_t**	tp;
	register Cdbschema_t*	sp;
	register Cdbformat_t*	fp;

	NoP(arg);
	NoP(disc);
	if (op == CDB_OPEN)
		for (tp = cdb->table; sp = *tp; tp++)
			for (fp = sp->format; fp < sp->format + sp->fields; fp++)
				fp->flags = 0;
	return 0;
}

/*
 * list available methods, types and maps
 */

static void
methods(register Sfio_t* op)
{
	register Cdbmapmeth_t*	map;
	register Cdbmeth_t*	meth;
	register Cdbtype_t*	type;

	if (meth = cdbgetmeth(NiL))
	{
		sfprintf(op, "            --- formats ---\n");
		do
		{
			sfprintf(op, "%10s  %s", meth->name, meth->description);
			if (meth->flags)
				sfprintf(op, " [%s]", cdbtypes(NiL, meth->flags));
			sfputc(op, '\n');
		} while (meth = meth->next);
	}
	if (type = cdbgettype(NiL))
	{
		sfprintf(op, "            --- types ---\n");
		do
		{
			sfprintf(op, "%10s  %s for %s\n", type->name, type->description, cdbtypes(NiL, type->flags));
		} while (type = type->next);
	}
	if (map = cdbgetmap(NiL))
	{
		sfprintf(op, "            ---- maps ---\n");
		do
		{
			sfprintf(op, "%10s  %s", map->name, map->description);
			if (map->flags)
				sfprintf(op, " [%s]", cdbflags(NiL, map->flags));
			sfputc(op, '\n');
		} while (map = map->next);
	}
}

/*
 * set options from line if != 0 or argv according to usage
 */

static void
set(char* line, char** argv, const char* usage, int* op, Db_t* db, Db_t* input, Db_t* output)
{
	int	n;
	int	index;
	int	offset;

	if (line)
	{
		index = opt_info.index;
		offset = opt_info.offset;
	}
	for (;;)
	{
		switch (line ? optstr(line, usage) : optget(argv, usage))
		{
		case 'c':
			*op |= COUNT;
			continue;
		case 'd':
			error_info.trace = -opt_info.num;
			continue;
		case 'h':
			*op |= HEADER;
			continue;
		case 'i':
			db = input;
			if (opt_info.arg)
				set(opt_info.arg, NiL, usage, op, db, input, output);
			continue;
		case 'l':
			cdblib(opt_info.arg, 1, &input->disc);
			continue;
		case 'm':
			*op |= METHODS;
			continue;
		case 'o':
			db = output;
			if (opt_info.arg)
				set(opt_info.arg, NiL, usage, op, db, input, output);
			continue;
		case 'B':
			db->bufsize = opt_info.num;
			continue;
		case 'C':
			if (opt_info.arg)
				db->disc.comment = strdup(opt_info.arg);
			continue;
		case 'D':
			if (opt_info.arg)
				db->disc.details = strdup(opt_info.arg);
			continue;
		case 'X':
			if (opt_info.num)
				db->flags |= CDB_DUMP;
			else
				db->flags &= ~CDB_DUMP;
			continue;
		case 'F':
			if (opt_info.arg && !(db->meth = cdbgetmeth(opt_info.arg)))
				error(3, "%s: unknown format", opt_info.arg);
			continue;
		case 'H':
			if (opt_info.num)
				db->flags |= CDB_HEADER;
			else
				db->flags &= ~CDB_HEADER;
			continue;
		case 'M':
			if (opt_info.arg)
				db->map = strdup(opt_info.arg);
			continue;
		case 'P':
			if (opt_info.num)
				db->flags |= CDB_PACK;
			else
				db->flags &= ~CDB_PACK;
			continue;
		case 'R':
			if (opt_info.num)
				db->flags |= CDB_RAW;
			else
				db->flags &= ~CDB_RAW;
			continue;
		case 'S':
			if (opt_info.arg)
				db->disc.schema = strdup(opt_info.arg);
			continue;
		case 'E':
			if (opt_info.num)
				db->flags |= CDB_TERMINATED;
			else
				db->flags &= ~CDB_TERMINATED;
			continue;
		case 'T':
			switch (opt_info.num & 0x03)
			{
			case 0x01:
				n = CDB_TEST1;
				continue;
			case 0x02:
				n = CDB_TEST2;
				continue;
			case 0x03:
				n = CDB_TEST1|CDB_TEST2;
				continue;
			}
			if (opt_info.name[2] == 'n')
				db->flags &= ~n;
			else
				db->flags |= n;
			continue;
		case 'V':
			if (opt_info.num)
				db->flags |= CDB_VERBOSE;
			else
				db->flags &= ~CDB_VERBOSE;
			continue;
		case 'W':
			db->watch = opt_info.num;
			continue;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			continue;
		case ':':
			error(2, "%s", opt_info.arg);
			continue;
		}
		break;
	}
	if (line)
	{
		opt_info.index = index;
		opt_info.offset = offset;
	}
}

int
main(int argc, char** argv)
{
	register Cdb_t*		idb;
	register Cdb_t*		odb;
	register Cdbschema_t*	sp;
	register Cdbschema_t**	tp;
	register int		n;
	int			op;
	unsigned long		w;
	char*			schema;
	Cdbmap_t*		map;
	Db_t			input;
	Db_t			output;

	error_info.id = "cdb";
	memset(&input, 0, sizeof(input));
	input.disc.version = CDB_VERSION;
	input.disc.errorf = (Cdberror_f)errorf;
	output = input;
	input.flags = CDB_READ;
	output.flags = CDB_WRITE;
	op = 0;
	set(NiL, argv, usage, &op, &input, &input, &output);
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	argv += opt_info.index;
	if (!output.meth)
		output.meth = Cdbflat;
	n = *argv && *(argv + 1);
	odb = 0;
	if (input.watch)
		op |= COUNT;
	else if ((op & COUNT) && !(input.flags & CDB_VERBOSE))
		input.disc.eventf = eventf;
	if (op & METHODS)
		methods(sfstdout);
	else do
	{
		if (idb = cdbopen(NiL, &input.disc, input.meth, NiL, *argv, input.flags))
		{
			if (input.bufsize)
				sfsetbuf(idb->io, NiL, input.bufsize);
			if (op & (COUNT|HEADER))
			{
				if (op & HEADER)
					sfprintf(sfstdout, "%s:\n\tformat=%s\n\tversion=%d.%d\n\tcomment='%s'\n\tschema='%s'\n", idb->path, idb->meth.name, idb->major, idb->minor, idb->comment, idb->schema);
				if (op & COUNT)
				{
					if (input.watch)
					{
						while (--input.watch && cdbread(idb, NiL));
						idb->flags |= CDB_DUMP|CDB_VERBOSE;
					}
					while (cdbread(idb, NiL));
				}
				w = 0;
				for (tp = idb->table; sp = *tp; tp++)
					if (op & HEADER)
					{
						if (idb->partitions > 1)
							sfprintf(sfstdout, "\t%s fields=%d", sp->name, sp->fields);
						else
							sfprintf(sfstdout, "\tfields=%d", sp->fields);
						if (idb->sized)
							sfprintf(sfstdout, " sized=%c", idb->sized);
						if (sp->fixed)
							sfprintf(sfstdout, " fixed=%d", sp->fixed);
						if (op & COUNT)
							sfprintf(sfstdout, " records=%I*u", sizeof(sp->count), sp->count);
						sfputc(sfstdout, '\n');
					}
					else if (sp->count)
					{
						w = 1;
						if (idb->partitions > 1)
							sfprintf(sfstdout, " %11s %11I*u", sp->name, sizeof(sp->count), sp->count);
						else if (n)
							sfprintf(sfstdout, " %11I*u", sizeof(sp->count), sp->count);
						else
							sfprintf(sfstdout, "%I*u", sizeof(sp->count), sp->count);
					}
				if (w)
				{
					if (n)
						sfprintf(sfstdout, " %s\n", idb->path);
					else
						sfputc(sfstdout, '\n');
				}
			}
			else if (!op)
			{
				map = 0;
				if (!odb)
				{
					if (output.map && !(map = cdbmapopen(idb, output.map, NiL)))
						break;
					if (!output.disc.schema)
						output.disc.schema = cdbschema(idb, map);
					if (!output.disc.comment)
					{
						if (output.map)
						{
							sfprintf(idb->tmp, "data from %s map=%s", idb->path, output.map);
							output.disc.comment = sfstruse(idb->tmp);
						}
						else
							output.disc.comment = idb->comment;
					}
					if (!(odb = cdbopen(NiL, &output.disc, output.meth, sfstdout, NiL, output.flags)))
						break;
					if (output.bufsize)
						sfsetbuf(odb->io, NiL, output.bufsize);
					if (n && !(schema = strdup(idb->schema)))
					{
						error(ERROR_SYSTEM|2, "out of space");
						break;
					}
				}
				else if (!streq(schema, idb->schema))
					error(3, "%s: all input schemas must match", idb->path);
				if (cdbmove(idb, odb, map, SF_UNBOUND) < 0)
					break;
				if (map)
					cdbmapclose(map);
				cdbclose(odb);
			}
			cdbclose(idb);
		}
	} while (n && *++argv);
	return error_info.errors != 0;
}
