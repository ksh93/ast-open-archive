/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1989-2001 AT&T Corp.                *
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
*                David Korn <dgk@research.att.com>                 *
*                 Eduardo Krell <ekrell@adexus.cl>                 *
*******************************************************************/
#pragma prototyped

/*
 * libcs 3d symbol mappings to minimize pollution
 */

#ifndef _NAME3D_CS_H
#define _NAME3D_CS_H

#define CS_INTERFACE	2

#define _cs_info_	_3d_cs_info_
#define csbind		_3d_csbind
#define cslocal		_3d_cslocal
#define csname		_3d_csname
#define csntoa		_3d_csntoa
#define cspeek		_3d_cspeek
#define cspipe		_3d_cspipe
#define cspoll		_3d_cspoll
#define csread		_3d_csread
#define csrecv		_3d_csrecv
#define cssend		_3d_cssend
#define csvar		_3d_csvar
#define cswrite		_3d_cswrite

#define _cs_addr	_3d_cs_addr
#define _cs_attr	_3d_cs_attr
#define _cs_auth	_3d_cs_auth
#define _cs_bind	_3d_cs_bind
#define _cs_challenge	_3d_cs_challenge
#define _cs_clone	_3d_cs_clone
#define _cs_daemon	_3d_cs_daemon
#define _cs_fd		_3d_cs_fd
#define _cs_from	_3d_cs_from
#define _cs_full	_3d_cs_full
#define _cs_info	_3d_cs_info
#define _cs_local	_3d_cs_local
#define _cs_name	_3d_cs_name
#define _cs_note	_3d_cs_note
#define _cs_ntoa	_3d_cs_ntoa
#define _cs_open	_3d_cs_open
#define _cs_path	_3d_cs_path
#define _cs_peek	_3d_cs_peek
#define _cs_ping	_3d_cs_ping
#define _cs_pipe	_3d_cs_pipe
#define _cs_poll	_3d_cs_poll
#define _cs_port	_3d_cs_port
#define _cs_read	_3d_cs_read
#define _cs_recv	_3d_cs_recv
#define _cs_send	_3d_cs_send
#define _cs_serve	_3d_cs_serve
#define _cs_stat	_3d_cs_stat
#define _cs_timeout	_3d_cs_timeout
#define _cs_to		_3d_cs_to
#define _cs_var		_3d_cs_var
#define _cs_wakeup	_3d_cs_wakeup
#define _cs_write	_3d_cs_write

#define _msg_info_	_3d_msg_info
#define msgcall		_3d_msgcall
#define msggetmask	_3d_msggetmask
#define msggetu		_3d_msggetu
#define msggetz		_3d_msggetz
#define msgindex	_3d_msgindex
#define msgname		_3d_msgname
#define msgputu		_3d_msgputu
#define msgputz		_3d_msgputz
#define msgread		_3d_msgread
#define msgreturn	_3d_msgreturn
#define msgsetmask	_3d_msgsetmask
#define msgvcall	_3d_msgvcall
#define msgvreturn	_3d_msgvreturn

#endif
