/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1989-2000 AT&T Corp.              *
*      and it may only be used by you under license from       *
*                     AT&T Corp. ("AT&T")                      *
*       A copy of the Source Code Agreement is available       *
*              at the AT&T Internet web site URL               *
*                                                              *
*     http://www.research.att.com/sw/license/ast-open.html     *
*                                                              *
*      If you have copied this software without agreeing       *
*      to the terms of the license you are infringing on       *
*         the license and copyright and are violating          *
*             AT&T's intellectual property rights.             *
*                                                              *
*               This software was created by the               *
*               Network Services Research Center               *
*                      AT&T Labs Research                      *
*                       Florham Park NJ                        *
*                                                              *
*             Glenn Fowler <gsf@research.att.com>              *
*                                                              *
***************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * ps -- list process status
 *
 * if no /proc then we punt to /bin/ps -- and you better match their args!
 */

static const char usage[] =
"[-?\n@(#)ps (AT&T Labs Research) 2000-05-11\n]"
USAGE_LICENSE
"[+NAME?ps - report process status]"
"[+DESCRIPTION?\bps\b lists process information subject to the appropriate"
"	privilege. If \apid\a arguments are specified then only those"
"	processes are listed, otherwise all processes with the same"
"	effective user id and controlling terminal are listed. The options"
"	may alter this default behavior.]"
"[+?The listings are sorted by <\bUID,START,PID\b>. Options taking list"
"	arguments accept either space or comma separators.]"

"[a:interactive?List all processes associated with terminals.]"
"[c:class?Equivalent to \b--fields=\"pid class pri tty time command\"\b.]"
"[d:no-session?List all processes except session leaders.]"
"[D:define?Define \akey\a with optional \avalue\a. \avalue\a will be expanded"
"	when \b%(\b\akey\a\b)\b is specified in \b--format\b. \akey\a may"
"	override internal \b--format\b identifiers.]:[key[=value]]]"
"[e|A:all?List all processes.]"
"[f:full?Equivalent to \b--fields=\"user pid ppid start tty time args\"\b.]"
"[F:format?Append to the listing format string (if \b--format\b is specified"
"	then \b--fields\b and all options that modify \b--fields\b are"
"	ignored.) The \bdf\b(1), \bls\b(1) and \bpax\b(1) commands also have"
"	\b--format\b options in this same style. \aformat\a follows"
"	\bprintf\b(3) conventions, except that \bsfio\b(3) inline ids are used"
"	instead of arguments:"
"	%[#-+]][\awidth\a[.\aprecis\a[.\abase\a]]]]]](\aid\a[:\asubformat\a]])\achar\a."
"	If \b#\b is specified then the internal width and percision are used."
"	If \achar\a is \bs\b then the string form of the item is listed,"
"	otherwise the corresponding numeric form is listed. If \achar\a is"
"	\bq\b then the string form of the item is $'...' quoted if it contains"
"	space or non-printing characters. If \awidth\a is omitted then the"
"	default width is assumed. \asubformat\a overrides the default"
"	formatting for \aid\a. Supported \aid\as and \asubformat\as"
"	are:]:[format]{\fformats\f}"
"[g:pgrps|process-groups?List processes with group leaders in the \apgrp\a"
"	list.]:[pgrp...]"
"[G:groups?List processes with real group id names or numbers in the \agroup\a"
"	list.]:[group...]"
"[j:jobs?Equivalent to \b--fields=\"pid pgrp sid tty time command\"\b.]"
"[l:long?Equivalent to \b--fields=\"flags state user pid ppid pri nice"
"	size rss wchan tty time command\"\b.]"
"[n:namelist?Specifies an alternate system namelist \afile\a. Ignored by"
"	this implementation.]"
"[N:default?Equivalent to \b--fields=\"pid tty time command\"\b. This is the"
"	format when \b--fields\b is not specified.]"
"[o:fields?(\b--format\b is more general.) List information according to"
"	\aformat\a. Multiple \b--fields\b options may be specified; the"
"	resulting format is a left-right ordered list with duplicate entries"
"	deleted from the right. The default width can be overriden by"
"	appending \a+width\a to \akey\a, and the default \alabel\a can be"
"	overridden by appending \a=label\a to \akey\a. The keys, labels and"
"	widths are listed under \b--format\b.]:[key[+width]][=label]]...]"
"[p:pids?List processes in the \apid\a list.]:[pid...]"
"[s:sessions?List processes with session leaders in the \asid\a list.]:[sid...]"
"[t:terminals|ttys?List processes with controlling terminals in the \atty\a"
"	list.]:[tty...]"
"[T:tree|forest?Display the process tree hierarchy in the \bCOMMAND\b"
"	field list.]"
"[u|U:users?List processes with real user id names or numbers in the \auser\a"
"	list.]:[user...]"
"[v:verbose?List verbose error messages for inaccessible processes.]"
"[x:hex?List numeric entries in hexadecimal notation.]"

"\n"
"\n[ pid ... ]\n"
"\n"

"[+SEE ALSO?\bdf\b(1), \bkill\b(1), \bls\b(1), \bnice\b(1), \bpax\b(1),"
"	\bps\b(1), \bsh\b(1), \btop\b(1)]"
;

#include <ast.h>
#include <ast_dir.h>
#include <cdt.h>
#include <ctype.h>
#include <dirent.h>
#include <error.h>
#include <int.h>
#include <ls.h>
#include <sfdisc.h>
#include <sfstr.h>
#include <tm.h>

#include "FEATURE/procfs"

#ifdef _PS_dir

#if !_mem_st_rdev_stat
#define st_rdev			st_dev
#endif

#define KEY_environ		(-1)
#define KEY_alias		0

#define KEY_addr		1
#define KEY_args		2
#define KEY_class		3
#define KEY_command		4
#define KEY_cpu			5
#define KEY_etime		6
#define KEY_flags		7
#define KEY_gid			8
#define KEY_group		9
#define KEY_nice		10
#define KEY_ntpid		11
#define KEY_pgrp		12
#define KEY_pid			13
#define KEY_ppid		14
#define KEY_pri			15
#define KEY_refcount		16
#define KEY_rss			17
#define KEY_sid			18
#define KEY_size		19
#define KEY_start		20
#define KEY_state		21
#define KEY_tgrp		22
#define KEY_time		23
#define KEY_tty			24
#define KEY_uid			25
#define KEY_user		26
#define KEY_wchan		27

