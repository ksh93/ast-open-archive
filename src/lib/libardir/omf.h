/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1986-2003 AT&T Corp.                *
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
*                David Korn <dgk@research.att.com>                 *
*                 Phong Vo <kpv@research.att.com>                  *
*                                                                  *
*******************************************************************/
/**************************
 * Record types:
 */

#define OMF_RHEADR	0x6E
#define OMF_REGINT	0x70
#define OMF_REDATA	0x72
#define OMF_RIDATA	0x74
#define OMF_OVLDEF	0x76
#define OMF_ENDREC	0x78
#define OMF_BLKDEF	0x7A
#define OMF_BLKEND	0x7C
#define OMF_DEBSYM	0x7E
#define OMF_THEADR	0x80
#define OMF_LHEADR	0x82
#define OMF_PEDATA	0x84
#define OMF_PIDATA	0x86
#define OMF_COMENT	0x88
#define OMF_MODEND	0x8A
#define OMF_EXTDEF	0x8C
#define OMF_TYPDEF	0x8E
#define OMF_PUBDEF	0x90
#define OMF_LOCSYM	0x92
#define OMF_LINNUM	0x94
#define OMF_LNAMES	0x96
#define OMF_SEGDEF	0x98
#define OMF_GRPDEF	0x9A
#define OMF_FIXUPP	0x9C
#define OMF_LEDATA	0xA0
#define OMF_LIDATA	0xA2
#define OMF_LIBHED	0xA4
#define OMF_LIBNAM	0xA6
#define OMF_LIBLOC	0xA8
#define OMF_LIBDIC	0xAA
#define OMF_COMDEF	0xB0
#define OMF_LEXTDEF	0xB4
#define OMF_LPUBDEF	0xB6
#define OMF_LCOMDEF	0xB8
#define OMF_CEXTDEF	0xBC
#define OMF_COMDAT	0xC2
#define OMF_LINSYM	0xC4
#define OMF_ALIAS	0xC6
#define OMF_NBKPAT	0xC8
#define OMF_LLNAMES	0xCA
#define OMF_LIBHDR	0xf0	/* library header */
#define OMF_LIBDHD	0xf1	/* library dictionary header */  

