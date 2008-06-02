/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2002-2008 AT&T Intellectual Property          *
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
typedef uintmax_t Nfcount_t;
typedef unsigned char Nfaddr_t[16];
typedef unsigned char Nfname_t[32];

/*
 * canonical netflow data
 */

typedef struct Netflow_s
{

Nfaddr_t	ipv6_src_addr;	/* IPv6 source address */
Nfaddr_t	ipv6_dst_addr;	/* IPv6 destination address */
Nfaddr_t	ipv6_next_hop;	/* IPv6 address of next hop router */
Nfaddr_t	bgp_ipv6_next_hop;/* Next hop router IPv6 address in the BGP domain */

Nftime_t	first_switched;	/* System uptime at which the first packet of this flow was switched */
Nftime_t	last_switched;	/* System uptime at which the last packet of this flow was switched */

Nfcount_t	in_bytes;	/* Incoming counter for the number of bytes associated with an IP Flow */
Nfcount_t	in_pkts;	/* Incoming counter for the number of packets associated with an IP Flow */
Nfcount_t	mul_dst_bytes;	/* Multicast outgoing byte count */
Nfcount_t	mul_dst_pkts;	/* Multicast outgoing packet count */
Nfcount_t	out_bytes;	/* Outgoing counter for the number of bytes associated with an IP Flow */
Nfcount_t	out_pkts;	/* Outgoing counter for the number of packets associated with an IP Flow */
Nfcount_t	flows;		/* Number of flows that were aggregated */
Nfcount_t	output_snmp;	/* Output interface index */
Nfcount_t	input_snmp;	/* Input interface index */
Nfcount_t	total_bytes_exp;/* The number of bytes exported by the observation domain */
Nfcount_t	total_pkts_exp;/* The number of packets exported by the observation domain */
Nfcount_t	total_flows_exp;/* The number of flows exported by the observation domain */
Nfcount_t	in_src_mac;	/* Incoming source MAC address */
Nfcount_t	out_dst_mac;	/* Outgoing destination MAC address */
Nfcount_t	in_dst_mac;	/* Incoming destination MAC address */
Nfcount_t	out_src_mac;	/* Outgoing source MAC address */
Nfcount_t	in_permanent_bytes;/* Permanent flow byte count */
Nfcount_t	in_permanent_pkts;/* Permanent flow packet count */

Nfcount_t	vendor_43;	/* vendor private value */
Nfcount_t	vendor_51;	/* vendor private value */
Nfcount_t	vendor_65;	/* vendor private value */
Nfcount_t	vendor_66;	/* vendor private value */
Nfcount_t	vendor_67;	/* vendor private value */
Nfcount_t	vendor_68;	/* vendor private value */
Nfcount_t	vendor_69;	/* vendor private value */
Nfcount_t	vendor_87;	/* vendor private value */

Nflong_t	ipv4_src_addr;	/* IPv4 source address */
Nflong_t	ipv4_dst_addr;	/* IPv4 destination address */
Nflong_t	ipv4_next_hop;	/* IPv4 address of next hop router */
Nflong_t	bgp_ipv4_next_hop;/* Next hop router's IPv4 address in the BGP domain */
Nflong_t	src_as;		/* Source BGP autonomous system number */
Nflong_t	dst_as;		/* Destination BGP autonomous system number */
Nflong_t	ipv6_flow_label;/* IPv6 RFC 2460 flow label */
Nflong_t	sampling_interval;/* Sampling interval. */
Nflong_t	ipv4_src_prefix;/* IPv4 source address prefix (catalyst architecture only) */
Nflong_t	ipv4_dst_prefix;/* IPv4 destination address prefix (catalyst architecture only) */
Nflong_t	mpls_top_label_ip_addr;/* Forwarding Equivalent Class corresponding to the MPLS Top Label */
Nflong_t	flow_sampler_random_interval;/* Packet interval at which to sample */
Nflong_t	ipv6_option_headers;/* Bit-encoded field identifying IPv6 option headers found in the flow */
Nflong_t	mpls_label_1;	/* Stack position 1 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack */
Nflong_t	mpls_label_2;	/* Stack position 2 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack */
Nflong_t	mpls_label_3;	/* Stack position 3 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack */
Nflong_t	mpls_label_4;	/* Stack position 4 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack */
Nflong_t	mpls_label_5;	/* Stack position 5 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack */
Nflong_t	mpls_label_6;	/* Stack position 6 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack */
Nflong_t	mpls_label_7;	/* Stack position 7 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack */
Nflong_t	mpls_label_8;	/* Stack position 8 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack */
Nflong_t	mpls_label_9;	/* Stack position 9 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack */
Nflong_t	mpls_label_10;	/* Stack position 10 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack */

Nfshort_t	src_port;	/* TCP/UDP source port number */    
Nfshort_t	dst_port;	/* TCP/UDP destination port number */    
Nfshort_t	min_pkt_length;	/* Minimum incoming IP packet length */
Nfshort_t	max_pkt_length;	/* Maximum incoming IP packet length */
Nfshort_t	icmp_type;	/* Internet Control Message Protocol packet type coded as ((type*256)+code) */
Nfshort_t	mul_igmp_type;	/* Internet Group Management Protocol packet type coded */
Nfshort_t	flow_active_timeout;/* Timeout value (in seconds) for active flow cache entries */
Nfshort_t	flow_inactive_timeout;/* Timeout value (in seconds) for inactive flow cache entries */
Nfshort_t	ipv4_ident;	/* IPv4 identification field */
Nfshort_t	src_vlan;	/* Virtual LAN identifier associated with ingress interface */
Nfshort_t	dst_vlan;	/* Virtual LAN identifier associated with egress interface */
Nfshort_t	fragment_offset;/* Fragmented packet fragment-offset */

Nfbyte_t	protocol;	/* IP protocol, e.g., 6=TCP, 17=UDP, ... */
Nfbyte_t	src_tos;	/* IP Type-of-Service upon entering incoming interface */
Nfbyte_t	tcp_flags;	/* Cumulative OR of tcp flags for this flow */
Nfbyte_t	src_mask;	/* Source address prefix mask bits */
Nfbyte_t	dst_mask;	/* Destination address prefix mask bits */
Nfbyte_t	ipv6_src_mask;	/* IPv6 source address prefix mask bits */
Nfbyte_t	ipv6_dst_mask;	/* IPv6 destination address prefix mask bits */
Nfbyte_t	sampling_algorithm;/* 0x01: deterministic, 0x02: random */
Nfbyte_t	engine_type;	/* Type of flow switching engine 0: RP, 1: VIP/linecard */
Nfbyte_t	engine_id;	/* ID number of the flow switching engine */
Nfbyte_t	mpls_top_label_type;/* MPLS Top Label Type: 0x00 UNKNOWN 0x01 TE-MIDPT 0x02 ATOM 0x03 VPN 0x04 BGP 0x05 LDP */
Nfbyte_t	flow_sampler_id;/* Flow sampler ID */
Nfbyte_t	flow_sampler_mode;/* Algorithm used for sampling data: 0x02 random sampling */
Nfbyte_t	min_ttl;	/* Minimum TTL on incoming packets */
Nfbyte_t	max_ttl;	/* Maximum TTL on incoming packets */
Nfbyte_t	dst_tos;	/* Type of Service on exiting outgoing interface */
Nfbyte_t	ip_protocol_version; /* IP version 6: IPv6, 4 or not specified: IPv4 */
Nfbyte_t	direction;	/* Flow direction: 0 - ingress flow, 1 - egress flow */
Nfbyte_t	forwarding_status;/* Forwarding status 0: unknown, 1: forwarded, 2: dropped, 3: consumed */
Nfbyte_t	forwarding_code;/* Forwarding reason code */

Nfname_t	if_name;	/* Shortened interface name */
Nfname_t	if_desc;	/* Full interface name */
Nfname_t	sampler_name;	/* Flow sampler name */

Nfbyte_t	flags;		/* Reason flow was discarded, etc...  */

#if 1

/*V7*/
Nflong_t	router_sc;	/* IP address of router shortcut by switch */

/*V1*/
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

/*synthesized*/
Nftime_t	start;		/* nanoseconds since epoch at flow start */
Nftime_t	end;		/* nanoseconds since epoch at flow end */

#endif

} Netflow_t;

#endif
