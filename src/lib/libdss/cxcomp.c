/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2002-2007 AT&T Intellectual Property          *
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
 * C expression library parser
 *
 * recursive descent parser snarfed from libast strexpr()
 * which was snarfed from ksh83
 *
 * Glenn Fowler
 * AT&T Research
 */

#include "cxlib.h"

/*
 * clear the current input line
 */

static void
clear(Cx_t* cx)
{
	cx->next = cx->last = cx->base = 0;
}

/*
 * return next input character and advance
 */

static int
next(Cx_t* cx)
{
	int	c;

	for (;;)
	{
		while (cx->next >= cx->last)
		{
			if (!cx->include)
			{
				cx->eof = 1;
				return 0;
			}
			if (cx->include->newline > 1)
			{
				cx->include->newline--;
				return '\n';
			}
			if (cx->include->prompt)
			{
				if (cx->head)
				{
					cx->head = 0;
					sfputr(sfstderr, cx->disc->ps1, -1);
				}
				else
					sfputr(sfstderr, cx->disc->ps2 ? cx->disc->ps2 : "> ", -1);
			}
			if ((cx->base = sfgetr(cx->ip, '\n', 0)) && !(cx->include->newline = 0) || (cx->base = sfgetr(cx->ip, '\n', -1)) && (cx->include->newline = 2))
			{
				cx->next = cx->base;
				cx->last = cx->base + sfvalue(cx->ip);
				error_info.line++;
				if (cx->flags & CX_TRACE)
					sfprintf(sfstderr, "+%d+ %-.*s%s", error_info.line, cx->last - cx->next, cx->base, cx->include->newline ? "\n" : "");
			}
			else if (cxpop(cx))
				return 0;
		}
		for (;;)
		{
			/*
			 * NOTE: sgi.mips3 gets memory fault here if
			 *	 cx->last is on the boundary of an
			 *	 unreadable page -- we caught one of
			 *	 these on the 3b2 in 85
			 */

#ifdef __sgi
			c = *cx->next;
			cx->next++;
			if (c != '\\' || cx->next >= cx->last)
#else
			if ((c = *cx->next++) != '\\' || cx->next >= cx->last)
#endif
				return c;
			if (*cx->next == '\r' && ++cx->next >= cx->last)
				break;
			if (*cx->next != '\n')
				return c;
			if (++cx->next >= cx->last)
				break;
		}
	}
}

/*
 * push back last input character
 */

static void
back(Cx_t* cx)
{
	if (cx->next > cx->base)
	{
		if (cx->include && cx->include->newline == 1)
			cx->include->newline++;
		else
			cx->next--;
	}
}

/*
 * peek next input character
 */

static int
peek(Cx_t* cx, int span)
{
	int	c;

	while (isspace(c = next(cx)) && (c != '\n' || span));
	if (c)
		back(cx);
	return c;
}

/*
 * return current input line context
 */

char*
cxcontext(Cx_t* cx)
{
	char*	s;
	char*	t;

	for (t = cx->next; t > cx->base && *(t-1) == '\n'; t--);
	if ((t - cx->base) < 40)
		sfprintf(cx->tp, "%-.*s<<<", t - cx->base, cx->base);
	else
	{
		for (s = t - 30; s > cx->base && (isalnum(*s) || *s == '_'); s--);
		sfprintf(cx->tp, ">>>%-.*s<<<", t - s, s);
	}
	if (!(s = sfstruse(cx->tp)))
		s = "out of space";
	return s;
}

/*
 * return operator name for code
 */

char*
cxcodename(int code)
{
	register char*	s;
	char*		b;
	int		n;
	int		i;

	static char	name[sizeof(CX_OPNAME)] = CX_OPNAME;

	b = s = fmtbuf(n = 16);
	i = code >> CX_ATTR;
	if (i >= (elementsof(name) - 1))
		sfsprintf(b, n, "<%d:%o>", i + 1, code & ((1<<CX_ATTR)-1));
	else
	{
		switch (name[i])
		{
		case 'C':
			s = strcopy(s, "CALL");
			break;
		case 'D':
			s = strcopy(s, "DEL");
			break;
		case 'G':
			s = strcopy(s, "GET");
			break;
		case 'J':
			s = strcopy(s, "JMP");
			break;
		case 'L':
			s = strcopy(s, "LOG");
			break;
		case 'R':
			s = strcopy(s, "RET");
			break;
		case 'S':
			s = strcopy(s, (code & CX_X2) ? "==" : "SET");
			break;
		case 'e':
			s = strcopy(s, "END");
			break;
		case 'n':
			s = strcopy(s, "NUM");
			break;
		case 'p':
			s = strcopy(s, "POP");
			break;
		case 's':
			s = strcopy(s, "STR");
			break;
		case 't':
			s = strcopy(s, "TST");
			break;
		case '0':
			s = strcopy(s, "NOP");
			break;
		case '~':
			if (code == CX_NOMATCH)
				*s++ = '!';
			else
			{
				code &= ~CX_ASSIGN;
				*s++ = '=';
			}
			/*FALLTHROUGH*/
		default:
			*s++ = name[i];
			if (code & CX_X2)
				*s++ = name[i];
			if ((code & CX_ASSIGN) && code != CX_SET)
				*s++ = '=';
			break;
		}
		*s = 0;
	}
	return b;
}

/*
 * trace instruction at pc to the standard error
 */

void
cxcodetrace(Cx_t* cx, const char* fun, Cxinstruction_t* pc, unsigned int offset)
{
	char*	o;
	char	val[64];

	o = cxcodename(pc->op);
	if ((*o == 'G' || *o == 'S') && *(o + 1) == 'E')
		sfsprintf(val, sizeof(val), "  %s", pc->data.variable->name);
	else if (pc->type == cx->state->type_string)
		sfsprintf(val, sizeof(val), "  \"%s\"", pc->data.string);
	else if (pc->type == cx->state->type_number)
		sfsprintf(val, sizeof(val), "  %Lf", pc->data.number);
	else if (pc->type != cx->state->type_void || *o == 'C')
		sfsprintf(val, sizeof(val), "  %08x", pc->data.pointer);
	else
		val[0] = 0;
	error(0, "%s %04u %8s %-12s  pp %2d%s", fun, offset, o, pc->type->name, pc->pp, val);
}

/*
 * return operator name
 */

char*
cxopname(int code, Cxtype_t* type1, Cxtype_t* type2)
{
	char*	b;
	char*	o;
	int	n;

	o = cxcodename(code);
	if (!type1 || streq(type1->name, "void"))
		return o;
	b = fmtbuf(n = 32);
	if ((code & CX_UNARY) || !type2)
		sfsprintf(b, n, "%s %s", o, type1->name);
	else
		sfsprintf(b, n, "%s %s %s", type1->name, o, type2->name);
	return b;
}

