/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1990-2001 AT&T Corp.                *
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
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * remote coshell initialization script
 */

static const char id[] = "\n@(#)$Id: coshell (AT&T Research) 2000-12-14 $\0\n";

const char*	version = id + 10;

const char	corinit[] = "\
print -u3 n 0 $HOSTNAME $$\n\
set -o bgnice -o monitor\n\
rating=1\n\
vpath - /# 2>/dev/null\n\
eval $COINIT\n\
alias ignore='ignore '\n\
function ignore\n\
{\n\
	\"$@\"\n\
	return 0\n\
}\n\
alias silent='set +x X$- \"$@\";_coshell_flags_=$1;shift;silent '\n\
function silent\n\
{\n\
	case $_coshell_flags_ in\n\
	*x*)	trap '	_coshell_status_=$?\n\
		if ((_coshell_status_==0))\n\
		then	set -x\n\
		else	set -x;(set +x;exit $_coshell_status_)\n\
		fi' 0\n\
		;;\n\
	esac\n\
	\"$@\"\n\
}\n\
typeset -xf ignore silent\n\
case $rating in\n\
1)	(\n\
	set -- $(package host rating cpu)\n\
	print -u3 r 0 rating=$1,cpu=$2\n\
	) &\n\
	;;\n\
esac\n\
";