typedef struct Key_s			/* format key			*/
{
	char*		name;		/* key name			*/
	char*		head;		/* heading name			*/
	char*		desc;		/* description			*/
	short		index;		/* index			*/
	short		width;		/* field width			*/
	unsigned long	maxval;		/* max value if !=0		*/
	unsigned char	hex;		/* optional hex output		*/
	unsigned char	missing;	/* missing in system		*/
	unsigned char	already;	/* already specified		*/
	short		cancel;		/* cancel this if specified	*/
	short		prec;		/* field precision		*/
	short		disable;	/* macro being expanded		*/
	unsigned char	skip;		/* skip this			*/
	char*		macro;		/* macro value			*/
	const char*	sep;		/* next field separator		*/
	Dtlink_t	hashed;		/* hash link			*/
	struct Key_s*	next;		/* format link			*/
} Key_t;

typedef struct				/* generic id table entry	*/
{
	Dtlink_t	hashed;		/* hash link			*/
	long		id;		/* id number			*/
	char		name[1];	/* id name			*/
} Id_t;

typedef struct Ps_s			/* process state		*/
{
	Dtlink_t	hashed;		/* pid hash link		*/
	Dtlink_t	sorted;		/* sorted link			*/
	struct prpsinfo	ps;		/* ps info			*/
	struct Ps_s*	children;	/* child list			*/
	struct Ps_s*	lastchild;	/* end of children		*/
	struct Ps_s*	sibling;	/* sibling list			*/
	struct Ps_s*	root;		/* (partial) root list		*/
	char*		user;		/* user name			*/
	int		level;		/* process tree level		*/
	int		shown;		/* list state			*/
} Ps_t;

typedef struct				/* program state		*/
{
	int		all;		/* popular categories		*/
	int		controlled;	/* matching state.ttydev only	*/
	int		detached;	/* detached too			*/
	int		leader;		/* pg leaders too		*/
	int		idindex;	/* id select index		*/
	int		tree;		/* list proc tree		*/
	int		verbose;	/* verbose error messages	*/
	int		hex;		/* output optional hex key form	*/
	int		width;		/* output width			*/
	unsigned long	now;		/* current time			*/
#ifdef _PS_scan_boot
	unsigned long	boot;		/* boot time			*/
#endif
	dev_t		ttydev;		/* controlling tty		*/
	uid_t		caller;		/* caller effective uid		*/
	Key_t*		fields;		/* format field list		*/
	char*		format;		/* sfkeyprintf() format		*/
	Key_t*		lastfield;	/* end of format list		*/
	Dt_t*		keys;		/* format keys			*/
	Dt_t*		bypid;		/* procs by pid			*/
	Dt_t*		byorder;	/* procs by pid			*/
	Dt_t*		ids;		/* select by id			*/
	Dt_t*		ttys;		/* tty base name bu dev		*/
	Ps_t*		pp;		/* next proc info slot		*/
	Sfio_t*		mac;		/* temporary string stream	*/
	Sfio_t*		nul;		/* temporary string stream	*/
	Sfio_t*		tmp;		/* temporary string stream	*/
	Sfio_t*		wrk;		/* temporary string stream	*/
	char		branch[1024];	/* process tree branch		*/
	char		buf[1024];	/* work buffer			*/
} State_t;

#ifndef _mem_pr_clname_prpsinfo
#define _mem_pr_clname_prpsinfo		0
#endif

#ifndef _mem_pr_lttydev_prpsinfo
#define _mem_pr_lttydev_prpsinfo	0
#endif

#ifndef _mem_pr_ntpid_prpsinfo
#define _mem_pr_ntpid_prpsinfo		0
#endif

#ifndef _mem_pr_psargs_prpsinfo
#define _mem_pr_psargs_prpsinfo		0
#endif

#ifndef _mem_pr_refcount_prpsinfo
#define _mem_pr_refcount_prpsinfo	0
#endif

#ifndef _mem_pr_tgrp_prpsinfo
#define _mem_pr_tgrp_prpsinfo		0
#endif

static Key_t	keys[] =
{
	{
		0
	},
	{
		"addr",
		"ADDR",
		"Physical address.",
		KEY_addr,
		8
	},
	{
		"args",
		"COMMAND",
		"Command path with arguments.",
		KEY_args,
		-12, 0,
		0,0,0,
		KEY_command
	},
	{
		"class",
		"CLS",
		"Scheduling class.",
		KEY_class,
		3, 0,
		0,!_mem_pr_clname_prpsinfo
	},
	{
		"command",
		"COMMAND",
		"Command file base name.",
		KEY_command,
		-12, 0,
		0,0,0,
		KEY_args
	},
	{
		"cpu",
		"%CPU",
		"Cpu percent usage.",
		KEY_cpu,
		4
	},
	{
		"etime",
		"ELAPSED",
		"Elapsed time since start.",
		KEY_etime,
		7
	},
	{
		"flags",
		"F",
		"State flags.",
		KEY_flags,
		2
	},
	{
		"gid",
		"GROUP",
		"Numeric group id.",
		KEY_gid,
		8, 0,
		0,0,0,
		KEY_group
	},
	{
		"group",
		"GROUP",
		"Group id name.",
		KEY_group,
		8, 0,
		0,0,0,
		KEY_gid
	},
	{
		"nice",
		"NI",
		"Adjusted scheduling priority.",
		KEY_nice,
		4
	},
	{
		"ntpid",
		"NTPID",
		"NT process id.",
		KEY_ntpid,
		5, 0,
		1,!_mem_pr_ntpid_prpsinfo
	},
	{
		"pgrp",
		"PGRP",
		"Process group id.",
		KEY_pgrp,
		5, PID_MAX,
		1,0,0
	},
	{
		"pid",
		"PID",
		"Process id.",
		KEY_pid,
		5, PID_MAX,
		1,0,0
	},
	{
		"ppid",
		"PPID",
		"Parent process id.",
		KEY_ppid,
		5, PID_MAX,
		1
	},
	{
		"pri",
		"PRI",
		"Scheduling priority.",
		KEY_pri,
		3
	},
	{
		"refcount",
		"REFS",
		"NT reference count.",
		KEY_refcount,
		4, 0,
		1,!_mem_pr_refcount_prpsinfo
	},
	{
		"rss",
		"RSS",
		"Resident page set size in kilobytes.",
		KEY_rss,
		5
	},
	{
		"sid",
		"SID",
		"Session id.",
		KEY_sid,
		5, PID_MAX,
		1
	},
	{
		"size",
		"SIZE",
		"Virtual memory size in kilobytes.",
		KEY_size,
		6
	},
	{
		"start",
		"START",
		"Start time.",
		KEY_start,
		8
	},
	{
		"state",
		"S",
		"Basic state.",
		KEY_state,
		1
	},
	{
		"tgrp",
		"TGRP",
		"Terminal group id.",
		KEY_tgrp,
		5, PID_MAX,
		1,!_mem_pr_tgrp_prpsinfo
	},
	{
		"time",
		"TIME",
		"usr+sys time.",
		KEY_time,
		6
	},
	{
		"tty",
		"TT",
		"Controlling terminal base name.",
		KEY_tty,
		-7,
	},
	{
		"uid",
		"USER",
		"Numeric user id.",
		KEY_uid,
		8, 0,
		0,0,0,
		KEY_user
	},
	{
		"user",
		"USER",
		"User id name.",
		KEY_user,
		8, 0,
		0,0,0,
		KEY_uid
	},
	{
		"wchan",
		"WCHAN",
		"Wait address.",
		KEY_wchan,
		8
	},

	/* aliases after this point */

	{ "comm",	0,	0,	KEY_command		},
	{ "pcpu",	0,	0,	KEY_cpu			},
	{ "rgroup",	0,	0,	KEY_group		},
	{ "ruser",	0,	0,	KEY_user		},
	{ "tid",	0,	0,	KEY_tgrp		},
	{ "vsz",	0,	0,	KEY_size		},

};

