/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1984-2004 AT&T Corp.                *
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
*                          AT&T Research                           *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * make options support
 *
 * option name mappings are here
 * option flag mappings are in options.h
 *
 * omitted letters are avilable for option flags:
 *
 *	ABCDEFGHIJKLMNOPQRSTUV XYZ
 *	abcdefg ijkl no  rst vwxyz
 */

#include "make.h"
#include "options.h"

#define getoption(n)	((struct option*)hashget(opttab,(n)))
#define putoption(n,o)	hashput(opttab,(n),(char*)(o))

#define OPT_OFFSET	10

static const char usage1[] =
"+"
"[-?%s\n]"
USAGE_LICENSE
"[+NAME?nmake - configure, manage and update file hierarchies]"
"[+DESCRIPTION?\bnmake\b reads input \amakefiles\a and triggers shell"
"	actions to build target files that are out of date with prerequisite"
"	files. Most information used to build targets is contained in"
"	the global \abase rules\a that are augmented by user \amakefiles\a."
"	Each argument may be an option, script, or target. An option argument"
"	is preceded by \b-\b or \b+\b. A script argument contains at least"
"	one of \bspace\b, \btab\b, \bnewline\b, \b:\b, \b=\b, \b\"\b, or"
"	\b\\\b and is parsed as a separate, complete makefile. Otherwise"
"	the argument is a \atarget\a that is generated according to the"
"	\amakefile\a and \aglobal\a rules. \atarget\a arguments are made"
"	in order from left to right and overrride the default targets.]"
"[+?Command line options, scripts and targets may appear in any order,"
"	with the exception that no option argument may appear after a"
"	\b--\b argument.]"
;

