/***********************************************************************
*                                                                      *
*              This software is part of the bzip package               *
*                       Copyright (c) 1996-2011                        *
*                                                                      *
* This software is provided 'as-is', without any express or implied    *
* warranty. In no event will the authors be held liable for any        *
* damages arising from the use of this software.                       *
*                                                                      *
* Permission is granted to anyone to use this software for any         *
* purpose, including commercial applications, and to alter it and      *
* redistribute it freely, subject to the following restrictions:       *
*                                                                      *
*  1. The origin of this software must not be misrepresented;          *
*     you must not claim that you wrote the original software. If      *
*     you use this software in a product, an acknowledgment in the     *
*     product documentation would be appreciated but is not            *
*     required.                                                        *
*                                                                      *
*  2. Altered source versions must be plainly marked as such,          *
*     and must not be misrepresented as being the original             *
*     software.                                                        *
*                                                                      *
*  3. This notice may not be removed or altered from any source        *
*     distribution.                                                    *
*                                                                      *
* This program, "bzip2" and associated library "libbzip2", are         *
* copyright (C) 1996-1998 Julian R Seward.  All rights reserved.       *
*                                                                      *
* Redistribution and use in source and binary forms, with or without   *
* modification, are permitted provided that the following conditions   *
* are met:                                                             *
*                                                                      *
* 1. Redistributions of source code must retain the above copyright    *
*    notice, this list of conditions and the following disclaimer.     *
*                                                                      *
* 2. The origin of this software must not be misrepresented; you must  *
*    not claim that you wrote the original software.  If you use this  *
*    software in a product, an acknowledgment in the product           *
*    documentation would be appreciated but is not required.           *
*                                                                      *
* 3. Altered source versions must be plainly marked as such, and must  *
*    not be misrepresented as being the original software.             *
*                                                                      *
* 4. The name of the author may not be used to endorse or promote      *
*    products derived from this software without specific prior written*
*    permission.                                                       *
*                                                                      *
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS      *
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED    *
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE   *
* ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY      *
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL   *
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE    *
* GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS        *
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,         *
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING            *
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS   *
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.         *
*                                                                      *
* Julian Seward, Guildford, Surrey, UK.                                *
* jseward@acm.org                                                      *
* bzip2/libbzip2 version 0.9.0 of 28 June 1998                         *
*                                                                      *
*                           Julian R Seward                            *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * sfio bzip discipline interface
 */

#ifndef _SFDCBZIP_H
#define _SFDCBZIP_H

#include <sfdisc.h>

#define SFBZ_VERIFY		0x0010

#define SFBZ_HANDLE		SFDCEVENT('B','Z',1)
#define SFBZ_GETPOS		SFDCEVENT('B','Z',2)
#define SFBZ_SETPOS		SFDCEVENT('B','Z',3)

#if _BLD_bz && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern int	sfdcbzip(Sfio_t*, int);

#undef	extern

#endif
