/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1987-2003 AT&T Corp.                *
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
 * AT&T Labs Research
 *
 * pax option definitions
 */

#define OPT_DISABLE		(1<<0)
#define OPT_GLOBAL		(1<<1)
#define OPT_HEADER		(1<<2)
#define OPT_IGNORE		(1<<3)
#define OPT_INVERT		(1<<4)
#define OPT_NUMBER		(1<<5)
#define OPT_OPTIONAL		(1<<6)
#define OPT_READONLY		(1<<7)
#define OPT_SET			(1<<8)
#define OPT_VENDOR		(1<<9)

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
#define OPT_delete		15
#define OPT_delta		16
#define OPT_descend		17
#define OPT_device		18
#define OPT_devmajor		19
#define OPT_devminor		20
#define OPT_dir			21
#define OPT_dots		22
#define OPT_edit		23
#define OPT_entry		24
#define OPT_eom			25
#define OPT_exact		26
#define OPT_extended_name	27
#define OPT_file		28
#define OPT_filter		29
#define OPT_format		30
#define OPT_from		31
#define OPT_gid			32
#define OPT_global_name		33
#define OPT_gname		34
#define OPT_header		35
#define OPT_ignore		36
#define OPT_ino			37
#define OPT_install		38
#define OPT_intermediate	39
#define OPT_invalid		40
#define OPT_invert		41
#define OPT_keepgoing		42
#define OPT_label		43
#define OPT_link		44
#define OPT_linkdata		45
#define OPT_linkop		46
#define OPT_linkpath		47
#define OPT_listformat		48
#define OPT_listmacro		49
#define OPT_local		50
#define OPT_logical		51
#define OPT_magic		52
#define OPT_mark		53
#define OPT_maxout		54
#define OPT_metaphysical	55
#define OPT_meter		56
#define OPT_mkdir		57
#define OPT_mode		58
#define OPT_mtime		59
#define OPT_name		60
#define OPT_nlink		61
#define OPT_options		62
#define OPT_ordered		63
#define OPT_owner		64
#define OPT_path		65
#define OPT_physical		66
#define OPT_pid			67
#define OPT_preserve		68
#define OPT_read		69
#define OPT_record_charset	70
#define OPT_record_delimiter	71
#define OPT_record_format	72
#define OPT_record_header	73
#define OPT_record_line		74
#define OPT_record_match	75
#define OPT_record_pad		76
#define OPT_record_size		77
#define OPT_record_trailer	78
#define OPT_release		79
#define OPT_reset_atime		80
#define OPT_sequence		81
#define OPT_size		82
#define OPT_strict		83
#define OPT_summary		84
#define OPT_symlink		85
#define OPT_tape		86
#define OPT_test		87
#define OPT_testdate		88
#define OPT_times		89
#define OPT_tmp			90
#define OPT_to			91
#define OPT_typeflag		92
#define OPT_uid			93
#define OPT_uname		94
#define OPT_unblocked		95
#define OPT_update		96
#define OPT_verbose		97
#define OPT_verify		98
#define OPT_version		99
#define OPT_write		100
#define OPT_yes			101

typedef struct Option_s
{
	char*		name;
	short		flag;
	short		index;
	char*		description;
	char*		argument;
	char*		details;
	short		flags;
	short		level;
	size_t		entry;
	char*		macro;
	Value_t		perm;
	Value_t		temp;
} Option_t;

extern Option_t		options[];
