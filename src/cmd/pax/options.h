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
#define OPT_delta_base_checksum	16
#define OPT_delta_base_size	17
#define OPT_delta_checksum	18
#define OPT_delta_compress	19
#define OPT_delta_index		20
#define OPT_delta_method	21
#define OPT_delta_op		22
#define OPT_delta_ordered	23
#define OPT_delta_version	24
#define OPT_descend		25
#define OPT_device		26
#define OPT_devmajor		27
#define OPT_devminor		28
#define OPT_dir			29
#define OPT_dots		30
#define OPT_edit		31
#define OPT_entry		32
#define OPT_eom			33
#define OPT_exact		34
#define OPT_extended_name	35
#define OPT_file		36
#define OPT_filter		37
#define OPT_format		38
#define OPT_from		39
#define OPT_gid			40
#define OPT_global_name		41
#define OPT_gname		42
#define OPT_header		43
#define OPT_ignore		44
#define OPT_ino			45
#define OPT_install		46
#define OPT_intermediate	47
#define OPT_invalid		48
#define OPT_invert		49
#define OPT_keepgoing		50
#define OPT_label		51
#define OPT_link		52
#define OPT_linkdata		53
#define OPT_linkop		54
#define OPT_linkpath		55
#define OPT_listformat		56
#define OPT_listmacro		57
#define OPT_local		58
#define OPT_logical		59
#define OPT_magic		60
#define OPT_mark		61
#define OPT_maxout		62
#define OPT_metaphysical	63
#define OPT_meter		64
#define OPT_mkdir		65
#define OPT_mode		66
#define OPT_mtime		67
#define OPT_name		68
#define OPT_nlink		69
#define OPT_options		70
#define OPT_ordered		71
#define OPT_owner		72
#define OPT_path		73
#define OPT_physical		74
#define OPT_pid			75
#define OPT_preserve		76
#define OPT_read		77
#define OPT_record_charset	78
#define OPT_record_delimiter	79
#define OPT_record_format	80
#define OPT_record_header	81
#define OPT_record_line		82
#define OPT_record_match	83
#define OPT_record_pad		84
#define OPT_record_size		85
#define OPT_record_trailer	86
#define OPT_release		87
#define OPT_reset_atime		88
#define OPT_sequence		89
#define OPT_size		90
#define OPT_strict		91
#define OPT_summary		92
#define OPT_symlink		93
#define OPT_tape		94
#define OPT_test		95
#define OPT_testdate		96
#define OPT_times		97
#define OPT_tmp			98
#define OPT_to			99
#define OPT_typeflag		100
#define OPT_uid			101
#define OPT_uname		102
#define OPT_unblocked		103
#define OPT_uncompressed	104
#define OPT_update		105
#define OPT_verbose		106
#define OPT_verify		107
#define OPT_version		108
#define OPT_warn		109
#define OPT_write		110
#define OPT_yes			111

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
