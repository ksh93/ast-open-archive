/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1991-2003 AT&T Corp.                *
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
 * cql declaration parse
 */

#include "cql.h"

/*
 * check and initialize structure forward reference name
 */

static Exid_t*
forward(Expr_t* prog, register Exid_t* sym, char* name)
{
	register Record_t*	rec;
	int			n;

	if (!sym && !(sym = (Exid_t*)dtmatch(prog->symbols, name)))
	{
		n = strlen(name);
		if (!(sym = newof(0, Exid_t, 1, n)))
			error(3, "out of space [Exid_t]");
		memcpy(sym->name, name, n);
		dtinsert(prog->symbols, sym);
	}
	sym->lex = ID;
	if (!sym->local.pointer)
	{
		if (!(sym->local.pointer = (char*)newof(0, Local_t, 1, 0)))
			error(3, "out of space [local]");
		message((-6, "lcl: sym=%s local=%p [%s:%d]", sym->name, sym->local.pointer, __FILE__, __LINE__));
	}
	if (!name && !(rec = ((Local_t*)(sym->local.pointer))->record))
	{
		if (!(rec = newof(0, Record_t, 1, 0)) || !(rec->member = newof(0, Member_t, MEMBERCHUNK, 0)))
			error(3, "out of space [record]");
		((Local_t*)(sym->local.pointer))->record = rec;
		rec->symbol = sym;
	}
	return sym;
}

/*
 * parse the rest of an initializer operand (<id>. already seen)
 * sym is the first component
 * op is the expected op '=', ';' or ',' (implying '}')
 * if key != 0 then the last component is expected to be a keyword
 * and key will point to the symbol
 */

static Exid_t*
operand(Expr_t* prog, Exid_t* sym, int op, Exid_t** key, int delay)
{
	register char*	s;
	register char*	t;
	register int	c;
	char*		e;
	Exid_t*		arg = 0;
	Exid_t*		rec = 0;
	char		buf[1024];

	t = buf;
	if (sym)
	{
		s = 0;
		if (sym->local.pointer && ((Local_t*)sym->local.pointer)->record)
		{
			if ((rec = ((Local_t*)sym->local.pointer)->record->symbol) != state.schema)
				s = rec->name;
			rec = sym;
		}
		else
			s = sym->name;
		if (s)
		{
			for (; *s && t < &buf[sizeof(buf) - 1]; *t++ = *s++);
			if (t < &buf[sizeof(buf) - 1])
				*t++ = '.';
		}
	}
	e = t;
	for (;;)
	{
		if ((c = extoken(prog)) == ID)
			arg = exlval.id;
		else if (c == NAME)
			arg = 0;
		else if (delay)
			return 0;
		else
			error(3, "%sreference file identifier expected", context(prog));
		s = exlval.id->name;
		if (t == buf || strncmp(s, buf, t - buf - 1))
			while (*s && t < &buf[sizeof(buf) - 1])
				*t++ = *s++;
		if ((c = extoken(prog)) != '.')
		{
			if (c == op || op == ',' && c == '}')
				break;
			if (delay)
				return 0;
			error(3, "%sreference file identifier expected", context(prog));
		}
		if (t < &buf[sizeof(buf) - 1])
			*t++ = '.';
		e = t;
	}
	*t = 0;
	s = buf;
	if (key)
	{
		*key = arg;
		if (e > s)
			*(e - 1) = 0;
	}
	message((-5, "set: %s", s));
	if (e > s)
	{
		if (rec)
		{
			sym = state.schema;
			state.schema = rec;
		}
		state.operand = 1;
		excomp(prog, error_info.file, error_info.line, s, NiL);
		state.operand = 0;
		if (rec)
			state.schema = sym;
		rec = forward(prog, NiL, s);
	}
	if (key && e > s)
		*(e - 1) = '.';
	exlval.op = c;
	return rec;
}

/*
 * parse a list of field names and insert on b
 * c is the current token type
 */