/*
 * push file or string on the include stack
 */

int
cxpush(Cx_t* cx, const char* file, Sfio_t* sp, const char* buf, ssize_t len)
{
	char*		path;
	Cxinclude_t*	ip;
	int		prompt;
	int		retain;
	char		tmp[PATH_MAX];

	if (sp)
	{
		retain = 1;
		if (buf && cx->disc->errorf)
			(*cx->disc->errorf)(NiL, cx->disc, 1, "both file and buffer specified -- buffer ignored");
	}
	else
	{
		retain = 0;
		if (buf && (!(sp = sfstropen()) || sfstrbuf(sp, (char*)buf, len >= 0 ? len : strlen(buf) + 1, 0)))
		{
			if (sp)
				sfclose(sp);
			if (cx->disc->errorf)
				(*cx->disc->errorf)(NiL, cx->disc, 2, "out of space");
			return -1;
		}
	}
	if (!file || streq(file, "-"))
	{
		path = 0;
		if (!sp)
		{
			sp = sfstdin;
			retain = 1;
		}
		prompt = cx->disc->ps1 && isatty(sffileno(sp));
	}
	else
	{
		if (sp)
			path = (char*)file;
		else if (!(path = pathfind(file, cx->id, NiL, tmp, sizeof(tmp))))
		{
			if (cx->disc->errorf)
				(*cx->disc->errorf)(NiL, cx->disc, 2, "%s: include file not found", file);
			return -1;
		}
		else if (!(sp = sfopen(NiL, path, "r")))
		{
			if (cx->disc->errorf)
				(*cx->disc->errorf)(NiL, cx->disc, 2, "%s: cannot read", path);
			return -1;
		}
		prompt = 0;
	}
	if (!(ip = vmnewof(cx->vm, 0, Cxinclude_t, 1, path ? strlen(path) : 0)))
	{
		if (!retain)
			sfclose(sp);
		if (cx->disc->errorf)
			(*cx->disc->errorf)(NiL, cx->disc, 2, "out of space");
		return -1;
	}
	ip->retain = retain;
	cx->interactive = ip->prompt = prompt;
	ip->ofile = error_info.file;
	error_info.file = path ? strcpy(ip->file, path) : (char*)0;
	ip->oline = error_info.line;
	error_info.line = 0;
	ip->sp = cx->ip;
	cx->ip = sp;
	ip->next = cx->include;
	cx->include = ip;
	cx->eof = 0;
	clear(cx);
	return 0;
}

/*
 * pop the top file off the include stack
 * -1 returned when the stack is empty
 */

int
cxpop(Cx_t* cx)
{
	Cxinclude_t*	ip;

	if (!(ip = cx->include))
	{
		cx->eof = 1;
		return -1;
	}
	cx->include = cx->include->next;
	if (!ip->retain)
		sfclose(cx->ip);
	cx->ip = ip->sp;
	error_info.file = ip->ofile;
	error_info.line = ip->oline;
	vmfree(cx->vm, ip);
	clear(cx);
	return cx->include ? 0 : -1;
}

/*
 * add (*donef)(cx,data,disc) to the list to be called at cxfree(cx,expr)
 * donef==0 pops the list
 */

int
cxatfree(Cx_t* cx, Cxexpr_t* expr, Cxdone_f donef, void* data)
{
	register Cxdone_t*	dp;

	if (donef)
	{
		if (!(dp = vmnewof(cx->pm, 0, Cxdone_t, 1, 0)))
		{
			if (cx->disc->errorf)
				(*cx->disc->errorf)(cx, cx->disc, 2, "out of space");
			return -1;
		}
		dp->donef = donef;
		dp->data = data;
		dp->next = expr->done;
		expr->done = dp;
	}
	else
	{
		for (dp = expr->done; dp; dp = dp->next)
			(*dp->donef)(cx, dp->data, cx->disc);
		expr->done = 0;
	}
	return 0;
}

/*
 * parse and return variable with first char c if c!=0
 */

static Cxvariable_t*
variable(Cx_t* cx, register int c, Cxtype_t* m)
{
	Cxoperand_t	r;
	Cxoperand_t	a;
	Cxoperand_t	b;

	if (!c)
		c = next(cx);
	if (!isalpha(c) && c != '_' && c != '$')
	{
		if (cx->disc->errorf)
			(*cx->disc->errorf)(cx, cx->disc, 2, "%s variable identifier expected", cxcontext(cx));
		return 0;
	}
	if (!cx->referencef)
	{
		if (cx->disc->errorf)
			(*cx->disc->errorf)(cx, cx->disc, 2, "%s variables not supported", cxcontext(cx));
		return 0;
	}
	do
	{
		sfputc(cx->tp, c);
	} while (isalnum(c = next(cx)) || c == '_' || c == '$');
	back(cx);
	a.type = cx->state->type_string;
	a.value.string.size = sfstrtell(cx->tp);
	a.value.string.data = sfstruse(cx->tp);
	b.type = m;
	if ((*cx->referencef)(cx, NiL, &r, &b, &a, NiL, cx->disc))
		return 0;
	return r.value.variable;
}

/*
 * encode and output instruction
 */

