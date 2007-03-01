/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*           Copyright (c) 2002-2007 AT&T Knowledge Ventures            *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                      by AT&T Knowledge Ventures                      *
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
 * cisco netflow data interface
 *
 * Glenn Fowler
 * AT&T Research
 */

#ifndef _NETFLOW_H
#define _NETFLOW_H

#include <ast_common.h>

#define NETFLOW_PACKET		1464

typedef   uint8_t Nfbyte_t;
typedef  uint16_t Nfshort_t;
typedef  uint32_t Nflong_t;
typedef uintmax_t Nftime_t;

/*
 * canonical netflow data
 */

typedef struct Netflow_s
{
Nflong_t	src_addr;	/* Source IP Address */
Nflong_t	dst_addr;	/* Destination IP Address */
Nflong_t	hop;		/* Next hop router's IP Address */
Nfshort_t	input;		/* Input interface index */
Nfshort_t	output;		/* Output interface index */
Nflong_t	packets;	/* Packets sent in Duration */
Nflong_t	bytes;		/* Bytes sent in Duration. */
Nflong_t	first;		/* SysUptime at start of flow */
Nflong_t	last;		/* and of last packet of flow */
Nfshort_t	src_port;	/* TCP/UDP source port number */    
Nfshort_t	dst_port;	/* TCP/UDP destination port number */

Nfbyte_t	flags;		/* Reason flow was discarded, etc...  */
Nfbyte_t	tcp_flags;	/* Cumulative OR of tcp flags */
Nfbyte_t	prot;		/* IP protocol, e.g., 6=TCP, 17=UDP, ... */
Nfbyte_t	tos;		/* IP Type-of-Service */

/*V5*/
Nfshort_t	src_as;		/* originating AS of source address */
Nfshort_t	dst_as;		/* originating AS of destination address */
Nfbyte_t	src_mask;	/* source address prefix mask bits */
Nfbyte_t	dst_mask;	/* destination address prefix mask bits */
Nfshort_t	pad2;

/*V7*/
Nflong_t	router_sc;	/* IP address of router shortcut by switch */

/*V1*/
Nfbyte_t	pad1;
Nfbyte_t	tcp_retx_cnt;	/* # mis-seq with delay > 1sec */
Nfbyte_t	tcp_retx_secs;	/* # seconds between mis-sequenced pkts */
Nfbyte_t	tcp_misseq_cnt;	/* # mis-sequenced tcp pkts */

/*header*/
Nfshort_t	version;	/* Record version. */
Nfshort_t	count;		/* # records in packet. */
Nflong_t	uptime;		/* Elapsed millisecs since router booted */
Nflong_t	time;		/* Current time since epoch. */
Nflong_t	nsec;		/* Residual nanoseconds. */
Nflong_t	flow_sequence;	/* Seq counter of total flows seen. */
Nfbyte_t	engine_type;	/* Type of flow switching engine */
Nfbyte_t	engine_id;	/* ID number of the flow switching engine */
Nfshort_t	sampling_interval;/* Sampling interval. */
Nfbyte_t	sampling_mode;	/* Sampling mode. */

/*synthesized*/
Nftime_t	start;		/* nanoseconds since epoch at flow start */
Nftime_t	end;		/* nanoseconds since epoch at flow end */
} Netflow_t;

#endif