static void
insert(Expr_t* prog, List_t** b, int c)
{
	register List_t*	p;
	register List_t*	e;
	Exid_t*			sym;

	state.insert = 1;
	switch (c)
	{
	case ID:
	case NAME:
		sym = exlval.id;
		if (!(e = newof(0, List_t, 1, 0)))
			error(3, "out of space [list]");
		e->value.symbol = ((c = extoken(prog)) == '.') ? operand(prog, sym, ';', NiL, 0) : sym;
		if (b == &state.index) SETINDEX(e->value.symbol);
		p = e;
		break;
	case '{':
		e = 0;
		do
		{
			if (e) e = e->next = newof(0, List_t, 1, 0);
			else p = e = newof(0, List_t, 1, 0);
			if (!e) error(3, "out of space [list]");
			e->value.symbol = operand(prog, NiL, ',', NiL, 0);
			if (b == &state.index) SETINDEX(e->value.symbol);
		} while (exlval.op != '}');
		break;
	default:
		error(3, "%sfield list expected", context(prog));
		break;
	}
	e->next = *b;
	*b = p;
	state.insert = 0;
}

/*
 * parse an initialization
 */

static Exid_t*
initialize(Expr_t* prog, Exid_t* larg, Exid_t* sym, int c, int delay)
{
	register char*	s;
	register char*	t;
	register int	i;
	int		m;
	char*		e;
	char*		u;
	Exid_t*		rarg;
	Exid_t*		key;
	File_t*		f;

	if (c != '.')
	{
		key = sym;
		sym = state.schema;
	}
	else if (!(sym = operand(prog, sym, '=', &key, delay)) && delay)
		return 0;
	if (sym)
		attach(sym, NiL, 0);
	c = extoken(prog);
	switch (key ? GETKEY(key) : 0)
	{
	case K_access:
		if (!sym) error(3, "%smain schema not defined", context(prog));
		if (c != STRING) error(3, "%sstring value expected", context(prog));
		switch (*exlval.string)
		{
		case 'a':
		case 'A':
			((Local_t*)sym->local.pointer)->file->access = ASSOCIATIVE;
			break;
		case 'i':
		case 'I':
			((Local_t*)sym->local.pointer)->file->access = INDEX;
			break;
		case 0:
		case 'n':
		case 'N':
			((Local_t*)sym->local.pointer)->file->access = NONE;
			break;
		default:
			error(3, "%sinvalid index method", context(prog));
			break;
		}
		break;
	case K_comment:
		if (!sym) error(3, "%smain schema not defined", context(prog));
		if (c != STRING) error(3, "%sstring value expected", context(prog));
		((Local_t*)sym->local.pointer)->file->comment = strdup(exlval.string);
		break;
	case K_delimiter:
		if (!sym) error(3, "%smain schema not defined", context(prog));
		if (c != STRING) error(3, "%sstring value expected", context(prog));
		((Local_t*)sym->local.pointer)->file->delimiter = *exlval.string;
		break;
	case K_details:
		if (!sym) error(3, "%smain schema not defined", context(prog));
		if (c != STRING) error(3, "%sstring value expected", context(prog));
		((Local_t*)sym->local.pointer)->file->details = strdup(exlval.string);
		break;
	case K_edge:
		insert(prog, &state.edge, c);
		break;
	case K_format:
		if (!sym)
			error(3, "%smain schema not defined", context(prog));
		if (c != STRING)
			error(3, "%sstring value expected", context(prog));
		if (!(((Local_t*)sym->local.pointer)->file->format = cdbgetmeth(exlval.string)))
			error(3, "%s: unknown format", exlval.string);
		break;
	case K_index:
		insert(prog, &state.index, c);
		break;
	case K_input:
		if (!sym) error(3, "%smain schema not defined", context(prog));
		if (((Local_t*)sym->local.pointer)->file->name)
			error(3, "%salready defined", context(prog));
		switch (c)
		{
		case STRING:
			((Local_t*)sym->local.pointer)->file->name = exstrdup(prog, exlval.string);
			break;
		case ID:
			rarg = exlval.id;
			if (!(f = ((Local_t*)initialize(prog, key, rarg, extoken(prog), 0)->local.pointer)->file)->here)
				((Local_t*)sym->local.pointer)->file->name = f->name;
			else if (f != ((Local_t*)sym->local.pointer)->file)
			{
				if (((Local_t*)sym->local.pointer)->file)
					free(((Local_t*)sym->local.pointer)->file);
				((Local_t*)sym->local.pointer)->file = f;
			}
			if (larg == key) larg = 0;
			break;
		case '{':
			i = 0;
			s = t = e = 0;
			for (;;)
			{
				switch (extoken(prog))
				{
				case '}':
					break;
				case ',':
					continue;
				case STRING:
					u = exlval.string;
					while (*u)
					{
						if (s >= e)
						{
							m = s - t;
							i += HERECHUNK;
							if (!(t = newof(t, char, i, 0)))
								error(3, "out of space [here]");
							s = t + m;
							e = t + i - 2;
						}
						*s++ = *u++;
					}
					*s++ = '\n';
					continue;
				default:
					error(3, "%sstring or } expected", context(prog));
					break;
				}
				break;
			}
			if (!s) error(3, "empty here file");
			*s++ = 0;
			if (s < e && !(t = newof(t, char, s - t, 0)))
				error(3, "out of space [here]");
			((Local_t*)sym->local.pointer)->file->name = t;
			((Local_t*)sym->local.pointer)->file->here = s - t;
			break;
		default:
			error(3, "%sinvalid input file name", context(prog));
			break;
		}
		if (larg == key) larg = 0;
		break;
	case K_permanent:
		if (!sym) error(3, "%smain schema not defined", context(prog));
		if (c != INTEGER) error(3, "%sinteger value expected", context(prog));
		((Local_t*)sym->local.pointer)->file->permanent = exlval.integer;
		break;
	case K_scanlimit:
		if (!sym) error(3, "%smain schema not defined", context(prog));
		if (c != INTEGER) error(3, "%sinteger value expected", context(prog));
		((Local_t*)sym->local.pointer)->file->scanlimit = exlval.integer;
		break;
	case K_schema:
		if (c == STRING)
		{
			if (!sym) error(3, "%smain schema not defined", context(prog));
			((Local_t*)sym->local.pointer)->file->schema = strdup(exlval.string);
		}
		else if (c != ID && c != NAME)
			error(3, "%sschema identifier expected", context(prog));
		else
		{
			state.schema = sym = forward(prog, exlval.id, NiL);
			attach(sym, NiL, 0);
		}
		break;
	case K_sort:
		insert(prog, &state.sort, c);
		break;
	case K_terminator:
		if (!sym) error(3, "%smain schema not defined", context(prog));
		if (c != STRING) error(3, "%sstring value expected", context(prog));
		((Local_t*)sym->local.pointer)->file->terminator = *exlval.string;
		((Local_t*)sym->local.pointer)->file->termset = 1;
		break;
	default:
		if (delay)
			return 0;
		error(3, "%sinvalid initializer", context(prog));
		break;
	}
	if (larg) error(3, "%sinitialization type conflict", context(prog));
	return sym;
}