static Cxtype_t*
code(Cx_t* cx, Cxexpr_t* expr, int op, int pp, Cxtype_t* type1, Cxtype_t* type2, void* pointer, Cxnumber_t number, Cxvariable_t* ref)
{
	Cxinstruction_t*	i1;
	Cxinstruction_t*	i2;
	Cxvariable_t*		v1;
	Cxvariable_t*		v2;
	Cxinstruction_t		x;
	Cxinstruction_t		c;
	Cxoperand_t		r;
	Cxrecode_f		f;
	Cxunsigned_t		m;

	static Cxformat_t	format;

	x.op = op;
	x.type = cx->table->comparison[op] ? cx->state->type_number : type1;
	x.pp = pp;
	if ((cx->pp += pp) > cx->depth)
		cx->depth = cx->pp;
	if (!pointer)
		x.data.number = number;
	else if (op == CX_GET || op == CX_SET || op == CX_REF || op == CX_CALL || !cxisstring(type1))
		x.data.pointer = pointer;
	else
		x.data.string.size = strlen(x.data.string.data = (char*)pointer);
	x.callout = 0;
	if (op == CX_REF || op == CX_GET)
	{
		if (((f = cxrecode(cx, op, type1, type2, cx->disc)) || (f = cxrecode(cx, op, cx->state->type_void, cx->state->type_void, cx->disc))) && (*f)(cx, expr, &x, NiL, NiL, NiL, cx->disc))
			return 0;
		if (op == CX_GET && ((Cxvariable_t*)pointer)->member)
		{
			x.callout = ((Cxvariable_t*)pointer)->member->member->getf;
			x.pp--;
		}
		i1 = i2 = (Cxinstruction_t*)(sfstrseek(cx->xp, 0, SEEK_CUR) - 1 * sizeof(Cxinstruction_t));
		v1 = v2 = i1->data.variable;
		if (op != CX_REF)
			type1 = type2 = i1->type;
	}
	else
	{
		i1 = (Cxinstruction_t*)(sfstrseek(cx->xp, 0, SEEK_CUR) - 2 * sizeof(Cxinstruction_t));
		i2 = i1 + 1;
		if (i1->op == CX_GET && (f = cxrecode(cx, op, type1, type2, cx->disc)))
		{
			if ((*f)(cx, expr, &x, i1, i2, NiL, cx->disc))
				return 0;
			i1 = (Cxinstruction_t*)(sfstrseek(cx->xp, 0, SEEK_CUR) - 2 * sizeof(Cxinstruction_t));
			i2 = i1 + 1;
			type1 = i1->type;
			type2 = i2->type;
		}
		v1 = ref ? ref : i1->op == CX_GET ? i1->data.variable : (Cxvariable_t*)0;
		v2 = i2->op == CX_GET ? i2->data.variable : (Cxvariable_t*)0;
	}
	if (x.callout)
		goto done;
	if (x.callout = cxcallout(cx, op, type1, type2, cx->disc))
		goto done;
	if (type1 == type2)
	{
		type1 = type2 = type1->fundamental;
		if (x.callout = cxcallout(cx, op, type1, type2, cx->disc))
			goto done;
	}
	if (type1 != type2)
	{
		if (x.callout = cxcallout(cx, op, type1, type1, cx->disc))
		{
			if (v1 && v1->format.map && cxisstring(type2) && !cxstr2num(cx, &v1->format, i2->data.string.data, i2->data.string.size, &m))
			{
				i2->op = CX_NUM;
				i2->type = cx->state->type_number;
				i2->data.number = (Cxinteger_t)m;
				goto done;
			}
			if (v2 && v2->format.map && cxisstring(type1) && !cxstr2num(cx, &v2->format, i1->data.string.data, i1->data.string.size, &m))
			{
				i1->op = CX_NUM;
				i1->type = cx->state->type_number;
				i1->data.number = (Cxinteger_t)m;
				goto done;
			}
			if (c.callout = cxcallout(cx, CX_CAST, type1, type2, cx->disc))
			{
				c.op = CX_CAST;
				c.type = type1;
				c.data.number = 0;
				c.pp = 0;
				if ((cx->flags & CX_DEBUG) && sfstrtell(cx->xp))
					cxcodetrace(cx, "comp", &c, (unsigned int)sfstrtell(cx->xp) / sizeof(c));
				sfwrite(cx->xp, &c, sizeof(c));
				goto done;
			}
			if (cxisstring(type2))
			{
				if (cxisnumber(type1) && i2->op == CX_STR && i2->data.string.size == 1)
				{
					i2->op = CX_NUM;
					i2->type = cx->state->type_number;
					i2->data.number = i2->data.string.data[0];
					goto done;
				}
				if (type1->internalf)
				{
					if ((*type1->internalf)(cx, type1, NiL, &format, &r.value, i2->data.string.data, i2->data.string.size, cx->pm, cx->disc) < 0)
						return 0;
					i2->op = CX_NUM;
					i2->type = cx->state->type_number;
					i2->data.number = r.value.number;
					goto done;
				}
			}
			if (cxisnumber(type2) && cxisstring(type1))
			{
				if (i1->op == CX_STR && i1->data.string.size == 1 && (x.callout = cxcallout(cx, op, type2, type2, cx->disc)))
				{
					i1->op = CX_NUM;
					i1->type = cx->state->type_number;
					i1->data.number = i1->data.string.data[0];
					goto done;
				}
			}
		}
		if (type1->fundamental == type2->fundamental && (x.callout = cxcallout(cx, op, type1->fundamental, type1->fundamental, cx->disc)))
			goto done;
		if (cxisstring(type2) && type1->internalf && (x.callout = cxcallout(cx, op, type1->fundamental, type1->fundamental, cx->disc)))
		{
			if (cxisnumber(type1) && v1 && v1->format.map && !cxstr2num(cx, &v1->format, i2->data.string.data, i2->data.string.size, &m))
			{
				i2->op = CX_NUM;
				i2->type = type1->fundamental;
				i2->data.number = (Cxinteger_t)m;
				goto done;
			}
			if ((*type1->internalf)(cx, type1, NiL, &format, &r.value, i2->data.string.data, i2->data.string.size, cx->pm, cx->disc) < 0)
				return 0;
			i2->op = CX_NUM;
			i2->type = type1->fundamental;
			i2->data = r.value;
			goto done;
		}
		if (cxisstring(type1) && type2->internalf && (x.callout = cxcallout(cx, op, type2->fundamental, type2, cx->disc)))
		{
			if (cxisnumber(type2) && v2 && v2->format.map && !cxstr2num(cx, &v2->format, i1->data.string.data, i1->data.string.size, &m))
			{
				i1->op = CX_NUM;
				i1->type = type2->fundamental;
				i1->data.number = (Cxinteger_t)m;
				goto done;
			}
			if ((*type2->internalf)(cx, type2, NiL, &format, &r.value, i1->data.string.data, i1->data.string.size, cx->pm, cx->disc) < 0)
				return 0;
			i1->op = CX_NUM;
			i1->type = type2->fundamental;
			i1->data = r.value;
			goto done;
		}
	}
	if (x.callout = cxcallout(cx, op, cx->state->type_void, cx->state->type_void, cx->disc))
		goto done;
	if (cx->disc->errorf)
		(*cx->disc->errorf)(cx, cx->disc, 2, "%s %s not supported [%d:%d]", cxcontext(cx), cxopname(op, type1, type2), cxrepresentation(type1), cxrepresentation(type2));
	return 0;
 done:
	if ((cx->flags & CX_DEBUG) && sfstrtell(cx->xp))
		cxcodetrace(cx, "comp", &x, (unsigned int)sfstrtell(cx->xp) / sizeof(x));
	if (sfwrite(cx->xp, &x, sizeof(x)) != sizeof(x))
	{
		if (cx->disc->errorf)
			(*cx->disc->errorf)(cx, cx->disc, 2, "out of space");
		return 0;
	}
	return cx->type = x.type;
}

