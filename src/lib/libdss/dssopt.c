/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 2002-2006 AT&T Corp.                  *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                            by AT&T Corp.                             *
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
 * dss optget() support
 */

#include "dsshdr.h"

/*
 * format optget(3) description item on sp given plain text and item name
 */

static int
optout(register Sfio_t* sp, const char* name, const char* type, const char* map, register const char* s, const char* x)
{
	sfputc(sp, '[');
	sfputc(sp, '+');
	if (name)
	{
		sfprintf(sp, "%s", name);
		if (type)
			sfputc(sp, ' ');
	}
	if (type)
	{
		sfprintf(sp, "(%s", type);
		if (map)
			sfprintf(sp, "::::%s", map);
		sfputc(sp, ')');
	}
	sfputc(sp, '?');
	if (s)
	{
		if (optesc(sp, s, 0))
			return -1;
		if (x)
		{
			sfputc(sp, ' ');
			if (optesc(sp, x, 0))
				return -1;
		}
	}
	else
		sfputc(sp, ' ');
	sfputc(sp, ']');
	return 0;
}

/*
 * structure type info output
 */

static int
optmem(register Sfio_t* sp, Cxtype_t* type)
{
	register Cxvariable_t*	mp;

	sfprintf(sp, "{\n");
	for (mp = (Cxvariable_t*)dtfirst(type->member->members); mp; mp = (Cxvariable_t*)dtnext(type->member->members, mp))
	{
		if (optout(sp, mp->name, mp->type->name, NiL, mp->description, mp->type->member ? "This structure type has the following members:" : (char*)0))
			return -1;
		if (mp->type->member && optmem(sp, mp->type))
			return -1;
	}
	sfprintf(sp, "}\n");
	return 0;
}

/*
 * type info output
 */

static int
opttype(Sfio_t* sp, register Cxtype_t* tp, int members)
{
	register const char*	x;
	register const char*	b;

	if (tp->member)
		x = "This structure type has the following members:";
	else if (tp->match)
		x = tp->match->description;
	else
		x = 0;
	if (tp->format.description)
	{
		b = !x ? "" : x[0] == '[' && x[1] == '+' && x[2] == '?' ? (x + 3) : x;
		if (tp->format.details)
			x = sfprints("%s The default value is \b%s\b. %s", tp->format.description, tp->format.details, b);
		else if (x)
			x = sfprints("%s %s", tp->format.description, b);
		else
			x = tp->format.description;
	}
	if (optout(sp, tp->name, tp->base ? tp->base->name : (const char*)0, NiL, tp->description, x))
		return -1;
	if (tp->member && optmem(sp, tp))
		return -1;
	else if (tp->base && tp->base->member && !(tp->base->header.flags & CX_REFERENCED) && opttype(sp, tp->base, 0))
		return -1;
	return 0;
}
/*
 * data map output
 */

static int
optmap(register Sfio_t* sp, Cxmap_t* map)
{
	size_t		n;
	Cxpart_t*	part;
	Cxitem_t*	item;

	sfprintf(sp, "{\n");
	if (map->shift)
		sfprintf(sp, "[+SHIFT=%u]", map->shift);
	if (~map->mask)
		sfprintf(sp, "[+MASK=0x%016llx]", map->mask);
	if (map->num2str)
	{
		if ((n = dtsize(map->num2str)) > 16)
			sfprintf(sp, "[+-----?%u entries omitted -----]", n);
		else
			for (item = (Cxitem_t*)dtfirst(map->num2str); item; item = (Cxitem_t*)dtnext(map->num2str, item))
				if (item->mask == item->value)
					sfprintf(sp, "[+%s?0x%016llx]", item->name, item->value);
				else
					sfprintf(sp, "[+%s?%llu]", item->name, item->value);
	}
	else
		for (part = map->part; part; part = part->next)
		{
			if (part->shift)
				sfprintf(sp, "[+SHIFT=%u]", part->shift);
			if (~part->mask)
				sfprintf(sp, "[+MASK=0x%016llx]", part->mask);
			for (item = part->item; item; item = item->next)
			{
				if (item->mask == item->value)
					sfprintf(sp, "[+%s?0x%016llx]", item->name, item->value);
				else
					sfprintf(sp, "[+%s?%llu]", item->name, item->value);
				if (item->map && optmap(sp, item->map))
					return -1;
			}
		}
	sfprintf(sp, "}\n");
	return 0;
}