/*
 * parse a declaration
 * delay!=0 delays expop() if expressions combined with declarations
 * 0 returned in input popped
 */

int
declare(Expr_t* prog, const char* s, Sfio_t* sp, int delay)
{
	register int		c;
	register Record_t*	rec;
	register Exid_t*	sym;
	char*			e;
	int			i;
	int			j;
	int			m;
	int			no;
	int			ma;
	int			mi;
	Format_t		format;
	Record_t*		mr;

	message((-10, "decl:"));
	if (expush(prog, NiL, 0, s, sp))
		return 0;
	format.delimiter = DELDEF;
	state.declaration++;
	for (;;)
	{
		exstatement(prog);
		switch (extoken(prog))
		{
		case 0:
			state.declaration--;
			return 0;
		case ';':
			continue;
		case DECLARE:
		case LABEL:
			if (delay)
			{
				exrewind(prog);
				state.declaration--;
				return 1;
			}
			expop(prog);
			state.declaration--;
			return 0;
		case ID:
		case NAME:
			break;
		default:
			if (delay)
			{
				exrewind(prog);
				state.declaration--;
				return 1;
			}
			error(3, "%sschema identifier expected", context(prog));
		}
		sym = forward(prog, exlval.id, NiL);
		switch (c = extoken(prog))
		{
		case '.':
		case '=':
			if (!initialize(prog, NiL, sym, c, delay))
			{
				exrewind(prog);
				state.declaration--;
				return 1;
			}
			continue;
		case '{':
			break;
		default:
			if (delay)
			{
				exrewind(prog);
				state.declaration--;
				return 1;
			}
			expop(prog);
			state.declaration--;
			return 0;
		}
		rec = ((Local_t*)(sym->local.pointer))->record;
		if (rec->fields)
			error(3, "%s: schema already defined", sym->name);
		if (!state.schema)
			state.schema = sym;
		format.referenced = 0;
		m = MEMBERCHUNK;
		mi = 0;
		while ((c = extoken(prog)) != '}')
		{
			i = rec->fields;
			for (;;)
			{
				if (c == DECLARE)
				{
					sym = exlval.id;
					format.type = sym->type;
					mr = 0;
				}
				else if (c == ID || c == NAME)
				{
					if ((c = GETKEY(exlval.id) == K_key) || GETKEY(exlval.id) == K_index)
					{
						if (c)
						{
							if (rec->key)
								error(2, "%s: %sindex already defined", rec->symbol->name, context(prog));
							else
								rec->key = i;
						}
						mi = 1;
						if ((c = extoken(prog)) == '*')
						{
							c = extoken(prog);
							error(1, "%svirtual indices not implemented", context(prog));
						}
						continue;
					}
					sym = forward(prog, exlval.id, NiL);
					if (GETBUILTIN(sym) == B_STATE)
						error(3, "%s: invalid schema name", sym->name);
					mr = ((Local_t*)(sym->local.pointer))->record;
					format.type = 0;
				}
				else
					error(3, "%stype expected", context(prog));
				break;
			}
			if ((c = extoken(prog)) == '*')
			{
				if ((c = extoken(prog)) == '*')
				{
					error(1, "%s**: obsolete: use %s*", sym->name, sym->name);
					c = extoken(prog);
				}
				if (mr)
				{
					if (mr->member)
						mr->member[0].index = 2;
					format.type = STRING;
					ma = ASSOCIATIVE;
				}
				else
					error(3, "%s*: invalid pointer type", sym->name);
			}
			else if (mr)
			{
				format.type = STRING;
				ma = DIRECT;
			}
			else
				ma = SCALAR;
			format.base = format.width = 0;
			format.direction = 0;
			format.string = 0;
			format.sorted = 0;
			format.format = 0;
			while (c == ':')
			{
				no = 0;
				if ((c = extoken(prog)) == INTEGER)
					j = 'w';
				else if (c == ID || c == NAME || c == LABEL)
				{
					if (exlval.id->name[0] == 'n' && exlval.id->name[1] == 'o')
					{
						no = 1;
						j = exlval.id->name[2];
					}
					else
					{
						j = exlval.id->name[0];
						if ((c = extoken(prog)) == '=')
						{
							c = extoken(prog);
							if (c != INTEGER && c != STRING)
								error(3, "%sinteger attribute value expected", context(prog));
						}
						else if (j != 's')
							error(3, "%sattribute value expected", context(prog));
					}
				}
				else
					j = 0;
				switch (j)
				{
				case 'b':
					format.base = exlval.integer;
					if (format.base <= 0 || format.base > T_BASE)
						error(3, "%sinvalid base -- 1 through %d expected", context(prog), T_BASE);
					break;
				case 'd':
					if (no)
						format.delimiter = DELOFF;
					else if (c == STRING)
						format.delimiter = *exlval.string;
					else
						error(3, "%sstring value expected", context(prog));
					break;
				case 'f':
					if (no)
						/*skip*/;
					else if (c == STRING)
					{
						format.format = strdup(exlval.string);
						if (*format.format)
						{
							e = format.format + strlen(format.format) - 1;
							switch (*e)
							{
							case 'i':
								*e = 'l';
								break;
							case 'I':
								*e = 'L';
								break;
							case 'u':
								*e = 'w';
								break;
							case 'U':
								*e = 'W';
								break;
							}
						}
					}
					else
						error(3, "%sstring value expected", context(prog));
					break;
				case 's':
					format.sorted = 1;
					continue;
				case 'w':
					format.width = exlval.integer;
					break;
				default:
					error(3, "%sinvalid attribute", context(prog));
				}
				c = extoken(prog);
			}
			for (;;)
			{
				if (c == ';')
				{
					if (rec->fields > i)
						break;
					sym = 0;
				}
				else if (c == ',')
				{
					c = extoken(prog);
					continue;
				}
				else if (c == ID || c == NAME)
				{
					sym = exlval.id;
					sym->lex = ID;
					if (GETBUILTIN(sym) == B_STATE)
						error(3, "%s: invalid member name", sym->name);
				}
				else
					error(3, "%sfield identifier expected [%d]", context(prog), c);
				if ((c = extoken(prog)) == '[')
				{
					if (format.type != STRING)
						error(3, "%sinvalid array type", context(prog));
					if ((c = extoken(prog)) == INTEGER)
					{
						format.elements = exlval.integer;
						c = extoken(prog);
					}
					else
						format.elements = -1;
					if (c != ']')
						error(3, "%sinteger array size expected", context(prog));
					c = extoken(prog);
					if (ma != ASSOCIATIVE)
						ma = DIRECT;
				}
				else
					format.elements = 0;
				if (sym)
					for (j = 0; j < rec->fields; j++)
						if (rec->member[j].symbol == sym)
							error(3, "%s.%s: field already defined", rec->symbol->name, sym->name);
				if (rec->fields >= m)
				{
					m = m * (5 * FIELDCHUNK) / 4;
					if (!(rec->member = newof(rec->member, Member_t, m, 0)))
						error(3, "out of space [member]");
				}
				format.string = ISSTRING(GETTYPE(sym)) || format.type == STRING;
				rec->member[rec->fields].symbol = sym;
				rec->member[rec->fields].access = ma;
				rec->member[rec->fields].format = format;
				if (mi && !rec->member[rec->fields].index)
					rec->member[rec->fields].index = mi;
				rec->member[rec->fields].record = mr;
				rec->fields++;
			}
			mi = 0;
		}
		if (!rec->fields)
			error(3, "%s: schema has no fields", rec->symbol->name);
		if (rec->fields < m && !(rec->member = newof(rec->member, Member_t, rec->fields, 0)))
			error(3, "out of space [member]");
		if (rec->fields > state.maxfield)
			state.maxfield = rec->fields;
	}
}