static struct option	options[] =	/* option table			*/
{

"accept",	OPT_accept,	(char*)&state.accept,		0,0,
	"Accept filesystem timestamps of existing targets.", 0,
"alias",	OPT_alias,	(char*)&state.alias,		0,0,
	"Enable directory aliasing.", 0,
"base",		OPT_base,	(char*)&state.base,		0,0,
	"Compile base or global rules.", 0,
"believe",	OPT_believe,	(char*)&state.believe,		0,0,
	"Believe the state file time of files lower than view level"
	" \alevel-1\a. The file system time will be checked for files with"
	" no state or files in views equal to or higher than \alevel\a."
	" \alevel=0\a causes the file system time to be checked for"
	" files on all view levels. The top view is level 0.", "level:=0",
"byname",	OPT_byname,	0,				0,0,
	"(obsolete) Set options by name.", "name[=value]]",
"compile",	OPT_compile,	(char*)&state.compileonly,	0,0,
	"Force makefile compilation.", 0,
"compatibility",OPT_compatibility,(char*)&state.compatibility,	0,0,
	"(obsolete) Enable compatibility messages.", 0,
"corrupt",OPT_corrupt,	(char*)&state.corrupt,		0,0,
	"\aaction\a determines the action to take for corrupt or invalid"
	" top view state files. \baccept\b is assumed if \aaction\a is"
	" omitted. The top view default is \berror\b and the lower view"
	" default is \baccept\b. \aaction\a may be one of:]:?[action]",
	"{"
	"	[+accept?print a warning and set \b--accept\b]"
	"	[+error?print a diagnostic and exit]"
	"	[+ignore?print a warning and set \b--noreadstate\b]"
	"}",
"cross",	OPT_cross,	(char*)&state.cross,		0,0,
	"Don't run generated executables.", 0,
"debug",	OPT_debug,	0,				0,0,
	"Set the debug trace level to \alevel\a. Higher levels produce"
	" more output.", "level",
"define",	OPT_define,	0,				0,0,
	"(obsolete) Pass macro definition to the makefile preprocessor.",
	"name[=value]]",
"errorid",	OPT_errorid,	(char*)&state.errorid,		0,0,
	"Add \aid\a to the error message command identifier.", "id:=make",
"exec",		OPT_exec,	(char*)&state.exec,		0,0,
	"Enable shell action execution. \b--noexec\b"
	" disables all but \b.ALWAYS\b shell actions and also disables"
	" make object and state file generation/updates.", 0,
"expandview",	OPT_expandview,	(char*)&state.expandview,	0,0,
	"Expand \a3d\a filesystem paths.", 0,
"explain",	OPT_explain,	(char*)&state.explain,		0,0,
	"Explain each action.", 0,
"file",		OPT_file,	(char*)&internal.makefiles,	0,0,
	"Read the makefile \afile\a. If \b--file\b is not specified then"
	" the makefile names specified by \b$(MAKEFILES)\b are attempted in"
	" order from left to right. The file \b-\b is equivalent"
	" to \b/dev/null\b.", "file",
"force",	OPT_force,	(char*)&state.force,		0,0,
	"Force all targets to be out of date.", 0,
"global",	OPT_global,	(char*)&internal.globalfiles,	0,0,
	"Read the global makefile \afile\a. The \b--file\b search is not"
	" affected.", "file",
"ignore",	OPT_ignore,	(char*)&state.ignore,		0,0,
	"Ignore shell action errors.", 0,
"ignorelock",	OPT_ignorelock,	(char*)&state.ignorelock,	0,0,
	"Ignore state file locks.", 0,
"import",	OPT_import,	(char*)&state.import,		0,0,
	"Current assignments will be marked \bimport\b.", 0,
"include",	OPT_include,	0,				0,0,
	"Add \adirectory\a to the makefile search list.", "directory",
"intermediate",	OPT_intermediate,(char*)&state.intermediate,	0,0,
	"Force intermediate target generation.", 0,
"jobs",		OPT_jobs,	(char*)&state.jobs,		0,0,
	"Set the shell action concurrency level to \alevel\a."
	" Level \b1\b allows dependency checking while an action is"
	" executing; level \b0\b stops all activity while an action"
	" is executing.", "level:=1",
"keepgoing",	OPT_keepgoing,	(char*)&state.keepgoing,	0,0,
	"Continue after error with sibling prerequisites.", 0,
"list",		OPT_list,	(char*)&state.list,		0,0,
	"List current rules and variables on the standard output in"
	" makefile form.", 0,
"mam",		OPT_mam,	(char*)&state.mam.options,	0,0,
	"Write \amake abstract machine\a output to \afile\a if specified or"
	" to the standard output otherwise. See \bmam\b(5) for details on"
	" the \amake abstract machine\a language. If \aparent\a !=0 then it is"
	" the process id of a parent \amam\a process. \adirectory\a is"
	" the working directory of the current \amam\a process relative"
	" to the root \amam\a process, \b.\b if not specified. \atype\a"
	" must be one of:]:"
	"[type[-subtype]][::file[::parent[::directory]]]]]]]",
	"{"
		"[+dynamic?\amam\a trace of an actual build]"
		"[+regress?\amam\a for regression testing; labels, path"
		" names and time stamps are canonicalized for easy comparison]"
		"[+static?\amam\a representation of the makefile assertions;"
		" used for makefile conversion]"
		"[+----?0 or more comma separated subtypes ----]"
		"[+port?used by the base rules to generate portable"
		" makefiles; some paths are parameterized; on by default]"
	"}",
"never",	OPT_never,	(char*)&state.never,		0,0,
	"Don't execute any shell actions. \b--noexec\b executes \b.ALWAYS\b"
	" shell actions.", 0,
"option",	OPT_option,	(char*)&internal.options,	0,0,
	"Define a new option. \achar\a is the single character option name,"
	" \aname\a is the long option name, \aset\a is an optional"
	" \b.FUNCTION\b that is called when the option is \bset\b,"
	" and \aflags\a are a combination of:]:[char+name+flags+set+\"description\"]",
	"{"
	"	[+a?multiple values appended]"
	"	[+b?boolean value]"
	"	[+i?internal value inverted]"
	"	[+n?numeric value]"
	"	[+o?\a-char\a means \b--no\b\aname\a]"
	"	[+s?string value]"
	"	[+x?not expanded in \b$(-)\b]"
	"}",
"override",	OPT_override,	(char*)&state.override,		0,0,
	"Implicit rules or metarules override explicit rules.", 0,
"preprocess",	OPT_preprocess,	0,				0,0,
	"(obsolete) Preprocess all makefiles.", 0,
"questionable",	OPT_questionable,(char*)&state.questionable,	0,0,
	"Enable questionable features defined by \amask\a. Questionable"
	" features are artifacts of previous implementations (\bnmake\b has"
	" been around since 1984-11-01) that will eventually be dropped."
	" The questionable \amask\a registry is in the \bmain.c\b \bnmake\b"
	" source file.", "mask",
"readonly",	OPT_readonly,	(char*)&state.readonly,		0,0,
	"Current assignments and assertions will be marked \breadonly\b.", 0,
"readstate",	OPT_readstate,	(char*)&state.readstate,	0,0,
	"Ignore state files lower than view level \alevel\a. \alevel=0\a"
	" ignores state files on all view levels. The top view is level 0.",
	"level:=0",
"regress",	OPT_regress,	(char*)&state.regress,		0,0,
	"Massage output for regression testing.", 0,
"reread",	OPT_reread,	(char*)&state.reread,		0,0,
	"Ignore any previously generated \b.mo\b files and re-read all"
	" input makefiles.", 0,
"ruledump",	OPT_ruledump,	(char*)&state.ruledump,		0,0,
	"Dump rule information in tabular form on the standard"
	" error when \bnmake\b exits.", 0,
"scan",		OPT_scan,	(char*)&state.scan,		0,0,
	"Scan for and/or check implicit file prerequisites. On by default.", 0,
"silent",	OPT_silent,	(char*)&state.silent,		0,0,
	"Do not trace shell actions as they are executed.", 0,
"strictview",	OPT_strictview,	(char*)&state.strictview,	0,0,
	"Set \bVPATH\b \b.SOURCE\b rule interpretation to follow strict"
	" \a3d\a filesystem semantics, where directories in the top views"
	" take precedence. On by default when running in \a2d\a with"
	" \bVPATH\b defined, off by default otherwise.", 0,
"targetcontext",OPT_targetcontext,(char*)&state.targetcontext,	0,0,
	"Expand and execute shell actions in the target directory context."
	" This allows a single makefile to control a directory tree while"
	" generating target files at the source file directory level. By"
	" default target files are generated in the current directory.", 0,
"test",		OPT_test,	(char*)&state.test,		0,0,
	"Enable test code defined by \amask\a. Test code is implementation"
	" specific. The test \amask\a registry is in the \bmain.c\b \bnmake\b"
	" source file.", "mask",
"tolerance",	OPT_tolerance,	(char*)&state.tolerance,	0,0,
	"Set the time comparison tolerance to \aseconds\a. Times within"
	" the tolerance range compare equal. Useful on systems that can't"
	" quite get the file system and local clocks in sync. A tolerance"
	" of more that 5 seconds soon becomes intolerable.", "seconds",
"touch",	OPT_touch,	(char*)&state.touch,		0,0,
	"Touch the time stamps of out of date targets rather than execute"
	" the shell action.", 0,
"undef",	OPT_undef,	0,				0,0,
	"(obsolete) Pass macro deletion to the makefile preprocessor.",
	"name",
"vardump",	OPT_vardump,	(char*)&state.vardump,		0,0,
	"Dump variable information in tabular form on the standard"
	" error when \bnmake\b exits.", 0,
"warn",		OPT_warn,	(char*)&state.warn,		0,0,
	"Enable verbose warning messages. Currently ignored.", 0,
"writeobject",	OPT_writeobject,(char*)&state.writeobject,	0,0,
	"Generate a \b.mo\b make object file in \afile\a that can be read"
	" instead of the input makefiles on the next \bnmake\b invocation."
	" On by default. \b--nowriteobject\b prevents the generation."
	" The default name is used if \afile\a is omitted or \b-\b."
	" If \afile\a is a directory then the default is placed in that"
	" directory.",
	"file:=$(MAKEFILE::B::S=.mo)",
"writestate",	OPT_writestate,	(char*)&state.writestate,	0,0,
	"Generate a \b.ms\b make state file in \afile\a when \bnmake\b exits."
	"The state contains the time stamps of all prerequisites and targets"
	" that have been accessed since the state file was first generated."
	" On by default. \b--nowritestate\b prevents the generation."
	" The default name is used if \afile\a is omitted or \b-\b."
	" If \afile\a is a directory then the default is placed in that"
	" directory.",
	"file:=$(MAKEFILE::B::S=.ms)",

};