/*
 * optget() info discipline function
 */

int
dssoptinfo(Opt_t* op, Sfio_t* sp, const char* s, Optdisc_t* dp)
{
	Dssdisc_t*	disc = ((Dssoptdisc_t*)dp)->disc;
	Dssstate_t*	state = dssstate(disc);
	Dsslib_t*	lib;
	Dssformat_t*	format;
	Cxmap_t*	mp;
	Cxtype_t*	tp;
	Cxvariable_t*	vp;
	long		pos;
	int		head;

	switch (*s)
	{
	case 'd':
		if (*(s + 1) == 'e' && *(s + 2) == 't')
		{
			/* details */
			tp = (Cxtype_t*)((Dssoptdisc_t*)dp)->header;
			if (tp->format.description && optesc(sp, tp->format.description, 0))
				return -1;
		}
		else
		{
			/* description */
			sfprintf(sp, "%s", state->cx->header ? state->cx->header->description : "unknown-description");
		}
		return 0;
	case 'i':
		/* ident */
		if (state->cx->header)
			sfprintf(sp, "%s - %s", state->cx->header->name, state->cx->header->description);
		else
			sfprintf(sp, "unknown - description");
		return 0;
	case 'm':
		if (*(s + 1) == 'a')
		{
			/* match */
			tp = (Cxtype_t*)((Dssoptdisc_t*)dp)->header;
			if (tp->match && optesc(sp, tp->match->description, 0))
				return -1;
		}
		else
		{
			/* methods */
			for (lib = dsslib(NiL, DSS_VERBOSE, disc); lib; lib = (Dsslib_t*)dtnext(state->cx->libraries, lib))
				if (lib->meth && optout(sp, lib->meth->name, NiL, NiL, lib->meth->description, NiL))
					return -1;
		}
		return 0;
	case 'n':
		/* name */
		sfprintf(sp, "%s", state->cx->header ? state->cx->header->name : "unknown-name");
		return 0;
	case 't':
		/* type */
		sfputc(sp, '{');
		if (opttype(sp, (Cxtype_t*)((Dssoptdisc_t*)dp)->header, 1))
			return -1;
		sfputc(sp, '}');
		return 0;
	}
	if (!state->meth)
	{
		sfprintf(sp, "[+NOTE::?Specify \b--method\b=\amethod\a for a list of supported \b%s\b.]", s);
		return 0;
	}
	pos = sfstrtell(sp);
	switch (*s)
	{
	case 'f':
		if (*(s + 1) != 'i')
		{
			/* formats */
			if (!dtsize(state->meth->formats))
				sfprintf(sp, "[+NOTE::?The \b%s\b method defines no formats.]", state->meth->name);
			else
				for (format = (Dssformat_t*)dtfirst(state->meth->formats); format; format = (Dssformat_t*)dtnext(state->meth->formats, format))
					if (optout(sp, format->name, NiL, NiL, format->description, NiL))
						return -1;
			break;
		}
		/*FALLTHROUGH*/
	case 'v':
		/* fields|variables */
		if (state->meth->description && optout(sp, state->meth->name, NiL, NiL, state->meth->description, NiL))
			return -1;
		head = 0;
		for (tp = (Cxtype_t*)dtfirst(state->cx->types); tp; tp = (Cxtype_t*)dtnext(state->cx->types, tp))
			if ((tp->base || tp->match) && (tp->header.flags & CX_REFERENCED))
			{
				if (!head)
				{
					if (optout(sp, "-----", NiL, NiL, "data types -----", NiL))
						return -1;
					head = 1;
				}
				if (opttype(sp, tp, 0))
					return -1;
			}
		head = 0;
		for (mp = (Cxmap_t*)dtfirst(state->cx->maps); mp; mp = (Cxmap_t*)dtnext(state->cx->maps, mp))
			if (mp->header.flags & CX_REFERENCED)
			{
				if (!head)
				{
					if (optout(sp, "-----", NiL, NiL, "data maps -----", NiL))
						return -1;
					head = 1;
				}
				if (optout(sp, mp->name, NiL, NiL, mp->description, NiL))
					return -1;
				if (optmap(sp, mp))
					return -1;
			}
		head = 0;
		for (vp = (Cxvariable_t*)dtfirst(state->meth->cx->fields); vp; vp = (Cxvariable_t*)dtnext(state->meth->cx->fields, vp))
		{
			if (!head)
			{
				if (optout(sp, "-----", NiL, NiL, "data fields -----", NiL))
					return -1;
				head = 1;
			}
			if (optout(sp, vp->name, vp->type->name, vp->format.map ? vp->format.map->name : (char*)0, vp->description, NiL))
				return -1;
		}
		break;
	}
	if (sfstrtell(sp) == pos)
		sfprintf(sp, "[+NOTE::?Specify a method schema to list the %s.]", s);
	return 0;
}