/*
 * return the next expected argument type for v
 * return:
 *	>0 ok
 *	=0 no more arguments
 *	<0 error
 */

static int
prototype(Cx_t* cx, Cxvariable_t* v, Cxtype_t** tp, char** sp, char** ep, int* op)
{
	char*		s;
	char*		e;
	char*		u;
	Cxtype_t*	t;
	int		c;
	int		o;
	int		r;
	char		buf[256];

	s = *sp;
	e = *ep;
	o = *op;
	t = 0;
	r = 0;
	for (;;)
	{
		if ((c = *s++) == 0)
		{
			s--;
			break;
		}
		else if (c == '[')
			o++;
		else if (c == ']')
			o--;
		else if (c == '.')
		{
			if (!e)
			{
				e = s;
				o = 0;
				while (--e > (char*)v->prototype)
				{
					if ((c = *e) == '[' || !o--)
						break;
					else if (c == ']')
						o++;
				}
				o = 1;
			}
			s = e;
			if (*s == '.')
			{
				t = cx->state->type_void;
				r = 1;
				break;
			}
		}
		else if (c == '&' && (isalpha(*s) || *s == '_') || isalpha(c) || c == '_')
		{
			if (c != '&')
				s--;
			for (u = buf; isalnum(*s) || *s == '_'; s++)
				if (u < &buf[sizeof(buf)-1])
					*u++ = *s;
			*u = 0;
			if (t = cxtype(cx, buf, cx->disc))
			{
				if (c == '&')
					t = cx->state->type_reference;
				r = 1;
			}
			else
			{
				if (cx->disc->errorf)
					(*cx->disc->errorf)(cx, cx->disc, 2, "%s %s: unknown prototype type name", cxcontext(cx), buf);
				r = -1;
			}
			break;
		}
	}
	*sp = s;
	*ep = e;
	*op = o;
	*tp = t;
	return r;
}

/*
 * parse the next (sub)expression
 */