static const char usage2[] =
"\n"
"[ script ... ] [ target ... ]\n"
"\n"
"[+SEE ALSO?\b3d\b(1), \bar\b(1), \bcc\b(1), \bcoshell\b(1), \bcpp\b(1),"
"	\bprobe\b(1), \bsh\b(1)]"
;

struct oplist				/* linked option list		*/
{
	char*		option;		/* option value for set()	*/
	struct oplist*	next;		/* next in list			*/
};

static struct oplist*	hidden;		/* options hidden by cmd line	*/
static struct oplist*	lasthidden;	/* tail of hidden		*/
static struct oplist*	delayed;	/* delayed unknown options	*/
static struct oplist*	lastdelayed;	/* tail of delayed		*/

static Hash_table_t*	opttab;

static Sfio_t*		usage;		/* generated optget() usage	*/
static int		lastusage;	/* next usage option offset	*/
static int		nextusage;	/* next usage index		*/

/*
 * initialize the option hash table
 */

void
optinit(void)
{
	register int	i;

	opttab = hashalloc(NiL, HASH_name, "options", 0);
	for (i = 0; i < elementsof(options); i++)
	{
		switch (options[i].flag)
		{
		case OPT_debug:
			options[i].value = (char*)&error_info.trace;
			break;
		}
		putoption(options[i].name, &options[i]);
	}
	hashset(opttab, HASH_ALLOCATE);
}

/*
 * return option table entry given OPT_[a-z]+ flag
 * type==0 panics if not in table
 */

struct option*
opentry(register int flag, int type)
{
	register struct option*	op;
	Hash_position_t*	pos;
	char			buf[3];

	if ((flag = OPT(flag)) != '?' && (pos = hashscan(opttab, 0)))
	{
		while (hashnext(pos))
		{
			op = (struct option*)pos->bucket->value;
			if (OPT(op->flag) == flag)
			{
				hashdone(pos);
				return op;
			}
		}
		hashdone(pos);
	}
	for (op = &options[0]; op < &options[elementsof(options)]; op++)
		if (OPT(op->flag) == flag)
			return op;
	if (type == '=')
		return 0;
	buf[0] = '-';
	buf[1] = flag;
	buf[2] = 0;
	if (!type || !state.readonly)
		error(type ? 1 : ERROR_PANIC, "%s: unknown option flag", buf);
	return getoption(buf);
}

/*
 * call op->set with new value
 */

static void
setcall(register struct option* op, int readonly)
{
	struct rule*	r;
	char*		oset;
	int		oreadonly;
	char		buf[16];

	if (op->set && (r = getrule(op->set)))
	{
		oset = op->set;
		op->set = 0;
		oreadonly = state.readonly;
		state.readonly = readonly;
		switch (op->flag & (Ob|On|Os))
		{
		case Ob:
			call(r, *((unsigned char*)op->value) ? "1" : null);
			break;
		case On:
			sfsprintf(buf, sizeof(buf), "%d", *((int*)op->value));
			call(r, buf);
			break;
		case Os:
			call(r, *((char**)op->value));
			break;
		}
		state.readonly = oreadonly;
		op->set = oset;
	}
}

/*
 * generate external option declaration
 */

static void
declare(register Sfio_t* sp, register struct option* op)
{
	sfputr(sp, opentry(OPT_option, 0)->name, '=');
	if (OPT(op->flag) != '?')
		sfputc(sp, OPT(op->flag));
	sfputc(sp, '+');
	sfputr(sp, op->name, -1);
	sfputc(sp, '+');
	if (op->flag & Oa)
		sfputc(sp, 'a');
	if (op->flag & Ob)
		sfputc(sp, 'b');
	if (op->flag & On)
		sfputc(sp, 'n');
	if (op->flag & Oo)
		sfputc(sp, 'o');
	if (op->flag & Os)
		sfputc(sp, 's');
	if (op->flag & Ov)
		sfputc(sp, 'v');
	if (op->flag & Ox)
		sfputc(sp, 'x');
	if (op->set)
	{
		sfputc(sp, '+');
		sfputr(sp, op->set, -1);
	}
	if (op->description)
	{
		sfputc(sp, '+');
		shquote(sp, op->description);
	}
}

/*
 * generate optget() usage for op
 */

static void
genusage(register struct option* op, int index, int last)
{
	if (op)
	{
		if (last)
			sfstrset(usage, lastusage);
		sfputc(usage, '[');
		if (!(op->flag & Of))
		{
			sfputc(usage, OPT(op->flag));
			if (op->flag & Oo)
				sfputc(usage, '!');
			sfputc(usage, '=');
		}
		sfprintf(usage, "%d:%s?%s", index + OPT_OFFSET, op->name, op->description);
		if (op->arg)
		{
			if (*op->arg == '{')
				sfputr(usage, op->arg, '\n');
			else
				sfprintf(usage, "]%s%s[%s]\n", (op->flag & On) ? "#" : ":", (op->flag & Ov) ? "?" : "", op->arg);
		}
		else
			sfputr(usage, "]", '\n');
	}
	if (last)
	{
		nextusage = index + (op != 0);
		lastusage = sfstrtell(usage);
		sfprintf(usage, usage2, version);
		sfputc(usage, 0);
		sfstrrel(usage, -1);
	}
}

/*
 * mam output discipline to parameterize local paths
 */

