/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1991-2004 AT&T Corp.                  *
*                      and is licensed under the                       *
*          Common Public License, Version 1.0 (the "License")          *
*                        by AT&T Corp. ("AT&T")                        *
*      Any use, downloading, reproduction or distribution of this      *
*      software constitutes acceptance of the License.  A copy of      *
*                     the License is available at                      *
*                                                                      *
*         http://www.research.att.com/sw/license/cpl-1.0.html          *
*         (with md5 checksum 8a5e0081c856944e76c69a1cf29c2e8b)         *
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
 * cql expression support
 */

#include "cql.h"

#include <regex.h>

#define SWITCH_TOT	256
#define SWITCH_MAX	(SWITCH_TOT/2)
#define SWITCH_MIN	(SWITCH_TOT/8)

typedef struct
{
	char*		next_data;
	int		next_item;
	Extype_t	base[SWITCH_TOT];
	Extype_t*	item[SWITCH_TOT];
	char		data[SWITCH_TOT * 32];
} Switch_t;

static Switch_t*	sw;

/*
 * return the next record with string field i matching the pattern k
 */

static Field_t*
match(File_t* f, char* k, long h, int i, int c, int p)
{
	register Field_t*	field;

	NoP(h);
	NoP(c);
	NoP(p);
	while ((field = record(f, NiL, 0, 0, 0, 1)) && !strmatch((field + i)->f_string, k));
	return field;
}

/*
 * low level for optimize()
 */

static int
collapse(Expr_t* p, register Exnode_t* x)
{
	register Exnode_t*	l;
	register Exnode_t*	r;
	register Exid_t*	sym;
	Exref_t*		ref;
	Exref_t*		fer;
	char*			k;
	char*			s;
	File_t*			f;
	Field_t*		field;
	Field_t*		(*get)(File_t*, char*, long, int, int, int);
	int			n;
	int			zero;
	long			hash;

	zero = 1;
	for (;;)
	{
		if (x->op == EQ)
		{
			l = x->data.operand.left;
			if (l->op != ID)
			{
				r = l;
				l = x->data.operand.right;
				if (l->op != ID)
					break;
			}
			else r = x->data.operand.right;
			if (r->op != CONSTANT)
				break;
			sym = l->data.variable.symbol;
			if (!GETINDEX(sym) || r->type != STRING || !l->data.variable.reference || !(f = ((Local_t*)l->data.variable.reference->symbol->local.pointer)->file))
				break;
			if (!f->field)
				load(p, f, l->data.variable.reference->symbol);
			if (f->access == DIRECT)
				break;
			k = r->data.constant.value.string;
			if (strmatch(r->data.constant.value.string, MATCHPATTERN))
			{
				get = match;
				hash = 0;
				n = SWITCH_MAX;
			}
			else
			{
				get = record;
				hash = strsum(k, 0L);
				n = SWITCH_MIN;
			}
			message((-3, "opt: sym=%s file=%s field=%d %s=%s hash=0x%08x", sym->name, filename(f), GETFIELD(sym), get == record ? "key" : "pattern", k, hash));
			load(p, f, sym);
			if (!(field = (*get)(f, k, hash, GETFIELD(sym), 1, 1)))
			{
				/*
				 * no match -> constant 0
				 */

				message((-3, "opt: sym=%s drops out with collapse=%d", sym->name, zero));
				x->op = CONSTANT;
				x->type = INTEGER;
				x->data.constant.value.integer = 0;
			}
			else
			{
				int	i;
				int	j;
				int	m;
				char*	td;

				zero = 0;
				j = GETFIELD(sym);
				m = f->record->key;
				if (!sw)
				{
					if (!(sw = newof(0, Switch_t, 1, 0)))
						error(3, "out of space [switch]");
					sw->next_item = 0;
					sw->next_data = sw->data;
				}
				td = sw->next_data;
				if (td + (i = strlen((field + m)->f_string) + 1) > sw->data + elementsof(sw->data))
				{
					message((-3, "opt: sym=%s cannot collapse -- switch too big", sym->name));
					return 0;
				}
				for (fer = 0, ref = l->data.variable.reference; ref->next; fer = ref, ref = ref->next);
				s = (char*)memcpy(td, (field + m)->f_string, i);
				td += i;
				if (!(field = (*get)(f, k, hash, j, 0, 1)))
				{
					/*
					 * 1 match -> constant EQ
					 */

					message((-3, "opt: sym=%s collapses to test on %s==\"%s\"", sym->name, ref->symbol->name, s));
					r->data.constant.value.string = s;
				}
				else
				{
					/*
					 * more than 1 match -> SWITCH
					 */

					register Extype_t**	tp;
					register Extype_t**	tl;
					register Extype_t*	tv;
					Extype_t**		tb;

					tb = tp = sw->item + sw->next_item;
					if (n > (i = elementsof(sw->item) - sw->next_item))
						n = i;
					tl = tp + n - 1;
					tv = sw->base + sw->next_item;
					tv->string = s;
					*tp++ = tv++;
					do
					{
						if (tp >= tl || td + (i = strlen((field + m)->f_string) + 1) > sw->data + elementsof(sw->data))
						{
							message((-3, "opt: sym=%s cannot collapse -- switch too big", sym->name));
							return 0;
						}
						tv->string = (char*)memcpy(td, (field + m)->f_string, i);
						td += i;
						*tp++ = tv++;
					} while (field = (*get)(f, k, hash, j, 0, 1));
					*tp = 0;
					r = exnewnode(p, CASE, 1, 0, NiL, NiL);
					r->data.select.constant = tb;
					x->op = SWITCH;
					x->type = INTEGER;
					x->data.operand.right = exnewnode(p, DEFAULT, 1, 0, NiL, r);
					sw->next_item += tp - tb + 1;
					message((-3, "opt: sym=%s collapses to %d item switch on %s", sym->name, tp - tb, ref->symbol->name));
					debug(while (*tb) error(-9, "     %s==\"%s\"", ref->symbol->name, (*(tb++))->string));
				}
				sw->next_data = td;
				l = x->data.operand.left;
				l->data.variable.symbol = ref->symbol;
				SETINDEX(l->data.variable.symbol);
				if (fer)
					fer->next = 0;
				else
					l->data.variable.reference = 0;
			}
			f->sequence = 0;
		}
		else if (x->binary)
		{
			l = x->data.operand.left;
			r = x->data.operand.right;
			if (x->op != AND)
			{
				zero = 0;
				if (l)
					collapse(p, l);
				if (x = r)
					continue;
			}
			else if (l && collapse(p, l) || r && collapse(p, r))
			{
				if (l)
					exfreenode(p, l);
				if (r)
					exfreenode(p, r);
				x->binary = 0;
				x->op = CONSTANT;
				x->type = INTEGER;
				x->data.constant.value.integer = 0;
			}
			else zero = 0;
		}
		else if (x->op != CONSTANT)
			zero = 0;
		else switch (x->type)
		{
		case FLOATING:
			if (x->data.constant.value.floating)
				zero = 0;
			break;
		case INTEGER:
			if (x->data.constant.value.integer)
				zero = 0;
			break;
		case STRING:
			if (*x->data.constant.value.string)
				zero = 0;
			break;
		}
		return zero;
	}
	return 0;
}