static Cxtype_t*
parse(Cx_t* cx, Cxexpr_t* expr, int precedence, Cxvariable_t** ref)
{
	register int	c;
	register int	p;
	char*		s;
	char*		e;
	Cxtype_t*	m;
	Cxtype_t*	t;
	Cxtype_t*	u;
	Cxvariable_t*	f;
	Cxvariable_t*	h;
	Cxvariable_t*	v;
	int		i;
	int		k;
	int		o;
	int		x;
	int		q;
	int		r;
	long		z;
	double		n;

	cx->level++;
 again:
	t = cx->state->type_void;
	h = 0;
	m = 0;
	x = 0;
	v = 0;
	while (c = next(cx))
	{
		if (o = cx->table->opcode[c])
		{
			i = 0;
			if ((p = next(cx)) == c)
			{
				p = next(cx);
				o |= CX_X2;
				i++;
			}
			if (p == '~')
			{
				if (c == '=')
				{
					o = CX_MATCH;
					i++;
				}
				else if (c == '!')
				{
					o = CX_NOMATCH;
					i++;
				}
				else
					back(cx);
			}
			else if (p == '=' && !(o & CX_ASSIGN))
			{
				o |= CX_ASSIGN;
				o &= ~CX_UNARY;
				i++;
			}
			else
				back(cx);
			if (o == CX_ADDADD || o == CX_SUBSUB)
			{
				if (!v)
				{
					if (x)
					{
						if (cx->disc->errorf)
							(*cx->disc->errorf)(cx, cx->disc, 2, "%s operator requires lvalue", cxcontext(cx));
						goto bad;
					}
					if (!(v = variable(cx, 0, m)))
						goto bad;
					h = v;
					m = 0;
					if (cxisvoid(v->type))
						goto undefined;
				}
				if (!code(cx, expr, CX_GET, 1, v->type, cx->state->type_void, v, 0, NiL))
					goto bad;
				if (!code(cx, expr, CX_NUM, 1, cx->state->type_number, cx->state->type_void, NiL, 1.0, NiL))
					goto bad;
				if (!code(cx, expr, o & ~CX_X2, -1, v->type, cx->state->type_number, NiL, 0, NiL))
					goto bad;
				if (!code(cx, expr, CX_SET, 0, cx->state->type_void, v->type, v, 0, NiL))
					goto bad;
				if (x)
				{
					if (!code(cx, expr, CX_NUM, 1, cx->state->type_number, cx->state->type_void, NiL, -1.0, NiL))
						goto bad;
					if (!code(cx, expr, o & ~CX_X2, -1, v->type, cx->state->type_number, NiL, 0, NiL))
						goto bad;
				}
				v = 0;
				x = 1;
				continue;
			}
			if (!x)
				o |= CX_UNARY;
			if (o == CX_AND && precedence == cx->table->precedence[CX_CALL])
			{
				while (i--)
					back(cx);
				goto done;
			}
			if ((o & CX_ASSIGN) && !cx->table->comparison[o])
			{
				if (!v)
				{
					if (cx->disc->errorf)
						(*cx->disc->errorf)(cx, cx->disc, 2, "%s assignment requires lvalue", cxcontext(cx));
					goto bad;
				}
				p = cx->table->precedence[CX_SET];
				if (o != CX_SET)
				{
					if (cxisvoid(v->type))
						goto undefined;
					o &= ~CX_ASSIGN;
					if (!code(cx, expr, CX_GET, 1, v->type, cx->state->type_void, v, 0, NiL))
						goto bad;
				}
			}
			else if (o == CX_REF)
			{
				if (!(v = variable(cx, 0, m)))
					goto bad;
				m = 0;
				if (!code(cx, expr, CX_REF, 1, cx->state->type_reference, cx->state->type_void, v, 0, NiL))
					goto bad;
				v = 0;
				x = 1;
				continue;
			}
			else
			{
				if (v)
				{
					if (cxisvoid(v->type))
						goto undefined;
					if (!code(cx, expr, CX_GET, 1, v->type, cx->state->type_void, v, 0, NiL))
						goto bad;
					v = 0;
				}
				p = cx->table->precedence[o];
			}
			if (!p)
			{
				if ((o & CX_UNARY) && c == '/')
					goto quote;
				if (cx->disc->errorf)
				{
					if (o & CX_UNARY)
						(*cx->disc->errorf)(cx, cx->disc, 2, "%s operand expected [o=%04x]", cxcontext(cx), o);
					else
						(*cx->disc->errorf)(cx, cx->disc, 2, "%s unknown operator", cxcontext(cx));
				}
				goto bad;
			}
			if (o & CX_UNARY)
			{
				if (x)
					goto operator_expected;
			}
			else
			{
				if (!x)
					goto operand_expected;
			}
			if (precedence >= p && o != CX_SET)
			{
				while (i--)
					back(cx);
				goto done;
			}
			z = 0;
			if (!(o & CX_UNARY) && cx->table->logical[o])
			{
				if (cx->type != cx->state->type_number && !code(cx, expr, CX_LOG, 0, cx->type, cx->state->type_void, NiL, 0, NiL))
					goto bad;
				if (o == CX_ANDAND || o == CX_OROR)
				{
					z = sfstrtell(cx->xp);
					if (!code(cx, expr, (o == CX_ANDAND) ? CX_SC0 : CX_SC1, 0, cx->state->type_number, cx->state->type_void, NiL, 0, NiL))
						goto bad;
				}
			}
			t = cx->type;
			if (parse(cx, expr, p, NiL) != cx->state->type_void || cx->error)
				goto bad;
			if (cx->table->logical[o] && cx->type != cx->state->type_number && !code(cx, expr, CX_LOG, 0, cx->type, cx->state->type_void, NiL, 0, NiL))
				goto bad;
			if (o != CX_SET && ((o & CX_UNARY) ? !code(cx, expr, o, 0, cx->type, cx->state->type_void, NiL, 0, NiL) : !code(cx, expr, o, -1, t, cx->type, NiL, 0, h)))
				goto bad;
			if (cx->table->comparison[o] || cx->table->logical[o])
				h = 0;
			if (v)
			{
				if (v->type != cx->type)
				{
					if (v->type != cx->state->type_void)
					{
						if (cx->disc->errorf)
							(*cx->disc->errorf)(cx, cx->disc, 2, "%s cannot assign %s to %s", cxcontext(cx), cx->type->name, v->type->name);
						goto bad;
					}
					v->type = cx->type;
				}
				if (!code(cx, expr, CX_SET, 0, cx->state->type_void, cx->type, v, 0, NiL))
					goto bad;
				v = 0;
			}
			if (z)
				((Cxinstruction_t*)(sfstrbase(cx->xp) + z))->data.number = (sfstrtell(cx->xp) - z) / sizeof(Cxinstruction_t);
			x = 1;
		}
		else if (isdigit(c))
		{
			if (x)
				goto operator_expected;
			i = 0;
			sfputc(cx->tp, c);
			while (isalnum(c = next(cx)) || c == '_' || c == '.' || c == '#')
			{
				sfputc(cx->tp, c);
				switch (c)
				{
				case '#':
					i = 4;
					break;
				case '.':
					i = 1;
					break;
				case 'e':
				case 'E':
					if (i < 3)
					{
						i = 3;
						if ((c = next(cx)) == '-' || c == '+')
							sfputc(cx->tp, c);
						else
							back(cx);
					}
					break;
				}
			}
			back(cx);
			s = sfstruse(cx->tp);
			if (!(i &= 1))
				n = (double)strtonll(s, &e, NiL, 0);
			if (i || *e)
				n = strtod(s, &e);
			if (*e)
			{
				if (cx->disc->errorf)
					(*cx->disc->errorf)(cx, cx->disc, 2, "%s: invalid numeric constant", s);
				goto bad;
			}
			if (!code(cx, expr, CX_NUM, 1, cx->state->type_number, cx->state->type_void, NiL, n, NiL))
				goto bad;
			x = 1;
		}
		else if (isalpha(c) || c == '_' || c == '$')
		{
			if (x)
				goto operator_expected;
			if (!(v = variable(cx, c, m)))
				goto bad;
			h = v;
			m = 0;
			while ((c = next(cx)) == ' ' || c == '\t' || c == '\r');
			if (v->function)
			{
				i = 0;
				if (c == '(')
					p = q = ')';
				else
				{
					p = '\n';
					q = ';';
					if (c && c != p && c != q)
						back(cx);
				}
				while (c == ' ' || c == '\t' || c == '\r')
					c = next(cx);
				if (v->type != cx->state->type_void && !code(cx, expr, CX_NUM, 1, cx->state->type_number, cx->state->type_void, NiL, 0, NiL))
					goto bad;
				o = 0;
				s = (char*)v->prototype;
				e = 0;
				if ((r = prototype(cx, v, &t, &s, &e, &o)) < 0)
					goto bad;
				if (c != p && c != q)
				{
					cx->collecting++;
					k = cx->paren;
					cx->paren = p == ')';
					for (;;)
					{
						z = sfstrtell(cx->xp);
						if (parse(cx, expr, cx->table->precedence[CX_CALL], NiL) != cx->state->type_void || cx->error)
						{
							cx->collecting--;
							cx->paren = k;
							goto bad;
						}
						if (sfstrtell(cx->xp) != z)
						{
							i++;
							while (cx->type != t && t != cx->state->type_void)
							{
								if (o && r > 0 && (r = prototype(cx, v, &t, &s, &e, &o)) > 0)
									continue;
								if (r < 0)
								{
									cx->collecting--;
									cx->paren = k;
									goto bad;
								}
								if (cx->disc->errorf)
								{
									if (r < 0)
										(*cx->disc->errorf)(cx, cx->disc, 2, "%s too many arguments for %s(%s)", cxcontext(cx), v->name, v->prototype);
									else
										(*cx->disc->errorf)(cx, cx->disc, 2, "%s argument type mismatch for %s(%s)", cxcontext(cx), v->name, v->prototype);
								}
								cx->collecting--;
								cx->paren = k;
								goto bad;
							}
							if ((r = prototype(cx, v, &t, &s, &e, &o)) < 0)
							{
								cx->collecting--;
								cx->paren = k;
								goto bad;
							}
						}
						if (!(c = next(cx)))
						{
							if (cx->disc->errorf)
								(*cx->disc->errorf)(cx, cx->disc, 2, "%s EOF in formal argument list", cxcontext(cx));
							cx->collecting--;
							cx->paren = k;
							goto bad;
						}
						if (c == p || c == q)
							break;
						if (c != ',')
							back(cx);
					}
					cx->collecting--;
					cx->paren = k;
					if (c != p && c != q)
					{
						if (cx->disc->errorf)
							(*cx->disc->errorf)(cx, cx->disc, 2, "%s missing %s in formal argument list", cxcontext(cx), p == '\n' ? "statement terminator" : ")");
						goto bad;
					}
				}
				if (r > 0 && !o)
				{
					if (cx->disc->errorf)
						(*cx->disc->errorf)(cx, cx->disc, 2, "%s not enough arguments for %s(%s)", cxcontext(cx), v->name, v->prototype);
					goto bad;
				}
				if (c == '\n')
					back(cx);
				if (!code(cx, expr, CX_CALL, -i, v->type, cx->state->type_void, v, 0, NiL))
					goto bad;
				v = 0;
			}
			else if (c == '(')
			{
				if (cx->disc->errorf)
					(*cx->disc->errorf)(cx, cx->disc, 2, "%s: unknown function", v->name);
				goto bad;
			}
			else
				back(cx);
			x = 1;
		}
		else
			switch (c)
			{
			case 0:
				goto done;
			case '.':
				if (!x || !v || !v->type->base || !v->type->base->member)
				{
					if (cx->disc->errorf)
						(*cx->disc->errorf)(cx, cx->disc, 2, "%s struct or union variable expected", cxcontext(cx));
					goto bad;
				}
				if (!code(cx, expr, CX_GET, 1, v->type, cx->state->type_void, v, 0, NiL))
					goto bad;
				m = v->type->base;
				x = 0;
				v = 0;
				continue;
			case ',':
				if (cx->collecting)
				{
					next(cx);
					goto done;
				}
				if (!code(cx, expr, CX_POP, -1, cx->state->type_void, cx->state->type_void, NiL, 0, NiL))
					goto bad;
				goto again;
			case ';':
				if (cx->collecting)
					x = 1;
				else
					precedence = 0;
				goto done;
			case '#':
				clear(cx);
				goto done;
			case '\n':
				if (cx->collecting)
				{
					if (!cx->paren)
						goto done;
				}
				else if (precedence <= cx->table->precedence[CX_CALL] || precedence > cx->table->precedence[CX_PAREN] && x)
					goto done;
				continue;
			case ' ':
			case '\t':
			case '\r':
				continue;
			case '(':
				if (x)
					goto operator_expected;
				x = 1;
				k = cx->balanced;
				cx->balanced = 0;
				o = cx->collecting;
				cx->collecting = 0;
				u = parse(cx, expr, cx->table->precedence[CX_PAREN], &f);
				h = f;
				cx->collecting = o;
				cx->balanced = k;
				if (u != cx->state->type_void || cx->error)
					goto bad;
				for (;;)
				{
					switch (next(cx))
					{
					case ' ':
					case '\n':
					case '\r':
					case '\t':
						continue;
					case ')':
						if (k)
							goto keep;
						do
						{
							if (!(c = next(cx)) || c == ';' || c == '\n')
							{
								if (cx->level > 0)
									goto done;
								goto keep;
							}
						} while (isspace(c));
						s = cx->next;
						back(cx);
						if (c == '|' || c == '?')
							do
							{
								if (s >= cx->last || *s == '{')
									goto keep;
							} while (isspace(*s++));
						break;
					default:
						back(cx);
						if (cx->disc->errorf)
							(*cx->disc->errorf)(cx, cx->disc, 2, "%s closing ) expected", cxcontext(cx));
						goto bad;
					}
					break;
				}
				break;
			case ')':
				if (!precedence)
				{
					if (cx->disc->errorf)
						(*cx->disc->errorf)(cx, cx->disc, 2, "%s too many )'s", cxcontext(cx));
					goto bad;
				}
				goto done;
			case '?':
				if (!x)
					goto operand_expected;
				if (precedence >= cx->table->precedence[CX_TST])
					goto done;
				if ((c = next(cx)) != ':')
				{
					back(cx);
					if (parse(cx, expr, cx->table->precedence[CX_TST], NiL) != cx->state->type_void || cx->error)
						goto bad;
					if (next(cx) != ':')
					{
						back(cx);
						if (cx->disc->errorf)
							(*cx->disc->errorf)(cx, cx->disc, 2, "%s : expected for ? operator", cxcontext(cx));
						goto bad;
					}
				}
				else if (!code(cx, expr, CX_NOP, 0, cx->state->type_void, cx->state->type_void, NiL, 0, NiL))
					goto bad;
				if (parse(cx, expr, cx->table->precedence[CX_TST], NiL) != cx->state->type_void || cx->error)
					goto bad;
				if (!code(cx, expr, CX_TST, 0, cx->state->type_void, cx->state->type_void, NiL, 0, NiL))
					goto bad;
				break;
			case ':':
				goto done;
			case '"':
			case '\'':
			case '/':
			quote:
				if (x)
					goto operator_expected;
				while ((p = next(cx)) != c)
				{
					if (!p)
					{
						if (cx->disc->errorf)
							(*cx->disc->errorf)(cx, cx->disc, 2, "%s EOF in string literal", cxcontext(cx));
						goto bad;
					}
					sfputc(cx->tp, p);
					if (p == '\\' && (p = next(cx)))
						sfputc(cx->tp, p);
				}
				stresc(s = sfstruse(cx->tp));
				if (!(s = vmstrdup(cx->pm, s)))
				{
					if (cx->disc->errorf)
						(*cx->disc->errorf)(cx, cx->disc, 2, "out of space");
					goto bad;
				}
				if (!code(cx, expr, CX_STR, 1, cx->state->type_string, cx->state->type_void, s, 0, NiL))
					goto bad;
				x = 1;
				break;
			default:
				if (cx->disc->errorf)
					(*cx->disc->errorf)(cx, cx->disc, 2, "syntax error: '%c' not expected: %s", c, cxcontext(cx));
				goto bad;
			}
	}
	if (!x)
	{
		if (ref)
			*ref = h;
		cx->level--;
		return t;
	}
 keep:
	c = 0;
 done:
	if (!x && precedence > cx->table->precedence[CX_CALL])
		goto operand_expected;
	if (c && (precedence || c != '\n' && c != ';'))
		back(cx);
	if (v)
	{
		if (cxisvoid(v->type))
			goto undefined;
		if (!code(cx, expr, CX_GET, 1, v->type, cx->state->type_void, v, 0, NiL))
			goto bad;
	}
	if (ref)
		*ref = h;
	cx->level--;
	return cx->state->type_void;
 undefined:
	if (cx->disc->errorf)
		(*cx->disc->errorf)(cx, cx->disc, 2, "%s undefined variable", cxcontext(cx));
	goto bad;
 operator_expected:
	if (cx->collecting)
		goto done;
	if (cx->disc->errorf)
		(*cx->disc->errorf)(cx, cx->disc, 2, "%s operator expected", cxcontext(cx));
	goto bad;
 operand_expected:
	if (cx->disc->errorf)
		(*cx->disc->errorf)(cx, cx->disc, 2, "%s operand expected", cxcontext(cx));
 bad:
	cx->error = cxtell(cx);
	if (precedence)
		back(cx);
	else
		clear(cx);
	cx->level--;
	return 0;
}