static const char	default_format[] = "pid tty time command";
static const char	newline[] = "\n";
static const char	space[] = " ";

static State_t		state;

/*
 * optget() info discipline function
 */

static int
optinfo(Opt_t* op, Sfio_t* sp, const char* s, Optdisc_t* dp)
{
	register int	i;

	if (streq(s, "formats"))
		for (i = 1; i < elementsof(keys); i++)
		{
			sfprintf(sp, "[+%s?", keys[i].name);
			if (keys[i].head)
				sfprintf(sp, "%s The title string is \b%s\b and the default width is %d.]", keys[i].desc, keys[i].head, keys[i].width);
			else
				sfprintf(sp, "Equivalent to \b%s\b.]", keys[keys[i].index].name);
		}
	return 0;
}

/*
 * initialize the ttyid hash
 */

static void
ttyinit(void)
{
	register DIR*		dir;
	register struct dirent*	ent;
	register Id_t*		ip;
	DIR*			sub = 0;
	Dtdisc_t*		dp;
	char*			base;
	char*			name;
	struct stat		st;
	char			path[PATH_MAX];

	if (!(dp = newof(0, Dtdisc_t, 1, 0)))
		error(ERROR_SYSTEM|3, "out of space [tty-disc]");
	dp->key = offsetof(Id_t, id);
	dp->size = sizeof(long);
	dp->link = offsetof(Id_t, hashed);
	if (!(state.ttys = dtopen(dp, Dthash)))
		error(ERROR_SYSTEM|3, "out of space [tty-dict]");
	strcpy(path, "/dev");
	if (!(dir = opendir(path)))
	{
		error(ERROR_SYSTEM|2, "%s: cannot read", path);
		return;
	}
	path[4] = '/';
	name = base = path + 5;
	for (;;)
	{
		while (ent = readdir(dir))
		{
			if (D_NAMLEN(ent) + (base - path) + 1 > sizeof(path))
				continue;
			if (!sub && (ent->d_name[0] != 'c' && ent->d_name[0] != 't' && ent->d_name[0] != 'p' && ent->d_name[0] != 'v' || ent->d_name[1] != 'o' && ent->d_name[1] != 't'))
				continue;
			strcpy(base, ent->d_name);
			if (stat(path, &st))
				continue;
			if (!S_ISCHR(st.st_mode))
			{
				if (sub || !S_ISDIR(st.st_mode))
					continue;
				sub = dir;
				if (dir = opendir(path))
				{
					base = path + strlen(path);
					*base++ = '/';
				}
				else
				{
					dir = sub;
					sub = 0;
				}
				continue;
			}
			if (!(ip = newof(0, Id_t, 1, strlen(name))))
				error(ERROR_SYSTEM|3, "out of space [tty-entry]");
			strcpy(ip->name, name);
			ip->id = st.st_rdev;
			dtinsert(state.ttys, ip);
		}
		if (!sub)
			break;
		closedir(dir);
		dir = sub;
		sub = 0;
		base = name;
	}
	closedir(dir);
}

/*
 * return device id given tty base name
 */

static int
ttyid(const char* name)
{
	register const char*	s;
	register Id_t*		ip;
	long			v;
	struct stat		st;

	s = name;
	if (stat(s, &st))
	{
		sfprintf(state.tmp, "/dev/%s", name);
		s = (const char*)sfstruse(state.tmp);
		if (stat(s, &st))
		{
			sfprintf(state.tmp, "/dev/tty%s", name);
			s = (const char*)sfstruse(state.tmp);
			if (stat(s, &st))
			{
				error(ERROR_SYSTEM|2, "%s: unknown tty", name);
				return -1;
			}
		}
	}
	if (!state.ttys)
		ttyinit();
	v = st.st_rdev;
	if (!(ip = (Id_t*)dtmatch(state.ttys, &v)))
	{
		if (!(ip = newof(0, Id_t, 1, strlen(s))))
			error(ERROR_SYSTEM|3, "out of space [tty]");
		strcpy(ip->name, s);
		ip->id = v;
		dtinsert(state.ttys, ip);
	}
	return st.st_rdev;
}

/*
 * return tty base name given tty device id
 */

static char*
ttybase(dev_t dev)
{
	register Id_t*	ip;
	long		v;

	static char	name[32];

	if (dev == (dev_t)PRNODEV)
		return "?";
	if (!state.ttys)
		ttyinit();
	v = dev;
	if (ip = (Id_t*)dtmatch(state.ttys, &v))
		return ip->name;
	sfsprintf(name, sizeof(name), "%03d,%03d", major(dev), minor(dev));
	return name;
}

/*
 * sfkeyprintf() lookup
 * handle==0 for heading
 */