/*
 * optimize the select expression by converting references to
 * main schema index equalities
 * 1 returned if subexpression x collapses to 0
 */

int
optimize(Expr_t* p, Exnode_t* x)
{
	if (sw)
	{
		sw->next_item = 0;
		sw->next_data = sw->data;
	}
	return collapse(p, x);
}

/*
 * low level for pivot()
 * *p set to candidate pivotal strmatch() pattern
 */

static int
piv(register Hix_t* hix, register Exnode_t* x, char** p)
{
	register Exnode_t*	l;
	register Exnode_t*	r;
	register long		hash;
	register Extype_t**	v;
	int			n;
	Exid_t*			sym;

	for (;;) switch (x->op)
	{
	case AND:
		if (piv(hix, x->data.operand.left, p))
		{
			if (piv(hix, x->data.operand.right, p))
			{
				hixand(hix);
				message((-6, "piv: and"));
			}
			return 1;
		}
		x = x->data.operand.right;
		break;
	case OR:
		n = hixpos(hix);
		message((-6, "piv: ["));
		if (piv(hix, x->data.operand.left, p) && piv(hix, x->data.operand.right, p))
		{
			hixor(hix);
			message((-6, "piv: ] or"));
			return 1;
		}
		hixset(hix, n);
		message((-6, "piv: ] nop"));
		return 0;
	case EQ:
		l = x->data.operand.left;
		if (l->op != ID)
		{
			r = l;
			l = x->data.operand.right;
			if (l->op != ID)
				return 0;
		}
		else r = x->data.operand.right;
		if (l->data.variable.reference || r->op != CONSTANT)
			return 0;
		sym = l->data.variable.symbol;
		if (!GETINDEX(sym))
			return 0;
		if (r->type != STRING)
		{
			hash = r->data.constant.value.integer;
			message((-6, "piv: %d==0x%08x [%d]", GETFIELD(sym), hash, hash));
		}
		else if (strmatch(r->data.constant.value.string, MATCHPATTERN))
		{
			*p = r->data.constant.value.string;
			return 0;
		}
		else
		{
			hash = strsum(r->data.constant.value.string, 0L);
			message((-6, "piv: %d==0x%08x [\"%s\"]", GETFIELD(sym), hash, r->data.constant.value.string));
		}
		hixeq(hix, GETFIELD(sym), hash);
		return 1;
	case SWITCH:
		l = x->data.operand.left;
		if (l->op != ID || l->data.variable.reference)
			return 0;
		sym = l->data.variable.symbol;
		if (!GETINDEX(sym))
			return 0;
		x = x->data.operand.right;
		if (x->data.operand.left)
			return 0;
		x = x->data.operand.right;
		v = x->data.select.constant;
		n = hixpos(hix);
		while (*v)
		{
			if (l->type != STRING)
				hash = (*v)->integer;
			else if (strmatch((*v)->string, MATCHPATTERN))
			{
				hixset(hix, n);
				return 0;
			}
			else hash = strsum((*v)->string, 0L);
			hixeq(hix, GETFIELD(sym), hash);
			message((-6, "piv: %d==0x%08x", GETFIELD(sym), hash));
			if (v++ > x->data.select.constant)
			{
				hixor(hix);
				message((-6, "piv: or"));
			}
		}
		return 1;
	default:
		return 0;
	}
}

/*
 * set *p to a pivotal strmatch() pattern in subexpression x
 */

static int
pat(register Exnode_t* x, char** p)
{
	register Exnode_t*	l;
	register Exnode_t*	r;
	register Extype_t**	v;
	Exid_t*			sym;

	for (;;) switch (x->op)
	{
	case AND:
		if (pat(x->data.operand.left, p))
		{
			pat(x->data.operand.right, p);
			return 1;
		}
		x = x->data.operand.right;
		break;
	case OR:
		if (pat(x->data.operand.left, p) && pat(x->data.operand.right, p))
			return 1;
		*p = 0;
		return 0;
	case EQ:
		l = x->data.operand.left;
		if (l->op != ID)
		{
			r = l;
			l = x->data.operand.right;
			if (l->op != ID)
			{
				*p = 0;
				return 0;
			}
		}
		else r = x->data.operand.right;
		if (l->data.variable.reference || r->op != CONSTANT)
		{
			*p = 0;
			return 0;
		}
		sym = l->data.variable.symbol;
		if (!GETINDEX(sym))
		{
			*p = 0;
			return 0;
		}
		if (r->type == STRING && strmatch(r->data.constant.value.string, MATCHPATTERN))
			*p = r->data.constant.value.string;
		return 1;
	case SWITCH:
		l = x->data.operand.left;
		if (l->op != ID || l->data.variable.reference)
		{
			*p = 0;
			return 0;
		}
		sym = l->data.variable.symbol;
		if (!GETINDEX(sym))
		{
			*p = 0;
			return 0;
		}
		x = x->data.operand.right;
		if (x->data.operand.left)
		{
			*p = 0;
			return 0;
		}
		x = x->data.operand.right;
		v = x->data.select.constant;
		while (*v)
		{
			if (l->type == STRING && strmatch((*v)->string, MATCHPATTERN))
			{
				*p = 0;
				return 0;
			}
		}
		return 1;
	default:
		*p = 0;
		return 0;
	}
}

