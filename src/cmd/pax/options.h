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
#define OPT_clobber		10
#define OPT_comment		11
#define OPT_complete		12
#define OPT_crossdevice		13
#define OPT_ctime		14
#define OPT_debug		15
#define OPT_delete		16
#define OPT_delta_base_checksum	17
#define OPT_delta_base_size	18
#define OPT_delta_checksum	19
#define OPT_delta_compress	20
#define OPT_delta_index		21
#define OPT_delta_method	22
#define OPT_delta_op		23
#define OPT_delta_ordered	24
#define OPT_delta_version	25
#define OPT_descend		26
#define OPT_device		27
#define OPT_devmajor		28
#define OPT_devminor		29
#define OPT_dir			30
#define OPT_dots		31
#define OPT_edit		32
#define OPT_entry		33
#define OPT_eom			34
#define OPT_exact		35
#define OPT_extended_name	36
#define OPT_file		37
#define OPT_filter		38
#define OPT_forceconvert	39
#define OPT_format		40
#define OPT_from		41
#define OPT_gid			42
#define OPT_global_name		43
#define OPT_gname		44
#define OPT_header		45
#define OPT_ignore		46
#define OPT_ino			47
#define OPT_install		48
#define OPT_intermediate	49
#define OPT_invalid		50
#define OPT_invert		51
#define OPT_keepgoing		52
#define OPT_label		53
#define OPT_link		54
#define OPT_linkdata		55
#define OPT_linkop		56
#define OPT_linkpath		57
#define OPT_listformat		58
#define OPT_listmacro		59
#define OPT_local		60
#define OPT_logical		61
#define OPT_magic		62
#define OPT_mark		63
#define OPT_maxout		64
#define OPT_metaphysical	65
#define OPT_meter		66
#define OPT_mkdir		67
#define OPT_mode		68
#define OPT_mtime		69
#define OPT_name		70
#define OPT_nlink		71
#define OPT_options		72
#define OPT_ordered		73
#define OPT_owner		74
#define OPT_passphrase		75
#define OPT_path		76
#define OPT_physical		77
#define OPT_pid			78
#define OPT_preserve		79
#define OPT_read		80
#define OPT_record_charset	81
#define OPT_record_delimiter	82
#define OPT_record_format	83
#define OPT_record_header	84
#define OPT_record_line		85
#define OPT_record_match	86
#define OPT_record_pad		87
#define OPT_record_size		88
#define OPT_record_trailer	89
#define OPT_release		90
#define OPT_reset_atime		91
#define OPT_sequence		92
#define OPT_size		93
#define OPT_strict		94
#define OPT_summary		95
#define OPT_symlink		96
#define OPT_sync		97
#define OPT_tape		98
#define OPT_test		99
#define OPT_testdate		100
#define OPT_times		101
#define OPT_tmp			102
#define OPT_to			103
#define OPT_typeflag		104
#define OPT_uid			105
#define OPT_uname		106
#define OPT_unblocked		107
#define OPT_uncompressed	108
#define OPT_update		109
#define OPT_verbose		110
#define OPT_verify		111
#define OPT_version		112
#define OPT_warn		113
#define OPT_write		114
#define OPT_yes			115

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
