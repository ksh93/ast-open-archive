/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*           Copyright (c) 1997-2006 AT&T Knowledge Ventures            *
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
 * cdb format conversion support
 */

#include "cdblib.h"

static int
empty(Cdbmap_t* map, register Cdbconvert_t* cvt)
{
	return cdbempty(map->odb, cvt->output.data, cvt->output.format, 1);
}

static int
copy(Cdbmap_t* map, register Cdbconvert_t* cvt)
{
	register char*	s;
	register char*	e;

	*cvt->output.data = *cvt->input.data;
	if ((cvt->input.format->flags ^ cvt->output.format->flags) & (CDB_NEGATIVE|CDB_SPACE|CDB_ZERO))
		switch (cvt->output.format->ultype)
		{
		case CDB_FLOATING:
			cvt->output.data->flags = 0;
			SETFLOATING(cvt->output.data, cvt->output.format, 0);
			break;
		case CDB_INTEGER:
			cvt->output.data->flags = 0;
			SETINTEGER(cvt->output.data, cvt->output.format, 0);
			break;
		case CDB_UNSIGNED|CDB_INTEGER:
			cvt->output.data->flags = 0;
			SETUINTEGER(cvt->output.data, cvt->output.format, 0);
			break;
		case CDB_LONG|CDB_INTEGER:
			cvt->output.data->flags = 0;
			SETLINTEGER(cvt->output.data, cvt->output.format, 0);
			break;
		case CDB_UNSIGNED|CDB_LONG|CDB_INTEGER:
			cvt->output.data->flags = 0;
			SETWINTEGER(cvt->output.data, cvt->output.format, 0);
			break;
		case CDB_STRING:
			if (cvt->output.format->flags & CDB_SPACE)
			{
				for (e = (s = cvt->output.data->string.base) + cvt->output.data->string.length; s < e && isspace(*s); s++);
				if (s >= e)
				{
					cvt->output.data->flags &= ~CDB_STRING;
					cvt->output.data->flags |= CDB_TERMINATED;
					cvt->output.data->string.base = "";
					cvt->output.data->string.length = 0;
				}
			}
			break;
		}
	return 0;
}

static int
string2integer(Cdbmap_t* map, register Cdbconvert_t* cvt)
{
	register char*	s;
	register int	n;
	register int	b;
	char*		e;

	if (!(s = cvt->input.data->string.base) || !(n = cvt->input.data->string.length))
		return cdbempty(map->odb, cvt->output.data, cvt->output.format, 1);
	b = cvt->output.format->base;
	if (b > 10 ? isalnum(s[n]) : isdigit(s[n]))
	{
		if (!(s = vmoldof(map->odb->record->vm, 0, char, n, 1)))
			return cdbnospace(map->odb);
		cvt->input.data->string.base = (char*)memcpy(s, cvt->input.data->string.base, n);
		s[n] = 0;
	}
	if (cvt->output.format->flags & CDB_UNSIGNED)
		cvt->output.data->number.uinteger = strtoul(s, &e, b);
	else
		cvt->output.data->number.integer = strtol(s, &e, b);
	cvt->output.data->flags = 0;
	SETINTEGER(cvt->output.data, cvt->output.format, (e - s) != n);
	return 0;
}

static int
integer2string(Cdbmap_t* map, register Cdbconvert_t* cvt)
{
	register char*	s;
	register int	n;
	char*		t;

	if (!(cvt->input.data->flags & CDB_INTEGER))
		return cdbempty(map->odb, cvt->output.data, cvt->output.format, 1);
	n = cvt->output.data->string.length = (cvt->input.format->flags & CDB_UNSIGNED) ? sfprintf(map->odb->tmp, "%..*lu", cvt->input.format->base, cvt->input.data->number.uinteger) : sfprintf(map->odb->tmp, "%..*ld", cvt->input.format->base, cvt->input.data->number.integer);
	if (!(s = vmoldof(map->odb->record->vm, 0, char, n, 1)) || !(t = sfstruse(map->odb->tmp)))
		return cdbnospace(map->odb);
	cvt->output.data->string.base = (char*)memcpy(s, t, n + 1);
	cvt->output.data->flags = CDB_STRING;
	return 0;
}

static int
string2floating(Cdbmap_t* map, register Cdbconvert_t* cvt)
{
	register char*	s;
	register int	n;
	register int	c;
	char*		e;

	if (!(s = cvt->input.data->string.base) || !(n = cvt->input.data->string.length))
		return cdbempty(map->odb, cvt->output.data, cvt->output.format, 1);
	c = s[n];
	if (c == '.' || isdigit(c) || c == 'e' || c == 'E')
	{
		if (!(s = vmoldof(map->odb->record->vm, 0, char, n, 1)))
			return cdbnospace(map->odb);
		cvt->input.data->string.base = (char*)memcpy(s, cvt->input.data->string.base, n);
		s[n] = 0;
	}
	cvt->output.data->number.floating = strtod(cvt->input.data->string.base, &e);
	cvt->output.data->flags = 0;
	SETFLOATING(cvt->output.data, cvt->output.format, (e - s) != n);
	return 0;
}

static int
floating2string(Cdbmap_t* map, register Cdbconvert_t* cvt)
{
	register char*	s;
	register int	n;
	char*		t;

	if (!(cvt->input.data->flags & CDB_FLOATING))
		return cdbempty(map->odb, cvt->output.data, cvt->output.format, 1);
	n = cvt->output.data->string.length = sfprintf(map->odb->tmp, "%g", cvt->input.data->number.floating);
	if (!(s = vmoldof(map->odb->record->vm, 0, char, n, 1)) || !(t = sfstruse(map->odb->tmp)))
		return cdbnospace(map->odb);
	cvt->output.data->string.base = (char*)memcpy(s, t, n + 1);
	cvt->output.data->flags = CDB_STRING;
	return 0;
}

