#pragma prototyped

/*
 * crypt decoder/encoder private definitions
 */

#ifndef _CRYPTLIB_H
#define _CRYPTLIB_H	1

#include <codex.h>

typedef unsigned _ast_int1_t ui1;
typedef unsigned _ast_int2_t ui2;
typedef unsigned _ast_int4_t ui4;

#define crypt_first		(&crypt_rar)
#define crypt_rar_next		(&crypt_zip)
#define crypt_zip_next		0

extern Codexmeth_t		crypt_rar;
extern Codexmeth_t		crypt_rar_13;
extern Codexmeth_t		crypt_rar_15;
extern Codexmeth_t		crypt_rar_20;
extern Codexmeth_t		crypt_zip;

extern const ui4		crc_tab[256];

#endif
