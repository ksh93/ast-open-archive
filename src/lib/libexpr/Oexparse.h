/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1989-2012 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                 Eclipse Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
*                                                                      *
*                A copy of the License is available at                 *
*          http://www.eclipse.org/org/documents/epl-v10.html           *
*         (with md5 checksum b35adb5213ca9657e911e9befb180842)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
/* : : generated by a yacc that works -- solaris take note : : */

/* : : generated by proto : : */
#ifndef _EXPARSE_H
#if !defined(__PROTO__)
#include <prototyped.h>
#endif
#if !defined(__LINKAGE__)
#define __LINKAGE__		/* 2004-08-11 transition */
#endif

#define _EXPARSE_H
#define MINTOKEN 257
#define CHAR 258
#define INT 259
#define INTEGER 260
#define UNSIGNED 261
#define FLOATING 262
#define STRING 263
#define VOID 264
#define STATIC 265
#define ADDRESS 266
#define BREAK 267
#define CALL 268
#define CASE 269
#define CONSTANT 270
#define CONTINUE 271
#define DECLARE 272
#define DEFAULT 273
#define DYNAMIC 274
#define ELSE 275
#define EXIT 276
#define FOR 277
#define FUNCTION 278
#define ITERATE 279
#define ID 280
#define IF 281
#define LABEL 282
#define MEMBER 283
#define NAME 284
#define POS 285
#define PRAGMA 286
#define PRE 287
#define PRINTF 288
#define PROCEDURE 289
#define QUERY 290
#define RETURN 291
#define SCANF 292
#define SPRINTF 293
#define SSCANF 294
#define SWITCH 295
#define WHILE 296
#define F2I 297
#define F2S 298
#define I2F 299
#define I2S 300
#define S2B 301
#define S2F 302
#define S2I 303
#define F2X 304
#define I2X 305
#define S2X 306
#define X2F 307
#define X2I 308
#define X2S 309
#define OR 310
#define AND 311
#define EQ 312
#define NE 313
#define LE 314
#define GE 315
#define LS 316
#define RS 317
#define UNARY 318
#define INC 319
#define DEC 320
#define CAST 321
#define MAXTOKEN 322
typedef union
{
	struct Exnode_s*expr;
	double		floating;
	struct Exref_s*	reference;
	struct Exid_s*	id;
	Sflong_t	integer;
	int		op;
	char*		string;
	struct Exbuf_s*	buffer;
} EXSTYPE;
extern __MANGLE__ EXSTYPE exlval;
#endif /* _EXPARSE_H */