/*
 * generate lib info usage
 */

int
dssoptlib(Sfio_t* sp, Dsslib_t* lib, Dssdisc_t* disc)
{
	register int	i;

	if (lib->libraries)
		for (i = 0; lib->libraries[i]; i++)
			if (dssload(lib->libraries[i], disc))
				return -1;
	if (dssadd(lib, disc))
		return -1;
	sfprintf(sp, "[+LIBRARY?%s - %s]\n", lib->name, lib->description ? lib->description : "support library");
	if (lib->path)
		sfprintf(sp, "[+PATH?%s]\n", lib->path);
	if (lib->meth)
	{
		sfprintf(sp, "[+METHOD]{\n");
		if (optout(sp, lib->meth->name, NiL, NiL, lib->meth->description, NiL))
			return -1;
		sfprintf(sp, "\t}\n");
	}
	if (lib->types)
	{
		sfprintf(sp, "[+TYPES]{\n");
		for (i = 0; lib->types[i].name; i++)
			if (optout(sp, lib->types[i].name, lib->types[i].base ? lib->types[i].base->name : (const char*)0, NiL, lib->types[i].description, lib->types[i].member ? "This structure type has the following members:" : (char*)0))
				return -1;
			else if (lib->types[i].member && optmem(sp, &lib->types[i]))
				return -1;
		sfprintf(sp, "}\n");
	}
	if (lib->maps)
	{
		sfprintf(sp, "[+MAPS]{\n");
		for (i = 0; lib->maps[i]; i++)
			if (optout(sp, lib->maps[i]->name, NiL, NiL, lib->maps[i]->description, NiL))
				return -1;
			else if (optmap(sp, lib->maps[i]))
				return -1;
		sfprintf(sp, "}\n");
	}
	if (lib->callouts)
	{
		sfprintf(sp, "[+CALLOUTS]{\n");
		for (i = 0; lib->callouts[i].callout; i++)
			if (optout(sp, cxopname(lib->callouts[i].op.code, lib->callouts[i].op.type1, lib->callouts[i].op.type2), NiL, NiL, lib->callouts[i].description, NiL))
				return -1;
		sfprintf(sp, "}\n");
	}
	if (lib->recodes)
	{
		sfprintf(sp, "[+RECODES]{\n");
		for (i = 0; lib->recodes[i].recode; i++)
			if (optout(sp, cxopname(lib->recodes[i].op.code, lib->recodes[i].op.type1, lib->recodes[i].op.type2), NiL, NiL, lib->recodes[i].description, NiL))
				return -1;
		sfprintf(sp, "}\n");
	}
	if (lib->queries)
	{
		sfprintf(sp, "[+QUERIES]{\n");
		for (i = 0; lib->queries[i].name; i++)
			if (optout(sp, lib->queries[i].name, NiL, NiL, lib->queries[i].description, lib->queries[i].method ? sfprints("Limited to methods matching \"%s\".", lib->queries[i].method) : (char*)0))
				return -1;
		sfprintf(sp, "}\n");
	}
	if (lib->constraints)
	{
		sfprintf(sp, "[+CONSTRAINTS]{\n");
		for (i = 0; lib->constraints[i].name; i++)
			if (optout(sp, lib->constraints[i].name, NiL, NiL, lib->constraints[i].description, NiL))
				return -1;
		sfprintf(sp, "}\n");
	}
	if (lib->edits)
	{
		sfprintf(sp, "[+EDITS]{\n");
		for (i = 0; lib->edits[i].name; i++)
			if (optout(sp, lib->edits[i].name, NiL, NiL, lib->edits[i].description, NiL))
				return -1;
		sfprintf(sp, "}\n");
	}
	return 0;
}