/*
 * generate a hix RPN and/or expression on pivotal index equalities in x
 * zero returned if subexpression is not pivotal
 */

int
pivot(File_t* f, Exnode_t* x)
{
	int	r;
	char*	p;

	p = 0;
	r = piv(f->hix, x, &p);
	if (!r && p && pat(x, &p))
	{
		message((-6, "piv: match pattern `%s'", p));
		f->scan.pattern = p;
	}
	return r;
}

/*
 * sym referenced in expression
 */

Extype_t
reference(Expr_t* prog, Exnode_t* node, Exid_t* sym, Exref_t* ref, char* str, int elt, Exdisc_t* disc)
{
	register int		n;
	register File_t*	f;
	register char*		s;
	char*			e;
	int			i;
	int			di;
	int			fmtsubfields;
	File_t*			df;
	Record_t*		rec;
	Record_t*		par;
	Format_t*		fmt;
	Exid_t*			mem;
	Exid_t*			var;
	Exid_t*			aka;
	Exid_t*			ds;
	Extype_t		val;
	Exnode_t*		x;

	NoP(disc);
	val = exzero(sym->type);
	if (sym->lex == PRAGMA)
		switch (sym->index)
		{
		case P_pragma:
			s = str;
			for (;;)
			{
				while (isspace(*s))
					s++;
				if (!*s)
					break;
				str = s;
				while (isalnum(*s))
					s++;
				if (s > str)
				{
					if (*s == '=')
					{
						*s++ = 0;
						n = strtol(s, &e, 0);
						while (*s && !isspace(*s))
							s++;
					}
					if (i = str[0] == 'n' && str[1] == 'o')
						str += 2;
					if (streq(str, "debug"))
					{
						error_info.trace = -n;
						continue;
					}
					if (streq(str, "local"))
					{
						if (i)
							state.hix.flags &= ~HIX_LOCAL;
						else
							state.hix.flags |= HIX_LOCAL;
						continue;
					}
					if (streq(str, "readonly"))
					{
						if (i)
							state.hix.flags &= ~HIX_READONLY;
						else
							state.hix.flags |= HIX_READONLY;
						continue;
					}
					if (streq(str, "regenerate"))
					{
						if (i)
							state.hix.flags &= ~HIX_REGENERATE;
						else
							state.hix.flags |= HIX_REGENERATE;
						continue;
					}
					if (streq(str, "replace"))
					{
						state.replace = i;
						continue;
					}
					if (streq(str, "test"))
					{
						if (i)
							state.test &= ~n;
						else
							state.test |= n;
						if (state.test & 000010)
							state.hix.flags |= HIX_TEST1;
						else
							state.hix.flags &= ~HIX_TEST1;
						if (state.test & 000020)
							state.hix.flags |= HIX_TEST2;
						else
							state.hix.flags &= ~HIX_TEST2;
						if (state.test & 000100)
							state.cdb_flags |= CDB_TEST1;
						else
							state.cdb_flags &= ~CDB_TEST1;
						if (state.test & 000200)
							state.cdb_flags |= CDB_TEST2;
						else
							state.cdb_flags &= ~CDB_TEST2;
						continue;
					}
					if (streq(str, "update"))
					{
						state.update = i;
						continue;
					}
					if (streq(str, "warn"))
					{
						if (i)
							state.verbose = -1;
						else
							state.verbose = 1;
						continue;
					}
				}
				error(2, "%s: unknown %s", str, sym->name);
				break;
			}
			return val;
		case P_static:
		case P_struct:
			declare(prog, str, NiL, 0);
			return val;
		default:
			error(3, "%s %s: unknown statement", sym->name, str);
			return val;
		}
	if (str)
		error(3, "%s: string variable references are not supported", str);
	if (ref && GETBUILTIN(ref->symbol) == B_STATE)
	{
		if (!(mem = (Exid_t*)dtmatch(prog->symbols, sym->name + strlen(ref->symbol->name) + 1)))
			mem = sym;
		switch (GETBUILTIN(mem))
		{
		case B_INPUT:
			if (mem->local.pointer && (f = ((Local_t*)mem->local.pointer)->file))
				f->force = 1;
			break;
		case 0:
			error(3, "%sunknown builtin", context(prog));
			break;
		default:
			if (mem->lex == FUNCTION)
			{
				if (elt >= 0)
					error(3, "%sis a function", context(prog));
				if (GETBUILTIN(mem) == B_present)
					for (x = node->data.operand.right; x; x = x->data.operand.right)
						SETSILENT(x->data.operand.left);
			}
			else if (elt >= 0)
				error(3, "%snot an array", context(prog));
			break;
		}
		if (mem != sym)
		{
			sym->index = mem->index;
			if (!(sym->type = GETTYPE(sym)))
				sym->type = mem->type;
		}
		message((-6, "ini: sym=%s builtin=%d type=%d%s", sym->name, GETBUILTIN(sym), GETTYPE(sym), elt == EX_CALL ? " function" : ""));
		return val;
	}
	if (ref && ref->symbol && (state.insert || ref->symbol == state.closure.relation) && ref->symbol->local.pointer && ((Local_t*)ref->symbol->local.pointer)->record)
		mem = ref->symbol;
	else if (state.operand && sym->local.pointer && ((Local_t*)sym->local.pointer)->record)
		mem = sym;
	else if (!(mem = state.schema))
		error(3, "main schema not defined");
	if (fmt = (rec = ((Local_t*)mem->local.pointer)->record) ? rec->format : (Format_t*)0)
		fmtsubfields = rec->subfields;
	f = ((Local_t*)mem->local.pointer)->file;

	/*
	 * NOTE: there is a parse bug that mishandles duplicate
	 *       member names across different schemas; this is a
	 *	 workaround until the place that sets the dup
	 *	 info is found
	 */

	if (ref && sym->local.pointer && (e = strchr(sym->name, '.')) && (var = (Exid_t*)dtmatch(prog->symbols, e + 1)) && sym->local.pointer == var->local.pointer)
	{
		sym->local.pointer = 0;
		CLREASY(sym);
	}
	if (ref && ref->symbol == mem)
	{
		n = strlen(mem->name) + 1;
		ref = ref->next;
	}
	else
		n = 0;
	if (!sym->local.pointer)
	{
		if (!(sym->local.pointer = (char*)newof(0, Local_t, 1, 0)))
			error(3, "out of space [local]");
		message((-6, "lcl: sym=%s local=%p [%s:%d]", sym->name, sym->local.pointer, __FILE__, __LINE__));
	}
	((Local_t*)sym->local.pointer)->reference = ref;
	message((-6, "get: sym=%s ref=%s mem=%s str=%s elt=%d", sym->name, ref ? ref->symbol->name : (char*)0, mem->name, str, elt));
	ds = sym;
	df = f;
	di = 0;
	for (;;)
	{
		if (ref)
			var = ref->symbol;
		else
		{
			var = sym;
			if (s = strrchr(var->name, '.'))
				n = s - var->name + 1;
			else
				n = 0;
		}
		if (var == rec->symbol)
			break;
		message((-6, "ref: var=%s end=%s rec=%s", var->name, var->name + n, rec->symbol->name));
		if (!rec->fields)
			error(3, "%s: %s schema not defined", var->name, rec->symbol->name);
		if (!rec->subfields)
			subfields(rec);
		mem = (Exid_t*)dtmatch(prog->symbols, var->name + n);
		for (i = 0; i < rec->fields; i++)
			if (mem == rec->member[i].symbol)
				break;
		if (i >= rec->fields)
			error(3, "%s: %s.%s not defined", var->name, rec->symbol->name, var->name + n);
		if (rec->member[i].index)
		{
			SETINDEX(var);
			attach(var, rec->member[i].record, 0);
		}
		n = rec->member[i].access;
		di += rec->member[i].subfield;
		message((-6, "mem: record=%s member=%s field=%d:%d access=%c index=%d", rec->symbol->name, mem->name, i, di, -n, rec->member[i].index));
		SETFIELD(var, di);
		if (rec->member[i].format.sorted)
			var->local.number |= S_sorted;
		if ((df || (df = f)) && di > df->maxfield)
		{
			df->maxfield = di;
			if (df->cache)
				df->cache->last = df->cache->first + df->maxfield;
		}
		if (n == DIRECT)
			SETEASY(ds);
		else
		{
			di = 0;
			df = 0;
			ds = var;
		}
		if (ref)
			var->type = rec->member[i].format.type;
		else
		{
			if (ISINTERNAL(rec->member[i].format.type))
				SETTYPE(var, rec->member[i].format.type);
			else var->type = rec->member[i].format.type;
			if (rec->member[i].format.elements)
				attach(var, NiL, 1);
			if (state.insert && (rec = rec->member[i].record))
			{
				if (!sym->local.pointer)
				{
					if (!(sym->local.pointer = (char*)newof(0, Local_t, 1, 0)))
						error(3, "out of space [local]");
					message((-6, "lcl: sym=%s local=%p [%s:%d]", sym->name, sym->local.pointer, __FILE__, __LINE__));
				}
				((Local_t*)sym->local.pointer)->record = rec;
			}
			if (node)
				SETDATA(node, 1, GETFIELD(var));
			break;
		}
		if (!state.declaration && rec->format)
		{
			rec->format[i].referenced |= var->type == STRING ? F_STRING : F_CONVERTED;
			fmt = 0;
		}
		par = rec;
		if (!(rec = rec->member[i].record))
			error(3, "%s: not a schema", var->name);
		if (!state.operand)
		{
			if (!var->local.pointer || !(f = ((Local_t*)var->local.pointer)->file))
			{
				if ((aka = (Exid_t*)dtmatch(prog->symbols, sfprints("%s.%s", par->symbol->name, var->name))) && aka->local.pointer)
				{
					var->local.pointer = aka->local.pointer;
					message((-6, "lcl: %s == %s", var->name, aka->name));
				}
				if (!var->local.pointer || !(f = ((Local_t*)var->local.pointer)->file))
					f = attach(var, rec, 0);
			}
			if (f->record != rec)
			{
				if (f->record)
					error(3, "%s: reference file record type %s conflicts with %s", var->name, f->record->symbol->name, rec->symbol->name);
				f->record = rec;
			}
			f->access = n;
			f->symbol = var;
		}
		message((-6, "def: var=%s type=%d record=%s field=%d", var->name, var->type, rec->symbol->name, GETFIELD(var)));
		n = strlen(var->name) + 1;
		ref = ref->next;
	}
	if (elt >= 0)
	{
		if (!((Local_t*)sym->local.pointer)->index)
			error(3, "%s: not an array", sym->name);
		if (!(s = strchr(sym->name, '.')) ||
		    !(mem = (Exid_t*)dtmatch(prog->symbols, s + 1)) ||
		    !mem->local.pointer ||
		    !((Local_t*)mem->local.pointer)->file
		    )
			mem = sym;
		if (((Local_t*)mem->local.pointer)->file)
			((Local_t*)sym->local.pointer)->index->delimiter = ((Local_t*)mem->local.pointer)->file->delimiter;
	}
	if (fmt)
	{
		i = GETFIELD(var);
		if (i >= fmtsubfields)
			message((-5, "ref: name=%s format subfield index %d >= %d", var->name, i, fmtsubfields));
		else if (!state.declaration)
			fmt[i].referenced |= var->type == STRING ? F_STRING : F_CONVERTED;
	}
	message((-5, "var: name=%s type=%d.%d field=%d referenced=%d", var->name, var->type, GETTYPE(var), GETFIELD(var), fmt ? fmt[GETFIELD(var)].referenced : 0));
	if (n = GETTYPE(var))
		var->type = n;
	return val;
}