static int
key(void* handle, register Sffmt_t* fp, const char* arg, char** ps, Sflong_t* pn)
{
	register Ps_t*		pp = (Ps_t*)handle;
	register char*		s = 0;
	register Sflong_t	n = 0;
	register Key_t*		kp;
	register int		i;
	int			j;
	unsigned long		u;

	static char		sbuf[2];

	if (!(kp = (Key_t*)dtmatch(state.keys, fp->t_str)))
	{
		if (*fp->t_str != '$')
		{
			error(3, "%s: unknown format key", fp->t_str);
			return 0;
		}
		if (!(kp = newof(0, Key_t, 1, strlen(fp->t_str) + 1)))
			error(3, "out of space [key]");
		kp->name = strcpy((char*)(kp + 1), fp->t_str);
		kp->macro = getenv(fp->t_str + 1);
		kp->index = KEY_environ;
		kp->disable = 1;
		dtinsert(state.keys, kp);
	}
	if (kp->macro && !kp->disable)
	{
		kp->disable = 1;
		sfkeyprintf(state.mac, handle, kp->macro, key, NiL);
		s = sfstruse(state.mac);
		kp->disable = 0;
	}
	else if (!pp)
	{
		if (fp->flags & SFFMT_ALTER)
		{
			if (kp->maxval)
			{
				for (i = 1; kp->maxval /= 10; i++);
				if (kp->width < 0)
				{
					i = -i;
					if (kp->width > i)
						kp->width = i;
				}
				else if (kp->width < i)
					kp->width = i;
			}
			if ((fp->width = kp->width) < 0)
			{
				fp->width = -fp->width;
				fp->flags |= SFFMT_LEFT;
			}
			fp->precis = fp->width;
		}
		kp->width = fp->width;
		if (fp->flags & SFFMT_LEFT)
			kp->width = -kp->width;
		fp->fmt = 's';
		*ps = kp->head;
	}
	else
	{
		if ((fp->flags & SFFMT_ALTER) && (fp->width = kp->width) < 0)
		{
			fp->width = -fp->width;
			fp->flags |= SFFMT_LEFT;
		}
		switch (kp->index)
		{
		case KEY_addr:
			if (PR_ZOMBIE(&pp->ps))
				goto zombie;
			n = (long)pp->ps.pr_addr;
			goto hex;
#if _mem_pr_clname_prpsinfo
		case KEY_class:
			if (PR_ZOMBIE(&pp->ps))
				goto zombie;
			s = pp->ps.pr_clname;
			break;
#endif
		case KEY_args:
#if _mem_pr_psargs_prpsinfo
			s = PR_ZOMBIE(&pp->ps) ? "<defunct>" : pp->ps.pr_psargs;
			goto branch;
#endif
#ifdef _PS_args
			sfprintf(state.tmp, _PS_path_num, pp->ps.pr_pid, _PS_args);
			if ((i = open(sfstruse(state.tmp), O_RDONLY)) >= 0)
			{
				n = read(i, state.tmp->data, state.tmp->size - 1);
				close(i);
				if (n > 0)
				{
					s = state.tmp->data;
					for (i = 0; i < n; i++)
						if (!s[i])
							s[i] = ' ';
					s[i] = 0;
					goto branch;
				}
			}
#endif
		case KEY_command:
			s = PR_ZOMBIE(&pp->ps) ? "<defunct>" : pp->ps.pr_fname;
#if _mem_pr_psargs_prpsinfo || defined(_PS_args)
		branch:
#endif
			if (s[0] == '(' && s[i = strlen(s) - 1] == ')')
			{
				s[i] = 0;
				s++;
			}
			if ((j = pp->level) > 0)
			{
				for (i = 0, j--; i < j; i++)
					sfputr(state.wrk, state.branch[i] ? " |  " : "    ", -1);
				sfputr(state.wrk, " \\_ ", -1);
				sfputr(state.wrk, s, -1);
				s = sfstruse(state.wrk);
			}
			break;
		case KEY_cpu:
			if (PR_ZOMBIE(&pp->ps))
				goto zombie;
			n = pp->ps.pr_cpu;
			goto percent;
		case KEY_etime:
			s = fmtelapsed(state.now - (unsigned long)PR_START(&pp->ps), 1);
			break;
		case KEY_flags:
			n = pp->ps.pr_flag & 0xff;
			goto hex;
		case KEY_gid:
			n = pp->ps.pr_gid;
			goto number;
		case KEY_group:
			s = fmtgid(pp->ps.pr_gid);
			break;
		case KEY_nice:
			if (PR_ZOMBIE(&pp->ps))
				goto zombie;
			n = pp->ps.pr_nice;
			goto number;
#if _mem_pr_ntpid_prpsinfo
		case KEY_ntpid:
			n = pp->ps.pr_ntpid;
			goto hex;
#endif
		case KEY_pgrp:
			n = pp->ps.pr_pgrp;
			goto number;
		case KEY_pid:
			n = pp->ps.pr_pid;
			goto number;
		case KEY_ppid:
			n = pp->ps.pr_ppid;
			break;
		case KEY_pri:
			if (PR_ZOMBIE(&pp->ps))
				goto zombie;
			n = pp->ps.pr_pri;
			goto number;
#if _mem_pr_refcount_prpsinfo
		case KEY_refcount:
			n = pp->ps.pr_refcount;
			goto number;
#endif
		case KEY_rss:
			if (PR_ZOMBIE(&pp->ps))
				goto zombie;
			n = pp->ps.pr_rssize;
			goto number;
		case KEY_sid:
			n = pp->ps.pr_sid;
			goto number;
		case KEY_size:
			if (PR_ZOMBIE(&pp->ps))
				goto zombie;
			n = pp->ps.pr_size;
			goto number;
		case KEY_start:
			if (PR_ZOMBIE(&pp->ps))
				goto zombie;
			u = PR_START(&pp->ps);
			s = fmttime((state.now - u) >= (24 * 60 * 60) ? "%y-%m-%d" : "%H:%M:%S", u);
			break;
		case KEY_state:
			*(s = sbuf) = pp->ps.pr_sname;
			*(s + 1) = 0;
			break;
#if _mem_pr_tgrp_prpsinfo
		case KEY_tgrp:
			n = pp->ps.pr_tgrp;
			goto number;
#endif
		case KEY_time:
			s = fmtelapsed(PR_TIME(&pp->ps), PR_HZ);
			break;
		case KEY_tty:
			if (PR_ZOMBIE(&pp->ps))
				goto zombie;
			s = ttybase(pp->ps.pr_ttydev);
			if ((i = strlen(s) - kp->prec) > 0)
			{
				if (s[0] == 'p' && s[1] == 't')
				{
					if (s[2] == 'y')
						s += 3;
					else
						s += 2;
				}
				else if (s[0] == 't' && s[1] == 't' && s[2] == 'y')
					s += 3;
				else
					s += i;
			}
			break;
		case KEY_uid:
			n = pp->ps.pr_uid;
			goto number;
		case KEY_user:
			s = pp->user;
			break;
		case KEY_wchan:
			if (PR_ZOMBIE(&pp->ps))
				goto zombie;
			n = (long)pp->ps.pr_wchan;
			goto hex;
		default:
			return 0;
		zombie:
			s = "";
			break;
		percent:
			sfprintf(state.tmp, "%%%ld", n);
			s = sfstruse(state.tmp);
			break;
		number:
			if (!state.hex || !kp->hex)
				break;
		hex:
			fp->fmt = 'x';
			if (!kp->hex)
				fp->flags |= SFFMT_ZERO;
			break;
		}
		if (s)
			*ps = s;
		else
			*pn = n;
	}
	return 1;
}

