/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 2002-2006 AT&T Corp.                  *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                            by AT&T Corp.                             *
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
 * netflow private definitions
 *
 * Glenn Fowler
 * AT&T Labs Research
 */

#ifndef _NETFLOWLIB_H_
#define _NETFLOWLIB_H_		1

#include <dsslib.h>
#include <netflow.h>

#define NS			((Nftime_t)1000000000)
#define US			((Nftime_t)1000000)
#define MS			((Nftime_t)1000)

#define netflow_first_format	(&netflow_fixed_format)
#define netflow_dump_next	(&netflow_tool_format)
#define netflow_fixed_next	(&netflow_dump_format)
#define netflow_tool_next	(&netflow_flat_format)
#define netflow_flat_next	0

#define netflow_method		_dss_netflow_method
#define netflow_formats		_dss_netflow_formats
#define netflow_dump_format	_dss_flow_dump_format
#define netflow_fixed_format	_dss_flow_fixed_format
#define netflow_flat_format	_dss_flow_flat_format
#define netflow_tool_format	_dss_flow_tool_format

extern Dssformat_t*		netflow_formats;
extern Dssformat_t		netflow_dump_format;
extern Dssformat_t		netflow_fixed_format;
extern Dssformat_t		netflow_flat_format;
extern Dssformat_t		netflow_tool_format;

#endif
