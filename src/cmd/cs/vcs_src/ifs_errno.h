/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1990-2000 AT&T Corp.                *
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
/*
 * File: ifs_cserrno.h
 */
#define E_NIL		0	/* nil */
#define E_COMMAND	301	/* unimplement command */
#define E_ARGUMENT	302	/* invalid arguments */
#define E_MOUNT		401	/* unmounted path */
#define E_PROTOCOL	402	/* unknown protocol */
#define E_OPENDEST	403	/* dest-file open error */
#define E_GETHOST	404	/* unknown hostname */
#define E_SOCKET	405	/* can't open stream socket */
#define E_CONNECT	406	/* can't connect to server */
#define E_USERAUTH	407	/* user authentication error */
#define E_DATAXFER	408	/* data transfer error */