static ssize_t
mamwrite(Sfio_t* fp, const void* buf, size_t n, Sfdisc_t* dp)
{
	char*		s;
	size_t		z;

	static char*	tmp;
	static int	siz;

	z = n;
	if (n > 1 && ((char*)buf)[n-1] == '\n')
	{
		if (n >= siz)
		{
			siz = roundof(n + 1, 1024);
			tmp = newof(tmp, char, siz, 0);
		}
		memcpy(tmp, buf, n);
		tmp[n-1] = 0;
		if (s = call(makerule(external.mamaction), tmp))
		{
			z = strlen(s);
			if (z >= siz)
			{
				siz = roundof(z + 1, 1024);
				tmp = newof(tmp, char, siz, 0);
			}
			memcpy(tmp, s, z);
			tmp[z++] = '\n';
			buf = (const char*)tmp;
		}
	}
	return sfwr(fp, buf, z, dp) == z ? n : -1;
}

/*
 * make sure the start time is > the last program invocation
 * and then make sure the current time is > the start time
 */

static void
regressinit(void)
{
	unsigned long	t;

	while (state.start == (t = CURTIME))
		sleep(1);
	state.start = t;
	while (state.start == CURTIME)
		sleep(1);
	error_info.version = 0;
}

/*
 * set an option given its pointer
 */

