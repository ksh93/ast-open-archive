/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1989-2004 AT&T Corp.                  *
*                      and is licensed under the                       *
*          Common Public License, Version 1.0 (the "License")          *
*                        by AT&T Corp. ("AT&T")                        *
*      Any use, downloading, reproduction or distribution of this      *
*      software constitutes acceptance of the License.  A copy of      *
*                     the License is available at                      *
*                                                                      *
*         http://www.research.att.com/sw/license/cpl-1.0.html          *
*         (with md5 checksum 8a5e0081c856944e76c69a1cf29c2e8b)         *
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
 * probe - generate/install/display language processor probe information
 *
 * three installation possibilities, from most inter-user sharing to least
 *
 *	(1) $INSTALLROOT/lib/probe/probe set-uid to the owner of the
 *	    probe hierarchy
 *	(2) probe hierarchy owned by the caller geteuid() (e.g., FAT fs)
 *	(3) otherwise info maintained per-user in $HOME/.probe/
 */

static const char usage[] =
"[-?\n@(#)$Id: probe (AT&T Labs Research) 2002-10-30 $\n]"
USAGE_LICENSE
"[+NAME?probe - generate/install/display language processor probe information]"
"[+DESCRIPTION?\bprobe\b generates, installs and displays on the standard"
"	output probe information for the \alanguage\a programming language"
"	with respect to the \atool\a command as implemented by the"
"	\aprocessor\a command. The probe information is in"
"	the \atool\a command syntax. In general the commands that rely on"
"	probe information (re)generate the information when it is out of date"
"	(see \b--generate\b), so direct control is not usually"
"	necessary. However, not all changes to \aprocessor\a that"
"	would affect the probe information are detected by this mechanism;"
"	such changes require manual intervention (see \b--delete\b and"
"	\b--override\b.) A probe information file (see \b--key\b) with"
"	write mode enabled signifies that manual changes have been made"
"	and disables automatic regeneration (see \b--generate\b.)]"
"[+?\bprobe\b information is kept in a file with a pathname based on a"
"	hash of \alanguage\a, \atool\a, and \aprocessor\a. The information"
"	is generated by a \bsh\b(1) script (\aprobe script\a) specific to"
"	each \alanguage-processor\a pair. Any options that change the"
"	semantics of the language handled by \aprocessor\a should be included"
"	in the \aprocessor\a operand. e.g., if \bcc -++\b processes C++ files,"
"	then \aprocessor\a should be \bcc -++\b, not \bcc\b.]"
"[+?\bprobe\b generation may take a few minutes on some systems or for"
"	some \aprocessors\a. Information is shared between users as much"
"	as possible. If the \b--key\b option produces a path under your"
"	\b$HOME\b directory then the probe installation does not support"
"	sharing and each user will have private copies of the generated"
"	information.]"
"[+?For some \alanguage\a/\atool\a combinations, the file"
"	\blib/probe/\b\alanguage\a\b/\b\atool\a\b/probe.lcl\b, if it"
"	exists in the same directory as the \bprobe\b script, is sourced"
"	(via the \b.\b \bsh\b(1) command) before the probe variables are"
"	emitted. The emitted values may be modified by assigning appropriate"
"	\bsh\b variables (non-identifier name characters converted"
"	to \b_\b.) Additional non-standard values may be written directly"
"	to the standard output.]"
"[+?For all \alanguage\a/\atool\a combinations, the file"
"	\blib/probe/\b\alanguage\a\b/\b\atool\a\b/probe.ini\b, if it"
"	exists in the same directory as the \bprobe\b script, is sourced"
"	(via the \b.\b \bsh\b(1) command) before the \bprobe\b script proper."
"	\bprobe.ini\b may completely override the \bprobe\b script by"
"	exiting (presumably after printing its own values on the"
"	standard output.)]"
"[+?Sometimes it is necessary to maintain private \bprobe\b information"
"	for some processors or tools. See \b--override\b for details.]"