/*
 * ps a single proc
 */

static void
ps(Ps_t* pp)
{
	register Key_t*			kp;
	register struct prpsinfo*	pr;
	register char*			s;
	register int			i;
	register long			n;
	unsigned long			u;
	int				j;
	char				sbuf[2];

	pp->shown = 1;
	if (state.format)
	{
		sfkeyprintf(sfstdout, pp, state.format, key, NiL);
		return;
	}
	pr = &pp->ps;
	for (kp = state.fields; kp; kp = kp->next)
	{
		switch (kp->index)
		{
		case KEY_addr:
			if (PR_ZOMBIE(pr))
				goto zombie;
			n = (long)pr->pr_addr;
			goto hex;
#if _mem_pr_clname_prpsinfo
		case KEY_class:
			if (PR_ZOMBIE(pr))
				goto zombie;
			s = pr->pr_clname;
			goto string;
#endif
		case KEY_args:
#if _mem_pr_psargs_prpsinfo
			s = PR_ZOMBIE(pr) ? "<defunct>" : pr->pr_psargs;
			goto branch;
#endif
#ifdef _PS_args
			sfprintf(state.tmp, _PS_path_num, pr->pr_pid, _PS_args);
			if ((i = open(sfstruse(state.tmp), O_RDONLY)) >= 0)
			{
				n = read(i, state.tmp->data, state.tmp->size - 1);
				close(i);
				if (n > 0)
				{
					s = state.tmp->data;
					for (i = 0; i < n; i++)
						if (!s[i])
							s[i] = ' ';
					s[i] = 0;
					goto branch;
				}
			}
#endif
		case KEY_command:
			s = PR_ZOMBIE(pr) ? "<defunct>" : pr->pr_fname;
#if _mem_pr_psargs_prpsinfo || defined(_PS_args)
		branch:
#endif
			if (s[0] == '(' && s[i = strlen(s) - 1] == ')')
			{
				s[i] = 0;
				s++;
			}
			if ((j = pp->level) > 0)
			{
				for (i = 0, j--; i < j; i++)
					sfputr(sfstdout, state.branch[i] ? " |  " : "    ", -1);
				sfputr(sfstdout, " \\_ ", -1);
			}
			goto string;
		case KEY_cpu:
			if (PR_ZOMBIE(pr))
				goto zombie;
			n = pr->pr_cpu;
			goto percent;
		case KEY_etime:
			s = fmtelapsed(state.now - (unsigned long)PR_START(pr), 1);
			goto string;
		case KEY_flags:
			n = pr->pr_flag & 0xff;
			goto hex;
		case KEY_gid:
			n = pr->pr_gid;
			goto number;
		case KEY_group:
			s = fmtgid(pr->pr_gid);
			goto string;
		case KEY_nice:
			if (PR_ZOMBIE(pr))
				goto zombie;
			n = pr->pr_nice;
			goto number;
#if _mem_pr_ntpid_prpsinfo
		case KEY_ntpid:
			n = pr->pr_ntpid;
			goto hex;
#endif
		case KEY_pgrp:
			n = pr->pr_pgrp;
			goto number;
		case KEY_pid:
			n = pr->pr_pid;
			goto number;
		case KEY_ppid:
			n = pr->pr_ppid;
			goto number;
		case KEY_pri:
			if (PR_ZOMBIE(pr))
				goto zombie;
			n = pr->pr_pri;
			goto number;
#if _mem_pr_refcount_prpsinfo
		case KEY_refcount:
			n = pr->pr_refcount;
			goto number;
#endif
		case KEY_rss:
			if (PR_ZOMBIE(pr))
				goto zombie;
			n = pr->pr_rssize;
			goto number;
		case KEY_sid:
			n = pr->pr_sid;
			goto number;
		case KEY_size:
			if (PR_ZOMBIE(pr))
				goto zombie;
			n = pr->pr_size;
			goto number;
		case KEY_start:
			if (PR_ZOMBIE(pr))
				goto zombie;
			u = PR_START(pr);
			s = fmttime((state.now - u) >= (24 * 60 * 60) ? "%y-%m-%d" : "%H:%M:%S", u);
			goto string;
		case KEY_state:
			*(s = sbuf) = pr->pr_sname;
			*(s + 1) = 0;
			goto string;
#if _mem_pr_tgrp_prpsinfo
		case KEY_tgrp:
			n = pr->pr_tgrp;
			goto number;
#endif
		case KEY_time:
			s = fmtelapsed(PR_TIME(pr), PR_HZ);
			goto string;
		case KEY_tty:
			if (PR_ZOMBIE(pr))
				goto zombie;
			s = ttybase(pr->pr_ttydev);
			if ((i = strlen(s) - kp->prec) > 0)
			{
				if (s[0] == 'p' && s[1] == 't')
				{
					if (s[2] == 'y')
						s += 3;
					else
						s += 2;
				}
				else if (s[0] == 't' && s[1] == 't' && s[2] == 'y')
					s += 3;
				else
					s += i;
			}
			goto string;
		case KEY_uid:
			n = pr->pr_uid;
			goto number;
		case KEY_user:
			s = pp->user;
			goto string;
		case KEY_wchan:
			if (PR_ZOMBIE(pr))
				goto zombie;
			n = (long)pr->pr_wchan;
			goto hex;
		}
		s = "????";
	string:
		sfprintf(sfstdout, "%*.*s%s", kp->width, kp->prec, s, kp->sep);
		continue;
	zombie:
		s = "";
		goto string;
	percent:
		sfprintf(state.tmp, "%%%ld", n);
		s = sfstruse(state.tmp);
		goto string;
	number:
		if (!state.hex || !kp->hex)
		{
			sfprintf(sfstdout, "%*ld%s", kp->width, n, kp->sep);
			continue;
		}
	hex:
		if (kp->hex)
			sfprintf(sfstdout, "%*lx%s", kp->width, n, kp->sep);
		else
			sfprintf(sfstdout, "%0*lx%s", kp->width, n, kp->sep);
		continue;
	}
}