static void
setop(register struct option* op, register int n, char* s)
{
	char*		t;
	struct rule*	r;
	int		readonly;

	readonly = state.readonly;
	if (OPT(op->flag) != OPT(OPT_option))
	{
		if (readonly)
			op->status |= OPT_READONLY;
		else if (!state.user && (op->status & OPT_READONLY))
		{
			struct oplist*	x;

			/*
			 * save for listops(*,0)
			 *
			 * NOTE: some of these may cancel during load()
			 */

			if ((op->status & (OPT_EXTERNAL|OPT_HIDDEN)) == OPT_EXTERNAL)
			{
				op->status |= OPT_HIDDEN;
				declare(internal.tmp, op);
				sfputc(internal.tmp, ' ');
			}
			sfprintf(internal.tmp, "%s%s", n ? null : "no", op->name);
			if (s)
				sfprintf(internal.tmp, "=%s", s);
			else if (n != 0 && n != 1)
				sfprintf(internal.tmp, "=%d", n);
			x = newof(0, struct oplist, 1, 0);
			x->option = strdup(sfstruse(internal.tmp));
			if (lasthidden)
				lasthidden = lasthidden->next = x;
			else
				hidden = lasthidden = x;
			return;
		}
	}
	if (!(op->status & OPT_READONLY))
	{
		if (!state.init)
			op->status |= OPT_COMPILE;
		if (state.global)
			op->status |= OPT_GLOBAL;
		else
			op->status &= ~OPT_GLOBAL;
	}
	op->status |= OPT_DEFINED;
	message((-3, "option(%s,'%c',%d,\"%s\")", op->name, op->flag & ((1<<8) - 1), n,  s ? s : null));
	if (op->flag & Oi)
	{
		if (op->flag & On)
			n = -n;
		else
			n = !n;
	}
	else if (op->flag & Ob)
		n = n != 0;
	else if ((op->flag & (Os|Ov)) == Os && n && !s)
		error(3, "-%c: option argument expected", OPT(op->flag));
	if (!n)
		s = 0;
	switch (OPT(op->flag))
	{
	case OPT(OPT_believe):
		if (state.compile < COMPILED)
			state.believe = n;
		else
			error(2, "%s: option must be set before %s", op->name, external.makeinit);
		return;
	case OPT(OPT_byname):
		if (s)
			set(s);
		return;
	case OPT(OPT_corrupt):
		if (!s)
			s = "-";
		if ((*s == *(state.corrupt = "accept") || *s == '-') && (!*(s + 1) || !strcmp(s, state.corrupt)))
			;
		else if (*s == *(state.corrupt = "error") && (!*(s + 1) || !strcmp(s, state.corrupt)))
			state.corrupt = 0;
		else if (*s == *(state.corrupt = "ignore") && (!*(s + 1) || !strcmp(s, state.corrupt)))
			;
		else
		{
			state.corrupt = 0;
			error(2, "%s: invalid corrupt action", s);
		}
		return;
	case OPT(OPT_include):
		addprereq(catrule(internal.source->name, external.source, NiL, 1), makerule(s), PREREQ_APPEND);
		if (!(op->status & OPT_READONLY))
			break;
		/*FALLTHROUGH*/
	case OPT(OPT_define):
	case OPT(OPT_undef):
		if (s)
		{
			sfprintf(internal.tmp, "-%c%s", OPT(op->flag), s);
			if (!(r = getrule(sfstruse(internal.tmp))))
				r = makerule(NiL);
			addprereq(internal.preprocess, r, PREREQ_APPEND);
		}
		return;
	case OPT(OPT_preprocess):
		if (!state.preprocess)
		{
			state.preprocess = 1;
			if (!state.compatibility)
				error(1, "makefile preprocessing is obsolete -- use make statements");
		}
		return;
	case OPT(OPT_errorid):
		if (s && *s)
		{
			if (state.errorid)
				sfprintf(internal.tmp, "%s/", state.errorid);
			sfprintf(internal.tmp, "%s", s);
			state.errorid = strdup(sfstruse(internal.tmp));
			sfprintf(internal.tmp, "%s [%s]", idname, state.errorid);
			error_info.id = strdup(sfstruse(internal.tmp));
		}
		else
		{
			op->status &= ~OPT_DEFINED;
			error_info.id = idname;
		}
		return;
	case OPT(OPT_import):
		if (n > 0)
			state.import++;
		else
			state.import--;
		return;
	case OPT(OPT_jobs):
		if (n >= MAXJOBS)
			n = MAXJOBS - 1;
		if (n < 1)
			n = 0;
		state.jobs = n;
		return;
	case OPT(OPT_mam):
		if (state.mam.label != null)
		{
			if (state.mam.label)
				free(state.mam.label);
			state.mam.label = null;
		}
		if (state.mam.options)
		{
			free(state.mam.options);
			state.mam.options = 0;
		}
		if (state.mam.root)
		{
			free(state.mam.root);
			state.mam.root = 0;
		}
		if (state.mam.out)
		{
			if (state.mam.out != sfstdout && state.mam.out != sfstderr)
				sfclose(state.mam.out);
			state.mam.out = 0;
		}
		state.mam.dontcare = state.mam.dynamic = state.mam.regress = state.mam.statix = state.mam.parent = 0;
		state.mam.port = 1;
		if (s)
		{
			char*	o;
			char*	u;
			Sfio_t*	tmp;

			tmp = sfstropen();
			sfputr(tmp, s, 0);
			s = sfstruse(tmp);
			if (t = strchr(s, ':'))
				*t++ = 0;
			if ((o = strchr(s, '+')) || (o = strchr(s, ',')))
				*o++ = 0;
			if (*s == *(state.mam.type = "dynamic") && (!*(s + 1) || !strcmp(s, state.mam.type)))
				state.mam.dynamic = 1;
			else if (*s == *(state.mam.type = "regress") && (!*(s + 1) || !strcmp(s, state.mam.type)))
			{
				state.regress = state.mam.regress = 1;
				state.silent = 1;
				if (!table.regress)
					table.regress = hashalloc(table.rule, HASH_name, "regress-paths", 0);
				regressinit();
			}
			else if (*s == *(state.mam.type = "static") && (!*(s + 1) || !strcmp(s, state.mam.type)))
				state.mam.statix = 1;
			else
				error(3, "%s: invalid mam type: {dynamic,regress,static} expected", s);
			while (s = o)
			{
				if ((o = strchr(s, '+')) || (o = strchr(s, ',')))
					*o++ = 0;
				if (*s == 'n' && *(s + 1) == 'o')
				{
					s += 2;
					n = 0;
				}
				else
					n = 1;
				if (*s == *(u = "dontcare") && (!*(s + 1) || !strcmp(s, u)))
					state.mam.dontcare = n;
				else if (*s == *(u = "port") && (!*(s + 1) || !strcmp(s, u)))
					state.mam.port = n;
				else
					error(3, "%s: invalid mam option: [no]{dontcare,port} expected", s);
			}
			if (t)
			{
				s = t;
				if (t = strchr(s, ':'))
				{
					*t++ = 0;
					if (isdigit(*t))
					{
						while (isdigit(*t))
							state.mam.parent = state.mam.parent * 10 + *t++ - '0';
						if (!state.mam.regress)
						{
							sfprintf(internal.tmp, "%05d ", state.pid);
							state.mam.label = strdup(sfstruse(internal.tmp));
						}
					}
					if (*t)
					{
						if (*t != ':')
							error(3, "%s: mam label expected", t);
						t++;
					}
					if (!*t)
						t = 0;
				}
			}
			else s = null;
			if (!*s || streq(s, "-") || streq(s, "/dev/fd/1") || streq(s, "/dev/stdout"))
			{
				s = "/dev/stdout";
				state.mam.out = sfstdout;
			}
			else if (streq(s, "/dev/fd/2") || streq(s, "/dev/stderr"))
				state.mam.out = sfstderr;
			else if (!(state.mam.out = sfopen(NiL, s, state.mam.parent ? "a" : "w")))
				error(ERROR_SYSTEM|3, "%s: cannot write mam output file", s);
			else
				fcntl(sffileno(state.mam.out), F_SETFD, FD_CLOEXEC);
			sfset(state.mam.out, SF_LINE, 1);
			state.mam.disc.writef = mamwrite;
			if (sfdisc(state.mam.out, &state.mam.disc) != &state.mam.disc)
				error(3, "%s: cannot push mam output discipline", s);
			sfprintf(internal.tmp, "%s", state.mam.type);
			if (state.mam.dontcare)
				sfprintf(internal.tmp, ",dontcare");
			sfprintf(internal.tmp, ",%sport", state.mam.port ? null : "no");
			sfprintf(internal.tmp, ":");
			if (*s != '/')
				sfprintf(internal.tmp, "%s/", internal.pwd);
			sfprintf(internal.tmp, "%s:%d", s, state.pid);
			if (t)
			{
				sfprintf(internal.nam, "$(\"%s\":P=A)", t);
				expand(tmp, sfstruse(internal.nam));
				state.mam.root = strdup(sfstruse(tmp));
				state.mam.rootlen = strlen(state.mam.root);
				sfprintf(internal.tmp, ":%s", state.mam.root);
			}
			state.mam.options = strdup(sfstruse(internal.tmp));
			sfstrclose(tmp);
		}
		return;
	case OPT(OPT_never):
		if (state.never = n)
			state.exec = 0;
		return;
	case OPT(OPT_regress):
		if (state.regress = n)
			regressinit();
		return;
	case OPT(OPT_reread):
		if (state.reread = n)
			state.forceread = 1;
		return;
	case OPT(OPT_silent):
		if (state.silent = n)
		{
			if (!error_info.trace)
				error_info.trace = 2;
		}
		else
			if (error_info.trace > 0)
				error_info.trace = 0;
		return;
	case OPT(OPT_tolerance):
		if ((state.tolerance = n) > 60)
			error(1, "the time comparison tolerance should probably be less than a minute");
		return;
	case OPT(OPT_writeobject):
		if (!n)
			s = 0;
		else if (state.makefile)
		{
			error(1, "%s: object file name cannot change after %s read", op->name, state.makefile);
			return;
		}
		else if (!s)
			s = "-";
		else
			s = strdup(s);
		state.writeobject = s;
		return;
	case OPT(OPT_writestate):
		if (!n)
			s = 0;
		else if (!s)
			s = "-";
		else
			s = strdup(s);
		state.writestate = s;
		if (state.statefile)
		{
			free(state.statefile);
			state.statefile = 0;
		}
		return;
	}
	if (op->value)
	{
		if (op->flag & Ob)
			*((unsigned char*)op->value) = n;
		else if (op->flag & On)
		{
			if (n && (op->flag & Oa))
				*((int*)op->value) |= n;
			else
				*((int*)op->value) = n;
		}
		else if (op->flag & Os)
		{
			if (op->flag & Oa)
			{
				if (s)
				{
					/*
					 * s is a ':' list
					 */

					for (;;)
					{
						if (t = strchr(s, ':'))
							*t = 0;
						addprereq((*(struct rule**)op->value), makerule(s), PREREQ_APPEND);
						if (OPT(op->flag) == OPT(OPT_option))
						{
							/*UNDENT*/
	n = ((*s == '+' || *s == ',') ? ('?'|Of) : *s++) | Ob;
	if (*s != '+' && *s != ',')
		error(2, "%c: option name omitted", OPT(n));
	else
	{
		char*		u;
		char*		v;
		char*		d;
		struct option*	nop;
		char		buf[16];

		d = 0;
		if ((u = strchr(++s, '+')) || (u = strchr(s, ',')))
		{
			*(v = u) = 0;
			for (;;)
			{
				switch (*++v)
				{
				case '+':
				case ',':
					if (!*++v)
						v = 0;
					/*FALLTHROUGH*/
				case 0:
					break;
				case 'a':
					n |= Oa;
					continue;
				case 'b':
					n &= ~(On|Os);
					n |= Ob;
					continue;
				case 'n':
					n &= ~(Ob|Os);
					n |= On;
					continue;
				case 'o':
					n |= Oo;
					continue;
				case 's':
					n &= ~(Ob|On);
					n |= Os;
					continue;
				case 'v':
					n |= Ov;
					continue;
				case 'x':
					n |= Ox;
					continue;
				default:
					error(1, "`%c': unknown option type flag", *v);
					continue;
				}
				break;
			}
			if (n & Os)
				n &= ~Oa;
			if (d = strchr(v, '+'))
				*d++ = 0;
		}
		else
			v = 0;
		for (nop = &options[0]; nop < &options[elementsof(options)]; nop++)
			if (OPT(nop->flag) == OPT(n))
				break;
		if (nop < &options[elementsof(options)])
			error(2, "%s: -%c conflicts with -%c %s", s, OPT(n), OPT(OPT_byname), nop->name);
		else
		{
			buf[0] = '-';
			buf[1] = OPT(n);
			buf[2] = 0;
			if (nop = getoption(buf))
			{
				if (n & On)
					*((int*)nop->value) = *((unsigned char*)nop->value);
				else if (n & Os)
					*((char**)nop->value) = 0;
				nop->flag = n;
				nop->status |= OPT_DECLARED;
			set_insert:
				if (v)
					nop->set = strdup(v);
				nop->name = strdup(s);
				putoption(nop->name, nop);
				if (d && !nop->description)
					nop->description = strdup(d);
				sfsprintf(buf, sizeof(buf), "-%d", nextusage);
				putoption(strdup(buf), nop);
				genusage(nop, nextusage, 1);
				if (nop->set && (nop->status & OPT_DEFINED))
					readonly = 1;
			}
			else if (!(nop = getoption(s)))
			{
				nop = newof(0, struct option, 1, sizeof(char*));
				nop->value = (char*)(nop + 1);
				nop->flag = n;
				nop->status = OPT_EXTERNAL|OPT_DECLARED;
				goto set_insert;
			}
			else if (!(nop->status & OPT_EXTERNAL))
				error(1, "%s is an internal option", nop->name);
			else
			{
				if (OPT(nop->flag) == '?')
				{
					if ((n & (Ob|On|Os)) != (nop->flag & (Ob|On|Os)))
					{
						if (n & Os)
							*((char**)nop->value) = 0;
						else if ((n & On) && (nop->flag & Ob))
							*((int*)nop->value) = *((unsigned char*)nop->value);
						else if ((n & Ob) && (nop->flag & On))
							*((unsigned char*)nop->value) = *((int*)nop->value) != 0;
					}
					nop->flag = n;
				}
				else if (OPT(nop->flag) != OPT(n))
					error(1, "%s: option flag -%c conflicts with -%c", nop->name, OPT(n), OPT(nop->flag));
				if (!(nop->status & OPT_DECLARED))
					nop->status |= OPT_DECLARED;
				else if ((nop->flag & (Ob|On|Os)) != (n & (Ob|On|Os)))
					error(1, "%s: option is %s", nop->name, (nop->flag & Ob) ? "boolean" : (nop->flag & On) ? "numeric" : "string valued");
			}
		}
		if (u)
			*u = '-';
	}
							/*INDENT*/
						}
						if (!t)
							break;
						*t++ = ':';
						s = t;
					}
				}
				else
				{
					freelist((*(struct rule**)op->value)->prereqs);
					(*(struct rule**)op->value)->prereqs = 0;
				}
			}
			else
				*((char**)op->value) = s ? strdup(s) : 0;
		}
	}
	if (op->set)
		setcall(op, readonly);
}