/*
 * blast direct fields in s for element n if necessary
 */

static Field_t*
direct(register File_t* f, register char* s, int n, register Field_t* field)
{
	register Field_t*	last;
	register Format_t*	format;
	register char*		t;
	register int		z;
	char*			e;
	char			base;
	int			delimiter;
	int			i;
	Format_t		strfmt;

	if (f->sequence != state.record)
	{
		f->sequence = state.record;
		if (!f->field)
		{
			if (!(f->field = newof(0, Cache_t, 1, DIRECTCHUNK * sizeof(Field_t))))
				error(3, "out of space [field cache]");
			f->field->last = f->field->first + DIRECTCHUNK;
			if (f->delimiter == DELDEF)
				f->delimiter = (n >= -1) ? SEPARATOR : DELIMITER;
			if (f->terminator == DELDEF)
				f->terminator = TERMINATOR;
		}
		e = s + strlen(s);
		field = f->field->first;
		last = f->field->last;
		if (f->record)
			format = f->record->format;
		else
		{
			format = &strfmt;
			format->width = 0;
			format->string = 1;
		}
		state.term[delimiter = f->delimiter]++;
		state.term[f->terminator]++;
		while (*s)
		{
			if (!(z = format->width))
			{
				for (t = s; !state.term[*((unsigned char*)t)]; t++);
				if (!format->string)
				{
					field->f_string = 0;
					if (format->type == INTEGER)
					{
						field->f_converted = CDB_LONG|CDB_INTEGER;
						base = format->base;
						field->f_unsigned = strtonll(t, NiL, &base, 0);
					}
					else
					{
						field->f_converted = CDB_FLOATING;
						field->f_double = strtod(t, NiL);
					}
					if (!*t)
						break;
					goto next;
				}
				z = t - s;
			}
			field->f_converted = 0;
			if (field->f_size < z + 1)
			{
				if (!field->f_size || (state.test & 1))
					field->f_data = 0;
				field->f_size = roundof(z + 1, 32);
				if (!(field->f_data = newof(field->f_data, char, field->f_size, 0)))
					error(ERROR_SYSTEM|3, "out of space [field data]");
			}
			if (z > e - s)
				z = e - s;
			*((field->f_string = (char*)memcpy(field->f_data, s, z)) + z) = 0;
			s += z;
			if (delimiter >= 0 && *s == delimiter)
				s++;
		next:
			field++;
			if (!z || n >= 0 && !n--)
				break;
			if (field >= last)
			{
				if (n < -1)
					break;
				if (!field->f_size || (state.test & 1))
					field->f_data = 0;
				i = last - f->field->first + DIRECTCHUNK;
				if (!(f->field = newof(f->field, Cache_t, 1, i * sizeof(Field_t))))
					error(3, "out of space [field cache]");
				field = f->field->first;
				last = f->field->last = f->field->first + i;
			}
			if (n < -1)
				format++;
		}
		state.term[delimiter]--;
		state.term[f->terminator]--;
		f->field->elements = field - f->field->first;
		if (n >= 0)
			for (; field <= last; field++)
			{
				field->f_converted = CDB_CACHED|CDB_TERMINATED;
				field->f_unsigned = 0;
				if (field->f_size < 1)
				{
					field->f_size = 32;
					if (!(field->f_data = newof(0, char, field->f_size, 0)))
						error(ERROR_SYSTEM|3, "out of space [field data]");
				}
				*field->f_string = 0;
			}
	}
	return f->field->first;
}

