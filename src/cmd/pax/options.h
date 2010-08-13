/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1987-2010 AT&T Intellectual Property          *
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
 * Glenn Fowler
 * AT&T Research
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
#define OPT_delta_update	26
#define OPT_delta_version	27
#define OPT_descend		28
#define OPT_device		29
#define OPT_devmajor		30
#define OPT_devminor		31
#define OPT_dir			32
#define OPT_dots		33
#define OPT_edit		34
#define OPT_entry		35
#define OPT_eom			36
#define OPT_exact		37
#define OPT_extended_name	38
#define OPT_file		39
#define OPT_filter		40
#define OPT_forceconvert	41
#define OPT_format		42
#define OPT_from		43
#define OPT_gid			44
#define OPT_global_name		45
#define OPT_gname		46
#define OPT_hdrcharset		47
#define OPT_header		48
#define OPT_ignore		49
#define OPT_ino			50
#define OPT_install		51
#define OPT_intermediate	52
#define OPT_invalid		53
#define OPT_invert		54
#define OPT_keepgoing		55
#define OPT_label		56
#define OPT_link		57
#define OPT_linkdata		58
#define OPT_linkop		59
#define OPT_linkpath		60
#define OPT_listformat		61
#define OPT_listmacro		62
#define OPT_local		63
#define OPT_logical		64
#define OPT_magic		65
#define OPT_mark		66
#define OPT_maxout		67
#define OPT_metaphysical	68
#define OPT_meter		69
#define OPT_mkdir		70
#define OPT_mode		71
#define OPT_mtime		72
#define OPT_name		73
#define OPT_newer		74
#define OPT_nlink		75
#define OPT_options		76
#define OPT_ordered		77
#define OPT_owner		78
#define OPT_passphrase		79
#define OPT_path		80
#define OPT_physical		81
#define OPT_pid			82
#define OPT_preserve		83
#define OPT_read		84
#define OPT_record_charset	85
#define OPT_record_delimiter	86
#define OPT_record_format	87
#define OPT_record_header	88
#define OPT_record_line		89
#define OPT_record_match	90
#define OPT_record_pad		91
#define OPT_record_size		92
#define OPT_record_trailer	93
#define OPT_release		94
#define OPT_reset_atime		95
#define OPT_sequence		96
#define OPT_size		97
#define OPT_strict		98
#define OPT_summary		99
#define OPT_symlink		100
#define OPT_sync		101
#define OPT_tape		102
#define OPT_test		103
#define OPT_testdate		104
#define OPT_times		105
#define OPT_tmp			106
#define OPT_to			107
#define OPT_typeflag		108
#define OPT_uid			109
#define OPT_uname		110
#define OPT_unblocked		111
#define OPT_uncompressed	112
#define OPT_update		113
#define OPT_verbose		114
#define OPT_verify		115
#define OPT_version		116
#define OPT_warn		117
#define OPT_write		118
#define OPT_yes			119

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
