/*
 * CDE - Common Desktop Environment
 *
 * Copyright (c) 1993-2012, The Open Group. All rights reserved.
 *
 * These libraries and programs are free software; you can
 * redistribute them and/or modify them under the terms of the GNU
 * Lesser General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * These libraries and programs are distributed in the hope that
 * they will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with these librararies and programs; if not, write
 * to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
 * Floor, Boston, MA 02110-1301 USA
 */
/***************************************************************
*                                                              *
*                      AT&T - PROPRIETARY                      *
*                                                              *
*        THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF        *
*                    AT&T BELL LABORATORIES                    *
*         AND IS NOT TO BE DISCLOSED OR USED EXCEPT IN         *
*            ACCORDANCE WITH APPLICABLE AGREEMENTS             *
*                                                              *
*                Copyright (c) 1995 AT&T Corp.                 *
*              Unpublished & Not for Publication               *
*                     All Rights Reserved                      *
*                                                              *
*       The copyright notice above does not evidence any       *
*      actual or intended publication of such source code      *
*                                                              *
*               This software was created by the               *
*           Advanced Software Technology Department            *
*                    AT&T Bell Laboratories                    *
*                                                              *
*               For further information contact                *
*                    {research,attmail}!dgk                    *
*                                                              *
***************************************************************/

/* : : generated by proto : : */

#if !defined(__PROTO__)
#if defined(__STDC__) || defined(__cplusplus) || defined(_proto) || defined(c_plusplus)
#if defined(__cplusplus)
#define __MANGLE__	"C"
#else
#define __MANGLE__
#endif
#define __STDARG__
#define __PROTO__(x)	x
#define __OTORP__(x)
#define __PARAM__(n,o)	n
#if !defined(__STDC__) && !defined(__cplusplus)
#if !defined(c_plusplus)
#define const
#endif
#define signed
#define void		int
#define volatile
#define __V_		char
#else
#define __V_		void
#endif
#else
#define __PROTO__(x)	()
#define __OTORP__(x)	x
#define __PARAM__(n,o)	o
#define __MANGLE__
#define __V_		char
#define const
#define signed
#define void		int
#define volatile
#endif
#if defined(__cplusplus) || defined(c_plusplus)
#define __VARARG__	...
#else
#define __VARARG__
#endif
#if defined(__STDARG__)
#define __VA_START__(p,a)	va_start(p,a)
#else
#define __VA_START__(p,a)	va_start(p)
#endif
#endif
#include        <ast.h>
#include        "FEATURE/options"
#include        "shtable.h"

#define	value(n,type)	((((unsigned)n)<<2)|(type))

