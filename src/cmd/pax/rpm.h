/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1987-2000 AT&T Corp.              *
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
 * rpm interface definitions
 */

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
