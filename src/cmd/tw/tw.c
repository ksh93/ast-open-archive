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
 * tw -- tree walk
 *
 * print [execute cmd on] path names in tree rooted at . [dir]
 * or on path names listed on stdin [-]
 */

static const char usage[] =
"[-?\n@(#)tw (AT&T Labs Research) 1999-10-01\n]"
USAGE_LICENSE
"[+NAME?tw - file tree walk]"
"[+DESCRIPTION?\btw\b recursively descends the file tree rooted at the"
"	current directory and lists the pathname of each file found."
"	If \acmd arg ...\a is specified then the pathnames are collected"
"	and appended to the end of the \aarg\alist and \acmd\a is executed"
"	by the equivalent of \bexecvp\b(2). \acmd\a will be executed 0 or more"
"	times, depending the number of generated pathname arguments.]"
"[+?If the last option is \b-\b and \b--find\b was not specified then the"
"	pathnames are read, one per line, from the standard input, the"
"	\b--directory\b options are ignored, and the directory tree is not"
"	traversed.]"
"[+?\bgetconf PATH_RESOLVE\b determines how symbolic links are handled. This"
"	can be explicitly overridden by the \b--logical\b, \b--metaphysical\b,"
"	and \b--physical\b options below. \bPATH_RESOLVE\b can be one of:]{"
"		[+logical?Follow all symbolic links.]"
"		[+metaphysical?Follow command argument symbolic links,"
"			otherwise don't follow.]"
"		[+physical?Don't follow symbolic links.]"
"}"

"[a:arg-list?The first \aarg\a named \astring\a is replaced by the current"
"	pathname list before \acmd\a is executed.]:[string]"
"[c:args|arg-count?\acmd\a is executed after \acount\a arguments are"
"	collected.]#[count]"
"[d:directory?The file tree traversal is rooted at \adir\a. Multiple"
"	\b--directory\b directories are traversed in order from left to right."
"	If the last option was \b-\b then all \b--directory\b are"
"	ignored.]:[dir]"
"[e:expr?\aexpr\a defines expression functions that control tree traversal."
"	Multiple \b--expr\b expressions are parsed in order from left to"
"	right. See EXPRESSIONS below for details.]:[expr]"
"[f:fast?Searches the \bfind\b(1) or \blocate\b(1) database for paths"
"	matching the \bksh\b(1) \apattern\a. See \bupdatedb\b(1) for"
"	details on this database. Any \b--expr\b expressions are applied"
"	to the matching paths.]:[pattern]"
"[i:ignore-errors?Ignore \acmd\a errors and inaccessible files and"
"	directories.]"
"[I:ignore-case?Ignore case in pathname comparisons.]"
"[m:intermediate?Before visiting a selected file select and visit"
"	intermediate directories leading to the file that have not already"
"	been selected.]"
"[n:notraverse?Evaluate the \bbegin\b, \bselect\b and \bend\b expressions"
"	but eliminate the tree traversal.]"
"[p:post?Visit each directory after its files have been processed."
"	By default directories are visited pre-order.]"
"[q:query?Emit an interactive query for each visited path. An affirmative"
"	response accepts the path, a negative response rejects the path,"
"	and a quit response exits \atw\a.]"
"[S:separator?The input file list separator is set to the first character"
"	of \astring\a.]:[string]"
"[s:size|max-chars?Use at most \achars\a characters per command. The default"
"	is as large as possible.]#[chars]"
"[t:trace|verbose?Print the command line on the standard error"
"	before executing it.]"
"[C:chop?Chop leading \b./\b from printed pathnames. This is implied by"
"	\b--logical\b.]"
"[F:codes?Set the \blocate\b(1) fast find codes database \apath\a.]:[path]"
"[G:generate?Generate a \aformat\a \blocate\b(1) database of the visited"
"	files and directories. Exit status 1 means some files were not"
"	accessible but the database was properly generated; exit status 2"
"	means that database was not generated. Format may be:]:[format]{"
"		[+dir|-?machine independent with directory trailing /.]"
"		[+old?old fast find]"
"		[+gnu?gnu \blocate\b(1)]"
"		[+type?machine independent with directory and mime types]"
"}"
"[L:logical|follow?Follow symbolic links. The default is determined by"
"	\bgetconf PATH_RESOLVE\b.]"
"[H:metaphysical?Follow command argument symbolic links, otherwise don't"
"	follow. The default is determined by \bgetconf PATH_RESOLVE\b.]"
"[P:physical?Don't follow symbolic links. The default is determined by"
"	\bgetconf PATH_RESOLVE\b.]"
"[X:xdev|mount?Do not descend into directories in different"
"	filesystems than their parents.]"
"[D:debug?Set the debug trace \alevel\a; higher levels produce more output.]#"
"	[level]"

