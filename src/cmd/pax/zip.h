/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1987-2000 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*                 This software was created by the                 *
*                 Network Services Research Center                 *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
#pragma prototyped

/*
 * zip interface definitions
 */

#define ZIP_HEADER	46		/* largest header size		*/
#define ZIP_COPY	0		/* no need to unzip		*/
#define ZIP_SUM_INIT	0xffffffffL	/* zipsum() initial value	*/

#define ZIP_LOC_HEADER	30		/* local header size		*/
#define ZIP_LOC_MAGIC	0x504b0304L	/* local header magic		*/
#define ZIP_LOC_VER	4		/* version needed to extract	*/
#define ZIP_LOC_FLG	6		/* encrypt, deflate flags	*/
#define ZIP_LOC_HOW	8		/* compression method		*/
#define ZIP_LOC_TIM	10		/* DOS format modify time	*/
#define ZIP_LOC_DAT	12		/* DOS format modify date	*/
#define ZIP_LOC_CRC	14		/* uncompressed data crc-32	*/
#define ZIP_LOC_SIZ	18		/* compressed data size		*/
#define ZIP_LOC_LEN	22		/* uncompressed data size	*/
#define ZIP_LOC_NAM	26		/* length of filename		*/
#define ZIP_LOC_EXT	28		/* length of extra field	*/

#define ZIP_CEN_HEADER	46		/* central header size		*/
#define ZIP_CEN_MAGIC	0x504b0102L	/* central header magic		*/
#define ZIP_CEN_VEM	4		/* version made by		*/
#define ZIP_CEN_VER	6		/* version needed to extract	*/
#define ZIP_CEN_FLG	8		/* encrypt, deflate flags	*/
#define ZIP_CEN_HOW	10		/* compression method		*/
#define ZIP_CEN_TIM	12		/* DOS format modify time	*/
#define ZIP_CEN_DAT	14		/* DOS format modify date	*/
#define ZIP_CEN_CRC	16		/* uncompressed data crc-32	*/
#define ZIP_CEN_SIZ	20		/* compressed data size		*/
#define ZIP_CEN_LEN	24		/* uncompressed data size	*/
#define ZIP_CEN_NAM	28		/* length of filename		*/
#define ZIP_CEN_EXT	30		/* length of extra field	*/
#define ZIP_CEN_COM	32		/* file comment length		*/
#define ZIP_CEN_DSK	34		/* disk number start		*/
#define ZIP_CEN_ATT	36		/* internal file attributes	*/
#define ZIP_CEN_ATX	38		/* external file attributes	*/
#define ZIP_CEN_OFF	42		/* local header relative offset	*/

#define ZIP_END_HEADER	22		/* end header size		*/
#define ZIP_END_MAGIC	0x504b0506L	/* end header magic		*/
#define ZIP_END_DSK	4		/* number of this disk		*/
#define ZIP_END_BEG	6		/* number of the starting disk	*/
#define ZIP_END_SUB	8		/* entries on this disk		*/
#define ZIP_END_TOT	10		/* total number of entries	*/
#define ZIP_END_SIZ	12		/* central directory total size	*/
#define ZIP_END_OFF	16		/* central offset starting disk	*/
#define ZIP_END_COM	20		/* length of zip file comment	*/

#define ZIP_EXT_HEADER	16		/* ext header size		*/
#define ZIP_EXT_MAGIC	0x504b0708L	/* ext header magic		*/
#define ZIP_EXT_SIZ	8		/* compressed data size		*/
#define ZIP_EXT_LEN	12		/* uncompressed data size	*/
