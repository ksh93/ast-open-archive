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
 * pax format list definitions
 */

#ifndef _FORMAT_H
#define _FORMAT_H	1

#include "pax.h"

/*
 * define PAX_DEBUG_FORMAT to join the builtins at the end
 * and set the makefile PAX.DEBUG.FORMAT=debug-format-source.c
 */

#ifdef PAX_DEBUG_FORMAT
#define pax_last_format		(&PAX_DEBUG_FORMAT)
extern Format_t			PAX_DEBUG_FORMAT;
#else
#define pax_last_format		0
#endif

#define pax_first_format	(&pax_slt_format)
#define pax_slt_next		(&pax_ibm_format)
#define pax_ibm_next		(&pax_binary_format)
#define pax_binary_next		(&pax_cpio_format)
#define pax_cpio_next		(&pax_ustar_format)
#define pax_ustar_next		(&pax_asc_format)
#define pax_asc_next		(&pax_aschk_format)
#define pax_aschk_next		(&pax_vmsbackup_format)
#define pax_vmsbackup_next	(&pax_ar_format)
#define pax_ar_next		(&pax_vdb_format)
#define pax_vdb_next		(&pax_rpm_format)
#define pax_rpm_next		(&pax_mime_format)
#define pax_mime_next		(&pax_tnef_format)
#define pax_tnef_next		(&pax_pds_format)
#define pax_pds_next		(&pax_compress_format)
#define pax_compress_next	(&pax_gzip_format)
#define pax_gzip_next		(&pax_bzip_format)
#define pax_bzip_next		(&pax_delta_format)
#define pax_delta_next		(&pax_delta_88_format)
#define pax_delta_88_next	(&pax_ignore_format)
#define pax_ignore_next		(&pax_patch_format)
#define pax_patch_next		pax_last_format

extern Format_t			pax_ar_format;
extern Format_t			pax_asc_format;
extern Format_t			pax_aschk_format;
extern Format_t			pax_binary_format;
extern Format_t			pax_bzip_format;
extern Format_t			pax_compress_format;
extern Format_t			pax_cpio_format;
extern Format_t			pax_delta_88_format;
extern Format_t			pax_delta_format;
extern Format_t			pax_gzip_format;
extern Format_t			pax_ibm_format;
extern Format_t			pax_ignore_format;
extern Format_t			pax_mime_format;
extern Format_t			pax_patch_format;
extern Format_t			pax_pax_format;
extern Format_t			pax_pds_format;
extern Format_t			pax_rpm_format;
extern Format_t			pax_slt_format;
extern Format_t			pax_tar_format;
extern Format_t			pax_tnef_format;
extern Format_t			pax_ustar_format;
extern Format_t			pax_vdb_format;
extern Format_t			pax_vmsbackup_format;

#endif