"\n"
"\n[ cmd [ arg ... ] ]\n"
"\n"

"[+EXPRESSIONS?Expressions are C style and operate on elements of the"
"	\bstat\b(2) struct with the leading \bst_\b omitted."
"	A function expression is defined by one of:]{"
"		[+?function-name : statement-list]"
"		[+?type function-name() { statement-list }]"
"}"
"[+?where \afunction-name\a is one of:]{"
"		[+begin?Evaluated before the traversal starts. The return"
"			value is ignored. The default is a no-op.]"
"		[+select?Evaluated as each file is visited. A 0 return"
"			value skips \baction\b for the file; otherwise"
"			\baction\b is evaluated. All files are selected"
"			by default. \bselect\b is assumed when"
"			\afunction-name\a: is omitted.]"
"		[+action?Evaluated for each select file. The return value"
"			is ignored. The default \baction\b list the"
"			file path name, with leading \b./\b stripped, one"
"			per line on the standard output.]"
"		[+end?Evaluated after the traversal completes. The return"
"			value is ignored.]"
"		[+sort?A pseudo-function: the statement list is a , separated"
"			list of identifiers used to sort the entries"
"			of each directory. If any identifier is preceded by"
"			\b!\b then the sort order is reversed. If any"
"			identifier is preceded by \b~\b then case is ignored.]"
"}"
"[+?\astatement-list\a is a C style \bexpr\b(3) expression that supports:"
"	\bint\b \avar\a, ...; and \bfloat\b \avar\a, ...; declarations,"
"	\b(int)\b and \b(float)\b casts, \bif\b-\belse\b conditionals,"
"	\bfor\b and \bwhile\b loops, and \b{...}\b blocks. The trailing"
"	\b;\b in any expression list is optional. The expression value is"
"	the value of the last evaluated expression in \astatement-list\a."
"	Numbers and comments follow C syntax. String operands must be quoted"
"	with either \b\"...\"\b or \b'...'\b. String comparisons \b==\b"
"	and \b!=\b treat the right hand operand as a \bksh\b(1) file"
"	match pattern.]"
"[+?The expressions operate on the current pathname file status that is"
"	provided by the following field identifiers, most of which are"
"	described under \bst_\b\afield\a in \bstat\b(2). In general, if"
"	a status identifier appears on the left hand side of a binary"
"	operator then the right hand side may be a string that is converted"
"	to an integral constant according to the identifier semantics.]{"
"		[+atime?access time; time/date strings are interpreted as"
"			\bdate\b(1) expressions]"
"		[+blocks?number of 1k blocks]"
"		[+ctime?status change time]"
"		[+dev?file system device]"
"		[+fstype?file system type name; \bufs\b if it can't"
"			be determined]"
"		[+gid?owner group id; \agid\a strings are interpreted as group"
"			names]"
"		[+gidok?1 if \agid\a is a valid group id in the system"
"			database, 0 otherwise.]"
"		[+ino?inode/serial number]"
"		[+level?the depth of the file relative to the traversal root]"
"		[+local?an integer valued field associated with each active"
"			object in the traversal; This field may be assigned."
"			The initial value is 0. Multiple \alocal\a elements"
"			may be declared by \bint local.\b\aelement1\a...;."
"			In this case the \blocal\b field itself is not"
"			accessible.]"
"		[+mode?type and permission bits; the \bFMT\b constant may"
"			be used to mask mask the file type and permission"
"			bits; \bmode\b strings are interpreted as \bchmod\b(1)"
"			expressions]"
"		[+mtime?modify time]"
"		[+name?file name with directory prefix stripped]"
"		[+nlink?hard link count]"
"		[+path?full path name relative to the current active"
"			\b--directory\b]"
"		[+perm?the permission bits of \bmode\b]"
"		[+rdev?the major.minor device number if the file is"
"			a device]"
"		[+size?size in bytes]"
"		[+status?the \bfts\b(3) \bFTS_\b* or \bftwalk\b(3) \bFTW_\b*"
"			status. This field may be assigned:]{"
"			[+AGAIN?visit the file again]"
"			[+FOLLOW?if the file is a symbolic link then follow it]"
"			[+NOPOST?cancel any post order visit to this file]"
"			[+SKIP?do not consider this file or any subdirectories"
"				if it is a directory]"
"		}"
"		[+type?the type bits of \bmode\b:]{"
"			[+BLK?block special]"
"			[+CHR?block special]"
"			[+DIR?directory]"
"			[+FIFO?fifo]"
"			[+LNK?symbolic link]"
"			[+REG?regular]"
"			[+SOCK?unix domain socket]"
"		}"
"		[+uid?owner user id; \auid\a strings are interpreted as user"
"			names]"
"		[+uidok?1 if \auid\a is a valid user id in the system"
"			database, 0 otherwise.]"
"		[+visit?an integer variable associated with each unique"
"			object visited; Objects are identified using the"
"			\bdev\b and \bino\b status identifiers. This field"
"			may be assigned. The initial value is 0. Multiple"
"			\bvisit\b elements may be declared by \bint visit.\b"
"			\aelement\a...;. In this case the \bvisit\b field"
"			itself is not accessible.]"
"}"
"[+?Status identifiers may be prefixed by 1 or more \bparent.\b references,"
"	to access ancestor directory information. The parent status"
"	information of a top level object is the same as the object"
"	except that \bname\b and \bpath\b are undefined. If a status"
"	identifier is immediately preceded by \b\"string\"\b. then"
"	string is a file pathname from which the status is taken.]"
"[+?The following \bexpr\b(3) functions are supported:]{"
"	[+exit(expr)?causes \atw\a to exit with the exit code"
"		\aexpr\a which defaults to 0 if omitted]"
"	[+printf(format[,arg...]])?print the arguments on"
"		the standard output using the \bprintf\b(3)"
"		specification \aformat\a.]"
"	[+eprintf(format[,arg...]])?print the arguments on"
"		the standard error using the \bprintf\b(3)"
"		specification \aformat\a.]"
"	[+query(format[,arg...]])?prompt with the \bprintf\b(3)"
"		message on the standard error an read an"
"		interactive response. An affirmative response"
"		returns 1, \bq\b or \bEOF\b causes \atw\a to"
"		to exit immediately, and any other input"
"		returns 0.]"
"}"
"[+EXAMPLES]{"
"	[+tw?Lists the current directory tree.]"
"	[+tw chmod go-w?Turns off the group and other write permissions"
"		for all files in the current directory tree using a minimal"
"		amount of \bchmod\b(1) command execs.]"
"	[+tw -e \"uid != 'bozo' || (mode & 'go=w')\"?Lists all files in the"
"		current directory that don't belong to the user \bbozo\b or"
"	have group or other write permission.]"
"	[+tw -m -d / -e \"fstype == '/'.fstype && mtime > '/etc/backup.time'.mtime\"?"
"		Lists all files and intermediate directories on the same"
"		file system type as \b/\b that are newer than the file"
"		\b/etc/backup.time\b.]"
"	[+tw - chmod +x < commands?Executes \bchmod +x\b on the pathnames"
"		listed in the file \bcommands\b.]"
"	[+tw -e \"int count;?\baction: count++;"
"		printf('name=%s inode=%08ld\\\\n', name, ino);"
"		end: printf('%d file%s\\\\n', count, count==1 ? '' :: 's');\"\b"
"		Lists the name and inode number of each file and also the"
"		total number of files.]"
"	[+tw -pP -e \"?\baction: if (visit++ == 0) {"
"		parent.local += local + blocks;"
"		if (type == DIR)"
"			printf('%d\\\\t%s\\\\n', local + blocks, path); }\"\b"
"		Exercise for the reader.]"
"}"
"[+EXIT STATUS]{"
"	[+0?All invocations of \acmd\a returned exit status 0.]"
"	[+1-125?A command line meeting the specified requirements could not"
"		be assembled, one or more of the invocations of \acmd\a"
"		returned non-0 exit status, or some other error occurred.]"
"	[+126?\acmd\a was found but could not be executed.]"
"	[+127?\acmd\a was not found.]"
"}"
"[+ENVIRONMENT]{"
"	[+FINDCODES?Path name of the \blocate\b(1) database.]"
"	[+LOCATE_PATH?Alternate path name of \blocate\b(1) database.]"
"}"
"[+FILES]{"
"	[+lib/find/find.codes?Default \blocate\b(1) database.]"
"}"
"[+SEE ALSO?\bfind\b(1), \bgetconf\b(1), \blocate\b(1), \bupdatedb\b(1),"
"	\bxargs\b(1)]"
;