"[a:attributes?List probe attribute definitions.]"
"[d:delete?Delete the current information. Information can only be deleted by"
"	the generating user.]"
"[g:generate?Generate the probe information if it is out of date. Only probe"
"	information files with write mode disabled can be regenerated.]"
"[k:key?List the probe key path name on the standard output.]"
"[l:list?List the probe information on the standard output. An error occurs"
"	if the information has not been generated, unless \b--generate\b"
"	is also specified.]"
"[o:override?Make a writable copy of the probe information file in the"
"	directory \abindir\a\b/../lib/probe/\b\alang\a/\atool\a and list the"
"	generated file pathname on the standard output. If \b--key\b is also"
"	specified then the override path is listed but not created. When"
"	\bprobe\b-based commands are run with \abindir\a in \b$PATH\b before"
"	the bin directory of the standard probe information the override"
"	information will be used. Note that since the override file is user"
"	writable automatic regeneration is disabled.]:[bindir]"
"[s:silent?By default a message is printed on the standard error when probe"
"	generation starts; \b--silent\b disables this message.]"
"[t:test?Start probe generation but do not save the results. Used for"
"	debugging probe scripts.]"
"[v:verify?Each probe information file contains a comment (in the"
"	\atool\a syntax) that can be used to verify the contents. This"
"	option does that verification.]"
"[D:debug?Start probe generation, do not save the results, and write the"
"	probe script trace (see \bsh\b(1) \b-x\b) to the standard error.]"

"\n"
"\nlanguage tool processor\n"
"\n"
"[+FILES]{"
"	[+lib/probe/\alanguage\a/\atool\a?\atool\a specific information"
"		directory for \alanguage\a processors.]"
"	[+$HOME/.probe?Per-user directory when \bprobe\b is installed"
"		non-set-uid or the probe directory is on a readonly"
"		filesystem.]"
"}"
"[+CAVEATS?To allow probe information to be generated and shared among all"
"	users the executable \alib/probe/probe\a must be set-uid to the owner"
"	of the \alib/probe\a directory hierarchy and the probe directory"
"	filesystem must be mounted read-write.]"
"[+?Automatic language processor probing is mostly black magic, but then"
"	so are most language processor implementations.]"

"[+SEE ALSO?\bcpp\b(1), \bnmake\b(1), \bpathkey\b(3), \bpathprobe\b(3)]"
;

#include <ast.h>
#include <ctype.h>
#include <error.h>
#include <ls.h>
#include <preroot.h>
#include <proc.h>
#include <sig.h>
#include <times.h>

#define BASE_MAX	14

#define COMMAND		"probe"

#define ATTRIBUTES	(1<<0)
#define DELETE		(1<<1)
#define GENERATE	(1<<2)
#define KEY		(1<<3)
#define LIST		(1<<4)
#define OVERRIDE	(1<<5)
#define SILENT		(1<<6)
#define TEST		(1<<7)
#define TRACE		(1<<8)
#define VERIFY		(1<<9)

static char*	tmp;

static int	signals[] =	/* signals caught by interrupt()	*/
{
	SIGINT,
	SIGTERM,
#ifdef SIGALRM
	SIGALRM,
#endif
#ifdef SIGHUP
	SIGHUP,
#endif
#ifdef SIGQUIT
	SIGQUIT,
#endif
};

#if defined(ST_RDONLY) || defined(ST_NOSUID)

/*
 * return non-0 if path is in a readonly or non-setuid fs
 */

static int
rofs(const char* path)
{
	struct statvfs	vfs;

	if (!statvfs(path, &vfs))
	{
#if defined(ST_RDONLY)
		if (vfs.f_flag & ST_RDONLY)
			return 1;
#endif
#if defined(ST_NOSUID)
		if (vfs.f_flag & ST_NOSUID)
			return 1;
#endif
	}
	return 0;
}

#else

#define rofs(p)		0

#endif

/*
 * check path for old processor name clash
 * if old!=0 then file path compared with file old
 * 0 returned if clash or file path the same as old
 */

