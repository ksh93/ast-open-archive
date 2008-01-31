/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2000-2008 AT&T Intellectual Property          *
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
 * bgp data interface
 *
 * Glenn Fowler
 * AT&T Research
 */

#ifndef _BGP_H
#define _BGP_H

#include <ast_common.h>

#define BGP_VERSION		20080115L	/* interface version	*/

#define BGP_SET			0xffff		/* as path set marker	*/

/*
 * attributes (even, 1 bit set)
 */

#define BGP_MESSAGE		(1<<1)
#define BGP_atomic		(1<<2)
#define BGP_best		(1<<3)
#define BGP_damped		(1<<4)
#define BGP_history		(1<<5)
#define BGP_internal		(1<<6)
#define BGP_rib_failure		(1<<7)
#define BGP_slot		(1<<8)
#define BGP_stale		(1<<9)
#define BGP_suppressed		(1<<10)
#define BGP_valid		(1<<11)
#define BGP_PART		(1<<15)

/*
 * indices (odd, 2 bits set)
 */

#define BGP_hop			((1<<1)|1)
#define BGP_local		((2<<1)|1)
#define BGP_med			((3<<1)|1)
#define BGP_path		((4<<1)|1)
#define BGP_prefix		((5<<1)|1)
#define BGP_weight		((6<<1)|1)
#define BGP_origin		((7<<1)|1)
#define BGP_time		((8<<1)|1)
#define BGP_agg_addr		((9<<1)|1)
#define BGP_agg_as		((10<<1)|1)
#define BGP_dpa_addr		((11<<1)|1)
#define BGP_dpa_as		((12<<1)|1)
#define BGP_cluster		((13<<1)|1)
#define BGP_community		((14<<1)|1)
#define BGP_bits		((15<<1)|1)
#define BGP_originator		((16<<1)|1)
#define BGP_src_addr		((17<<1)|1)
#define BGP_src_as		((18<<1)|1)
#define BGP_dst_addr		((19<<1)|1)
#define BGP_dst_as		((20<<1)|1)
#define BGP_type		((21<<1)|1)
#define BGP_old_state		((22<<1)|1)
#define BGP_new_state		((23<<1)|1)
#define BGP_stamp		((24<<1)|1)
#define BGP_message		((25<<1)|1)
#define BGP_as32		((26<<1)|1)
#define BGP_addr32		((27<<1)|1)
#define BGP_path32		((28<<1)|1)
#define BGP_agg_addr32		((29<<1)|1)
#define BGP_agg_as32		((30<<1)|1)
#define BGP_unknown		((31<<1)|1)

#define BGP_LAST		31

/*
 * BGP_type
 */

#define BGP_TYPE_announce	'A'
#define BGP_TYPE_keepalive	'K'
#define BGP_TYPE_notification	'N'
#define BGP_TYPE_open		'O'
#define BGP_TYPE_state_change	'S'
#define BGP_TYPE_table_dump	'T'
#define BGP_TYPE_withdraw	'W'

/*
 * BGP_origin
 */

#define BGP_ORIGIN_incomplete	'?'
#define BGP_ORIGIN_egp		'e'
#define BGP_ORIGIN_igp		'i'

#define BGPCLUSTER(r)		((Bgpnum_t*)((r)->data+(r)->cluster.offset))
#define BGPCOMMUNITY(r)		((Bgpasn_t*)((r)->data+(r)->community.offset))
#define BGPPATH(r)		((Bgpasn_t*)((r)->data+(r)->path.offset))
#define BGPPATH32(r)		((Bgpasn_t*)((r)->data+(r)->path32.offset))

struct Bgproute_s; typedef struct Bgproute_s Bgproute_t;
struct Bgpvec_s; typedef struct Bgpvec_s Bgpvec_t;

typedef uint16_t Bgpasn_t;
typedef uint32_t Bgpnum_t;

struct Bgpvec_s				/* vector data			*/
{
	Bgpasn_t	offset;		/* Bgproute_t.data[] offset	*/
	Bgpasn_t	size;		/* # elements			*/
	Bgpasn_t	flags;		/* data-specific flags		*/
	Bgpasn_t	attr;		/* data-specific attribute	*/
};

/* BGP_TYPE_state_change */

#define old_state	agg_as
#define new_state	dpa_as

/* BGP_TYPE_open */

#define open_version	med
#define open_as		agg_as
#define open_hold	dpa_as
#define open_id		originator
#define open_size	local

/* BGP_TYPE_notification */

#define note_code	agg_as
#define note_subcode	dpa_as
#define note_size	local

struct Bgproute_s
{
	/* 32 bit members */

	Bgpnum_t	size;		/* actual record size		*/
	Bgpnum_t	addr;		/* prefix address		*/
	Bgpnum_t	hop;		/* next hop address		*/
	Bgpnum_t	local;		/* local preference		*/
	Bgpnum_t	med;		/* med				*/
	Bgpnum_t	weight;		/* router proprietary weight	*/
	Bgpnum_t	time;		/* packet event time stamp	*/
	Bgpnum_t	stamp;		/* data time stamp		*/
	Bgpnum_t	originator;	/* originator id		*/
	Bgpnum_t	agg_addr;	/* aggregator addr		*/
	Bgpnum_t	dpa_addr;	/* dpa addr			*/
	Bgpnum_t	dst_addr;	/* destination addr		*/
	Bgpnum_t	src_addr;	/* source addr			*/
	Bgpnum_t	message;	/* message group index		*/
	Bgpnum_t	agg_addr32;	/* aggregator as32 addr		*/
	Bgpnum_t	agg_as32;	/* aggregator as32		*/

	/* 16 bit members */

	Bgpvec_t	path;		/* as path			*/
	Bgpvec_t	cluster;	/* clusters			*/
	Bgpvec_t	community;	/* communities			*/
	Bgpvec_t	path32;		/* as32 path			*/
	Bgpvec_t	unknown;	/* unknown attributes		*/

	Bgpasn_t	attr;		/* BGP_[a-z]* route attributes	*/
	Bgpasn_t	agg_as;		/* aggregator as		*/
	Bgpasn_t	dpa_as;		/* dpa as			*/
	Bgpasn_t	dst_as;		/* destination as		*/
	Bgpasn_t	src_as;		/* source as			*/

	/* 8 bit members */

	unsigned char	bits;		/* prefix bits			*/
	unsigned char	type;		/* BGP_TYPE_*			*/
	unsigned char	origin;		/* BGP_ORIGIN_*			*/
	unsigned char	blocks;		/* # blocks for this record	*/
	unsigned char	p1;		/* parameter 1			*/
	unsigned char	p2;		/* parameter 2			*/

	char		data[904];	/* vector data (round to 1K)	*/
};

#endif
