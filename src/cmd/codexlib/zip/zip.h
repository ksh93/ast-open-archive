#pragma prototyped

/*
 * zip decoder private interface
 */

#ifndef _ZIP_H
#define _ZIP_H		1

#include <codex.h>

typedef unsigned _ast_int1_t uch;
typedef unsigned _ast_int2_t ush;
typedef unsigned _ast_int4_t ulg;

extern Codexmeth_t	codex_zip_shrink;
extern Codexmeth_t	codex_zip_reduce;
extern Codexmeth_t	codex_zip_implode;
extern Codexmeth_t	codex_zip_deflate;

#endif