#include "tw.h"

#include <ctype.h>
#include <proc.h>
#include <wait.h>

#define ALL		((Exnode_t*)0)
#define LIST		((Exnode_t*)1)

#define FTW_LIST	(FTW_USER<<0)	/* files listed on stdin	*/

typedef struct Dir			/* directory list		*/
{
	struct Dir*	next;		/* next in list			*/
	char*		name;		/* dir name			*/
} Dir_t;

State_t			state;

static void		intermediate(Ftw_t*, char*);

/*
 * do the action
 */

static void
act(register Ftw_t* ftw, int op)
{
	Sfio_t*	fp;

	switch (op)
	{
	case ACT_CMDARG:
		cmdarg(state.cmd, ftw->path, ftw->pathlen);
		break;
	case ACT_CODE:
		if (findwrite(state.find, ftw->path, ftw->pathlen, (ftw->info & FTW_D) ? "system/dir" : (char*)0))
			state.finderror = 1;
		break;
	case ACT_CODETYPE:
		fp = sfopen(NiL, PATH(ftw), "r");
		if (findwrite(state.find, ftw->path, ftw->pathlen, magictype(state.magic, fp, PATH(ftw), &ftw->statb)))
			state.finderror = 1;
		if (fp)
			sfclose(fp);
		break;
	case ACT_EVAL:
		eval(state.action, ftw);
		break;
	case ACT_INTERMEDIATE:
		intermediate(ftw, ftw->path);
		break;
	case ACT_LIST:
		sfputr(sfstdout, ftw->path, '\n');
		break;
	}
}