/*
 * return value for symbol sym
 */

Extype_t
value(Expr_t* prog, Exnode_t* node, Exid_t* sym, Exref_t* ref, void* env, int elt, Exdisc_t* disc)
{
	register int		n;
	register Field_t*	field;
	register Field_t*	element;
	register File_t*	f;
	register char*		s;
	Record_t*		r;
	char			b;
	int			i;
	char*			t;
	Hix_t*			hix;
	Extype_t		val;
	Field_t			dir;
	Assoc_t*		ap;

	NoP(node);
	NoP(disc);
	if (ref)
	{
		if (GETBUILTIN(ref->symbol) == B_STATE)
		{
			switch (GETBUILTIN(sym))
			{
			case B_CLOCK:
				if (!(val.integer = ((unsigned long)time(NiL) - state.date) * ELAPSED))
					val.integer = ELAPSED / 2;
				break;
			case B_DATE:
				val.integer = state.date;
				break;
			case B_DELIMITER:
				val.integer = (f = ((Local_t*)state.schema->local.pointer)->file) ? f->delimiter : DELIMITER;
				break;
			case B_ERRORS:
				val.integer = error_info.errors;
				break;
			case B_INPUT:
				if (f = ((Local_t*)state.schema->local.pointer)->file)
				{
					if (!f->image && !(f->image = sfstropen()))
						error(3, "out of space [image]");
					image(f, f->image, -1);
					val.string = sfstruse(f->image);
				}
				else
					val.string = "";
				break;
			case B_ITERATION:
				val.integer = state.iteration;
				break;
			case B_LINE:
				val.integer = error_info.line;
				break;
			case B_OFFSET:
				val.integer = (hix = ((Local_t*)state.schema->local.pointer)->file->hix) ? hix->offset : 0;
				break;
			case B_RECORD:
				val.integer = state.record;
				break;
			case B_SCANLIMIT:
				val.integer = (f = ((Local_t*)state.schema->local.pointer)->file) ? f->scanlimit : SCANLIMIT;
				break;
			case B_SCHEMA:
				val.string = (f = ((Local_t*)state.schema->local.pointer)->file) ? f->schema : "";
				break;
			case B_SELECTED:
				val.integer = state.selected;
				break;
			case B_SIZE:
				val.integer = (hix = ((Local_t*)state.schema->local.pointer)->file->hix) ? hix->size : 0;
				break;
			case B_TERMINATOR:
				val.integer = (f = ((Local_t*)state.schema->local.pointer)->file) ? f->terminator : TERMINATOR;
				break;
			case B_TIME:
				val.integer = (unsigned long)time(NiL);
				break;
			case B_getenv:
				if (!(s = getenv(((Extype_t*)env)[0].string)))
					s = "";
				val.string = s;
				break;
			case B_length:
				val.integer = strlen(((Extype_t*)env)[0].string);
				break;
			case B_loop:
				state.again = 1;
				for (n = BEGIN + 1; n <= END; n++)
					state.loop[n].next = ((Extype_t*)env)[n-BEGIN-1].string;
				break;
			case B_path:
				s = ((Extype_t*)env)[0].string;
				i = ((Extype_t*)env)[1].integer;
				if ((n = strlen(s)) > i)
				{
					s += n - i;
					*s = '*';
				}
				val.string = s;
				break;
			case B_present:
				val.integer = *((Extype_t*)env)[0].string != 0;
				break;
			case B_sub:
				{
					regex_t		re;
					regflags_t	flags = 0;
					int		minmatch = 0;
					regmatch_t	match[10];

					if ((n = regcomp(&re, ((Extype_t*)env)[1].string, REG_AUGMENTED|REG_LENIENT|REG_NULL)) || (n = regsubflags(&re, ((Extype_t*)env)[3].string, NiL, 0, NiL, &minmatch, &flags)) || (n = regsubcomp(&re, ((Extype_t*)env)[2].string, NiL, minmatch, flags)))
					{
						regfatal(&re, 2, n);
						val.string = ((Extype_t*)env)[0].string;
					}
					else
					{
						if ((n = regexec(&re, ((Extype_t*)env)[0].string, elementsof(match), match, 0)) || (n = regsubexec(&re, ((Extype_t*)env)[0].string, elementsof(match), match)))
						{
							if (n != REG_NOMATCH)
								regfatal(&re, 2, n);
							val.string = ((Extype_t*)env)[0].string;
						}
						else
						{
							n = strlen(re.re_sub->re_buf) + 1;
							if (val.string = fmtbuf(n))
								memcpy(val.string, re.re_sub->re_buf, n);
						}
						regfree(&re);
					}
				}
				break;
			}
			return val;
		}
		if (ref->symbol == state.schema)
			ref = ref->next;
	}
	field = (Field_t*)env;
	r = ((Local_t*)state.schema->local.pointer)->record;
	if (!GETEASY(sym))
		while (ref)
		{
			r = ((Local_t*)ref->symbol->local.pointer)->record;
			if (!(f = ((Local_t*)ref->symbol->local.pointer)->file))
				error(ERROR_PANIC, "%s: no schema", ref->symbol->name);
			if (!f->field)
				load(prog, f, ref->symbol);
			if (ref->index)
			{
				i = exeval(prog, ref->index, env).integer;
				val = value(prog, node, ref->symbol, NiL, env, i, disc);
				element = &dir;
				element->f_string = val.string;
				element->f_converted = CDB_STRING;
			}
			else
				element = (field + GETFIELD(ref->symbol));
			if (f->hix)
			{
				if (!(element->f_converted & CDB_INTEGER))
				{
					element->f_converted |= CDB_LONG|CDB_INTEGER;
					element->f_unsigned = strsum(element->f_string, 0L);
				}
				if (!(field = record(f, element->f_string, element->f_unsigned, f->record->key, 1, 1)))
				{
					if (!ISSILENT(node) && (*element->f_string || element->f_string != (state.empty + GETFIELD(ref->symbol))->f_string))
						error(state.verbose, "%s: %s index %s not found", filename(f), f->symbol->name, element->f_string);
					field = state.empty;
				}
			}
			else if (!f->name)
				field = direct(f, element->f_string, -2, field);
			else if (!f->assoc)
			{
				if (!(element->f_converted & CDB_INTEGER))
				{
					element->f_converted |= CDB_LONG|CDB_INTEGER;
					n = strtol(element->f_string, NiL, 0) - f->offset;
					if (n < 0 || n >= f->records)
					{
						if (!ISSILENT(node))
							error(state.verbose, "%s: %s record reference %d out of range -- expected %d..%d", filename(f), ref->symbol->name, n + f->offset, f->offset, f->records + f->offset);
						n = 0;
					}
					element->f_unsigned = n;
				}
				field = INDIRECT(f, element->f_unsigned);
			}
			else if (!(ap = (Assoc_t*)dtmatch(f->assoc, element->f_string)) && !(ap = (Assoc_t*)dtmatch(f->assoc, "")))
			{
				if (!ISSILENT(node))
					error(state.verbose, "%s: %s reference %s not found", filename(f), ref->symbol->name, element->f_string);
				field = INDIRECT(f, 0);
			}
			else
				field = ap->field;
			ref = ref->next;
		}
	field += (n = GETFIELD(sym));
	if (elt >= 0)
	{
		if ((i = r->member[n].format.elements) == (unsigned char)(-1) && (i = -1) || elt < i)
		{
			f = ((Local_t*)sym->local.pointer)->index;
			field = direct(f, field->f_string, i, field) + elt;
			if (node->op == ITERATE)
			{
				val.integer = f->field->elements;
				message((-7, "val: sym=elementsof(%s) dat=\"%s\" val=0x%08I*x fld=%d", sym->name, field->f_string, sizeof(val.integer), val.integer, n));
				return val;
			}
		}
		else if (!ISSILENT(node))
			error(state.verbose, "%s[%d]: array index out of bounds", sym->name, elt);
	}
	switch (sym->type)
	{
	case FLOATING:
		if (!(field->f_converted & CDB_FLOATING))
		{
			field->f_converted |= CDB_FLOATING;
			field->f_double = (field->f_converted & CDB_STRING) ? strtod(field->f_string, NiL) : 0.0;
		}
		val.floating = field->f_double;
		message((-7, "val: sym=%s dat=\"%s\" val=%f fld=%d", sym->name, field->f_string, val.floating, n));
		break;
	case STRING:
		if (!field->f_string)
		{
			if (!field->f_data && !(field->f_data = newof(0, char, WIDTH, 1)))
				error(ERROR_SYSTEM|3, "out of space [WIDTH]");
			field->f_string = strcpy(field->f_data, "*NOTFOUND*");
		}
		val.string = field->f_string;
		message((-7, "val: sym=%s val=\"%s\" fld=%d", sym->name, val.string, n));
		break;
	default:
		switch (b = GETTYPE(sym))
		{
		case T_DATE:
			if (!(field->f_converted & CDB_INTEGER))
			{
				if (!(field->f_converted & CDB_STRING))
					goto zero;
				field->f_converted |= CDB_LONG|CDB_INTEGER;
				field->f_unsigned = (unsigned long)tmdate(field->f_string, &t, NiL);
				if (t == field->f_string)
					field->f_unsigned = 0;
			}
			break;
		case T_ELAPSED:
			if (!(field->f_converted & CDB_INTEGER))
			{
				if (!(field->f_converted & CDB_STRING))
					goto zero;
				field->f_converted |= CDB_LONG|CDB_INTEGER;
				field->f_unsigned = strelapsed(field->f_string, &t, ELAPSED);
				if (t == field->f_string)
					field->f_unsigned = 0;
			}
			break;
		default:
			if (!(field->f_converted & CDB_INTEGER))
			{
				if (!(field->f_converted & CDB_STRING))
					goto zero;
				field->f_converted |= CDB_LONG|CDB_INTEGER;
				field->f_unsigned = strtonll(field->f_string, NiL, &b, 0);
			}
			break;
		zero:
			field->f_converted |= CDB_LONG|CDB_INTEGER;
			field->f_unsigned = 0;
			break;
		}
		val.integer = field->f_unsigned;
		if (sym->type == INTEGER)
			message((-7, "val: sym=%s b=%d dat=\"%s\" val=0x%08I*x fld=%d", sym->name, b, field->f_string, sizeof(val.integer), val.integer, n));
		else
			message((-7, "val: sym=%s val=%I*d fld=%d", sym->name, sizeof(val.integer), val.integer, n));
		break;
	}
	return val;
}