/*
 * ps() a process and its children
 */

static void
kids(register Ps_t* pp, int level)
{
	pp->level = level;
	ps(pp);
	if (level > 0)
		state.branch[level - 1] = pp->sibling != 0;
	if (level < elementsof(state.branch) - 1)
		level++;
	for (pp = pp->children; pp; pp = pp->sibling)
		kids(pp, level);
}

/*
 * ps() the selected procs
 */

static void
list(void)
{
	register int	n;
	register Ps_t*	pp;
	register Ps_t*	xp;
	register Ps_t*	zp;
	register Key_t*	kp;
	Ps_t*		rp;

	/*
	 * output the header
	 */

	if (state.fields)
	{
		while (state.fields->skip)
			state.fields = state.fields->next;
		kp = state.fields;
		while (kp->next)
		{
			if (!kp->next->skip)
				kp = kp->next;
			else if (!(kp->next = kp->next->next))
			{
				state.lastfield = kp;
				break;
			}
		}
		n = 0;
		for (kp = state.fields; kp; kp = kp->next)
		{
			if ((kp->prec = kp->width) < 0)
				kp->prec = -kp->prec;
			if (*kp->head)
				n = 1;
		}
		kp = state.lastfield;
		if (kp->width < 0)
			kp->width = 0;
		if (kp->index == KEY_args)
			kp->prec = 80;
		if (n)
			for (kp = state.fields; kp; kp = kp->next)
				sfprintf(sfstdout, "%*s%s", kp->width, kp->head, kp->sep);
	}
	else
		sfkeyprintf(sfstdout, NiL, state.format, key, NiL);
	if (state.tree)
	{
		/*
		 * list the process tree
		 */

		rp = zp = 0;
		for (pp = (Ps_t*)dtfirst(state.byorder); pp; pp = (Ps_t*)dtnext(state.byorder, pp))
			if (pp->ps.pr_ppid != pp->ps.pr_pid && (xp = (Ps_t*)dtmatch(state.bypid, &pp->ps.pr_ppid)))
			{
				if (xp->lastchild)
					xp->lastchild = xp->lastchild->sibling = pp;
				else
					xp->children = xp->lastchild = pp;
			}
			else if (zp)
				zp = zp->root = pp;
			else
				rp = zp = pp;
		for (pp = rp; pp; pp = pp->root)
			kids(pp, 0);
	}
	else
	{
		/*
		 * list by order
		 */

		for (pp = (Ps_t*)dtfirst(state.byorder); pp; pp = (Ps_t*)dtnext(state.byorder, pp))
			if (!pp->shown)
			{
				pp->shown = 1;
				pp->level = 0;
				ps(pp);
			}
	}
}

/*
 * order procs by <uid,start,pid>
 */

static int
order(Dt_t* dt, void* a, void* b, Dtdisc_t* disc)
{
	register Ps_t*	pa = (Ps_t*)((char*)a - offsetof(Ps_t, ps.pr_pid));
	register Ps_t*	pb = (Ps_t*)((char*)b - offsetof(Ps_t, ps.pr_pid));
	register int	i;

	NoP(dt);
	NoP(disc);
	if (i = strcmp(pa->user, pb->user))
		return i;
	if (pa->ps.pr_pgrp < pb->ps.pr_pgrp)
		return -1;
	if (pa->ps.pr_pgrp > pb->ps.pr_pgrp)
		return 1;
	if (i = (pa->ps.pr_pgrp == pa->ps.pr_pid) - (pb->ps.pr_pgrp == pb->ps.pr_pid))
		return i;
	if (PR_START(&pa->ps) < PR_START(&pb->ps))
		return -1;
	if (PR_START(&pa->ps) > PR_START(&pb->ps))
		return 1;
	if (pa->ps.pr_pid < pb->ps.pr_pid)
		return -1;
	if (pa->ps.pr_pid > pb->ps.pr_pid)
		return 1;
	return 0;
}
/*
 * add the procs in the pid list
 */

