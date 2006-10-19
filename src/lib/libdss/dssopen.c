/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*           Copyright (c) 2002-2006 AT&T Knowledge Ventures            *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                      by AT&T Knowledge Ventures                      *
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
 * dss open/close/library/method implementation
 *
 * Glenn Fowler
 * AT&T Research
 */

static const char usage[] =
"[-?\n@(#)$Id: dss library (AT&T Research) 2006-02-02 $\n]"
USAGE_LICENSE
"[+NAME?\findex\f]"
"[+DESCRIPTION?The \bdss\b default method schema is a pure XML (tags only)"
"	file that specifies the \bdss\b method and optional field value maps"
"	and constraints. Public schemas are usually placed in a"
"	\b../lib/dss\b sibling directory on \b$PATH\b. The supported tags"
"	are:]{"
;

#include "dsshdr.h"

#include <dlldefs.h>
#include <pzip.h>
#include <stak.h>

typedef Dsslib_t* (*Dsslib_f)(const char*, Dssdisc_t*);

static const char	id[] = DSS_ID;

static Dssstate_t	state;

/*
 * find and open file for read
 */

Sfio_t*
dssfind(const char* name, const char* suffix, Dssflags_t flags, char* path, size_t size, Dssdisc_t* disc)
{
	Sfio_t*		sp;

	if (!suffix)
		suffix = id;
	if (!pathfind(name, id, suffix, path, size))
	{
		if ((flags & DSS_VERBOSE) && disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: %s file not found", name, suffix);
		return 0;
	}
	else if (!(sp = sfopen(NiL, path, "r")))
	{
		if ((flags & DSS_VERBOSE) && disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "%s: cannot read %s file", path, suffix);
		return 0;
	}
	return sp;
}

/*
 * load tags file
 */

static Dssmeth_t*
loadtags(const char* name, const char* suffix, Dssdisc_t* disc, Dssmeth_t* meth)
{
	Sfio_t*		sp;
	char		path[PATH_MAX];

	if (!(sp = dssfind(name, suffix, DSS_VERBOSE, path, sizeof(path), disc)))
		return 0;
	return dsstags(sp, path, 1, 0, disc, meth);
}

/*
 * dss identf
 */

static int
dssidentf(Dssfile_t* file, void* buf, size_t size, Dssdisc_t* disc)
{
	register char*	s;
	register char*	e;

	s = (char*)buf;
	e = s + size;
	while (s < e && isspace(*s))
		s++;
	if (*s++ != '<')
		return 0;
	if (*s == '!')
		while (s < e && *s++ != '!');
	else
		while (s < e && isalpha(*s))
			s++;
	return s < e && *s == '>';
}

/*
 * dss openf
 */

static int
dssopenf(Dssfile_t* file, Dssdisc_t* disc)
{
	return 0;
}

/*
 * dss readf
 */

static int
dssreadf(Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	return 0;
}

/*
 * dss writef
 */

static int
dsswritef(Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	return 0;
}

/*
 * dss seekf
 */

static Sfoff_t
dssseekf(Dssfile_t* file, Sfoff_t offset, Dssdisc_t* disc)
{
	return 0;
}

/*
 * dss closef
 */

static int
dssclosef(Dssfile_t* file, Dssdisc_t* disc)
{
	return 0;
}

static Dssformat_t	dss_format =
{
	&id[0],
	"pseudo-format that treats all files as /dev/null",
	{0},
	dssidentf,
	dssopenf,
	dssreadf,
	dsswritef,
	dssseekf,
	dssclosef
};

/*
 * dss methf
 */

static Dssmeth_t*
dssmethf(const char* name, const char* options, const char* schema, Dssdisc_t* disc, Dssmeth_t* meth)
{
	Sfio_t*		up;
	char*		us;
	Tagdisc_t	tagdisc;

	if (options)
	{
		if (!(up = sfstropen()))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return 0;
		}
		sfprintf(up, "%s", usage);
		taginit(&tagdisc, disc->errorf);
		if (tagusage(dss_tags, up, &tagdisc))
		{
			sfclose(up);
			return 0;
		}
		sfputc(up, '}');
		sfputc(up, '\n');
		if (!(us = sfstruse(up)))
		{
			sfclose(up);
			return 0;
		}
		for (;;)
		{
			switch (optstr(options, us))
			{
			case '?':
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, ERROR_USAGE|4, "%s", opt_info.arg);
				return 0;
			case ':':
				if (disc->errorf)
					(*disc->errorf)(NiL, disc, 2, "%s", opt_info.arg);
				return 0;
			}
			break;
		}
		sfclose(up);
	}
	if (schema)
		return loadtags(schema, NiL, disc, meth);
	dtinsert(meth->formats, &dss_format);
	return meth;
}