/*
 * allocate an expression node
 */

static Cxexpr_t*
node(Cx_t* cx, size_t n)
{
	Cxexpr_t*	expr;

	if (!(expr = vmnewof(cx->pm, 0, Cxexpr_t, 1, n)))
	{
		if (cx->disc->errorf)
			(*cx->disc->errorf)(NiL, cx->disc, 2, "out of space");
		return 0;
	}
	return expr;
}

/*
 * compile the next complete expression
 */

static Cxexpr_t*
compile(Cx_t* cx, int balanced)
{
	Cxexpr_t*	expr;
	size_t		pos;
	size_t		n;

	cx->pp = 4;
	cx->balanced = balanced;
	cx->head = 1;
	cx->type = cx->state->type_void;
	if (!(expr = node(cx, sizeof(Cxquery_t))))
		return 0;
	expr->query = (Cxquery_t*)(expr + 1);
	sfstrseek(cx->xp, 0, SEEK_SET);
	if (!code(cx, expr, CX_END, 0, cx->state->type_void, cx->state->type_void, NiL, 0, NiL))
		return 0;
	pos = sfstrtell(cx->xp);
	if (parse(cx, expr, 0, NiL) != cx->state->type_void || cx->error || !code(cx, expr, CX_END, 0, cx->state->type_void, cx->state->type_void, NiL, 0, NiL))
		return 0;
	n = sfstrtell(cx->xp) - pos;
	if (!(expr->query->prog = vmnewof(cx->pm, 0, Cxinstruction_t, n / sizeof(Cxinstruction_t), 0)))
	{
		if (cx->disc->errorf)
			(*cx->disc->errorf)(NiL, cx->disc, 2, "out of space");
		return 0;
	}
	memcpy(expr->query->prog, sfstrbase(cx->xp) + pos, n);
	if (cx->depth > cx->stacksize)
	{
		cx->stacksize = roundof(cx->depth, 64);
		if (!(cx->stack = vmnewof(cx->vm, cx->stack, Cxoperand_t, cx->stacksize, 0)))
		{
			if (cx->disc->errorf)
				(*cx->disc->errorf)(cx, cx->disc, 2, "out of space");
			return 0;
		}
	}
	return expr;
}

