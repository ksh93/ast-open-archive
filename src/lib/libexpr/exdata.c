/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1989-2003 AT&T Corp.                *
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
*                 Phong Vo <kpv@research.att.com>                  *
*                                                                  *
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * expression library readonly tables
 */

static const char id[] = "\n@(#)$Id: libexpr (AT&T Research) 2000-01-01 $\0\n";

#include <exlib.h>

const char*	exversion = id + 10;

Exid_t		exbuiltin[] =
{

	/* id_string references the first entry */

	EXID("string",	DECLARE,	STRING,		STRING,	0),

	/* order not important after this point (but sorted anyway) */

	EXID("break",	BREAK,		BREAK,		0,	0),
	EXID("case",	CASE,		CASE,		0,	0),
	EXID("char",	DECLARE,	CHAR,		CHAR,	0),
	EXID("continue",CONTINUE,	CONTINUE,	0,	0),
	EXID("default",	DEFAULT,	DEFAULT,	0,	0),
	EXID("double",	DECLARE,	FLOATING,	FLOATING,0),
	EXID("else",	ELSE,		ELSE,		0,	0),
	EXID("exit",	EXIT,		EXIT,		INTEGER,0),
	EXID("for",	FOR,		FOR,		0,	0),
	EXID("float",	DECLARE,	FLOATING,	FLOATING,0),
	EXID("if",	IF,		IF,		0,	0),
	EXID("int",	DECLARE,	INTEGER,	INTEGER,0),
	EXID("long",	DECLARE,	INTEGER,	INTEGER,0),
	EXID("printf",	PRINTF,		PRINTF,		INTEGER,0),
	EXID("query",	QUERY,		QUERY,		INTEGER,0),
	EXID("return",	RETURN,		RETURN,		0,	0),
	EXID("sprintf",	SPRINTF,	SPRINTF,	STRING,	0),
	EXID("switch",	SWITCH,		SWITCH,		0,	0),
	EXID("unsigned",DECLARE,	UNSIGNED,	UNSIGNED,0),
	EXID("void",	DECLARE,	VOID,		0,	0),
	EXID("while",	WHILE,		WHILE,		0,	0),
	EXID(0,		0,		0,		0,	0)

};