/*
 * generate a single option setting in sp given the option pointer
 * setting!=0 generates the option name and value suitable for set()
 * setting==0 only the value is generated (s unchanged if option not set)
 * flag!=0 if relative to option flag rather than option name
 */

static void
genop(register Sfio_t* sp, register struct option* op, int setting, int flag)
{
	register int		n;
	char*			v;
	struct list*		p;

	switch (op->flag & (Ob|On|Os))
	{
	case Ob:
		if (op->value) n = *((unsigned char*)op->value);
		else n = 0;
		if (op->flag & Oi) n = !n;
		if (flag && (op->flag & Oo)) n = !n;
		if (setting) sfprintf(sp, "--%s%s", n ? null : "no", op->name);
		else if (n) sfputc(sp, '1');
		break;
	case On:
		if (op->value) n = *((int*)op->value);
		else n = 0;
		if (op->flag & Oi) n = -n;
		if (setting) sfprintf(sp, "--%s%s", n ? null : "no", op->name);
		if (n) sfprintf(sp, (op->flag & Oa) ? "%s0x%08x" : "%s%d", setting ? "=" : null, n);
		break;
	case Os:
		if ((op->flag & Oa) && op->value)
		{
			p = (*(struct rule**)op->value)->prereqs;
			if (setting) sfprintf(sp, "--%s%s%s", p ? null : "no", op->name, p ? "=" : null);
			if (p) for (;;)
			{
				sfputr(sp, p->rule->name, -1);
				if (!(p = p->next)) break;
				sfputc(sp, ':');
			}
		}
		else
		{
			if (op->value) v = *((char**)op->value);
			else v = 0;
			if (setting) sfprintf(sp, "--%s%s", v ? null : "no", op->name);
			if (v) sfprintf(sp, "%s%s", setting ? "=" : null, v);
		}
		break;
#if DEBUG
	default:
		error(PANIC, "%s: option has%s%s%s", op->name, (op->flag & Ob) ? " Ob" : null, (op->flag & On) ? " On" : null, (op->flag & Os) ? " Os" : null);
		break;
#endif
	}
}