/*
 * initialize r.subfields
 * called after all schemas declared
 */

int
subfields(register Record_t* r)
{
	register Member_t*	m;
	register Member_t*	e;
	register Record_t*	s;
	register Format_t*	x;
	register Format_t*	y;
	register Format_t*	z;
	register File_t*	f;
	int			rd;
	int			sd;

	if (!r->subfields)
	{
		e = r->member + r->fields;
		for (m = r->member; m < e; m++)
		{
			m->subfield = r->subfields++;
			if (s = m->record)
			{
				if (!(sd = s->subfields))
					sd = subfields(s);
				if (m->access == DIRECT)
					r->subfields += sd - 1;
			}
		}
	}
	if (!r->format)
	{
		if (!(r->format = newof(0, Format_t, r->subfields, 0)))
			error(3, "out of space [Format_t]");
		x = r->format;
		e = r->member + r->fields;
		rd = (Local_t*)r->symbol->local.pointer && (f = ((Local_t*)r->symbol->local.pointer)->file) && f->delimiter != DELDEF ? f->delimiter : DELIMITER;
		for (m = r->member; m < e; m++)
		{
			if (m->access == DIRECT && (s = m->record))
			{
				if (!s->symbol->local.pointer || !(f = ((Local_t*)s->symbol->local.pointer)->file) || !(sd = f->delimiter))
					sd = rd;
				for (y = s->format, z = y + s->subfields; y < z; x++)
				{
					*x = *y++;
					if (y == z)
					{
						x->delimiter = rd;
						x->direction = '-';
					}
					else
					{
						if (x->delimiter == DELDEF)
							x->delimiter = sd;
						x->direction = '+';
					}
				}
			}
			else
			{
				*x = m->format;
				if (x->delimiter == DELDEF)
					x->delimiter = rd;
				if (m->record)
					x->string = 1;
				x++;
			}
		}
	}
	return r->subfields;
}

