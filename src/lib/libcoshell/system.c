/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1990-2005 AT&T Corp.                  *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                            by AT&T Corp.                             *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * coshell system(3)
 */

#include "colib.h"

int
system(const char* cmd)
{
	Coshell_t*	co;
	Cojob_t*	cj;
	int		status;

	if (!cmd)
		return !eaccess(pathshell(), X_OK);
	if (!(co = coopen(NiL, CO_ANY, NiL)))
		return -1;
	if (cj = coexec(co, cmd, CO_SILENT, NiL, NiL, NiL))
		cj = cowait(co, cj);
	if (!cj)
		return -1;

	/*
	 * synthesize wait() status from shell status
	 * lack of synthesis is the standard's proprietary sellout
	 */

	status = cj->status;
	if (EXITED_TERM(status))
		status &= ((1<<(EXIT_BITS-1))-1);
	else
		status = (status & ((1<<EXIT_BITS)-1)) << EXIT_BITS;
	return status;
}