static int
verify(char* path, char* old, char* processor, int must)
{
	char*	ns;
	char*	os;
	int	nz;
	int	oz;
	int	r;
	Sfio_t*	nf;
	Sfio_t*	of;

	r = 0;
	if (nf = sfopen(NiL, path, "r"))
	{
		if ((ns = sfgetr(nf, '\n', 1)) && (nz = sfvalue(nf) - 1) > 0)
		{
			ns += nz;
			if ((oz = strlen(processor)) <= nz && !strcmp(processor, ns - oz))
				r = 1;
			else
				error(2, "%s: %s clashes with %s", path, processor, ns - nz);
		}
		if (r && old && sfseek(nf, 0L, 0) == 0 && (of = sfopen(NiL, old, "r")))
		{
			for (;;)
			{
				ns = sfreserve(nf, 0, 0);
				nz = sfvalue(nf);
				os = sfreserve(of, 0, 0);
				oz = sfvalue(nf);
				if (nz <= 0 || oz <= 0)
					break;
				if (nz > oz)
					nz = oz;
				if (memcmp(ns, os, nz))
					break;
				nz = sfread(nf, ns, nz);
				oz = sfread(of, os, nz);
				if (!nz || !oz)
					break;
			}
			sfclose(of);
			if (!nz && !oz && !touch(old, (time_t)-1, (time_t)-1, 0))
				r = 0;
		}
		sfclose(nf);
	}
	else if (must)
		error(ERROR_SYSTEM|2, "%s: cannot read", path);
	return r;
}

/*
 * yes this is sleazy
 */

static void
interrupt(int sig)
{
	if (tmp)
		remove(tmp);
	signal(sig, SIG_DFL);
	kill(getpid(), sig);
	pause();
	/*NOTREACHED*/
}

int
main(int argc, char** argv)
{
	register int	n;
	register char*	base;
	register char*	path;
	char*		script;
	char*		probe;
	char*		language;
	char*		tool;
	char*		processor;
	char*		sentinel;
	char*		s;
	char*		v;
	char*		override;
	int		i;
	int		suid;
	int		options = 0;
	Proc_t*		pp;
	Sfio_t*		fp;
	Sfio_t*		pf;
	char		attributes[PATH_MAX];
	char		cmd[PATH_MAX];
	char		key[BASE_MAX + 1];
	char*		cmdargv[5];
	char*		cmdenvv[2];
	struct stat	ps;
	struct stat	vs;
	unsigned long	ptime;

	NoP(argc);
	error_info.id = probe = COMMAND;
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'a':
			options |= ATTRIBUTES;
			continue;
		case 'd':
			options |= DELETE;
			continue;
		case 'g':
			options |= GENERATE;
			continue;
		case 'k':
			options |= KEY;
			continue;
		case 'l':
			options |= LIST;
			continue;
		case 'o':
			options |= OVERRIDE;
			override = opt_info.arg;
			continue;
		case 's':
			options |= SILENT;
			continue;
		case 't':
			options |= TEST;
			continue;
		case 'v':
			options |= VERIFY;
			continue;
		case 'D':
			options |= TRACE;
			continue;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			break;
		case ':':
			error(2, "%s", opt_info.arg);
			break;
		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors || !(language = *argv++) || !(tool = *argv++) || !(processor = *argv++) || *argv)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	if (*processor != '/')
	{
		if (s = strchr(processor, ' '))
		{
			n = s - processor;
			processor = strncpy(attributes, processor, n);
			*(processor + n) = 0;
		}
		v = processor;
		if (!(processor = pathpath(cmd, processor, NiL, PATH_ABSOLUTE|PATH_REGULAR|PATH_EXECUTE)))
			error(3, "%s: processor not found", v);
		if (s)
			strcpy(processor + strlen(processor), s);
	}
#if FS_PREROOT
	if (path = getpreroot(NiL, processor))
		setpreroot(NiL, path);