/*
 * check and set delayed options
 * this gives global and local makefiles a chance
 */

void
optcheck(void)
{
	struct oplist*	x;
	int		errors = error_info.errors;

	while (x = delayed)
	{
		delayed = x->next;
		set(x->option);
		free(x);
	}
	if (error_info.errors != errors)
		finish(1);
}

/*
 * generate option setting list in sp suitable for set()
 * object<0 generates all options, suitable for debugging
 * object=0 generates all non-Ox OPT_DEFINED options
 * object>0 generates only non-Ox OPT_COMPILE[OPT_GLOBAL] options
 *	    suitable for saving in make object files
 */

void
listops(register Sfio_t* sp, int object)
{
	register struct option*	op;
	register struct oplist*	x;
	int			sep;
	int			mask;
	int			test;
	int			clear;
	Hash_position_t*	pos;

	sep = 0;
	if (object > 0)
	{
		mask = OPT_COMPILE|OPT_GLOBAL;
		test = OPT_COMPILE|(!state.base && state.rules ? 0 : OPT_GLOBAL);
		clear = ~OPT_COMPILE;
		for (x = hidden; x; x = x->next)
		{
			if (sep)
				sfputc(sp, ' ');
			else
				sep = 1;
			sfputr(sp, x->option, -1);
		}
	}
	else if (object < 0)
	{
		mask = 0;
		test = 0;
		clear = ~0;
	}
	else
	{
		mask = OPT_DEFINED|OPT_GLOBAL;
		test = OPT_DEFINED;
		clear = ~0;
	}
	if (pos = hashscan(opttab, 0))
	{
		while (hashnext(pos))
		{
			if (*pos->bucket->name != '-')
			{
				op = (struct option*)pos->bucket->value;
				if (object < 0 || !(op->flag & Ox) && (op->status & mask) == test || object > 0 && (op->status & (OPT_EXTERNAL|OPT_HIDDEN)) == OPT_EXTERNAL)
				{
					if (op->status & OPT_EXTERNAL)
					{
						if (sep)
							sfputc(sp, ' ');
						else
							sep = 1;
						declare(sp, op);
						if (object >= 0 && (!(op->status & OPT_DEFINED) || object && (op->status & OPT_READONLY)))
							continue;
					}
					if (sep)
						sfputc(sp, ' ');
					else
						sep = 1;
					genop(sp, op, 1, 0);
					op->status &= clear;
				}
			}
		}
		hashdone(pos);
	}
}

/*
 * generate a single option setting in sp given the option name
 * setting!=0 generates the option name and value suitable for set()
 * setting==0 only the value is generated ("" if option not set)
 * end of s is returned
 */

void
getop(register Sfio_t* sp, char* name, int setting)
{
	register struct option*	op;
	int			flag = 0;

	if ((op = getoption(name)) || name[0] && !name[1] && (op = opentry(name[0], '=')) && (flag = 1))
		genop(sp, op, setting, flag);
}

/*
 * set an option by its optget()/optstr() index
 */

static void
optset(int i, char* v)
{
	register char*	s;
	int		n;
	struct option*	op;
	struct oplist*	x;
	char		buf[16];

	if (i > 0)
	{
		if (i == '?')
			error(ERROR_USAGE|(state.readonly ? 4 : 2), "%s", opt_info.arg);
		else if (i == ':')
		{
			if (state.readonly && !*opt_info.option && !state.interpreter)
			{
				x = newof(0, struct oplist, 1, strlen(v) + 1);
				strcpy(x->option = (char*)(x + 1), v);
				if (lastdelayed)
					lastdelayed = lastdelayed->next = x;
				else
					delayed = lastdelayed = x;
			}
			else
				error(2, "%s", opt_info.arg);
		}
	}
	else
	{
		if ((i = -OPT_OFFSET - i) < elementsof(options))
			op = &options[i];
		else
		{
			sfsprintf(buf, sizeof(buf), "-%d", i);
			op = getoption(buf);
		}
		n = (op->flag & On) ? (opt_info.arg ? opt_info.num : 0) : (op->flag & Ob) ? (opt_info.num != 0) : (opt_info.num != 0) == !(op->flag & Oi);
		if (*opt_info.option == '+')
			n = !n;
		if ((s = opt_info.arg) && (!*s || !(op->flag & Os)))
			s = 0;
		setop(op, n, s);
	}
}

/*
 * set options by name
 */

void
set(char* s)
{
	register int	i;
	Opt_t		info;

	info = opt_info;
	while (i = optstr(s, sfstrbase(usage)))
	{
		s += opt_info.offset;
		optset(i, opt_info.argv[1]);
	}
	opt_info = info;
}

/*
 * set command line options with optget(3)
 * options may appear in any position before --
 * read command line assignments
 * mark the command line scripts and targets
 * index of the first command line script or target is returned
 */