/*
 * propagate sub-schema index counts to parent schema
 * called after all schemas declared
 */

void
propagate(register Record_t* r)
{
	register Member_t*	m;
	register Member_t*	e;
	register Record_t*	s;

	if (r->visit)
		return;
	r->visit = 1;
	if (!r->subfields)
		subfields(r);
	e = r->member + r->fields;
	for (m = r->member; m < e; m++)
	{
		if (s = m->record)
		{
			propagate(s);
			if (s->symbol->local.pointer && ((Local_t*)s->symbol->local.pointer)->file)
			{
				m->index = 2;
				s->member[0].index = 2;
			}
		}
	}
	if (state.maxfield < r->subfields)
		state.maxfield = r->subfields;
	debug(	if (error_info.trace <= -4)
		{
			error(-4, "rec: schema=%s key=%d fields=%d subfields=%d", r->symbol->name, r->key, r->fields, r->subfields);
			for (m = r->member; m < e; m++)
				error(-4, "     [%d:%d] %s record=%s type=%d access=%c index=%d base=%d width=%d elements=%d", m - r->member, m->subfield, m->symbol->name, m->record ? m->record->symbol->name : (char*)0, m->format.type, -m->access, m->index, m->format.base, m->format.width, m->format.elements);
		}
		);
}