/*
 * compose dynamic and interpreted queries
 */

static Cxexpr_t*
compose(Cx_t* cx, int prec)
{
	Cxexpr_t*	fp;
	Cxexpr_t*	rp;
	Cxexpr_t*	gp;
	int*		x;
	char**		v;
	char*		f;
	char*		r;
	char*		s;
	int		c;
	int		m;
	int		n;
	int		p;
	int		q;
	unsigned long	o;

	fp = 0;
	rp = 0;
	p = 0;
	q = 0;
	r = 0;
	o = sfstrtell(cx->buf);
	for (;;)
	{
		switch (c = next(cx))
		{
		case '"':
		case '\'':
			if (c == q)
				q = 0;
			else if (!q)
				q = c;
			else
				sfputc(cx->buf, c);
			continue;
		case '\\':
			if (!(c = next(cx)))
				break;
			sfputc(cx->buf, c);
			continue;
		case '{':
			if (q)
				sfputc(cx->buf, c);
			else if (sfstrtell(cx->buf) != o)
				goto syntax;
			else
			{
				if (!(fp = compose(cx, '}')))
					return 0;
				if (next(cx) != '}')
				{
					if (cx->disc->errorf)
						(*cx->disc->errorf)(cx, cx->disc, 2, "unbalanced {...}: %s", cxcontext(cx));
					return 0;
				}
				while (isspace(c = next(cx)));
				back(cx);
				if (c != 0 && c != '|' && c != '?' && c != ':' && c != ';' && c != ',' && c != '}' && c != '>')
				{
					if (cx->disc->errorf)
						(*cx->disc->errorf)(NiL, cx->disc, 2, "operator expected: %s", cxcontext(cx));
					return 0;
				}
				if (fp->next || fp->pass || fp->fail)
				{
					gp = fp;
					if (!(fp = node(cx, 0)))
						return 0;
					fp->group = gp;
				}
			}
			continue;
		case '(':
			if (q)
				sfputc(cx->buf, c);
			else if (sfstrtell(cx->buf) != o)
				goto syntax;
			else
			{
				back(cx);
				if (!(fp = compile(cx, 0)))
					return 0;
			}
			continue;
		case ')':
			if (q)
				sfputc(cx->buf, c);
			else
				goto syntax;
			continue;
		case ':':
			if (next(cx) == c)
			{
				sfputc(cx->buf, c);
				sfputc(cx->buf, c);
				continue;
			}
			back(cx);
			/*FALLTHROUGH*/
		case ',':
			if (c == ',')
				o = sfstrtell(cx->buf);
			/*FALLTHROUGH*/
		case 0:
		case ' ':
		case '\n':
		case '\r':
		case '\t':
		case '>':
		case '|':
		case '?':
		case ';':
		case '}':
			if (q)
			{
				if (c)
					sfputc(cx->buf, c);
				else
				{
					if (cx->disc->errorf)
						(*cx->disc->errorf)(cx, cx->disc, 2, "unterminated %c quote: %s", q, cxcontext(cx));
					return 0;
				}
			}
			else
			{
				if (sfstrtell(cx->buf) != p)
				{
					sfputc(cx->buf, 0);
					if (r)
					{
						s = sfstrbase(cx->buf) + p;
						if (!*s)
							goto syntax;
						if (!(f = (char*)vmstrdup(cx->pm, s)))
						{
							if (cx->disc->errorf)
								(*cx->disc->errorf)(NiL, cx->disc, 2, "out of space");
							return 0;
						}
						sfstrseek(cx->buf, p, SEEK_SET);
					}
					else
					{
						sfwrite(cx->tp, &p, sizeof(p));
						p = sfstrtell(cx->buf);
					}
				}
				if (c == '>')
				{
					if (r)
						goto syntax;
					else if (next(cx) == '>')
						r = "a";
					else
					{
						back(cx);
						r = "w";
					}
				}
				else if (!isspace(c))
				{
					if (!(n = sfstrtell(cx->buf)) && prec == '}')
					{
						back(cx);
						return node(cx, 0);
					}
					if (!fp)
					{
						if (!n)
							goto syntax;
						m = sfstrtell(cx->tp) / sizeof(p);
						if (!(fp = node(cx, (m + 1) * sizeof(char*) + n)))
							return 0;
						fp->argv = v = (char**)(fp + 1);
						s = (char*)(v + m + 1);
						memcpy(s, sfstrseek(cx->buf, 0, SEEK_SET), n);
						x = (int*)sfstrseek(cx->tp, 0, SEEK_SET);
						while (m-- > 0)
							*v++ = s + *x++;
						*v = 0;
						if (!(fp->query = cxquery(cx, s, cx->disc)) && !(cx->test & 0x00000400))
						{
							if (cx->disc->errorf)
								(*cx->disc->errorf)(cx, cx->disc, 2, "%s: query not found", s);
							return 0;
						}
					}
					else if (n)
						goto syntax;
					if (r)
					{
						if (!(fp->op = sfopen(NiL, f, r)))
						{
							if (cx->disc->errorf)
								(*cx->disc->errorf)(cx, cx->disc, ERROR_SYSTEM|2, "%s: cannot write", f);
							return 0;
						}
						fp->file = f;
						r = 0;
					}
					if (!rp)
						rp = fp;
					if (c == 0)
						break;
					if (prec == '|' || prec == '?' || prec == ';')
					{
						if (c == ';' || c == '}')
						{
							back(cx);
							break;
						}
					}
					else if (prec == '}')
					{
						if (c == '}')
						{
							back(cx);
							break;
						}
					}
					if (c == ':')
					{
						back(cx);
						break;
					}
					else if (c == ';' || c == ',')
					{
						if (!(fp = fp->next = compose(cx, c)))
							return 0;
					}
					else if (c == '?')
					{
						if (peek(cx, 1) != ':' && !(fp->pass = compose(cx, c)))
							return 0;
						if (peek(cx, 1) == ':')
						{
							next(cx);
							if (!(fp->fail = compose(cx, c)))
								return 0;
						}
					}
					else if (!(fp->pass = compose(cx, c)))
						return 0;
					p = 0;
				}
			}
			continue;
		default:
			sfputc(cx->buf, c);
			continue;
		}
		break;
	}
	return rp;
 syntax:
	if (cx->disc->errorf)
	{
		if (c)
			(*cx->disc->errorf)(cx, cx->disc, 2, "syntax error: '%c' not expected: %s", c, cxcontext(cx));
		else
			(*cx->disc->errorf)(cx, cx->disc, 2, "syntax error: unterminated expression: %s", cxcontext(cx));
	}
	return 0;
}