int
scanargs(int argc, char** argv, int* argf)
{
	register int	i;
	register char*	s;
	register int	c;
	int		args;
	int		done;
	char*		e;

	/*
	 * generate the optget() usage string from options[]
	 */

	if (!(usage = sfstropen()))
		error(ERROR_SYSTEM|3, "out of space [usage]");
	sfprintf(usage, usage1, version);
	for (i = 0; i < elementsof(options); i++)
		genusage(options + i, i, 0);
	genusage(NiL, i, 1);
	args = 0;
	done = 0;
 again:
	while (i = optget(argv, sfstrbase(usage)))
		optset(i, argv[opt_info.index - (opt_info.offset == 0)]);
	if (!done && streq(argv[opt_info.index - 1], "--"))
		done = 1;
	for (i = opt_info.index; i < argc; i++)
	{
		s = argv[i];
		while (isspace(*s))
			s++;
		if (!done && (*s == '-' || *s == '+') && *(s + 1))
		{
			opt_info.index = i;
			opt_info.offset = 0;
			goto again;
		}
		if (*s)
		{
			for (e = s; c = *s; s++)
				if (istype(c, C_TERMINAL) && c != '+' && c != '&')
				{
					while (isspace(*s))
						s++;
					if (*s == '=' || *(s + 1) == '=')
					{
						argf[i] |= ARG_ASSIGN;
						state.reading = 1;
						parse(NiL, e, "command line assignment", 0);
						state.reading = 0;
					}
					else
					{
						argf[i] |= ARG_SCRIPT;
						if (!args)
							args = i;
					}
					break;
				}
			if (!*s)
			{
				argf[i] |= ARG_TARGET;
				if (!args)
					args = i;
			}
		}
	}
	return error_info.errors ? -1 : args ? args : argc;
}

/*
 * called when things get messed up enough to re-exec
 *
 * old!=0 execs external.old for backwards compatibility
 * otherwise re-exec forcing input files to be read
 */

void
punt(int old)
{
	register char*		s;
	register char**		av;
	int			i;
	struct list*		p;
	Hash_position_t*	pos;
	Sfio_t*			vec;

	if (state.reread > 1) error(PANIC, "makefile prerequisites cause unbounded make exec recursion");
	vec = sfstropen();
	if (old)
	{
		/*
		 * this chunk must track external.old options
		 */

		sfputc(internal.tmp, '-');

		/*
		 * options with same flag and meaning
		 */

		if (error_info.trace < -3) sfputc(internal.tmp, 'd');
		if (state.ignore) sfputc(internal.tmp, 'i');
		if (state.keepgoing) sfputc(internal.tmp, 'k');
		if (!state.mam.options && !state.exec) sfputc(internal.tmp, 'n');
		if (state.silent) sfputc(internal.tmp, 's');
		if (state.touch) sfputc(internal.tmp, 't');

		/*
		 * options with different flag but same meaning
		 */

		if (state.vardump) sfputc(internal.tmp, 'p');
		if (!state.mam.options && state.force) sfputc(internal.tmp, 'u');

		/*
		 * options with different flag and meaning
		 * the external.old meaning prevails
		 */

		if (state.base) sfputc(internal.tmp, 'b');
		if (state.explain) sfputc(internal.tmp, 'e');
		if (state.ruledump) sfputc(internal.tmp, 'r');

		/*
		 * unknown flags
		 */

		if (pos = hashscan(opttab, 0))
		{
			while (hashnext(pos))
				if (((struct option*)pos->bucket->value)->status & OPT_FLAG)
					sfputc(internal.tmp, *(((struct option*)pos->bucket->value)->name + 1));
			hashdone(pos);
		}
		s = sfstruse(internal.tmp);
		putptr(vec, getval(external.old, VAL_PRIMARY));
		if (s[1]) putptr(vec, strdup(s));

		/*
		 * mam arguments -- assume oldmake knows mam
		 */

		if (state.mam.options)
		{
			sfputc(internal.tmp, '-');
			if (state.never) sfputc(internal.tmp, 'N');
			else if (!state.exec) sfputc(internal.tmp, 'n');
			if (state.force) sfputc(internal.tmp, 'F');
			sfputc(internal.tmp, 'M');
			sfputr(internal.tmp, state.mam.options, -1);
			putptr(vec, strdup(sfstruse(internal.tmp)));
		}

		/*
		 * makefile arguments
		 */

		if (!(p = internal.makefiles->prereqs))
		{
			putptr(vec, "-f");
			putptr(vec, state.makefile);
		}
		else for (; p; p = p->next)
		{
			putptr(vec, "-f");
			putptr(vec, p->rule->name);
		}

		/*
		 * variable assignment arguments
		 */

		for (i = 1; i < state.argc; i++)
			if (state.argf[i] & (ARG_ASSIGN|ARG_TARGET))
				putptr(vec, state.argv[i]);
		if (!state.silent)
		{
			/*
			 * echo the exec action external.old style
			 */

#if !__sun__ && !sun
			sfprintf(sfstderr, "\t");
#endif
			putptr(vec, 0);
			av = (char**)sfstrbase(vec);
			while (*av) sfprintf(sfstderr, "%s ", *av++);
			sfprintf(sfstderr, "\n");
		}
	}
	else
	{
		/*
		 * copy the original argv adding OPT_reread
		 * and possibly OPT_preprocess
		 */

		for (av = state.argv; *av; putptr(vec, *av++));
		sfprintf(internal.tmp, "-%c%s=%d%s%s", OPT(OPT_byname), opentry(OPT_reread, 0)->name, state.reread + 1, state.preprocess ? " " : null, state.preprocess ? opentry(OPT_preprocess, 0)->name : null);
		putptr(vec, sfstruse(internal.tmp));
	}
	putptr(vec, 0);

	/*
	 * tidy up
	 */

	sfsync(sfstdout);
	sfsync(sfstderr);
	if (internal.openfile) close(internal.openfd);

	/*
	 * start fresh
	 */

	av = (char**)sfstrbase(vec);
	execvp(av[0], av);
	error(3, "cannot exec %s", av[0]);
}