/*
 * set value for symbol sym
 */

static int
assign(Expr_t* prog, Exnode_t* node, Exid_t* sym, Exref_t* ref, void* env, int elt, Extype_t val, Exdisc_t* disc)
{
	register int		n;
	register Field_t*	field;
	register File_t*	f;
	register char*		s;
	register Field_t*	last;
	Assoc_t*		ap;
	char			num[32];

	NoP(node);
	NoP(disc);
	if (ref)
	{
		if (GETBUILTIN(ref->symbol) == B_STATE)
		{
			error(2, "%s: cannot assign", sym->name);
			return -1;
		}
		if (ref->symbol == state.schema)
			ref = ref->next;
	}
	field = (Field_t*)env;
	f = 0;
	while (ref)
	{
		f = ((Local_t*)ref->symbol->local.pointer)->file;
		if (!f->field)
			load(prog, f, ref->symbol);
		s = (field + GETFIELD(ref->symbol))->f_string;
		if (f->hix)
		{
			if (!(field = record(f, s, strsum(s, 0L), f->record->key, 1, 1)))
			{
				if (!ISSILENT(node) && (*s || s != (state.empty + GETFIELD(ref->symbol))->f_string))
					error(state.verbose, "%s: %s index %s not found", filename(f), f->symbol->name, s);
				field = state.empty;
			}
		}
		else if (!f->name)
		{
			if (f->sequence != state.record)
			{
				f->sequence = state.record;
				state.term[n = f->delimiter]++;
				state.term[f->terminator]++;
				field = f->field->first;
				last = f->field->last;
				for (;;)
				{
					if (field <= last)
						(field++)->f_string = s;
					while (!state.term[*((unsigned char*)s)])
						s++;
					if (*s != n)
					{
						*s = 0;
						break;
					}
					*s++ = 0;
				}
				state.term[n]--;
				state.term[f->terminator]--;
				while (field <= last) (field++)->f_string = s;
			}
			field = f->field->first;
		}
		else if (!f->assoc)
		{
			n = strtol(s, NiL, 0) - f->offset;
			if (n < 0 || n >= f->records)
			{
				if (!ISSILENT(node))
					error(state.verbose, "%s: %s record reference %d out of range -- expected %d..%d", filename(f), ref->symbol->name, n + f->offset, f->offset, f->records + f->offset);
				n = 0;
			}
			field = INDIRECT(f, n);
		}
		else if (!(ap = (Assoc_t*)dtmatch(f->assoc, s)) && !(ap = (Assoc_t*)dtmatch(f->assoc, "")))
		{
			if (!ISSILENT(node))
				error(state.verbose, "%s: %s reference %s not found", filename(f), ref->symbol->name, s);
			field = INDIRECT(f, 0);
		}
		else
			field = ap->field;
		ref = ref->next;
	}
	n = GETFIELD(sym);
	if (elt >= 0 && (n += elt) >= ((Local_t*)sym->local.pointer)->record->fields)
	{
		n -= elt;
		if (!ISSILENT(node))
			error(state.verbose, "%s[%d]: array index out of bounds", sym->name, elt);
	}
	if (!f && !(f = ((Local_t*)state.schema->local.pointer)->file) || !f->hix && !state.end)
		return -1;
	switch (sym->type)
	{
	case FLOATING:
		sfsprintf(s = num, sizeof(num), "%f", val.floating);
		break;
	case STRING:
		strlen(s = val.string);
		break;
	default:
		sfsprintf(s = num, sizeof(num), "%lld", val.integer);
		break;
	}
	message((-7, "set: sym=%s val=%s", sym->name, s));
	if (update(f, field, n, s))
		return -1;
	return 0;
}

