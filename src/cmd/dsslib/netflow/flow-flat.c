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
 * netflow dump type
 *
 * Glenn Fowler
 * AT&T Research
 */

#include "flowlib.h"

typedef struct State_s
{
	Netflow_t	record;
} State_t;

/*
 * identf
 */

static int
flatident(Dssfile_t* file, void* buf, size_t n, Dssdisc_t* disc)
{
	register char*	s = buf;
	register char*	e = s + n;
	register int	d = 0;

	while (s < e)
		switch (*s++)
		{
		case '\n':
			return d == 19;
		case '|':
			d++;
			break;
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
		case '.': case '-': case '+': case ':':
			break;
		default:
			return 0;
		}
	return 0;
}

/*
 * openf
 */

static int
flatopen(Dssfile_t* file, Dssdisc_t* disc)
{
	if ((file->flags & DSS_FILE_READ) && !(file->data = (void*)vmnewof(file->dss->vm, 0, State_t, 1, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	return 0;
}

/*
 * readf
 */

static int
flatread(Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	register Netflow_t*	rp = &((State_t*)file->data)->record;
	char*			a;
	char*			b;
	char*			s;

	if (!(a = s = sfgetr(file->io, '\n', 0)))
	{
		if (sfvalue(file->io))
		{
			if (disc->errorf)
				(*disc->errorf)(NiL, disc, 2, "%slast record incomplete", cxlocation(file->dss->cx, record));
			return -1;
		}
		return 0;
	}
	if (strtoip4(a, &b, &rp->src_addr, NiL) || *b++ != '|')
		goto bad;
	if (strtoip4(b, &a, &rp->dst_addr, NiL) || *a++ != '|')
		goto bad;
	if (strtoip4(a, &b, &rp->hop, NiL) || *b++ != '|')
		goto bad;
	rp->input = strtoul(b, &a, 10);
	if (*a++ != '|')
		goto bad;
	rp->output = strtoul(a, &b, 10);
	if (*b++ != '|')
		goto bad;
	rp->packets = strtoul(b, &a, 10);
	if (*a++ != '|')
		goto bad;
	rp->bytes = strtoul(a, &b, 10);
	if (*b++ != '|')
		goto bad;
	rp->first = strtoul(b, &a, 10);
	if (*a++ != '|')
		goto bad;
	rp->last = strtoul(a, &b, 10);
	if (*b++ != '|')
		goto bad;
	rp->src_port = strtoul(b, &a, 10);
	if (*a++ != '|')
		goto bad;
	rp->dst_port = strtoul(a, &b, 10);
	if (*b++ != '|')
		goto bad;
	rp->flags = strtoul(b, &a, 10);
	if (*a++ != '|')
		goto bad;
	rp->tcp_flags = strtoul(a, &b, 10);
	if (*b++ != '|')
		goto bad;
	rp->prot = strtoul(b, &a, 10);
	if (*a++ != '|')
		goto bad;
	rp->tos = strtoul(a, &b, 10);
	if (*b++ != '|')
		goto bad;
	rp->src_as = strtoul(b, &a, 10);
	if (*a++ != '|')
		goto bad;
	rp->dst_as = strtoul(a, &b, 10);
	if (*b++ != '|')
		goto bad;
	rp->src_mask = strtoul(b, &a, 10);
	if (*a++ != '|')
		goto bad;
	rp->dst_mask = strtoul(a, &b, 10);
	if (*b++ != '|')
		goto bad;
	rp->flow_sequence = strtoul(b, &a, 10);
	if (*a++ != '\n')
		goto bad;
	rp->start = (Nftime_t)rp->first * NS;
	rp->end = (Nftime_t)rp->last * NS;
	record->data = rp;
	record->size = sizeof(*rp);
	return 1;
 bad:
	if (disc->errorf)
	{
		if (a < b)
			a = b;
		(*disc->errorf)(NiL, disc, 2, "%s%-.*s<<<: invalid %s record field", cxlocation(file->dss->cx, record), a - s, s, file->format->name);
	}
	return -1;
}

/*
 * writef
 */

#define IPQ(a)	(a>>24)&0xff, (a>>16)&0xff, (a>>8)&0xff, (a)&0xff

static int
flatwrite(Dssfile_t* file, Dssrecord_t* record, Dssdisc_t* disc)
{
	register Netflow_t*	rp = (Netflow_t*)record->data;

	if (sfprintf(file->io, "%d.%d.%d.%d|%d.%d.%d.%d|%d.%d.%d.%d|%u|%u|%u|%u|%u|%u|%u|%u|%u|%u|%u|%u|%u|%u|%u|%u|%u\n"
		, IPQ(rp->src_addr)
		, IPQ(rp->dst_addr)
		, IPQ(rp->hop)
		, rp->input
		, rp->output
		, rp->packets
		, rp->bytes
		, (unsigned long)(rp->start / NS)
		, (unsigned long)(rp->end / NS)
		, rp->src_port
		, rp->dst_port
		, rp->flags
		, rp->tcp_flags
		, rp->prot
		, rp->tos
		, rp->src_as
		, rp->dst_as
		, rp->src_mask
		, rp->dst_mask
		, rp->flow_sequence
			) < 0)
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, 2, "%s: write error", file->format->name);
		return -1;
	}
	return 0;
}

/*
 * closef
 */

static int
flatclose(Dssfile_t* file, Dssdisc_t* disc)
{
	if (file->data)
		vmfree(file->dss->vm, file->data);
	return 0;
}

Dssformat_t netflow_flat_format =
{
	"flat",
	"Cisco netflow flat format. A |-separated, \\n-terminated record of these fields: src_addr, dst_addr, hop, input, output, packets, bytes, first, last, src_port, dst_port, flags, tcp_flags, prot, tos, src_as, dst_as, src_mask, dst_mask, flow_sequence.",
	{0},
	flatident,
	flatopen,
	flatread,
	flatwrite,
	0,
	flatclose,
	0,
	0,
	netflow_flat_next
};
