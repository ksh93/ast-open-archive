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
 * netflow method
 *
 * Glenn Fowler
 * AT&T Research
 */

#include "flowlib.h"

#define NETFLOW_src_addr		1
#define NETFLOW_dst_addr		2
#define NETFLOW_hop			3
#define NETFLOW_input			4
#define NETFLOW_output			5
#define NETFLOW_packets			6
#define NETFLOW_bytes			7
#define NETFLOW_first			8
#define NETFLOW_last			9
#define NETFLOW_src_port		10
#define NETFLOW_dst_port		11
#define NETFLOW_flags			12
#define NETFLOW_tcp_flags		13
#define NETFLOW_prot			14
#define NETFLOW_tos			15
#define NETFLOW_src_as			16
#define NETFLOW_dst_as			17
#define NETFLOW_src_mask		18
#define NETFLOW_dst_mask		19
#define NETFLOW_router_sc		20
#define NETFLOW_tcp_retx_cnt		21
#define NETFLOW_tcp_retx_secs		22
#define NETFLOW_tcp_misseq_cnt		23
#define NETFLOW_version			24
#define NETFLOW_count			25
#define NETFLOW_uptime			26
#define NETFLOW_time			27
#define NETFLOW_nsec			28
#define NETFLOW_flow_sequence		29
#define NETFLOW_engine_type		30
#define NETFLOW_engine_id		31
#define NETFLOW_sampling_interval	32
#define NETFLOW_sampling_mode		33
#define NETFLOW_start			34
#define NETFLOW_end			35

Dssformat_t*		netflow_formats = netflow_first_format;

static Cxvariable_t	fields[] =
{
CXV("src_addr",		"ipaddr_t",	NETFLOW_src_addr,
			"Source IP address.")
CXV("dst_addr",		"ipaddr_t",	NETFLOW_dst_addr,
			"Destination IP address.")
CXV("hop",		"ipaddr_t",	NETFLOW_hop,	
			"Next hop IP address.")
CXV("input",		"number",	NETFLOW_input,	
			"Input interface index.")
CXV("output",		"number",	NETFLOW_output,	
			"Output interface index.")
CXV("packets",		"number",	NETFLOW_packets,
			"Packets sent in duration.")
CXV("bytes",		"number",	NETFLOW_bytes,	
			"Number of bytes sent in duration.")
CXV("first",		"elapsed_t",	NETFLOW_first,	
			"Elapsed milliseconds at flow start.")
CXV("last",		"elapsed_t",	NETFLOW_last,	
			"Elapsed milliseconds at flow end.")
CXV("src_port",		"number",	NETFLOW_src_port,
			"TCP/UDP source port number.")
CXV("dst_port",		"number",	NETFLOW_dst_port,
			"TCP/UDP destination port number.")
CXV("flags",		"number",	NETFLOW_flags,
			"Reason flow was discarded, etc...")
CXV("tcp_flags",	"number",	NETFLOW_tcp_flags,
			"Cumulative OR of tcp flags.")
CXV("prot",		"number",	NETFLOW_prot,
			"IP protocol, e.g., 6=TCP, 17=UDP, ...")
CXV("tos",		"number",	NETFLOW_tos,
			"IP type-of-service.")
CXV("src_as",		"number",	NETFLOW_src_as,
			"Originating AS of source address.")
CXV("dst_as",		"number",	NETFLOW_dst_as,
			"Originating AS of destination address.")
CXV("src_mask",		"number",	NETFLOW_src_mask,
			"Source address prefix mask bits.")
CXV("dst_mask",		"number",	NETFLOW_dst_mask,
			"Destination address prefix mask bits.")
CXV("router_sc",	"ipaddr_t",	NETFLOW_router_sc,
			"IP address of router shortcut by switch.")
CXV("tcp_retx_cnt",	"number",	NETFLOW_tcp_retx_cnt,
			"Number of mis-seq with delay > 1sec.")
CXV("tcp_retx_secs",	"number",	NETFLOW_tcp_retx_secs,
			"Number of seconds between mis-sequenced pkts.")
CXV("tcp_misseq_cnt",	"number",	NETFLOW_tcp_misseq_cnt,
			"Number of mis-sequenced tcp pkts.")
CXV("version",		"number",	NETFLOW_version,
			"Record version.")
CXV("count",		"number",	NETFLOW_count,
			"Number of records that follow in packet.")
CXV("uptime",		"elapsed_t",	NETFLOW_uptime,
			"Elapsed milliseconds since the router booted.")
CXV("time",		"time_t",	NETFLOW_time,
			"Current time in seconds since the epoch.")
CXV("nsec",		"number",	NETFLOW_nsec,
			"Residual nanoseconds.")
CXV("flow_sequence",	"number",	NETFLOW_flow_sequence,
			"Flow sequence counter.")
CXV("engine_type",	"number",	NETFLOW_engine_type,
			"Switching engine type.")
CXV("engine_id",	"number",	NETFLOW_engine_id,
			"Switching engine type id.")
CXV("sampling_interval","number",	NETFLOW_sampling_interval,
			"Sampling interval.")
CXV("sampling_mode",	"number",	NETFLOW_sampling_mode,
			"Sampling mode.")
CXV("start",		"ns_t",		NETFLOW_start,
			"Flow start time in 64 bit nanoseconds since the epoch.")
CXV("end",		"ns_t",		NETFLOW_end,
			"Flow end time in 64 bit nanoseconds since the epoch.")
{0}
};