/*
 * propagate expression defaults
 */

static void
defaults(register Cxexpr_t* expr, Cxexpr_t* parent, Sfio_t* op)
{
	static Cxquery_t	null;

	do
	{
		expr->parent = parent;
		if (!expr->query)
			expr->query = &null;
		if (!expr->op)
			expr->op = op;
		if (expr->group)
			defaults(expr->group, parent, expr->op);
		if (expr->pass)
			defaults(expr->pass, expr, expr->op);
		if (expr->fail)
			defaults(expr->fail, expr, expr->op);
	} while (expr = expr->next);
}

/*
 * cxlist helper
 */

static void
list(Sfio_t* sp, Cxexpr_t* expr)
{
	register Cxinstruction_t*	pc;
	char**				v;

	for (;;)
	{
		if (expr->group)
		{
			sfputc(sp, '{');
			list(sp, expr->group);
			sfputc(sp, '}');
		}
		else if (expr->argv)
			for (v = expr->argv; *v; v++)
				sfputr(sp, *v, *(v + 1) ? ' ' : -1);
		else if (pc = expr->query->prog)
		{
			sfputc(sp, '(');
			while (pc->op != CX_END)
			{
				sfprintf(sp, "%s %s %d ", cxcodename(pc->op), pc->type->name, pc->pp > 0 ? pc->pp : pc->pp - 1);
				if (pc->op == CX_GET || pc->op == CX_SET || pc->op == CX_REF || pc->op == CX_CALL)
					sfprintf(sp, "%s", pc->data.variable->name);
				else
					sfprintf(sp, "%I*g", sizeof(pc->data.number), pc->data.number);
				sfputc(sp, ';');
				pc++;
			}
			sfputc(sp, ')');
		}
		else
			sfprintf(sp, "()");
		if (expr->file)
			sfprintf(sp, ">%s", expr->file);
		if (expr->fail)
		{
			sfputc(sp, '?');
			if (expr->pass)
				list(sp, expr->pass);
			sfputc(sp, ':');
			list(sp, expr->fail);
		}
		else if (expr->pass)
		{
			sfputc(sp, '|');
			list(sp, expr->pass);
		}
		if (!(expr = expr->next))
			break;
		sfputc(sp, ';');
	}
}
/*
 * list query expression
 */

int
cxlist(Cx_t* cx, Cxexpr_t* expr, Sfio_t* sp)
{
	list(sp, expr);
	sfputc(sp, '\n');
	return sfsync(sp);
}

/*
 * compile the next expression/query
 */

Cxexpr_t*
cxcomp(Cx_t* cx)
{
	Cxexpr_t*	expr;
	int		c;

	if (cx->eof)
		return 0;
	cx->error = 0;
	cx->reclaim = !!(cx->deletef = cxcallout(cx, CX_DEL, cx->state->type_void, cx->state->type_void, cx->disc));
	cx->returnf = cxcallout(cx, CX_RET, cx->state->type_void, cx->state->type_void, cx->disc);
	cx->referencef = cxcallout(cx, CX_REF, cx->state->type_string, cx->state->type_void, cx->disc);
	if (sfsync(cx->op) < 0)
	{
		if (cx->disc->errorf)
			(*cx->disc->errorf)(cx, cx->disc, 2, "write error");
		return 0;
	}
	if (!(cx->pm = vmopen(Vmdcheap, Vmlast, 0)))
	{
		if (cx->disc->errorf)
			(*cx->disc->errorf)(cx, cx->disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	if ((c = peek(cx, !cx->interactive)) == '{' || c == '(')
	{
		sfstrseek(cx->buf, 0, SEEK_SET);
		if (!(expr = compose(cx, 0)))
		{
			vmclose(cx->pm);
			return 0;
		}
		clear(cx);
	}
	else if (!(expr = compile(cx, cx->flags & CX_BALANCED)))
	{
		vmclose(cx->pm);
		return 0;
	}
	expr->vm = cx->pm;
	defaults(expr, expr, sfstdout);
	if (!cx->include && !(cx->flags & CX_BALANCED))
		cx->eof = 1;
	return expr;
}

/*
 * free a cxcomp() expr
 */

int
cxfree(Cx_t* cx, Cxexpr_t* expr)
{
	if (!expr->vm)
		return -1;
	cxatfree(cx, expr, NiL, NiL);
	vmclose(expr->vm);
	expr->vm = 0;
	return 0;
}

ssize_t
cxtell(Cx_t* cx)
{
	if (cx->eof)
		return -1;
	if (cx->error)
		return cx->error;
	if (cx->next >= cx->last)
		return -1;
	return cx->next - cx->base;
}