/*
 * generate intermediate (missing) directories for terminal elements
 */

static void
intermediate(register Ftw_t* ftw, register char* path)
{
	register char*	s;
	register char*	t;
	register int	c;

	if (!(ftw->info & FTW_D) || ftw->statb.st_nlink)
	{
		ftw->statb.st_nlink = 0;
		if (ftw->level > 1)
			intermediate(ftw->parent, path);
		s = path + ftw->pathlen;
		c = *s;
		*s = 0;
		t = ftw->path;
		ftw->path = path;
		act(ftw, state.actII);
		ftw->path = t;
		*s = c;
	}
}

/*
 * tw a single file
 */

static int
tw(register Ftw_t* ftw)
{
	Local_t*	lp;

	switch (ftw->info)
	{
	case FTW_NS:
		if (!state.info)
		{
			if (!state.pattern && !state.ignore)
				error(2, "%s: not found", ftw->path);
			return 0;
		}
		break;
	case FTW_DC:
		if (!state.info)
		{
			if (!state.ignore)
				error(2, "%s: directory causes cycle", ftw->path);
			return 0;
		}
		break;
	case FTW_DNR:
		if (!state.info)
		{
			if (!state.ignore)
				error(2, "%s: cannot read directory", ftw->path);
		}
		break;
	case FTW_DNX:
		if (!state.info)
		{
			if (!state.ignore)
				error(2, "%s: cannot search directory", ftw->path);
			ftw->status = FTW_SKIP;
		}
		break;
	case FTW_DP:
		if (!(state.ftwflags & FTW_TWICE) || (state.ftwflags & FTW_DOT) && stat(PATH(ftw), &ftw->statb))
			goto pop;
		break;
	case FTW_D:
		ftw->ignorecase = (state.icase || (!ftw->level || !ftw->parent->ignorecase) && strchr(astconf("PATH_ATTRIBUTES", ftw->path, NiL), 'c')) ? STR_ICASE : 0;
		break;
	default:
		ftw->ignorecase = ftw->level ? ftw->parent->ignorecase : (state.icase || strchr(astconf("PATH_ATTRIBUTES", ftw->path, NiL), 'c')) ? STR_ICASE : 0;
		break;
	}
	if (state.select == ALL || eval(state.select, ftw) && ftw->status != FTW_SKIP)
		act(ftw, state.act);
 pop:
	if (state.localmem && (lp = (Local_t*)ftw->local.pointer))
	{
		lp->next = state.local;
		state.local = lp;
	}
	if (state.ftwflags & FTW_LIST)
		ftw->status = FTW_SKIP;
	return 0;
}

