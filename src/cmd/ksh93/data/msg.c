/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1982-2001 AT&T Corp.                *
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
*                David Korn <dgk@research.att.com>                 *
*******************************************************************/
#pragma prototyped
/*
 *	UNIX shell
 *	S. R. Bourne
 *	Rewritten by David Korn
 *
 *	AT&T Labs
 *
 */

#include	<ast.h>
#include	<errno.h>
#include	"defs.h"
#include	"path.h"
#include	"io.h"
#include	"shlex.h"
#include	"timeout.h"
#include	"history.h"
#include	"builtins.h"
#include	"jobs.h"
#include	"edit.h"
#include	"national.h"

#ifdef SHOPT_MULTIBYTE
    const char e_version[]	= "\n@(#)$Id: Version M 1993-12-28 k+ $\0\n";
#else
    const char e_version[]	= "\n@(#)$Id: Version 1993-12-28 k+ $\0\n";
#endif /* SHOPT_MULTIBYTE */

/* error messages */
const char e_timewarn[]		= "\r\n\ashell will timeout in 60 seconds due to inactivity";
const char e_runvi[]		= "\\hist -e \"${VISUAL:-${EDITOR:-vi}}\" ";
const char e_timeout[]		= "timed out waiting for input";
const char e_mailmsg[]		= "you have mail in $_";
const char e_query[]		= "no query process";
const char e_history[]		= "no history file";
const char e_histopen[]		= "history file cannot open";
const char e_option[]		= "%s: bad option(s)";
const char e_toomany[]		= "open file limit exceeded";
const char e_argtype[]		= "invalid argument of type %c";
const char e_formspec[]		= "%c: unknown format specifier";
const char e_badregexp[]	= "%s: invalid regular expression";
const char e_number[]		= "%s: bad number";
const char e_badlocale[]	= "%s: unknown locale";
const char e_nullset[]		= "%s: parameter null";
const char e_notset[]		= "%s: parameter not set";
const char e_noparent[]		= "%s: no parent";
const char e_subst[]		= "%s: bad substitution";
const char e_create[]		= "%s: cannot create";
const char e_tmpcreate[]	= "cannot create temporary file";
const char e_restricted[]	= "%s: restricted";
const char e_pexists[]		= "process already exists";
const char e_exists[]		= "%s: file already exists";
const char e_pipe[]		= "cannot create pipe";
const char e_alarm[]		= "cannot set alarm";
const char e_open[]		= "%s: cannot open";
const char e_logout[]		= "Use 'exit' to terminate this shell";
const char e_exec[]		= "%s: cannot execute";
const char e_pwd[]		= "cannot access parent directories";
const char e_found[]		= "%s: not found";
const char e_subscript[]	= "%s: subscript out of range";
const char e_toodeep[]		= "%s: recursion too deep";
const char e_access[]		= "permission denied";
#ifdef _cmd_universe
    const char e_nouniverse[]	= "universe not accessible";
#endif /* _cmd_universe */
const char e_direct[]		= "bad directory";
const char e_file[]		= "%s: bad file unit number";
const char e_trap[]		= "%s: bad trap";
const char e_readonly[]		= "%s: is read only";
const char e_badfield[]		= "%d: negative field size";
const char e_ident[]		= "%s: is not an identifier";
const char e_badname[]		= "%s: invalid name";
const char e_varname[]		= "%s: invalid variable name";
const char e_funname[]		= "%s: invalid function name";
const char e_aliname[]		= "%s: invalid alias name";
const char e_badexport[]	= "%s: invalid export name";
const char e_badref[]		= "%s: reference variable cannot be an array";
const char e_noref[]		= "%s: no reference name";
const char e_selfref[]		= "%s: invalid self reference";
const char e_noalias[]		= "%s: alias not found\n";
const char e_format[]		= "%s: bad format";
const char e_nolabels[]		= "%s: label not implemented";
const char e_notimp[]		= "%s: not implemented";
const char e_nosupport[]	= "not supported";
const char e_badrange[]		= "%d-%d: invalid range";
const char e_eneedsarg[]	= "-e - requires single argument";
const char e_badbase[]		= "%s unknown base";
const char e_loop[]		= "%s: would cause loop";
const char e_overlimit[]	= "%s: limit exceeded";
const char e_badsyntax[]	= "incorrect syntax";
const char e_badwrite[]		= "write to %d failed";
const char e_on	[]		= "on";
const char e_off[]		= "off";
const char is_reserved[]	= " is a keyword";
const char is_builtin[]		= " is a shell builtin";
const char is_builtver[]	= "is a shell builtin version of";
const char is_alias[]		= "%s is an alias for ";
const char is_xalias[]		= "%s is an exported alias for ";
const char is_talias[]		= "is a tracked alias for";
const char is_function[]	= " is a function";
const char is_xfunction[]	= " is an exported function";
const char is_ufunction[]	= " is an undefined function";
#ifdef JOBS
#   ifdef SIGTSTP
	const char e_newtty[]	= "Switching to new tty driver...";
	const char e_oldtty[]	= "Reverting to old tty driver...";
	const char e_no_start[]	= "Cannot start job control";
#   endif /*SIGTSTP */
    const char e_no_jctl[]	= "No job control";
    const char e_terminate[]	= "You have stopped jobs";
    const char e_done[]		= " Done";
    const char e_nlspace[]	= "\n      ";
    const char e_running[]	= " Running";
    const char e_ambiguous[]	= "%s: Ambiguous";
    const char e_jobsrunning[]	= "You have running jobs";
    const char e_no_job[]	= "no such job";
    const char e_no_proc[]	= "no such process";
    const char e_jobusage[]	= "%s: Arguments must be %%job or process ids";
#endif /* JOBS */
const char e_coredump[]		= "(coredump)";
const char e_alphanum[]		= "[_[:alpha:]]*([_[:alnum:]])";
const char e_devfdNN[]		= "/dev/fd/+([0-9])";
const char e_signo[]		= "Signal %d";
#ifdef SHOPT_FS_3D
    const char e_cantget[]	= "cannot get %s";
    const char e_cantset[]	= "cannot set %s";
    const char e_mapping[]	= "mapping";
    const char e_versions[]	= "versions";
#endif /* SHOPT_FS_3D */

/* string constants */
const char e_heading[]		= "Current option settings";
const char e_sptbnl[]		= " \t\n";
const char e_defpath[]		= "/bin:/usr/bin:";
const char e_defedit[]		= "/bin/ed";
const char e_unknown []		= "<command unknown>";
const char e_devnull[]		= "/dev/null";
const char e_traceprompt[]	= "+ ";
const char e_supprompt[]	= "# ";
const char e_stdprompt[]	= "$ ";
const char e_profile[]		= "${HOME:-.}/.profile";
const char e_sysprofile[]	= "/etc/profile";
const char e_suidprofile[]	= "/etc/suid_profile";
const char e_crondir[]		= "/usr/spool/cron/atjobs";
const char e_prohibited[]	= "login setuid/setgid shells prohibited";
#ifdef SHOPT_SUID_EXEC
   const char e_suidexec[]	= "/etc/suid_exec";
#endif /* SHOPT_SUID_EXEC */
const char hist_fname[]		= "/.sh_history";
const char e_dot[]		= ".";
const char e_envmarker[]	= "A__z";
const char e_real[]		= "\nreal";
const char e_user[]		= "user";
const char e_sys[]		= "sys";
const char e_dict[]		= "libshell";