static int
integer2floating(Cdbmap_t* map, register Cdbconvert_t* cvt)
{
	if (!(cvt->input.data->flags & CDB_INTEGER))
		return cdbempty(map->odb, cvt->output.data, cvt->output.format, 1);
	cvt->output.data->number.floating = cvt->input.data->number.integer;
	cvt->output.data->flags = CDB_FLOATING;
	return 0;
}

static int
floating2integer(Cdbmap_t* map, register Cdbconvert_t* cvt)
{
	if (!(cvt->input.data->flags & CDB_FLOATING))
		return cdbempty(map->odb, cvt->output.data, cvt->output.format, 1);
	cvt->output.data->number.integer = cvt->input.data->number.floating;
	cvt->output.data->flags = CDB_INTEGER;
	return 0;
}

#define CONVERTF(ifp,ofp)	convertf[(((ifp)->type>>1)<<2)|((ofp)->type>>1)]

static const Cdbconvert_f	convertf[] =
{
	copy,
	string2integer,
	string2floating,
	0,
	integer2string,
	copy,
	integer2floating,
	0,
	floating2string,
	floating2integer,
	copy,
};

/*
 * move count records
 * if map!=0 then only selected fields are moved
 * -(count+1) returned on error
 */

ssize_t
cdbmove(register Cdb_t* idb, register Cdb_t* odb, Cdbmap_t* map, ssize_t count)
{
	register Cdbconvert_t*	cvt;
	register Cdbdata_t*	idp;
	register Cdbdata_t*	odp;
	Cdbschema_t*		isp;
	Cdbschema_t*		osp;
	Cdbformat_t*		ifp;
	Cdbformat_t*		ofp;
	Cdbrecord_t*		irp;
	Cdbrecord_t*		orp;
	int			i;
	int			n;
	int			closemap;

	/*
	 * generate/initialize the map with
	 * compatibility checks along the way
	 */

	if ((closemap = !map) && !(map = cdbmapopen(idb, NiL, NiL)))
		return -1;
	if (map->meth.eventf)
	{
		map->idb = idb;
		map->odb = odb;
		orp = odb->record;
		if ((*map->meth.eventf)(map, CDB_OPEN) < 0)
			return -1;
	}
	else
	{
		if (idb->partitions != odb->partitions)
		{
			if (idb->disc->errorf)
				(*idb->disc->errorf)(idb, idb->disc, 2, "%s with %d partitions does not match %s with %d", idb->path, idb->partitions, odb->path, odb->partitions);
			return -1;
		}
		if (map->idb != idb || map->odb != odb)
		{
			map->idb = idb;
			irp = idb->record;
			idp = irp->data;
			map->odb = odb;
			orp = odb->record;
			odp = orp->data;
			for (i = 0; i < odb->partitions; i++)
			{
				isp = idb->table[i];
				ifp = isp->format;
				osp = odb->table[i];
				ofp = osp->format;
				if (isp->fields > map->schema[i].ifields)
				{
					if (idb->disc->errorf)
						(*idb->disc->errorf)(idb, idb->disc, 2, "%s %s record with %d fields exceeds map with %d", idb->path, isp->name, isp->fields, map->schema[i].ifields);
					return -1;
				}
				if (osp->fields > map->schema[i].ofields)
				{
					if (idb->disc->errorf)
						(*idb->disc->errorf)(idb, idb->disc, 2, "%s %s record with %d fields exceeds map with %d", odb->path, osp->name, osp->fields, map->schema[i].ofields);
					return -1;
				}
				for (cvt = map->schema[i].convert; cvt->output.index >= 0; cvt++)
					if (cvt->output.index < osp->fields)
					{
						cvt->output.data = odp + cvt->output.index;
						cvt->output.format = ofp + cvt->output.index;
						cvt->input.data = idp + cvt->input.index;
						cvt->input.format = ifp + cvt->input.index;
						cvt->convertf = (cvt->input.format->type == CDB_IGNORE || cvt->input.index == CDB_MAP_NIL) ? empty : CONVERTF(cvt->input.format, cvt->output.format);
					}
			}
		}
		idb->flags |= CDB_RAW;
		odb->flags |= CDB_RAW;
	}

	/*
	 * now read and convert each record
	 */

	while ((count < 0 || count-- > 0) && (irp = cdbread(idb, NiL)))
	{
		vmclear(orp->vm);
		orp->schema = odb->table[irp->schema->index < odb->partitions ? irp->schema->index : 0];
		if (map->meth.mapf)
		{
			if ((n = (*map->meth.mapf)(map, irp, orp)) <= 0)
			{
				if (n < 0)
					return -(odb->count + 1);
				continue;
			}
		}
		else for (cvt = map->schema[irp->schema->index].convert; cvt->convertf; cvt++)
			if ((*cvt->convertf)(map, cvt) < 0)
				return -(odb->count + 1);
		if (cdbwrite(odb, NiL, orp) < 0)
			return -(odb->count + 1);
	}
	if (map->meth.eventf && (*map->meth.eventf)(map, CDB_CLOSE) < 0)
		return -1;
	if (closemap)
		cdbmapclose(map);
	return odb->count;
}