static void
addpid(register char* s, int verbose)
{
	register char*			t;
	register int			fd;
	register Ps_t*			pp;
	register struct prpsinfo*	pr;
	register int			c;
	int				n;
	unsigned long			x;
#if defined(_PS_scan_binary) || defined(_PS_scan_format)
	struct stat			st;
#endif

	do
	{
		for (; isspace(*s) || *s == ','; s++);
		for (t = s; *s && !isspace(*s) && *s != ','; s++);
		c = *s;
		*s = 0;
		if (!*t)
			break;
		sfprintf(state.tmp, _PS_path_str, t, _PS_status);
		if ((fd = open(sfstruse(state.tmp), O_RDONLY)) >= 0)
		{
			if (!(pp = state.pp) && !(state.pp = pp = newof(0, Ps_t, 1, 0)))
				error(ERROR_SYSTEM|3, "out of space [proc]");
			pr = &pp->ps;
#ifdef _PS_scan_format
			if ((n = read(fd, state.buf, sizeof(state.buf))) <= 0 || fstat(fd, &st))
			{
				n = -1;
				errno = EINVAL;
			}
			else
			{
				memset(pr, sizeof(*pr), 0);
				n = sfsscanf(state.buf, _PS_scan_format, _PS_scan_args(pr));
				if (n != _PS_scan_count)
					error(1, "%s: scan count %d, expected %d", t, n, _PS_scan_count);
#ifdef _PS_scan_fix
				_PS_scan_fix(pr);
#endif
				pr->pr_uid = st.st_uid;
				pr->pr_gid = st.st_gid;
				pr->pr_nice = pr->pr_priority - 15;
				pr->pr_size /= 1024;
				pr->pr_rssize /= 1024;
#ifdef _PS_scan_boot
				if (!state.boot)
				{
					register char*	s;
					Sfio_t*		fp;

					state.boot = 1;
					if (fp = sfopen(NiL, "/proc/stat", "r"))
					{
						while (s = sfgetr(fp, '\n', 0))
							if (strneq(s, "btime ", 6))
							{
								state.boot = strtol(s + 6, NiL, 10);
								break;
							}
						sfclose(fp);
					}
				}
				pr->pr_start = state.boot + pr->pr_start / PR_HZ;
#endif
			}
#else
#ifdef _PS_scan_binary
			n = read(fd, pr, sizeof(*pr)) == sizeof(*pr) ? 1 : -1;
#else
			n = ioctl(fd, PIOCPSINFO, pr);
#endif
#endif
			close(fd);
			if (n < 0)
			{
				error(ERROR_SYSTEM|2, "%s: cannot get process info", t);
				return;
			}
			if (state.controlled)
			{
				if (pr->pr_ttydev != state.ttydev || pr->pr_uid != state.caller)
					return;
			}
			else if (!state.detached && pr->pr_ttydev == (dev_t)(-1))
				return;
			else if (!state.leader && pr->pr_pid == pr->pr_sid)
				return;
			if (!state.all)
			{
				if (state.idindex)
				{
					switch (state.idindex)
					{
					case KEY_group:
						x = pr->pr_gid;
						break;
					case KEY_pgrp:
						x = pr->pr_pgrp;
						break;
					case KEY_sid:
						x = pr->pr_sid;
						break;
					case KEY_tty:
						x = pr->pr_ttydev;
						break;
					case KEY_user:
						x = pr->pr_uid;
						break;
					default:
						error(2, "internal error: %s selection not implemented", keys[state.idindex].name);
						return;
					}
					if (!dtmatch(state.ids, &x))
						return;
				}
			}
			pp->user = fmtuid(pr->pr_uid);
			dtinsert(state.byorder, pp);
			if (state.tree)
				dtinsert(state.bypid, pp);
			state.pp = 0;
		}
		else if (verbose || errno != ENOENT && errno != EACCES)
			error(ERROR_SYSTEM|2, "%s: cannot open process", t);
	} while (*s++ = c);
}

/*
 * add the ids in s into state.ids
 * getid!=0 translates alnum to id number
 */

static void
addid(register char* s, int index, int (*getid)(const char*))
{
	register char*	t;
	register int	c;
	char*		e;
	long		n;
	Dtdisc_t*	dp;
	Id_t*		ip;

	if (index != state.idindex)
	{
		if (state.idindex)
		{
			error(2, "only one of %s and %s may be selected", keys[state.idindex].name, keys[index].name);
			return;
		}
		state.idindex = index;
		if (!(dp = newof(0, Dtdisc_t, 1, 0)))
			error(ERROR_SYSTEM|3, "out of space [id-disc]");
		dp->key = offsetof(Id_t, id);
		dp->size = sizeof(long);
		dp->link = offsetof(Id_t, hashed);
		if (!(state.ids = dtopen(dp, Dthash)))
			error(ERROR_SYSTEM|3, "out of space [id-dict]");
		state.detached = state.leader = 1;
	}
	do
	{
		for (; isspace(*s) || *s == ','; s++);
		for (t = s; *s && !isspace(*s) && *s != ','; s++);
		if ((c = s - t) >= sizeof(state.buf))
			c = sizeof(state.buf) - 1;
		memcpy(state.buf, t, c);
		(t = state.buf)[c] = 0;
		if (!*t)
			break;
		if (isdigit(*t))
		{
			n = strtol(t, &e, 10);
			if (*e)
			{
				error(1, "%s: invalid %s", t, keys[index].name);
				continue;
			}
		}
		else if (!getid || (n = (*getid)(t)) < 0)
		{
			error(1, "%s: invalid %s", t, keys[index].name);
			continue;
		}
		if (!(ip = newof(0, Id_t, 1, 0)))
			error(ERROR_SYSTEM|3, "out of space [id]");
		ip->id = n;
		dtinsert(state.ids, ip);
	} while (*s++);
}

/*
 * add the format key in s into state.fields
 */

static void
addkey(const char* k)
{
	register char*	s = (char*)k;
	register char*	t;
	register int	c;
	register Key_t*	kp;
	register Key_t*	ap;
	char*		e;
	int		w;

	if (streq(s, "?"))
	{
		sfprintf(sfstdout, "%-8s %-8s %s\n", "KEY", "HEADING", "DESCRIPTION");
		for (kp = keys + 1; kp < keys + elementsof(keys); kp++)
		{
			ap = kp->head ? kp : (keys + kp->index);
			sfprintf(sfstdout, "%-8s %-8s %s%s%s\n", kp->name, ap->head, ap->desc, ap == kp ? "" : " [alias]", ap->missing ? " [not available]" : "");
		}
		exit(0);
	}
	do
	{
		for (; isspace(*s) || *s == ','; s++);
		for (t = s; *s && !isspace(*s) && *s != ',' && *s != '=' && *s != ':' && *s != '+'; s++);
		if ((c = s - t) >= sizeof(state.buf))
			c = sizeof(state.buf) - 1;
		memcpy(state.buf, t, c);
		(t = state.buf)[c] = 0;
		if (!*t)
			break;
		if (*s == ':' || *s == '+')
		{
			c = (int)strtol(s + 1, &e, 10);
			s = e;
		}
		else
			c = 0;
		if (!(kp = (Key_t*)dtmatch(state.keys, t)))
		{
			error(2, "%s: unknown format key", t);
			continue;
		}

		/*
		 * aliases have Key_t.head == 0
		 */

		if (!kp->head)
			kp = keys + kp->index;
		if (kp->missing)
		{
			error(1, "%s: format key not implemented for this system", t);
			continue;
		}

		/*
		 * adjust the width field
		 */

		if (*s == '=')
		{
			kp->head = s + 1;
			if ((w = strlen(kp->head)) > kp->width)
				kp->width = w;
		}
		if (c >= strlen(kp->head))
			kp->width = c;

		/*
		 * except for width and head adjustments
		 * we ignore keys already specified to let
		 * shell aliases work with least suprise
		 */

		if (!kp->already)
		{
			kp->already = keys[kp->cancel].already = keys[kp->cancel].skip = 1;
			if (state.lastfield)
				state.lastfield = state.lastfield->next = kp;
			else
				state.fields = state.lastfield = kp;
			kp->sep = space;
			if (kp->maxval)
			{
				for (c = 1; kp->maxval /= 10; c++);
				if (c >= kp->width)
					kp->width = c;
			}
		}
	} while (*s != '=' && *s++);
}

