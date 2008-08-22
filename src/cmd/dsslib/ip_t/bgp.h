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
#include <ip6.h>

#define BGP_VERSION		20080611L	/* interface version	*/

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
#define BGP_ipv6		(1<<7)
#define BGP_rib_failure		(1<<8)
#define BGP_slot		(1<<9)
#define BGP_stale		(1<<10)
#define BGP_suppressed		(1<<11)
#define BGP_valid		(1<<12)
#define BGP_PART		(1<<15)

/*
 * indices (odd, 2 bits set)
 */

#define BGP_agg_addr		((1<<1)|1)
#define BGP_agg_addrv4		((2<<1)|1)
#define BGP_agg_addrv6		((3<<1)|1)
#define BGP_agg_addr32		((4<<1)|1)
#define BGP_agg_addr32v4	((5<<1)|1)
#define BGP_agg_addr32v6	((6<<1)|1)
#define BGP_agg_as		((7<<1)|1)
#define BGP_agg_as16		((8<<1)|1)
#define BGP_agg_as32		((9<<1)|1)
#define BGP_bits		((10<<1)|1)
#define BGP_bitsv4		((11<<1)|1)
#define BGP_bitsv6		((12<<1)|1)
#define BGP_cluster		((13<<1)|1)
#define BGP_community		((14<<1)|1)
#define BGP_dpa_addr		((15<<1)|1)
#define BGP_dpa_addrv4		((16<<1)|1)
#define BGP_dpa_addrv6		((17<<1)|1)
#define BGP_dpa_as		((18<<1)|1)
#define BGP_dpa_as16		((19<<1)|1)
#define BGP_dpa_as32		((20<<1)|1)
#define BGP_dst_addr		((21<<1)|1)
#define BGP_dst_addrv4		((22<<1)|1)
#define BGP_dst_addrv6		((23<<1)|1)
#define BGP_dst_as		((24<<1)|1)
#define BGP_dst_as16		((25<<1)|1)
#define BGP_dst_as32		((26<<1)|1)
#define BGP_extended		((27<<1)|1)
#define BGP_hop			((28<<1)|1)
#define BGP_hopv4		((29<<1)|1)
#define BGP_hopv6		((30<<1)|1)
#define BGP_local		((31<<1)|1)
#define BGP_med			((32<<1)|1)
#define BGP_message		((33<<1)|1)
#define BGP_new_state		((34<<1)|1)
#define BGP_old_state		((35<<1)|1)
#define BGP_origin		((36<<1)|1)
#define BGP_originator		((37<<1)|1)
#define BGP_path		((38<<1)|1)
#define BGP_path16		((39<<1)|1)
#define BGP_path32		((40<<1)|1)
#define BGP_prefix		((41<<1)|1)
#define BGP_prefixv4		((42<<1)|1)
#define BGP_prefixv6		((43<<1)|1)
#define BGP_safi		((44<<1)|1)
#define BGP_src_addr		((45<<1)|1)
#define BGP_src_addrv4		((46<<1)|1)
#define BGP_src_addrv6		((47<<1)|1)
#define BGP_src_as		((48<<1)|1)
#define BGP_src_as16		((49<<1)|1)
#define BGP_src_as32		((50<<1)|1)
#define BGP_stamp		((51<<1)|1)
#define BGP_time		((52<<1)|1)
#define BGP_type		((53<<1)|1)
#define BGP_weight		((54<<1)|1)
#define BGP_unknown		((55<<1)|1)

#define BGP_LAST		55

#define BGP_INDEX(x)		(((x)>>1)-1)

/*
 * field set bits
 */

#define BGP_SET_agg_addrv4	(1<<0)
#define BGP_SET_agg_addrv6	(1<<1)
#define BGP_SET_agg_addr32v4	(1<<2)
#define BGP_SET_agg_addr32v6	(1<<3)
#define BGP_SET_cluster		(1<<4)
#define BGP_SET_community	(1<<5)
#define BGP_SET_dpa_addrv4	(1<<6)
#define BGP_SET_dpa_addrv6	(1<<7)
#define BGP_SET_dst_addrv4	(1<<8)
#define BGP_SET_dst_addrv6	(1<<9)
#define BGP_SET_extended	(1<<10)
#define BGP_SET_hopv4		(1<<11)
#define BGP_SET_hopv6		(1<<12)
#define BGP_SET_path16		(1<<13)
#define BGP_SET_path32		(1<<14)
#define BGP_SET_prefixv4	(1L<<15)
#define BGP_SET_prefixv6	(1L<<16)
#define BGP_SET_src_addrv4	(1L<<17)
#define BGP_SET_src_addrv6	(1L<<18)
#define BGP_SET_unknown		(1L<<19)

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
#define BGPEXTENDED(r)		((Bgpasn_t*)((r)->data+(r)->extended.offset))
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
	Bgpnum_t	dpa_as32;	/* dpa as32			*/
	Bgpnum_t	dst_as32;	/* destination as32		*/
	Bgpnum_t	src_as32;	/* source as32			*/
	Bgpnum_t	set;		/* BGP_SET_* bitmask		*/

	/* 16 bit members */

	Bgpvec_t	path;		/* as path			*/
	Bgpvec_t	cluster;	/* clusters			*/
	Bgpvec_t	community;	/* communities			*/
	Bgpvec_t	extended;	/* extended communities		*/
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
	unsigned char	safi;		/* announce subsequent afi	*/
	unsigned char	p1;		/* parameter 1			*/

	/* unaligned fixed buffer members */

	unsigned char	hopv6[16];	/* next hop address		*/
	unsigned char	agg_addrv6[16];	/* aggregator addr		*/
	unsigned char	dpa_addrv6[16];	/* dpa addr			*/
	unsigned char	dst_addrv6[16];	/* destination addr		*/
	unsigned char	src_addrv6[16];	/* source addr			*/
	unsigned char	agg_addr32v6[16];/* aggregator as32 addr	*/
	unsigned char	prefixv6[17];	/* prefix			*/

	char		pad[15];	/* pad to 8 byte boundary	*/

	char		data[752];	/* vector data (round to 1K)	*/
};

#endif
