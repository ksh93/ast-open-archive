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