/*
 * order child entries
 */

static int
order(register Ftw_t* f1, register Ftw_t* f2)
{
	register int	v;
	long		n1;
	long		n2;

	if (state.sortkey->index == F_name)
		v = state.icase ? strcasecmp(f1->name, f2->name) : strcoll(f1->name, f2->name);
	else
	{
		n1 = getnum(state.sortkey, f1);
		n2 = getnum(state.sortkey, f2);
		if (n1 < n2)
			v = -1;
		else if (n1 > n2)
			v = 1;
		else v = 0;
	}
	if (state.reverse)
		v = -v;
	message((-2, "order(%s,%s) = %d [n1=%ld n2=%ld]", f1->name, f2->name, v, n1, n2));
	return v;
}

int
main(int argc, register char** argv)
{
	register int	n;
	register char*	s;
	char*		args;
	char*		codes;
	char**		av;
	char**		ap;
	int		count;
	int		len;
	int		traverse;
	int		size;
	Dir_t*		firstdir;
	Dir_t*		lastdir;
	Exnode_t*	x;
	Finddisc_t	disc;

	setlocale(LC_ALL, "");
	error_info.id = "tw";
	av = argv + 1;
	args = 0;
	codes = 0;
	count = 0;
	size = 0;
	traverse = 1;
	firstdir = lastdir = newof(0, Dir_t, 1, 0);
	firstdir->name = ".";
	state.action = LIST;
	state.cmdflags = CMD_IMPLICIT|CMD_NEWLINE;
	state.ftwflags = ftwflags();
	state.select = ALL;
	state.separator = '\n';
	memset(&disc, 0, sizeof(disc));
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'a':
			args = opt_info.arg;
			state.cmdflags |= CMD_INSERT;
			continue;
		case 'c':
			if ((count = opt_info.num) < 0)
				error(3, "argument count must be >= 0");
			continue;
		case 'd':
			lastdir = lastdir->next = newof(0, Dir_t, 1, 0);
			lastdir->name = opt_info.arg;
			continue;
		case 'e':
			compile(opt_info.arg);
			continue;
		case 'f':
			state.pattern = opt_info.arg;
			continue;
		case 'i':
			state.ignore = 1;
			continue;
		case 'm':
			state.intermediate = 1;
			continue;
		case 'n':
			traverse = 0;
			continue;
		case 'p':
			state.ftwflags |= FTW_TWICE;
			continue;
		case 'q':
			state.cmdflags |= CMD_QUERY;
			continue;
		case 's':
			if ((size = opt_info.num) < 0)
				error(3, "command size must be >= 0");
			continue;
		case 't':
			state.cmdflags |= CMD_TRACE;
			continue;
		case 'C':
			state.ftwflags |= FTW_NOSEEDOTDIR;
			continue;
		case 'D':
			error_info.trace = -opt_info.num;
			continue;
		case 'F':
			codes = opt_info.arg;
			continue;
		case 'G':
			disc.flags |= FIND_GENERATE;
			if (streq(opt_info.arg, "old"))
				disc.flags |= FIND_OLD;
			else if (streq(opt_info.arg, "gnu") || streq(opt_info.arg, "locate"))
				disc.flags |= FIND_GNU;
			else if (streq(opt_info.arg, "type"))
				disc.flags |= FIND_TYPE;
			else if (streq(opt_info.arg, "?"))
			{
				error(2, "formats are { default|dir type old gnu|locate }");
				return 0;
			}
			else if (!streq(opt_info.arg, "-") && !streq(opt_info.arg, "default") && !streq(opt_info.arg, "dir"))
				error(3, "%s: invalid find codes format -- { default|dir type old gnu|locate } expected", opt_info.arg);
			continue;
		case 'H':
			state.ftwflags |= FTW_META|FTW_PHYSICAL;
			continue;
		case 'I':
			state.icase = 1;
			continue;
		case 'L':
			state.ftwflags &= ~(FTW_META|FTW_PHYSICAL);
			continue;
		case 'P':
			state.ftwflags &= ~FTW_META;
			state.ftwflags |= FTW_PHYSICAL;
			continue;
		case 'S':
			state.separator = *opt_info.arg;
			continue;
		case 'X':
			state.ftwflags |= FTW_MOUNT;
			continue;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			continue;
		case ':':
			error(2, "%s", opt_info.arg);
			continue;
		}
		break;
	}
	argv += opt_info.index;
	argc -= opt_info.index;
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));

	/*
	 * do it
	 */

	if (x = exexpr(state.program, "begin", NiL, 0))
		eval(x, NiL);
	if ((x = exexpr(state.program, "select", NiL, INTEGER)) || (x = exexpr(state.program, NiL, NiL, INTEGER)))
		state.select = x;
	if (traverse)
	{
		if (x = exexpr(state.program, "action", NiL, 0))
			state.action = x;
		if (x = exexpr(state.program, "sort", NiL, 0))
		{
			for (;;)
			{
				switch (x->op)
				{
				case '~':
					state.icase = 1;
					x = x->data.operand.left;
					continue;
				case '!':
					state.reverse = 1;
					x = x->data.operand.left;
					continue;
				}
				break;
			}
			if (x->op == S2B)
				x = x->data.operand.left;
			if (x->op != ID)
				error(3, "invalid sort identifier");
			state.sortkey = x->data.variable.symbol;
			state.sort = order;
			message((-1, "sortkey = %s", state.sortkey->name));
		}
		if (*argv && (*argv)[0] == '-' && (*argv)[1] == 0)
		{
			state.ftwflags |= FTW_LIST;
			argv++;
			argc--;
		}
		if (*argv || args || count || !(state.cmdflags & CMD_IMPLICIT))
		{
			state.cmd = cmdopen(argv, count, size, args, state.cmdflags);
			state.ftwflags |= FTW_DOT;
		}
		else state.cmdflags &= ~CMD_IMPLICIT;
		if (codes && (disc.flags & FIND_GENERATE))
		{
			char*	p;
			Dir_t*	dp;
			char	pwd[PATH_MAX];
			char	tmp[PATH_MAX];

			disc.version = FIND_VERSION;
			if (state.cmdflags & CMD_TRACE)
				disc.flags |= FIND_TYPE;
			if (state.cmdflags & CMD_QUERY)
				disc.flags |= FIND_OLD;
			disc.errorf = (Finderror_f)errorf;
			if (!(state.find = findopen(codes, NiL, NiL, &disc)))
				exit(2);
			if (disc.flags & FIND_TYPE)
			{
				state.act = ACT_CODETYPE;
				compile("_tw_init:mime;");
				state.magicdisc.flags |= MAGIC_MIME;
			}
			else
				state.act = ACT_CODE;
			state.icase = 1;
			state.pattern = 0;
			state.sort = order;
			if (!state.program)
				compile("1");
			state.sortkey = (Exid_t*)dtmatch(state.program->symbols, "name");
			s = p = 0;
			for (dp = (firstdir == lastdir) ? firstdir : firstdir->next; dp; dp = dp->next)
			{
				if (*(s = dp->name) == '/')
					sfsprintf(tmp, sizeof(tmp), "%s", s);
				else if (!p && !(p = getcwd(pwd, sizeof(pwd))))
					error(ERROR_SYSTEM|3, "cannot determine pwd path");
				else
					sfsprintf(tmp, sizeof(tmp), "%s/%s", p, s);
				pathcanon(tmp, PATH_PHYSICAL);
				if (!(dp->name = strdup(tmp)))
					error(ERROR_SYSTEM|3, "out of space [PATH_PHYSICAL]");
			}
		}
		else if (state.cmdflags & CMD_IMPLICIT)
			state.act = ACT_CMDARG;
		else if (state.action == LIST)
			state.act = ACT_LIST;
		else if (state.action)
			state.act = ACT_EVAL;
		if (state.intermediate)
		{
			state.actII = state.act;
			state.act = ACT_INTERMEDIATE;
		}
		if (state.pattern)
		{
			disc.version = FIND_VERSION;
			if (state.icase)
				disc.flags |= FIND_ICASE;
			disc.errorf = (Finderror_f)errorf;
			disc.dirs = ap = av;
			if (firstdir != lastdir)
				firstdir = firstdir->next;
			do *ap++ = firstdir->name; while (firstdir = firstdir->next);
			*ap = 0;
			if (!(state.find = findopen(codes, state.pattern, NiL, &disc)))
				exit(1);
			state.ftwflags |= FTW_TOP;
			n = state.select == ALL ? state.act : ACT_EVAL;
			while (s = findread(state.find))
				switch (n)
				{
				case ACT_CMDARG:
					cmdarg(state.cmd, s, strlen(s));
					break;
				case ACT_LIST:
					sfputr(sfstdout, s, '\n');
					break;
				default:
					ftwalk(s, tw, state.ftwflags, NiL);
					break;
				}
		}
		else if (state.ftwflags & FTW_LIST)
		{
			sfopen(sfstdin, NiL, "rt");
			n = state.select == ALL && state.act == ACT_CMDARG;
			for (;;)
			{
				if (s = sfgetr(sfstdin, state.separator, 1))
					len = sfvalue(sfstdin) - 1;
				else if (state.separator != '\n')
				{
					state.separator = '\n';
					continue;
				}
				else if (s = sfgetr(sfstdin, state.separator, -1))
					len = sfvalue(sfstdin);
				else
					break;
				if (n)
					cmdarg(state.cmd, s, len);
				else
					ftwalk(s, tw, state.ftwflags, NiL);
			}
			if (sferror(sfstdin))
				error(ERROR_SYSTEM|2, "input read error");
		}
		else if (firstdir == lastdir)
			ftwalk(firstdir->name, tw, state.ftwflags, state.sort);
		else
		{
			ap = av;
			while (firstdir = firstdir->next)
				*ap++ = firstdir->name;
			*ap = 0;
			ftwalk((char*)av, tw, state.ftwflags|FTW_MULTIPLE, state.sort);
		}
		if (state.cmd)
			cmdflush(state.cmd);
		if (state.find && (findclose(state.find) || state.finderror))
			exit(2);
	}
	else if (state.select)
		error_info.errors = eval(state.select, NiL) == 0;
	if (x = exexpr(state.program, "end", NiL, 0))
		eval(x, NiL);
	exit(error_info.errors != 0);
}