int
main(int argc, register char** argv)
{
	register int	n;
	register char*	s;
	DIR*		dir;
	struct dirent*	ent;
	Sfio_t*		fmt;
	Key_t*		kp;
	Dtdisc_t	kd;
	Dtdisc_t	pd;
	Dtdisc_t	sd;
	Optdisc_t	od;
	struct stat	st;

	NoP(argc);
	error_info.id = "ps";
	setlocale(LC_ALL, "");
	state.now = time((time_t*)0);
	state.caller = geteuid();
	if (!(fmt = sfstropen()) || !(state.tmp = sfstropen()) || !(state.wrk = sfstropen()))
		error(3, "out of space [tmp]");

	/*
	 * set up the disciplines
	 */

	memset(&od, 0, sizeof(od));
	od.version = OPT_VERSION;
	od.infof = optinfo;
	opt_info.disc = &od;
	memset(&kd, 0, sizeof(kd));
	kd.key = offsetof(Key_t, name);
	kd.size = -1;
	kd.link = offsetof(Key_t, hashed);
	memset(&pd, 0, sizeof(pd));
	pd.key = offsetof(Ps_t, ps.pr_pid);
	pd.size = sizeof(state.pp->ps.pr_pid);
	pd.link = offsetof(Ps_t, hashed);
	memset(&sd, 0, sizeof(sd));
	sd.key = offsetof(Ps_t, ps.pr_pid);
	sd.size = sizeof(state.pp->ps.pr_pid);
	sd.link = offsetof(Ps_t, sorted);
	sd.comparf = order;

	/*
	 * initialize the format key table
	 */

	if (!(state.keys = dtopen(&kd, Dthash)) || !(state.bypid = dtopen(&pd, Dthash)) || !(state.byorder = dtopen(&sd, Dttree)))
		error(3, "out of space [dict]");
	for (n = 1; n < elementsof(keys); n++)
		dtinsert(state.keys, keys + n);

	/*
	 * grab the options
	 */

	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'a':
			state.all = 1;
			continue;
		case 'c':
			addkey("pid class pri tty time command");
			continue;
		case 'd':
			state.all = state.detached = 1;
			continue;
		case 'e':
		case 'A':
			state.all = state.detached = state.leader = 1;
			continue;
		case 'f':
			addkey("user pid ppid start tty time args");
			continue;
		case 'g':
			addid(opt_info.arg, KEY_pgrp, NiL);
			continue;
		case 'j':
			addkey("pid pgrp sid tty time command");
			continue;
		case 'l':
			addkey("flags state user pid ppid pri nice size rss wchan tty time command");
			continue;
		case 'n':
			continue;
		case 'o':
			addkey(opt_info.arg);
			continue;
		case 'p':
			addpid(opt_info.arg, 1);
			continue;
		case 's':
			addid(opt_info.arg, KEY_sid, NiL);
			continue;
		case 't':
			addid(opt_info.arg, KEY_tty, ttyid);
			continue;
		case 'u':
			addid(opt_info.arg, KEY_user, struid);
			continue;
		case 'v':
			state.verbose = 1;
			continue;
		case 'x':
			state.hex = !state.hex;
			continue;
		case 'D':
			if (s = strchr(opt_info.arg, '='))
				*s++ = 0;
			if (*opt_info.arg == 'n' && *(opt_info.arg + 1) == 'o')
			{
				opt_info.arg += 2;
				s = 0;
			}
			if (!(kp = (Key_t*)dtmatch(state.keys, opt_info.arg)))
			{
				if (!s)
					continue;
				if (!(kp = newof(0, Key_t, 1, strlen(opt_info.arg) + 1)))
					error(ERROR_SYSTEM|3, "out of space [macro]");
				kp->name = strcpy((char*)(kp + 1), opt_info.arg);
				dtinsert(state.keys, kp);
			}
			if (kp->macro = s)
				stresc(s);
			continue;
		case 'F':
			if (sfstrtell(fmt))
				sfputc(fmt, ' ');
			sfputr(fmt, opt_info.arg, -1);
			continue;
		case 'G':
			addid(opt_info.arg, KEY_group, strgid);
			continue;
		case 'N':
			addkey(default_format);
			continue;
		case 'T':
			state.tree = 1;
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
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	if (sfstrtell(fmt))
	{
		sfputc(fmt, '\n');
		state.format = sfstruse(fmt);
	}
	else
	{
		sfclose(fmt);
		fmt = 0;
		if (!state.fields)
			addkey(default_format);
	}

	/*
	 * add each proc by name
	 */

	if (argv[0])
	{
		state.all = state.detached = state.leader = 1;
		while (s = *argv++)
			addpid(s, 1);
	}
	else
	{
		if (state.controlled = !state.all && !state.detached && !state.ids)
			for (n = 0; n <= 2; n++)
				if (isatty(n) && !fstat(n, &st))
				{
					state.ttydev = st.st_rdev;
					break;
				}
		if (!(dir = opendir(_PS_dir)))
			error(ERROR_SYSTEM|3, "%s: cannot read process directory", _PS_dir);
		while (ent = readdir(dir))
			if (isdigit(*ent->d_name))
				addpid(ent->d_name, state.verbose);
		closedir(dir);
	}

	/*
	 * list the procs
	 */

	if (state.fields)
		state.lastfield->sep = newline;
	list();
	return error_info.errors != 0;
}
#if __OBSOLETE__ < 20010101
#include "../../lib/libast/disc/sfkeyprintf.c"
#endif

#else

int
main(int argc, register char** argv)
{
	execv("/bin/ps", argv);
	exit(EXIT_NOTFOUND);
}

#endif