#endif
	if (!(path = pathprobe(NiL, attributes, language, tool, processor, (options & TEST) ? -3 : -2)))
		error(3, "cannot generate probe key");
	if (stat(path, &ps))
	{
		ps.st_mtime = 0;
		ps.st_mode = 0;
	}
	if ((options & TEST) || !(ps.st_mode & S_IWUSR))
	{
		/*
		 * verify the hierarchy before any ops
		 */

		if (!(base = strrchr(path, '/')) || strlen(++base) > BASE_MAX)
			error(3, "%s: invalid probe path", path);
		strcpy(key, base);
		strcpy(base, "../../");
		strcpy(base + 6, probe);
		if (stat(path, &vs))
			error(ERROR_SYSTEM|3, "%s: not found", path);
		if ((vs.st_mode & S_ISUID) && !rofs(path))
		{
			suid = (n = geteuid()) != getuid();
			if (suid && vs.st_uid != n)
				error(3, "%s: effective uid mismatch", path);
			strcpy(base, ".");
			if (stat(path, &vs))
				error(ERROR_SYSTEM|3, "%s: not found", path);
			if (suid && vs.st_uid != n)
				error(3, "%s: effective uid mismatch", path);
		}
		else
			suid = -1;
		strcpy(base, probe);
		if (stat(path, &vs))
			error(ERROR_SYSTEM|3, "%s: not found", path);
		ptime = vs.st_mtime;
		n = strlen(path);
		if (!(script = newof(0, char, n, 5)))
			error(ERROR_SYSTEM|3, "out of space");
		strcpy(script, path);
		strcpy(script + n, ".ini");
		if (!stat(script, &vs) && vs.st_size && ptime < (unsigned long)vs.st_mtime)
			ptime = vs.st_mtime;
		*(script + n) = 0;
		if (suid >= 0)
			strcpy(base, key);
		else if (!(path = pathprobe(NiL, attributes, language, tool, processor, -1)))
			error(3, "cannot generate probe key");
		else
		{
			if (stat(path, &ps))
			{
				ps.st_mtime = 0;
				ps.st_mode = 0;
			}
			if (!(base = strrchr(path, '/')) || strlen(++base) > BASE_MAX)
				error(3, "%s: invalid probe path", path);
			strcpy(key, base);
		}
	}

	/*
	 * ops ok now
	 */

	if (options & DELETE)
	{
		if (lstat(processor, &vs) || getuid() != vs.st_uid && (!ps.st_mode || getgid() != ps.st_gid))
		{
			/*
			 * request not by owner of processor
			 * so limit to probe owner
			 */

			setuid(getuid());
			setgid(getgid());
		}
		if (remove(path))
			error(ERROR_SYSTEM|3, "%s: cannot delete", path);
	}
	else
	{
		if (!(options & TEST) && ps.st_mode && (ptime <= (unsigned long)ps.st_mtime || ptime <= (unsigned long)ps.st_ctime || (ps.st_mode & S_IWUSR)))
		{
			if (ptime <= (unsigned long)ps.st_mtime || ptime <= (unsigned long)ps.st_ctime)
			{
				if (!options)
					error(1, "%s: information already generated", path);
			}
			else if (!(options & OVERRIDE) && (ps.st_mode & S_IWUSR))
				error(0, "%s probe information for %s language processor %s must be manually regenerated", tool, language, processor);
		}
		else if ((options & (GENERATE|TEST)) || !(options & ~(GENERATE|SILENT|TRACE)))
		{
			/*
			 * recursion check using environment sentinel
			 */

			n = strlen(probe) + strlen(key) + strlen(processor) + 2;
			if (!(sentinel = newof(0, char, n, 0)))
				error(ERROR_SYSTEM|3, "out of space");
			sfsprintf(sentinel, n, "%s%s", probe, key);
			for (s = sentinel; *s; s++)
				if (!isalnum(*s))
					*s = '_';
			if ((v = getenv(sentinel)) && !strcmp(v, processor))
				error(3, "%s: recursive probe", processor);
			sfsprintf(s, n - (s - sentinel), "=%s", processor);

			/*
			 * generate the new info in a tmp file
			 */

			if (suid < 0)
			{
				strcpy(base, ".");
				if (access(path, F_OK))
					for (s = path + (*path == '/'); s = strchr(s, '/'); *s++ = '/')
					{
						*s = 0;
						if (access(path, F_OK) && mkdir(path, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH))
							error(ERROR_SYSTEM|3, "%s: cannot create directory", path);
					}
			}
			if (!(options & SILENT))
				error(0, "probing %s language processor %s for %s information", language, processor, tool);
			sfsprintf(base, BASE_MAX + 1, "%s.%06d", probe, getpid());
			if (!(tmp = strdup(path)))
				error(ERROR_SYSTEM|3, "out of space");
			for (n = 0; n < elementsof(signals); n++)
				if (signal(signals[n], interrupt) == SIG_IGN)
					signal(signals[n], SIG_IGN);
			if (!(options & TEST))
			{
				if (!(fp = sfopen(NiL, tmp, "w")))
					error(ERROR_SYSTEM|3, "%s: cannot write", tmp);
				chmod(tmp, S_IRUSR|S_IRGRP|S_IROTH);
			}
			n = 0;
			cmdargv[n++] = script;
			if (options & TRACE)
				cmdargv[n++] = "-d";
			cmdargv[n++] = processor;
			cmdargv[n++] = attributes;
			cmdargv[n] = 0;
			n = 0;
			cmdenvv[n++] = sentinel;
			cmdenvv[n] = 0;
			if (!(pp = procopen(script, cmdargv, cmdenvv, NiL, PROC_UID|PROC_GID|((options&TEST)?0:PROC_READ))))
				n = -1;
			else if (!(options & TEST) && (!(pf = sfnew(NiL, NiL, SF_UNBOUND, pp->rfd, SF_READ)) || !(pp->rfd = -1) || sfmove(pf, fp, SF_UNBOUND, -1) < 0 || sfclose(pf) || sfclose(fp)))
				n = -2;
			else
				n = 0;
			i = procclose(pp);
			if (!n) 
				n = i;
			if (n < -1)
				error(ERROR_SYSTEM|2, "%s: command io error", script);
			else if (n == -1)
				error(ERROR_SYSTEM|2, "%s: command exec error", script);
			else if (n > 0)
				error(2, "%s: command exit code %d", script, n);
			else if (!(options & TEST))
			{
				/*
				 * verify and rename to the real probe key path
				 */

				sfclose(fp);
				strcpy(base, key);
				if (verify(tmp, path, processor, 1))
				{
					remove(path);
					if (!rename(tmp, path))
					{
						/*
						 * warp the file to yesterday to
						 * accomodate make installations
						 */

						if (!stat(path, &ps))
						{
							ps.st_mtime = (unsigned long)ps.st_mtime - 24 * 60 * 60;
							touch(path, ps.st_mtime, ps.st_mtime, 0);
						}
					}
					else if (!verify(path, NiL, processor, 0))
					{
						n = -1;
						error(ERROR_SYSTEM|2, "%s: cannot rename to %s", tmp, path);
					}
				}
				else
					n = -1;
			}
			if (n && !(options & TEST))
				remove(tmp);
		}
		if (!error_info.errors)
		{
			if (options & OVERRIDE)
			{
				n = 5;
				base = path + strlen(path);
				while (base > path && (*--base != '/' || --n));
				if (n || !strneq(base, "/lib/", 5) || !strneq(base + 5, probe, strlen(probe)))
					error(3, "%s: probe information already private", path);
				if (!(v = strrchr(override, '/')))
					v = override;
				sfsprintf(cmd, sizeof(cmd), "%-.*s%s", v - override, override, base);
				if (!(options & KEY))
				{
					if (!access(cmd, F_OK))
						error(3, "%s: override already generated", cmd);
					if (!(pf = sfopen(NiL, cmd, "w")))
					{
						for (s = cmd + (*cmd == '/'); s = strchr(s, '/'); *s++ = '/')
						{
							*s = 0;
							if (access(cmd, F_OK) && mkdir(cmd, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH))
								error(ERROR_SYSTEM|3, "%s: cannot create override directory", cmd);
						}
						if (!(pf = sfopen(NiL, cmd, "w")))
							error(ERROR_SYSTEM|3, "%s: cannot write", cmd);
					}
					if (!(fp = sfopen(NiL, path, "r")))
						error(ERROR_SYSTEM|3, "%s: cannot read", path);
					if (sfmove(fp, pf, SF_UNBOUND, -1) < 0 || sfclose(fp) || sfclose(pf))
						error(ERROR_SYSTEM|3, "%s: copy error", cmd);
				}
				sfprintf(sfstdout, "%s\n", cmd);
			}
			else
			{
				if (options & KEY)
					sfprintf(sfstdout, "%s\n", path);
				if (options & ATTRIBUTES)
					sfprintf(sfstdout, "%s\n", attributes);
				if (!(options & TEST))
				{
					if (options & LIST)
					{
						if (!(fp = sfopen(NiL, path, "r")))
							error(ERROR_SYSTEM|3, "%s: cannot read", path);
						sfmove(fp, sfstdout, SF_UNBOUND, -1);
						sfclose(fp);
					}
					if (options & VERIFY)
						verify(path, NiL, processor, 1);
				}
			}
		}
	}
	return error_info.errors != 0;
}