static Dssmeth_t	dss_method =
{
	&id[0],
	"A pseudo-method that specifies a method, value maps and constraints.",
	{0},
	dssmethf
};

#include "dss-compress.h"
#include "dss-count.h"
#include "dss-null.h"
#include "dss-print.h"
#include "dss-scan.h"
#include "dss-write.h"

static Cxquery_t	queries[] =
{
	QUERY_compress,
	QUERY_count,
	QUERY_null,
	QUERY_print,
	QUERY_scan,
	QUERY_write,
	{0}
};

static Dsslib_t		dss_library =
{
	&id[0],
	"dss method",
	{0},
	0,
	&dss_method,
	0,
	0,
	0,
	0,
	&queries[0]
};

/*
 * initialize library given name and dlopen() handle
 */

static Dsslib_t*
init(void* dll, const char* path, Dssflags_t flags, Dssdisc_t* disc)
{
	Dsslib_t*	lib;
	Dsslib_f	libf;
	char		buf[64];

	/*
	 * check for the Dsslib_t* function
	 */

	sfsprintf(buf, sizeof(buf), "%s_lib", id);
	if ((libf = (Dsslib_f)dlllook(dll, buf)) && (lib = (*libf)(path, disc)))
	{
		if (!(lib->path = (const char*)strdup(path)))
		{
			if (disc && disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
			return 0;
		}
		if (!dtsearch(state.cx->libraries, lib))
			dtinsert(state.cx->libraries, lib);
		return lib;
	}
	if ((flags & DSS_VERBOSE) && disc && disc->errorf)
		(*disc->errorf)(NiL, disc, 2, "%s: %s: initialization function not found in library", path, buf);
	return 0;
}

/*
 * open and return library info for name
 * name==0 scans for all related libraries on $PATH
 */

Dsslib_t*
dsslib(const char* name, Dssflags_t flags, Dssdisc_t* disc)
{
	register char*		s;
	register char*		t;
	register Dsslib_t*	lib;
	int			order;
	Dllscan_t*		dls;
	Dllent_t*		dle;
	void*			dll;
	char			base[256];
	char			path[PATH_MAX];
	char*			test[2];

	dssstate(disc);
	if (!name)
	{
		if (!state.scanned)
		{
			state.scanned++;
			if (dtsize(state.cx->libraries) == 1 && (dls = dllsopen(id, NiL, NiL)))
			{
				while (dle = dllsread(dls))
					if (dll = dlopen(dle->path, RTLD_LAZY))
						init(dll, dle->path, 0, disc);
					else if (disc && disc->errorf)
						(*disc->errorf)(NiL, disc, 2, "%s: %s", dle->path, dlerror());
				dllsclose(dls);
			}
		}
		return (Dsslib_t*)dtfirst(state.cx->libraries);
	}

	/*
	 * determine the base name
	 */

	if ((s = strrchr(name, '/')) || (s = strrchr(name, '\\')))
	{
		s++;
		order = 0;
	}
	else
	{
		s = (char*)name;
		order = 1;
	}
	if (strneq(s, "lib", 3))
		s += 3;
	for (t = s; *t && *t != '.' && *t != '-'; t++);
	base[sfsprintf(base, sizeof(base) - 1, "%-.*s", t - s, s)] = 0;

	/*
	 * check if its already loaded
	 */

	if (!(lib = (Dsslib_t*)dtmatch(state.cx->libraries, base)))
	{
		/*
		 * load
		 */

		test[order] = (char*)name;
		test[!order] = base;
		if (!(dll = dllplug(id, test[0], NiL, RTLD_LAZY, path, sizeof(path))) && (streq(test[0], test[1]) || !(dll = dllplug(id, test[1], NiL, RTLD_LAZY, path, sizeof(path)))))
		{
			if ((flags & DSS_VERBOSE) && disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%s: library not found", name);
			return 0;
		}
		lib = init(dll, path, flags|DSS_VERBOSE, disc);
	}
	return lib;
}

/*
 * add lib tables
 */

int
dssadd(register Dsslib_t* lib, Dssdisc_t* disc)
{
	register int	i;

	if (lib->header.flags & CX_INITIALIZED)
		return 0;
	lib->header.flags |= CX_INITIALIZED;
	if (lib->libraries)
		for (i = 0; lib->libraries[i]; i++)
			if (dssload(lib->libraries[i], disc))
				return -1;
	if (lib->types)
		for (i = 0; lib->types[i].name; i++)
			if (cxaddtype(NiL, &lib->types[i], disc))
				return -1;
	if (lib->callouts)
		for (i = 0; lib->callouts[i].callout; i++)
			if (cxaddcallout(NiL, &lib->callouts[i], disc))
				return -1;
	if (lib->recodes)
		for (i = 0; lib->recodes[i].recode; i++)
			if (cxaddrecode(NiL, &lib->recodes[i], disc))
				return -1;
	if (lib->maps)
		for (i = 0; lib->maps[i]; i++)
			if (cxaddmap(NiL, lib->maps[i], disc))
				return -1;
	if (lib->queries)
		for (i = 0; lib->queries[i].name; i++)
			if (cxaddquery(NiL, &lib->queries[i], disc))
				return -1;
	if (lib->constraints)
		for (i = 0; lib->constraints[i].name; i++)
			if (cxaddconstraint(NiL, &lib->constraints[i], disc))
				return -1;
	if (lib->edits)
		for (i = 0; lib->edits[i].name; i++)
			if (cxaddedit(NiL, &lib->edits[i], disc))
				return -1;
	if (!dtsearch(state.cx->libraries, lib))
		dtinsert(state.cx->libraries, lib);
	return 0;
}

/*
 * find and add library name
 */

int
dssload(const char* name, Dssdisc_t* disc)
{
	Dsslib_t*	lib;

	if (!(lib = dsslib(name, DSS_VERBOSE, disc)))
		return -1;
	return dssadd(lib, disc);
}

/*
 * return the input location string for data
 * suitable for errorf
 */

static char*
location(Cx_t* cx, void* data, Cxdisc_t* disc)
{
	register Dssfile_t*	file = DSSRECORD(data)->file;
	char*			path;
	char*			sep;
	char*			loc;
	char*			nxt;
	char*			end;
	size_t			n;

	if (!file)
		return "";
	if (path = strrchr(file->path, '/'))
		path++;
	else
		path = file->path;
	n = strlen(path) + 3;
	sep = ": ";
	if (file->count || file->offset)
	{
		sep = ", ";
		n += 64;
	}
	loc = nxt = fmtbuf(n);
	end = loc + n;
	nxt += sfsprintf(nxt, end - nxt, "%s%s", path, sep);
	if (file->count || file->offset)
		sfsprintf(nxt, end - nxt, "%s %I*u, %s %I*d: ", ERROR_translate(NiL, NiL, id, "record"), sizeof(file->count), file->count, ERROR_translate(NiL, NiL, id, "offset"), sizeof(file->offset), file->offset);
	return loc;
}

#define DSS_MEM_file	1
#define DSS_MEM_format	2
#define DSS_MEM_offset	3
#define DSS_MEM_record	4

static Cxvariable_t dss_mem_struct[] =
{
CXV("file",   "string", DSS_MEM_file,   "Current record file name.")
CXV("format", "string", DSS_MEM_format, "Current record format.")
CXV("offset", "number", DSS_MEM_offset, "Current record offset.")
CXV("record", "number", DSS_MEM_record, "Current record number.")
{0}
};

static int
dss_mem_get(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	Dssfile_t*	file = DSSRECORD(data)->file;

	switch (pc->data.variable->index)
	{
	case DSS_MEM_file:
		r->value.string.data = (char*)file->path;
		r->value.string.size = strlen(file->path);
		break;
	case DSS_MEM_format:
		r->value.string.data = (char*)file->format->name;
		r->value.string.size = strlen(file->format->name);
		break;
	case DSS_MEM_offset:
		r->value.number = file->offset;
		break;
	case DSS_MEM_record:
		r->value.number = file->count;
		break;
	default:
		return -1;
	}
	return 0;
}

static Cxmember_t	dss_member =
{
	dss_mem_get,
	0,
	(Dt_t*)&dss_mem_struct[0]
};

static Cxtype_t dss_mem_type =
{
	&id[0],	"Global state.", {0}, (Cxtype_t*)"void", 0, 0, 0, 0, 0, { 0 }, 0, &dss_member
};

/*
 * open a dss session
 */

Dss_t*
dssopen(Dssflags_t flags, Dssflags_t test, Dssdisc_t* disc, Dssmeth_t* meth)
{
	register Dss_t*		dss;
	register Vmalloc_t*	vm;
	Cxvariable_t*		var;
	Cxtype_t*		type;
	Dsslib_t*		lib;

	if (!disc)
		return 0;
	dssstate(disc);
	if (!meth)
	{
		/*
		 * find the first user library that defines a method
		 */

		lib = (Dsslib_t*)dtfirst(state.cx->libraries);
		while ((lib = (Dsslib_t*)dtnext(state.cx->libraries, lib)) && !lib->meth);
		if (lib)
			meth = lib->meth;
		else
		{
			if (!(flags & DSS_QUIET) && disc->errorf)
				(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "a method must be specified");
			return 0;
		}
	}
	if (!(vm = vmopen(Vmdcheap, Vmbest, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	if (!(dss = vmnewof(vm, 0, Dss_t, 1, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		vmclose(vm);
		return 0;
	}
	if (!disc->loadf)
		disc->loadf = dssload;
	if (!disc->locationf)
		disc->locationf = location;
	dss->id = id;
	dss->vm = vm;
	dss->disc = disc;
	dss->meth = meth;
	dss->flags = flags;
	dss->test = test;
	dss->state = &state;
	if (!(dss->cx = cxscope(NiL, meth->cx, flags & DSS_CX_FLAGS, test, disc)) || disc->map && !loadtags(disc->map, "map", disc, meth))
		goto bad;
	dss->cx->caller = dss;
	if (meth->openf && (*meth->openf)(dss, dss->disc))
		goto bad;
	for (var = (Cxvariable_t*)dtfirst(dss->cx->variables); var; var = (Cxvariable_t*)dtnext(dss->cx->variables, var))
		if (var->format.map)
			var->format.map->header.flags |= CX_REFERENCED;
	if (cxaddtype(dss->cx, &dss_mem_type, disc) || !(type = cxtype(dss->cx, dss_mem_type.name, disc)))
		goto bad;
	dss_mem_type = *type;
	if (!(var = vmnewof(vm, 0, Cxvariable_t, 1, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		goto bad;
	}
	var->name = id;
	var->type = dss->cx->state->type_void;
	var->type->base = type;
	dtinsert(dss->cx->variables, var);
	return state.dss = dss;
 bad:
	dssclose(dss);
	return 0;
}

/*
 * close a dss session
 */

int
dssclose(register Dss_t* dss)
{
	int	r;

	if (!dss)
		return -1;
	if (dss->meth->closef)
		r = (*dss->meth->closef)(dss, dss->disc);
	else
		r = 0;
	if (dss == state.dss)
		state.dss = 0;
	if (!dss->vm)
		r = -1;
	else
		vmclose(dss->vm);
	return r;
}

/*
 * initialize method given pointer
 * this is a library private global for dssmeth() and dssstatic()
 */

Dssmeth_t*
dssmethinit(const char* name, const char* options, const char* schema, Dssdisc_t* disc, Dssmeth_t* meth)
{
	Dssmeth_t*	ometh;
	Opt_t		opt;

	if (!(meth->cx = cxopen(0, 0, disc)) || !(meth->formats = dtopen(&state.cx->namedisc, Dttree)))
	{
		if (meth->cx)
			cxclose(meth->cx);
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	if (meth->methf)
	{
		ometh = meth;
		opt = opt_info;
		state.cx->header = (Cxheader_t*)meth;
		meth = (*meth->methf)(name, options, schema, disc, meth);
		opt_info = opt;
		if (!meth)
			return 0;
		if (meth != ometh)
			dtinsert(state.cx->methods, meth);
	}
	else if (options)
	{
		return 0;
	}
	return state.meth = meth;
}

/*
 * return method given name
 */

Dssmeth_t*
dssmeth(const char* name, Dssdisc_t* disc)
{
	register char*	s;
	const char*	options;
	const char*	schema;
	Dsslib_t*	lib;
	Dssmeth_t*	meth;
	char		buf[1024];
	char		path[1024];

	buf[sfsprintf(buf, sizeof(buf) - 1, "%s", name)] = 0;
	options = schema = 0;
	for (s = buf; *s; s++)
		if (*s == ',' || *s == '\t' || *s == '\r' || *s == '\n')
		{
			if (!options)
			{
				*s++ = 0;
				options = (char*)s;
			}
		}
		else if (*s == ':')
		{
			*s++ = 0;
			schema = name + (s - buf);
			break;
		}
	name = (const char*)buf;
	if (!*name)
		name = id;
	if (!(meth = (Dssmeth_t*)dtmatch(state.cx->methods, name)))
	{
		if (pathfind(name, id, id, path, sizeof(path)))
		{
			meth = &dss_method;
			name = id;
			schema = path;
		}
		else if (!(lib = dsslib(name, 0, disc)) || !(meth = lib->meth) || dssadd(lib, disc))
			return 0;
	}
	return dssmethinit(name, options, schema, disc, meth);
}

/*
 * return initialized global state pointer
 */

Dssstate_t*
dssstate(Dssdisc_t* disc)
{
	if (!state.initialized && !state.initialized++)
	{
		error(-1, "%s", fmtident(usage));
		state.cx = cxstate(disc);
		dtinsert(state.cx->libraries, &dss_library);
		if (dssadd(&dss_library, disc))
			error(ERROR_PANIC, "%s library initialization error", id);
	}
	return &state;
}

/*
 * return 1 if expr contains a query
 */

static int
hasquery(register Dssexpr_t* expr)
{
	do
	{
		if (!expr->query->prog)
			return 1;
		if (expr->pass && hasquery(expr->pass))
			return 1;
		if (expr->fail && hasquery(expr->fail))
			return 1;
		if (expr->group && hasquery(expr->group))
			return 1;
	} while (expr = expr->next);
	return 0;
}

/*
 * apply expression with optional head and tail queries to files in argv
 */

int
dssrun(Dss_t* dss, const char* expression, const char* head, const char* tail, char** argv)
{
	register Dssexpr_t*	x;
	Dssexpr_t*		expr;
	Dssexpr_t*		xh;
	Dssexpr_t*		xt;
	int			errors;
	int			r;

	errors = error_info.errors;
	if (!expression || !*expression || *expression == '-' && !*(expression + 1))
		expression = tail ? tail : "{write}";
	if (!(expr = dsscomp(dss, expression, NiL)))
		return -1;
	xh = xt = 0;
	r = -1;
	if (expression == tail)
		tail = 0;
	else if (!tail && !hasquery(expr))
		tail = "{write}";
	if (tail)
	{
		if (!(xt = dsscomp(dss, tail, NiL)))
			goto bad;
		if (xt->query->beg == null_beg)
		{
			dssfree(dss, xt);
			xt = 0;
		}
	}
	for (x = expr; x->group; x = x->group);
	if (!x->query->head)
	{
		if (!head)
			head = "{scan}";
		if (!(xh = dsscomp(dss, head, NiL)))
			goto bad;
		if (!xh->query->head)
		{
			if (dss->disc->errorf)
				(*dss->disc->errorf)(dss, dss->disc, 2, "%s: not a head query", head);
			goto bad;
		}
		xh->files = argv;
	}
	else if (head)
	{
		if (dss->disc->errorf)
			(*dss->disc->errorf)(dss, dss->disc, 2, "%s: expression already has %s head", head, x->query->name);
		goto bad;
	}
	else
		x->files = argv;
	if (xh || xt)
	{
		if (expr->pass || expr->fail || expr->next)
		{
			if (!(x = vmnewof(expr->vm, 0, Cxexpr_t, 1, sizeof(Cxquery_t))))
			{
				if (dss->disc->errorf)
					(*dss->disc->errorf)(dss, dss->disc, ERROR_SYSTEM|2, "out of space");
				goto bad;
			}
			x->vm = expr->vm;
			x->op = expr->op;
			x->query = (Cxquery_t*)(x + 1);
			x->group = expr;
			expr = x;
		}
		if (xt)
		{
			expr->pass = xt;
			xt->parent = expr;
		}
		if (xh)
		{
			x = xh->pass = expr;
			expr = xh;
			xh = x;
		}
	}
	if (expr->pass)
		expr->pass->parent = expr->pass;
	if (dss->test & 0x00000100)
		dsslist(dss, expr, sfstdout);
	if (dssbeg(dss, expr) || dssend(dss, expr))
		goto bad;
	dssfree(dss, expr);
	r = error_info.errors != errors ? -1 : 0;
 bad:
	if (xh)
		dssfree(dss, xh);
	if (xt)
		dssfree(dss, xt);
	dssfree(dss, expr);
	return r;
}
