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
 * Glenn Fowler
 * AT&T Labs Research
 *
 * pax option definitions
 */

#define OPT_DISABLE		(1<<0)
#define OPT_HEADER		(1<<1)
#define OPT_INVERT		(1<<2)
#define OPT_NUMBER		(1<<3)
#define OPT_OPTIONAL		(1<<4)
#define OPT_READONLY		(1<<5)
#define OPT_SET			(1<<6)

#define OPT_environ		(-1)

#define OPT_append		1
#define OPT_atime		2
#define OPT_base		3
#define OPT_blocksize		4
#define OPT_blok		5
#define OPT_charset		6
#define OPT_checksum		7
#define OPT_chksum		8
#define OPT_clobber		9
#define OPT_comment		10
#define OPT_complete		11
#define OPT_crossdevice		12
#define OPT_ctime		13
#define OPT_debug		14
#define OPT_delta		15
#define OPT_descend		16
#define OPT_device		17
#define OPT_devmajor		18
#define OPT_devminor		19
#define OPT_dots		20
#define OPT_edit		21
#define OPT_eom			22
#define OPT_exact		23
#define OPT_extended_ignore	24
#define OPT_extended_path	25
#define OPT_file		26
#define OPT_filter		27
#define OPT_format		28
#define OPT_from		29
#define OPT_gname		30
#define OPT_ignore		31
#define OPT_ino			32
#define OPT_install		33
#define OPT_intermediate	34
#define OPT_invalid		35
#define OPT_invert		36
#define OPT_keepgoing		37
#define OPT_label		38
#define OPT_label_insert	39
#define OPT_link		40
#define OPT_linkdata		41
#define OPT_linkop		42
#define OPT_linkpath		43
#define OPT_listformat		44
#define OPT_listmacro		45
#define OPT_local		46
#define OPT_logical		47
#define OPT_magic		48
#define OPT_mark		49
#define OPT_maxout		50
#define OPT_metaphysical	51
#define OPT_meter		52
#define OPT_mkdir		53
#define OPT_mode		54
#define OPT_mtime		55
#define OPT_name		56
#define OPT_nlink		57
#define OPT_options		58
#define OPT_ordered		59
#define OPT_owner		60
#define OPT_path		61
#define OPT_physical		62
#define OPT_preserve		63
#define OPT_read		64
#define OPT_record_charset	65
#define OPT_record_delimiter	66
#define OPT_record_format	67
#define OPT_record_header	68
#define OPT_record_line		69
#define OPT_record_match	70
#define OPT_record_pad		71
#define OPT_record_size		72
#define OPT_record_trailer	73
#define OPT_sequence		74
#define OPT_size		75
#define OPT_summary		76
#define OPT_symlink		77
#define OPT_tape		78
#define OPT_test		79
#define OPT_to			80
#define OPT_typeflag		81
#define OPT_uname		82
#define OPT_unblocked		83
#define OPT_update		84
#define OPT_verbose		85
#define OPT_verify		86
#define OPT_version		87
#define OPT_write		88
#define OPT_yes			89

typedef struct
{
	char*		string;
	long		number;
	int		size;
} Value_t;

typedef struct
{
	char*		name;
	short		flag;
	short		index;
	char*		description;
	char*		argument;
	char*		details;
	short		flags;
	short		entry;
	short		level;
	char*		macro;
	Value_t		perm;
	Value_t		temp;
} Option_t;

extern Option_t		options[];