/*
 * convert operands
 */

static int
convert(Expr_t* prog, register Exnode_t* x, int type, register Exid_t* xref, int arg, Exdisc_t* disc)
{
	time_t		t;
	char*		s;

	static char	buf[64];

	NoP(prog);
	NoP(arg);
	NoP(disc);
	switch (type)
	{
	case T_DATE:
		switch (x->type)
		{
		case FLOATING:
			x->data.constant.value.integer = x->data.constant.value.floating;
			break;
		case STRING:
			x->data.constant.value.integer = tmdate(x->data.constant.value.string, &s, NiL);
			if (*s)
				goto ignored;
			break;
		}
		break;
	case T_ELAPSED:
		switch (x->type)
		{
		case FLOATING:
			x->data.constant.value.integer = x->data.constant.value.floating;
			break;
		case STRING:
			x->data.constant.value.integer = strelapsed(x->data.constant.value.string, &s, ELAPSED);
			if (*s)
				goto ignored;
			break;
		}
		break;
	default:
		if (type <= T_BASE)
			switch (x->type)
			{
			case FLOATING:
				x->data.constant.value.integer = x->data.constant.value.floating;
				break;
			case STRING:
				x->data.constant.value.integer = strtoll(x->data.constant.value.string, &s, type);
				if (*s)
					goto ignored;
				break;
			}
		else
		{
			t = x->data.constant.value.integer;
			switch (type)
			{
			case FLOATING:
				x->data.constant.value.floating = t;
				break;
			case STRING:
				switch (x->type)
				{
				case T_DATE:
					tmfmt(buf, sizeof(buf), "%?%l", &t);
					break;
				case T_ELAPSED:
					strcpy(buf, fmtelapsed((unsigned long)x->data.constant.value.integer, ELAPSED));
					break;
				default:
					sfsprintf(buf, sizeof(buf), "%I*..*d", sizeof(x->data.constant.value.integer), x->type, x->data.constant.value.integer);
					break;
				}
				x->data.constant.value.string = buf;
				break;
			default:
				if (x->type > T_BASE + T_TYPES)
					return -1;
				break;
			}
		}
		break;
	}
	x->type = type;
	return 0;
 ignored:
	if (!ISSILENT(x))
	{
		if (xref)
			error(state.verbose, "%s: `%s' ignored in conversion", xref->name, s);
		else
			error(state.verbose, "`%s' ignored in conversion", s);
	}
	x->type = type;
	return 0;
}