const Shtable_t shtab_config[] =
{
#ifdef _SC_ARG_MAX
	"ARG_MAX",		value(_SC_ARG_MAX,2),
#else
	"ARG_MAX",		value(1,0),
#endif /* _SC_ARG_MAX */
#ifdef _SC_BC_BASE_MAX
	"BC_BASE_MAX",		value(_SC_BC_BASE_MAX,2),
#else
	"BC_BASE_MAX",		value(1,0),
#endif /* _SC_BC_BASE_MAX */
#ifdef _SC_BC_DIM_MAX
	"BC_DIM_MAX",		value(_SC_BC_DIM_MAX,2),
#else
	"BC_DIM_MAX",		value(1,0),
#endif /* _SC_BC_DIM_MAX */
#ifdef _SC_BC_SCALE_MAX
	"BC_SCALE_MAX",		value(_SC_BC_SCALE_MAX,2),
#else
	"BC_SCALE_MAX",		value(1,0),
#endif /* _SC_BC_SCALE_MAX */
#ifdef _SC_BC_STRING_MAX
	"BC_STRING_MAX",	value(_SC_BC_STRING_MAX,2),
#else
	"BC_STRING_MAX",	value(1,0),
#endif /* _SC_BC_BASE_MAX */
#ifdef _SC_CHILD_MAX
	"CHILD_MAX",		value(_SC_CHILD_MAX,2),
#else
	"CHILD_MAX",		value(1,0),
#endif /* _SC_CHILD_MAX */
#ifdef _SC_LINE_MAX
	"LINE_MAX",		value(_SC_LINE_MAX,2),
#else
	"LINE_MAX",		value(1,0),
#endif /* _SC_LINE_MAX */
#ifdef _PC_LINK_MAX
	"LINK_MAX",		value(_PC_LINK_MAX,3),
#else
	"LINK_MAX",		value(1,0),
#endif /* _PC_LINK_MAX */
#ifdef _PC_MAX_CANON
	"MAX_CANON",		value(_PC_MAX_CANON,3),
#else
	"MAX_CANON",		value(1,0),
#endif /* _PC_MAX_CANON */
#ifdef _PC_MAX_INPUT
	"MAX_INPUT",		value(_PC_MAX_INPUT,3),
#else
	"MAX_INPUT",		value(1,0),
#endif /* _PC_MAX_INPUT */
#ifdef _PC_NAME_MAX
	"NAME_MAX",		value(_PC_NAME_MAX,3),
#else
	"NAME_MAX",		value(1,0),
#endif /* _PC_NAME_MAX */
#ifdef _SC_CLK_TCK
	"CLK_TCK",		value(_SC_CLK_TCK,2),
#else
	"CLK_TCK",		value(1,0),
#endif /* _SC_CLK_TCK */
#ifdef _SC_COLL_WEIGHTS_MAX
	"COLL_WEIGHTS_MAX",		value(_SC_COLL_WEIGHTS_MAX,2),
#else
	"COLL_WEIGHTS_MAX",		value(1,0),
#endif /* _SC_COLL_WEIGHTS_MAX */
#ifdef _SC_EXPR_NEST_MAX
	"EXPR_NEST_MAX",		value(_SC_EXPR_NEST_MAX,2),
#else
	"EXPR_NEST_MAX",		value(1,0),
#endif /* _SC_EXPR_NEST_MAX */
#ifdef _SC_NGROUPS_MAX
	"NGROUPS_MAX",		value(_SC_NGROUPS_MAX,2),
#else
	"NGROUPS_MAX",		value(1,0),
#endif /* _SC_NGROUPS_MAX */
#ifdef _SC_OPEN_MAX
	"OPEN_MAX",		value(_SC_OPEN_MAX,2),
#else
	"OPEN_MAX",		value(1,0),
#endif /* _SC_OPEN_MAX */
#ifdef _CS_PATH
	"PATH",		value(_CS_PATH,1),
#else
	"PATH",		value(1,0),
#endif /* _SC_PATH */
#ifdef _PC_PATH_MAX
	"PATH_MAX",		value(_PC_PATH_MAX,3),
#else
	"PATH_MAX",		value(1,0),
#endif /* _PC_PATH_MAX */
#ifdef _PC_PIPE_BUF
	"PIPE_BUF",		value(_PC_PIPE_BUF,3),
#else
	"PIPE_BUF",		value(1,0),
#endif /* _PC_PIPE_BUF */
#ifdef _SC_2_C_BIND
	"POSIX2_C_BIND",	value(_SC_2_C_BIND,2),
#else
	"POSIX2_C_BIND",	value(1,0),
#endif /* _SC_C_BIND */
#ifdef _SC_2_C_DEV
	"POSIX2_C_DEV",	value(_SC_2_C_DEV,2),
#else
	"POSIX2_C_DEV",	value(1,0),
#endif /* _SC_C_DEV */
#ifdef _SC_2_FORT_DEV
	"POSIX2_FORT_DEV",	value(_SC_2_FORT_DEV,2),
#else
	"POSIX2_FORT_DEV",	value(1,0),
#endif /* _SC_FORT_DEV */
#ifdef _SC_2_FORT_RUN
	"POSIX2_FORT_RUN",	value(_SC_2_FORT_RUN,2),
#else
	"POSIX2_FORT_RUN",	value(1,0),
#endif /* _SC_FORT_RUN */
#ifdef _SC_2_CHAR_TERM
	"POSIX2_CHAR_TERM",	value(_SC_2_CHAR_TERM,2),
#else
	"POSIX2_CHAR_TERM",	value(1,0),
#endif /* _SC_CHAR_TERM */
#ifdef _SC_2_LOCALDEF
	"POSIX2_LOCALDEF",	value(_SC_2_LOCALDEF,2),
#else
	"POSIX2_LOCALDEF",	value(1,0),
#endif /* _SC_LOCALDEF */
#ifdef _SC_2_SW_DEV
	"POSIX2_SW_DEV",	value(_SC_2_SW_DEV,2),
#else
	"POSIX2_SW_DEV",	value(1,0),
#endif /* _SC_SW_DEV */
#ifdef _SC_2_UPE
	"POSIX2_UPE",		value(_SC_2_UPE,2),
#else
	"POSIX2_UPE",		value(1,0),
#endif /* _SC_UPE */
#ifdef _SC_2_VERSION
	"POSIX2_VERSION",	value(_SC_2_VERSION,2),
#else
	"POSIX2_VERSION",	value(1,0),
#endif /* _SC_VERSION */
#ifdef _SC_RE_DUP_MAX
	"RE_DUP_MAX",		value(_SC_RE_DUP_MAX,2),
#else
	"RE_DUP_MAX",		value(1,0),
#endif /* _SC_RE_DUP_MAX */
#ifdef _SC_STREAM_MAX
	"STREAM_MAX",		value(_SC_STREAM_MAX,2),
#else
	"STREAM_MAX",		value(1,0),
#endif /* _SC_STREAM_MAX */
#ifdef _SC_TZNAME_MAX
	"TZNAME_MAX",		value(_SC_TZNAME_MAX,2),
#else
	"TZNAME_MAX",		value(1,0),
#endif /* _SC_TZNAME_MAX */
#ifdef _SC_JOB_CONTROL
	"_POSIX_JOB_CONTROL",	value(_SC_JOB_CONTROL,2),
#else
	"_POSIX_JOB_CONTROL",	value(1,0),
#endif /* _SC_JOB_CONTROL */
#ifdef _PC_CHOWN_RESTRICTED
	"_POSIX_CHOWN_RESTRICTED",	value(_PC_CHOWN_RESTRICTED,3),
#else
	"_POSIX_CHOWN_RESTRICTED",	value(1,0),
#endif /* _PC_CHOWN_RESTRICTED */
#ifdef _PC_NO_TRUNC
	"_POSIX_NO_TRUNC",	value(_PC_NO_TRUNC,3),
#else
	"_POSIX_NO_TRUNC",	value(1,0),
#endif /* _PC_NO_TRUNC */
#ifdef _SC_SAVED_IDS
	"_POSIX_SAVED_IDS",	value(_SC_SAVED_IDS,2),
#else
	"_POSIX_SAVED_IDS",	value(1,0),
#endif /* _SC_SAVED_IDS */
#ifdef _PC_VDISABLE
	"_POSIX_VDISABLE",	value(_PC_VDISABLE,3),
#else
	"_POSIX_VDISABLE",	value(1,0),
#endif /* _PC_VDISABLE */
#ifdef _SC_VERSION
	"_POSIX_VERSION",	value(_SC_VERSION,2),
#else
	"_POSIX_VERSION",	value(1,0),
#endif /* _SC_VERSION */
	"",		0
};
