/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1987-2004 AT&T Corp.                *
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
 * pax rpm format
 */

#include "format.h"

#define RPM_MAGIC	0xedabeedb
#define RPM_CIGAM	0xdbeeabed

#define RPM_HEAD_MAGIC	0x8eade801

typedef struct
{
	unsigned int_4	magic;
	unsigned char	major;
	unsigned char	minor;
	int_2		type;
} Rpm_magic_t;

typedef struct
{
	int_2		archnum;
	char		name[66];
	int_2		osnum;
	int_2		sigtype;
	char		pad[16];
} Rpm_lead_t;

typedef struct
{
	int_2		archnum;
	char		name[66];
	unsigned int_4	specoff;
	unsigned int_4	speclen;
	unsigned int_4	archoff;
} Rpm_lead_old_t;

typedef struct
{
	unsigned int_4	entries;
	unsigned int_4	datalen;
} Rpm_head_t;

typedef struct
{
	unsigned int_4	tag;
	unsigned int_4	type;
	unsigned int_4	offset;
	unsigned int_4	size;
} Rpm_entry_t;

static int
rpm_getprologue(Pax_t* pax, Format_t* fp, register Archive_t* ap, File_t* f, unsigned char* buf, size_t size)
{
	Rpm_magic_t	magic;
	Rpm_magic_t	verify;
	Rpm_lead_t	lead;
	Rpm_lead_old_t	lead_old;
	Rpm_head_t	head;
	char*		s;
	int		i;
	int		swap;
	long		num;

	if (size < sizeof(magic))
		return 0;
	memcpy(&magic, buf, sizeof(magic));
	verify.magic = RPM_MAGIC;
	if ((swap = swapop(&verify.magic, &magic.magic, sizeof(magic.magic))) < 0)
		return 0;
	if (magic.major == 1)
	{
		if (size < (sizeof(magic) + sizeof(lead_old)))
			return 0;
		paxread(pax, ap, NiL, (off_t)sizeof(magic), (off_t)sizeof(magic), 0);
		if (paxread(pax, ap, &lead_old, (off_t)sizeof(lead_old), (off_t)sizeof(lead_old), 0) <= 0)
			return 0;
		if (swap)
			swapmem(swap, &lead_old, &lead_old, sizeof(lead_old));
		if (paxseek(pax, ap, (off_t)lead_old.archoff, SEEK_SET, 0) != (off_t)lead_old.archoff)
		{
			error(2, "%s: %s embedded archive seek error", ap->name, fp->name);
			return -1;
		}
	}
	else if (magic.major)
	{
		if (size < (sizeof(magic) + sizeof(lead)))
			return 0;
		paxread(pax, ap, NiL, (off_t)sizeof(magic), (off_t)sizeof(magic), 0);
		if (paxread(pax, ap, &lead, (off_t)sizeof(lead), (off_t)sizeof(lead), 0) <= 0)
			return 0;
		memcpy(state.volume, lead.name, sizeof(state.volume) - 1);
		if (s = strrchr(ap->name, '/'))
			s++;
		else
			s = ap->name;
		if (!memcmp(s, state.volume, strlen(state.volume)))
			state.volume[0] = 0;
		if (swap & 1)
			swapmem(swap & 1, &lead, &lead, sizeof(lead));
		switch (lead.sigtype)
		{
		case 0:
			num = 0;
			break;
		case 1:
			num = 256;
			break;
		case 5:
			if (paxread(pax, ap, &verify, (off_t)sizeof(verify), (off_t)sizeof(verify), 0) <= 0)
			{
				error(2, "%s: %s format header magic expected at offset %ld", ap->name, fp->name, ap->io->offset + ap->io->count);
				return -1;
			}
			if (swap)
				swapmem(swap, &verify, &verify, sizeof(verify));
			if (verify.magic != RPM_HEAD_MAGIC)
			{
				error(2, "%s: invalid %s format signature header magic", ap->name, fp->name);
				return -1;
			}
			if (paxread(pax, ap, &head, (off_t)sizeof(head), (off_t)sizeof(head), 0) <= 0)
			{
				error(2, "%s: %s format signature header expected", ap->name, fp->name);
				return -1;
			}
			if (swap)
				swapmem(swap, &head, &head, sizeof(head));
			num = head.entries * sizeof(Rpm_entry_t) + head.datalen;
			num += (8 - (num % 8)) % 8;
			break;
		default:
			error(2, "%s: %s format version %d.%d signature type %d not supported", ap->name, fp->name, magic.major, magic.minor, lead.sigtype);
			return -1;
		}
		if (num && paxread(pax, ap, NiL, (off_t)num, (off_t)num, 0) <= 0)
		{
			error(2, "%s: %s format header %ld byte data block expected", ap->name, fp->name, num);
			return -1;
		}
		if (magic.major >= 3)
		{
			if (paxread(pax, ap, &verify, (off_t)sizeof(verify), (off_t)sizeof(verify), 0) <= 0)
			{
				error(2, "%s: %s format header magic expected", ap->name, fp->name);
				return -1;
			}
			if (swap)
				swapmem(swap, &verify, &verify, sizeof(verify));
			if (verify.magic != RPM_HEAD_MAGIC)
			{
				error(2, "%s: invalid %s format header magic", ap->name, fp->name);
				return -1;
			}
		}
		if (paxread(pax, ap, &head, (off_t)sizeof(head), (off_t)sizeof(head), 0) <= 0)
		{
			error(2, "%s: %s format header expected", ap->name, fp->name);
			return -1;
		}
		if (swap)
			swapmem(swap, &head, &head, sizeof(head));
		num = head.entries * sizeof(Rpm_entry_t) + head.datalen;
		if (num && paxread(pax, ap, NiL, (off_t)num, (off_t)num, 0) <= 0)
		{
			error(2, "%s: %s format header %ld byte entry+data block expected", ap->name, fp->name, num);
			return -1;
		}
	}
	else
	{
		error(2, "%s: %s format version %d.%d not supported", ap->name, fp->name, magic.major, magic.minor);
		return -1;
	}
	ap->entry = 0;
	ap->swap = 0;
	ap->swapio = 0;
	ap->volume--;
	i = state.volume[0];
	if (getprologue(ap) <= 0)
	{
		error(2, "%s: %s format embedded archive expected", ap->name, fp->name);
		return -1;
	}
	state.volume[0] = i;
	ap->package = strdup(sfprints("%s %d.%d", fp->name, magic.major, magic.minor));
	return 1;
}

Format_t	pax_rpm_format =
{
	"rpm",
	0,
	"Redhat rpm package encapsulated cpio",
	0,
	ARCHIVE|IN,
	DEFBUFFER,
	DEFBLOCKS,
	0,
	PAXNEXT(pax_rpm_next),
	0,
	0,
	rpm_getprologue,
};

PAXLIB(&pax_rpm_format)