/*
 * initialize the expression state from the symbols in sym
 * the expression handle is returned
 */

Expr_t*
init(register Exid_t* sym)
{
	Expr_t*	prog;

	state.date = (unsigned long)time(NiL);

	/*
	 * allocate an expression handle
	 */

	state.expr.version = EX_VERSION;
	state.expr.flags = EX_CHARSTRING|EX_FATAL|EX_QUALIFY|EX_RETAIN|EX_STRICT;
	state.expr.symbols = sym;
	state.expr.lib = state.expr.type = error_info.id;
	state.expr.convertf = convert;
	state.expr.errorf = errorf;
	state.expr.getf = value;
	state.expr.reff = reference;
	state.expr.setf = assign;
	if (!(prog = exopen(&state.expr)))
		error(3, "expression allocation error");

	/*
	 * initialize the symbol table conversion types
	 */

	state.type[T_DATE - T_BASE - 1] = UNSIGNED;
	state.type[T_ELAPSED - T_BASE - 1] = INTEGER;
	for (; *sym->name; sym++)
		if (GETBUILTIN(sym) && sym->lex == ID && ISINTERNAL(sym->type))
			SETTYPE(sym, sym->type);

	/*
	 * set up the default record and field terminators
	 */

	state.term[0] = 1;
	return prog;
}

/*
 * low level for g2()
 */

static void
g2record(Sfio_t* fp, Sfio_t* ap, Sfio_t* vp, Expr_t* prog, Record_t* rec, const char* sym, int indent)
{
	register int	n;
	register int	i;
	long		pos;

	indent++;
	for (i = 1; i < indent; i++)
		sfputc(fp, '\t');
	sfputr(fp, sym, '\n');
	rec->visit = 2;
	for (n = 0; n < rec->fields; n++)
		if (rec->member[n].record && rec->member[n].record->visit != 2)
		{
			sfstrrel(vp, -1);
			pos = sfstrtell(vp);
			sfprintf(vp, ".%s%c", rec->member[n].symbol->name, 0);
			g2record(fp, ap, vp, prog, rec->member[n].record, rec->member[n].symbol->name, indent);
			sfstrset(vp, pos);
			sfputc(vp, 0);
		}
		else
		{
			for (i = 0; i < indent; i++)
				sfputc(fp, '\t');
			sfputr(fp, rec->member[n].symbol->name, '\t');
			sfputr(fp, rec->member[n].format.type == STRING ? "%s" : "%ld", '\n');
			sfprintf(ap, ", %s.%s", sfstrbase(vp), rec->member[n].symbol->name);
		}
	rec->visit = 0;
}

/*
 * generate g2 printf expression
 */

Exnode_t*
g2print(Expr_t* prog, Record_t* rec)
{
	Sfio_t*	ap;
	Sfio_t*	fp;
	Sfio_t*	vp;

	exexpr(prog, "action", NiL, DELETE);
	ap = sfstropen();
	fp = sfstropen();
	vp = sfstropen();
	sfputr(fp, "action:printf(", '"');
	sfputr(vp, rec->symbol->name, 0);
	g2record(fp, ap, vp, prog, rec, rec->symbol->name, 0);
	sfputc(fp, '"');
	sfputr(fp, sfstruse(ap), ')');
	sfputc(fp, ';');
	excomp(prog, NiL, 0, sfstruse(fp), NiL);
	sfstrclose(vp);
	sfstrclose(fp);
	sfstrclose(ap);
	return exexpr(prog, "action", NiL, 0);
}
