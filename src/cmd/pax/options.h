/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1987-2004 AT&T Corp.                  *
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

#define OPT_action		1
#define OPT_append		2
#define OPT_atime		3
#define OPT_base		4
#define OPT_blocksize		5
#define OPT_blok		6
#define OPT_charset		7
#define OPT_checksum		8
#define OPT_chksum		9
#define OPT_chmod		10
#define OPT_clobber		11
#define OPT_comment		12
#define OPT_complete		13
#define OPT_crossdevice		14
#define OPT_ctime		15
#define OPT_debug		16
#define OPT_delete		17
#define OPT_delta_base_checksum	18
#define OPT_delta_base_size	19
#define OPT_delta_checksum	20
#define OPT_delta_compress	21
#define OPT_delta_index		22
#define OPT_delta_method	23
#define OPT_delta_op		24
#define OPT_delta_ordered	25
#define OPT_delta_version	26
#define OPT_descend		27
#define OPT_device		28
#define OPT_devmajor		29
#define OPT_devminor		30
#define OPT_dir			31
#define OPT_dots		32
#define OPT_edit		33
#define OPT_entry		34
#define OPT_eom			35
#define OPT_exact		36
#define OPT_extended_name	37
#define OPT_file		38
#define OPT_filter		39
#define OPT_forceconvert	40
#define OPT_format		41
#define OPT_from		42
#define OPT_gid			43
#define OPT_global_name		44
#define OPT_gname		45
#define OPT_header		46
#define OPT_ignore		47
#define OPT_ino			48
#define OPT_install		49
#define OPT_intermediate	50
#define OPT_invalid		51
#define OPT_invert		52
#define OPT_keepgoing		53
#define OPT_label		54
#define OPT_link		55
#define OPT_linkdata		56
#define OPT_linkop		57
#define OPT_linkpath		58
#define OPT_listformat		59
#define OPT_listmacro		60
#define OPT_local		61
#define OPT_logical		62
#define OPT_magic		63
#define OPT_mark		64
#define OPT_maxout		65
#define OPT_metaphysical	66
#define OPT_meter		67
#define OPT_mkdir		68
#define OPT_mode		69
#define OPT_mtime		70
#define OPT_name		71
#define OPT_nlink		72
#define OPT_options		73
#define OPT_ordered		74
#define OPT_owner		75
#define OPT_passphrase		76
#define OPT_path		77
#define OPT_physical		78
#define OPT_pid			79
#define OPT_preserve		80
#define OPT_read		81
#define OPT_record_charset	82
#define OPT_record_delimiter	83
#define OPT_record_format	84
#define OPT_record_header	85
#define OPT_record_line		86
#define OPT_record_match	87
#define OPT_record_pad		88
#define OPT_record_size		89
#define OPT_record_trailer	90
#define OPT_release		91
#define OPT_reset_atime		92
#define OPT_sequence		93
#define OPT_size		94
#define OPT_strict		95
#define OPT_summary		96
#define OPT_symlink		97
#define OPT_sync		98
#define OPT_tape		99
#define OPT_test		100
#define OPT_testdate		101
#define OPT_times		102
#define OPT_tmp			103
#define OPT_to			104
#define OPT_typeflag		105
#define OPT_uid			106
#define OPT_uname		107
#define OPT_unblocked		108
#define OPT_uncompressed	109
#define OPT_update		110
#define OPT_verbose		111
#define OPT_verify		112
#define OPT_version		113
#define OPT_warn		114
#define OPT_write		115
#define OPT_yes			116

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
