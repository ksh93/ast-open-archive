#pragma prototyped

/*
 * sfio gzip discipline interface
 */

#ifndef _SFDCGZIP_H
#define _SFDCGZIP_H

#include <sfdisc.h>

#define SFGZ_VERIFY		0x0010
#define SFGZ_NOCRC		0x0020

#define SFGZ_HANDLE		SFDCEVENT('G','Z',1)
#define SFGZ_GETPOS		SFDCEVENT('G','Z',2)
#define SFGZ_SETPOS		SFDCEVENT('G','Z',3)

#if _BLD_z && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

extern int	sfdcgzip(Sfio_t*, int);
extern int	sfdclzw(Sfio_t*, int);

#undef	extern

#endif