static int
op_get(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	Netflow_t*	rp = (Netflow_t*)DSSDATA(data);
	Cxvariable_t*	vp = pc->data.variable;

	switch (vp->index)
	{
	case NETFLOW_src_addr:
		r->value.number = rp->ipv4_src_addr;
		break;
	case NETFLOW_dst_addr:
		r->value.number = rp->ipv4_dst_addr;
		break;
	case NETFLOW_hop:
		r->value.number = rp->ipv4_next_hop;
		break;
	case NETFLOW_input:
		r->value.number = rp->input_snmp;
		break;
	case NETFLOW_output:
		r->value.number = rp->output_snmp;
		break;
	case NETFLOW_packets:
		r->value.number = rp->in_pkts;
		break;
	case NETFLOW_bytes:
		r->value.number = rp->in_bytes;
		break;
	case NETFLOW_first:
		r->value.number = rp->first_switched;
		break;
	case NETFLOW_last:
		r->value.number = rp->first_switched;
		break;
	case NETFLOW_src_port:
		r->value.number = rp->src_port;
		break;
	case NETFLOW_dst_port:
		r->value.number = rp->dst_port;
		break;
	case NETFLOW_flags:
		r->value.number = rp->forwarding_code;
		break;
	case NETFLOW_tcp_flags:
		r->value.number = rp->tcp_flags;
		break;
	case NETFLOW_prot:
		r->value.number = rp->protocol;
		break;
	case NETFLOW_tos:
		r->value.number = rp->src_tos;
		break;
	case NETFLOW_src_as:
		r->value.number = rp->src_as;
		break;
	case NETFLOW_dst_as:
		r->value.number = rp->dst_as;
		break;
	case NETFLOW_src_mask:
		r->value.number = rp->src_mask;
		break;
	case NETFLOW_dst_mask:
		r->value.number = rp->dst_mask;
		break;
	case NETFLOW_router_sc:
		r->value.number = rp->router_sc;
		break;
	case NETFLOW_tcp_retx_cnt:
		r->value.number = rp->tcp_retx_cnt;
		break;
	case NETFLOW_tcp_retx_secs:
		r->value.number = rp->tcp_retx_secs;
		break;
	case NETFLOW_tcp_misseq_cnt:
		r->value.number = rp->tcp_misseq_cnt;
		break;
	case NETFLOW_version:
		r->value.number = rp->version;
		break;
	case NETFLOW_count:
		r->value.number = rp->count;
		break;
	case NETFLOW_uptime:
		r->value.number = rp->uptime;
		break;
	case NETFLOW_time:
		r->value.number = rp->time;
		break;
	case NETFLOW_nsec:
		r->value.number = rp->nsec;
		break;
	case NETFLOW_flow_sequence:
		r->value.number = rp->flow_sequence;
		break;
	case NETFLOW_engine_type:
		r->value.number = rp->engine_type;
		break;
	case NETFLOW_engine_id:
		r->value.number = rp->engine_id;
		break;
	case NETFLOW_sampling_interval:
		r->value.number = rp->sampling_interval;
		break;
	case NETFLOW_sampling_mode:
		r->value.number = rp->flow_sampler_mode;
		break;
	case NETFLOW_start:
#if _typ_int64_t
		r->value.number = (int64_t)rp->start; /* ms cc requires signed */
#else
		r->value.number = rp->start;
#endif
		break;
	case NETFLOW_end:
#if _typ_int64_t
		r->value.number = (int64_t)rp->end; /* ms cc requires signed */
#else
		r->value.number = rp->start;
#endif
		break;
	default:
		if (disc->errorf)
			(*disc->errorf)(cx, disc, ERROR_PANIC, "%s: variable index %d not implemented", vp->name, vp->index);
		return -1;
	}
	return 0;
}

static Cxcallout_t	local_callouts[] =
{
CXC(CX_GET, "void", "void", op_get, 0)
};

/*
 * methf
 */

static Dssmeth_t*
netflowmeth(const char* name, const char* options, const char* schema, Dssdisc_t* disc, Dssmeth_t* meth)
{
	Dssformat_t*	fp;
	int		i;

	for (fp = netflow_formats; fp; fp = fp->next)
		dtinsert(meth->formats, fp);
	for (i = 0; i < elementsof(local_callouts); i++)
		if (cxaddcallout(meth->cx, &local_callouts[i], disc))
			return 0;
	for (i = 0; fields[i].name; i++)
		if (cxaddvariable(meth->cx, &fields[i], disc))
			return 0;
	return meth;
}

/*
 * openf
 */

static int
netflowopen(Dss_t* dss, Dssdisc_t* disc)
{
	return 0;
}

static const char*	libraries[] = { "time_t", "ip_t", 0 };

static Dssmeth_t	method =
{
	"netflow",
	"Cisco router netflow dump.",
	CXH,
	netflowmeth,
	netflowopen,
	0,
	0,
	"%(time:%+u%K)s %(prot)d %(src_addr)s:%(src_port)d %(dst_addr)s:%(dst_port)d %(hop)s"
};

static Dsslib_t		lib =
{
	"netflow",
	"netflow method",
	CXH,
	&libraries[0],
	&method,
};

Dsslib_t*
dss_lib(const char* name, Dssdisc_t* disc)
{
	return &lib;
}
