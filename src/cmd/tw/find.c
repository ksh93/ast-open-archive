/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 1989-2005 AT&T Corp.                  *
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
 * David Korn
 * Glenn Fowler
 * AT&T Research
 *
 * rewrite of find program to use ftwalk() and optget()
 * this implementation should have all your favorite
 * find options plus these extensions:
 * 
 *	-fast pattern
 *		traverses the fast find database (updatedb(1))
 *		under the dirs specified on the command line
 *		for paths that contain pattern; all other
 *		expression operators apply to matching paths
 *	-magic pattern
 *		matches pattern agains the file(1) magic description
 *	-mime type/subtype
 *		matches type/subtype against the file mime type
 *	-ignorecase
 *		case ingnored in path pattern matching
 *	-xargs command ... \;
 *		like -exec except that command will be invoked
 *		with as many file arguments as the system
 *		allows per command
 *	-test now
 *		set the current time to now for testing
 */

static const char usage1[] =
"[-1p1?@(#)$Id: find (AT&T Labs Research) 2004-12-08 $\n]"
USAGE_LICENSE
"[+NAME?find - find files]"
"[+DESCRIPTION?\bfind\b recursively descends the directory hierarchy for each"
"	\apath\a and applies an \aoption\a expression to each file in the"
"	hierarchy. \b-print\b is implied if there is no action that"
"	generates output. The expression starts with the first argument"
"	that matches [(-!]]. Option expressions may occur before and/or"
"	after the \apath\a arguments. For numeric arguments \an\a, \a+n\a"
"	means \a>n\a, \a-n\a means \a<n\a, and \an\a means exactly \an\a.]"
;

static const char usage2[] =
"\n"
"[ path ... ] [ options ]\n"
"\n"
"[+EXIT STATUS?If no commands were executed (\b-exec\b, \b-xargs\b) the exit"
"	status is 1 if errors were detected in the directory traversal and"
"	0 if no errors were ecountered. Otherwise the status is:]{"
"	[+0?All \acommand\a executions returned exit status 0.]"
"	[+1-125?A command line meeting the specified requirements could not"
"		be assembled, one or more of the invocations of \acommand\a"
"		returned  non-0 exit status, or some other error occurred.]"
"	[+126?\acommand\a was found but could not be executed.]"
"	[+127?\acommand\a was not found.]"
"}"
"[+ENVIRONMENT]{"
"	[+FINDCODES?Path name of the \blocate\b(1) database.]"
"	[+LOCATE_PATH?Alternate path name of \blocate\b(1) database.]"
"}"
"[+FILES]{"
"	[+lib/find/codes?Default \blocate\b(1) database.]"
"}"
"[+NOTES?In order to access the \bslocate\b(1) database the \bfind\b executable"
"	must be setgid to the \bslocate\b group.]"
"[+SEE ALSO?\bcpio\b(1), \bfile\b(1), \blocate\b(1), \bls\b(1), \bsh\b(1),"
"	\bslocate\b(1), \btest\b(1), \btw\b(1), \bupdatedb\b(1),"
"	\bxargs\b(1), \bstat\b(2)]"
;

#include <ast.h>
#include <ls.h>
#include <modex.h>
#include <find.h>
#include <ftwalk.h>
#include <dirent.h>
#include <error.h>
#include <proc.h>
#include <tm.h>
#include <ctype.h>
#include <magic.h>
#include <mime.h>
#include <regex.h>

#include "cmdarg.h"
#include "ftwlocal.h"

#define ignorecase	fts_number

#define PATH(f)		((f)->status==FTW_NAME?(f)->name:(f)->path)

#define DAY		(unsigned long)(24*60*60)

/*
 * this is the list of operations
 * retain the order
 */

#undef	NOGROUP
#undef	NOUSER

enum Command
{
	CFALSE, CTRUE,

	PRINT, PRINT0, PRINTF, PRINTX, FPRINT, FPRINT0, FPRINTF, FPRINTX,
	LS, FLS,
	ATIME, AMIN, ANEWER, CTIME, CMIN, CNEWER, MTIME, MMIN, NEWER,
	POST, LOCAL, XDEV, PHYS,
	NAME, USER, GROUP, INUM, SIZE, LINKS, PERM, EXEC, OK, CPIO, NCPIO,
	TYPE, AND, OR, NOT, COMMA, LPAREN, RPAREN, LOGIC, PRUNE,
	CHECK, SILENT, IGNORE, SORT, REVERSE, FSTYPE, META,
	NOGROUP, NOUSER, FAST, ICASE, MAGIC, MIME, XARGS,
	DAYSTART, MAXDEPTH, MINDEPTH, NOLEAF, EMPTY,
	ILNAME, INAME, IPATH,
	IREGEX, LNAME, PATH, REGEX, USED, XTYPE, CHOP,
	LEVEL, TEST, CODES
};

#define Unary		(1<<0)
#define Num		(1<<1)
#define Str		(1<<2)
#define Exec		(1<<3)
#define Op		(1<<4)
#define File		(1<<5)
#define Re		(1<<6)
#define Stat		(1<<7)

struct Args
{
	const char*	name;
	enum Command	action;
	int		type;
	int		primary;
	const char*	arg;
	const char*	values;
	const char*	help;
};

union Value
{
	char**		p;
	char*		s;
	unsigned long	u;
	long		l;
	int		i;
	short		h;
	char		c;
};

struct Fmt
{
	Sffmt_t		fmt;
	Ftw_t*		ftw;
	char		tmp[PATH_MAX];
};

/*
 * Except for pathnames, these are the only legal arguments
 */

const struct Args commands[] =
{
"begin",	LPAREN,		Unary,		0,	0,	0,
	"Equivalent to \\(. Begin nested expression.",
"end",		RPAREN,		Unary,		0,	0,	0,
	"Equivalent to \\). End nested expression.",
"a|and",	AND,		Op,		0,	0,	0,
	"Equivalent to `\\&'. \aexpr1\a \b-and\b \aexpr2\a:"
	" Both \aexpr1\a and \aexpr2\a must evaluate \btrue\b."
	" This is the default operator for two expression in sequence.",
"amin",		AMIN,		Num|Stat,	0,	"minutes",	0,
	"File was last accessed \aminutes\a minutes ago.",
"anewer",	ANEWER,		Str|Stat,	0,	"file",	0,
	"File was last accessed more recently than \afile\a was modified.",
"atime",	ATIME,		Num|Stat,	0,	"days",	0,
	"File was last accessed \adays\a days ago.",
"check",	CHECK,		Unary,		0,	0,	0,
	"Turns off \b-silent\b; enables inaccessible file and directory"
	" warnings. This is the default.",
"chop",		CHOP,		Unary,		0,	0,	0,
	"Chop leading \b./\b from printed pathnames.",
"cmin",		CMIN,		Num|Stat,	0,	"minutes",	0,
	"File status changed \aminutes\a minutes ago.",
"cnewer",	CNEWER,		Str|Stat,	0,	"file",	0,
	"File status changed more recently than \afile\a was modified.",
"codes",	CODES,		Str,		0,	"path",	0,
	"Sets the \bfind\b or \blocate\b(1) database \apath\a."
	" See \bupdatedb\b(1) for a description of this database.",
"comma",	COMMA,		Op,		0,	0,	0,
	"Equivalent to `,'. Joins two expressions; the status of the first"
	" is ignored.",
"cpio",		CPIO,		Unary|Stat,	1,	0,	0,
	"File is written as a binary format \bcpio\b(1) file entry.",
"ctime",	CTIME,		Num|Stat,	0,	"days",	0,
	"File status changed \adays\a days ago.",
"daystart",	AMIN,		Unary|Stat,	0,	0,	0,
	"Measure times (-amin -atime -cmin -ctime -mmin -mtime) from the"
	" beginning of today. The default is 24 hours ago.",
"depth",	POST,		Unary,		0,	0,	0,
	"Process directory contents before the directory itself.",
"empty",	EMPTY,		Unary|Stat,	0,	0,	0,
	"A directory with size 0 or with no entries other than . or .., or a"
	" regular file with size 0.",
"exec",		EXEC,		Exec,		1,	"command ... \\;", 0,
	"Execute \acommand ...\a; true if 0 exit status is returned."
	" Arguments up to \\; are taken as arguments to \acommand\a."
	" The string `{}' is globally replaced by the current filename"
	" in \acommand ...\a. The command is executed in the directory"
	" from which \bfind\b was executed.",
"false",	CFALSE,		Unary,		0,	0,	0,
	"Always false.",
"fast",		FAST,		Str,		0,	"pattern",	0,
	"Searches the \bfind\b or \blocate\b(1) database for paths"
	" matching the \bksh\b(1) \apattern\a. See \bupdatedb\b(1) for"
	" details on this database. The command line arguments limit"
	" the search and the expression, but all depth options are ignored."
	" The remainder of the expression is applied to the matching paths.",
"fls",		FLS,		File|Stat,	1,	"file",	0,
	"Like -ls except the output is written to \afile\a.",
"fprint",	FPRINT,		File|Stat,	1,	"file",	0,
	"Like -print except the output is written to \afile\a.",
"fprint0",	FPRINT0,	File|Stat,	1,	"file",	0,
	"Like -print0 except the output is written to \afile\a.",
"fprintf",	FPRINTF,	File|Stat,	1,	"file format",	0,
	"Like -printf except the output is written to \afile\a.",
"fprintx",	FPRINTX,	File|Stat,	1,	"file",	0,
	"Like -printx except the output is written to \afile\a.",
"fstype",	FSTYPE,		Str|Stat,	0,	"type",	0,
	"File is on a filesystem \atype\a. See \bdf\b(1) or"
	" \b-printf %F\b for local filesystem type names.",
"group|gid",	GROUP,		Str|Stat,	0,	"id",	0,
	"File group id name or number matches \aid\a.",
"ignorecase",	ICASE,		Unary,		0,	0,	0,
	"Ignore case in all pattern match expressions.",
"ilname",	ILNAME,		Str,		0,	"pattern",	0,
	"A case-insensitive version of \b-lname\b \apattern\a.",
"iname",	INAME,		Str,		0,	"pattern",	0,
	"A case-insensitive version of \b-name\b \apattern\a.",
"inum",		INUM,		Num|Stat,	0,	"number",	0,
	"File has inode number \anumber\a.",
"ipath",	IPATH,		Str,		0,	"pattern",	0,
	"A case-insensitive version of \b-path\b \apattern\a.",
"iregex",	IREGEX,		Re,		0,	"pattern",	0,
	"A case-insensitive version of \b-regex\b \apattern\a.",
"level",	LEVEL,		Num,		0,	"level",	0,
	"Current level (depth) is \alevel\a.",
"links",	LINKS,		Num|Stat,	0,	"number",	0,
	"File has \anumber\a links.",
"lname",	LNAME,		Str,		0,	"pattern",	0,
	"File is a symbolic link with text that matches \apattern\a.",
"local",	LOCAL,		Unary|Stat,	0,	0,	0,
	"File is on a local filesystem.",
"logical|follow|L",LOGIC,	Unary,		0,	0,	0,
	"Follow symbolic links.",
"ls",		LS,		Unary|Stat,	1,	0,	0,
	"List the current file in `ls -dils' format to the standard output.",
"magic",	MAGIC,		Str,		0,	"pattern",	0,
	"File magic number matches the \bfile\b(1) and \bmagic\b(3)"
	" description \apattern\a.",
"maxdepth",	MAXDEPTH,	Num,		0,	"level",	0,
	"Descend at most \alevel\a directory levels below the command"
	" line arguments. \b-maxdepth 0\b limits the search to the command"
	" line arguments.",
"metaphysical|H",META,		Unary,		0,	0,	0,
	"\b-logical\b for command line arguments, \b-physical\b otherwise.",
"mime",		MIME,		Str,		0,	"type/subtype",	0,
	"File mime type matches the pattern \atype/subtype\a.",
"mindepth",	MINDEPTH,	Num,		0,	"level",	0,
	"Do not apply tests or actions a levels less than \alevel\a."
	" \b-mindepth 1\b processes all but the command line arguments.",
"mmin",		MMIN,		Num|Stat,	0,	"minutes",	0,
	"File was modified \aminutes\a minutes ago.",
"mount|x|xdev|X",XDEV,		Unary|Stat,	0,	0,	0,
	"Do not descend into directories in different filesystems than"
	" their parents.",
"mtime",	MTIME,		Num|Stat,	0,	"days",	0,
	"File was modified \adays\a days ago.",
"name",		NAME,		Str,		0,	"pattern",	0,
	"File base name (no directory components) matches \apattern\a.",
"ncpio",	NCPIO,		Unary|Stat,	1,	0,		0,
	"File is written as a character format \bcpio\b(1) file entry.",
"newer",	NEWER,		Str|Stat,	0,	"file",	0,
	"File was modified more recently than \afile\a.",
"nogroup",	NOGROUP,	Unary|Stat,	0,	0,	0,
	"There is no group name matching the file group id.",
"noleaf",	NOLEAF,		Unary|Stat,	0,	0,	0,
	"Disable \b-physical\b leaf file \bstat\b(2) optimizations."
	" Only required on filesystems with . and .. as the first entries"
	" and link count not equal to 2 plus the number of subdirectories.",
"not",		NOT,		Op,		0,	0,	0,
	"\b-not\b \aexpr\a: inverts the truth value of \aexpr\a.",
"nouser",	NOUSER,		Unary|Stat,	0,	0,	0,
	"There is no user name matching the file user id.",
"ok",		OK,		Exec,		1,	"command ... \\;", 0,
	"Like \b-exec\b except a prompt is written to the terminal."
	" If the response does not match `[yY]].*' then the command"
	" is not run and false is returned.",
"o|or",		OR,		Op,		0,	0,	0,
	"Equivalent to `\\|'. \aexpr1\a \b-or\b \aexpr2\a:"
	" \aexpr2\a is not"
	" evaluated if \aexpr1\a is true.",
"path",		PATH,		Str,		0,	"pattern",	0,
	"File path name (with directory components) matches \apattern\a.",
"perm",		PERM,		Num|Stat,	0,	"mode",	0,
	"File permission bits tests; \amode\a may be octal or symbolic as"
	" in \bchmod\b(1). \amode\a: exactly \amode\a; \a-mode\a: all"
	" \amode\a bits are set; \a+mode\a: at least one of \amode\a"
	" bits are set.",
"physical|phys|P",PHYS,		Unary,		0,	0,	0,
	"Do not follow symbolic links. This is the default.",
"post",		POST,		Unary,		0,	0,	0,
	"Process directories before and and after the contents are processed.",
"print",	PRINT,		Unary,		1,	0,	0,
	"Print the path name (including directory components) to the"
	" standard output, followed by a newline.",
"print0",	PRINT0,		Unary,		1,	0,	0,
	"Like \b-print\b, except that the path is followed by a NUL character.",
"printf",	PRINTF,		Str|Stat,	1,	"format",
	"[+----- escape sequences -----?]"
		"[+\\a?alert]"
		"[+\\b?backspace]"
		"[+\\f?form feed]"
		"[+\\n?newline]"
		"[+\\t?horizontal tab]"
		"[+\\v?vertical tab]"
		"[+\\xnn?hexadecimal character \ann\a]"
		"[+\\nnn?octal character \annn\a]"
	"[+----- format directives -----?]"
		"[+%%?literal %]"
		"[+%a?access time in \bctime\b(3) format]"
		"[+%Ac?access time is \bstrftime\b(3) %\ac\a format]"
		"[+%b?file size in 512 byte blocks]"
		"[+%c?status change time in \bctime\b(3) format]"
		"[+%Cc?status change time is \bstrftime\b(3) %\ac\a format]"
		"[+%d?directory tree depth; 0 means command line argument]"
		"[+%f?file base name (no directory components)]"
		"[+%F?filesystem type name; use this for \b-fstype\b]"
		"[+%g?group name, or numeric group id if no name found]"
		"[+%G?numeric group id]"
		"[+%h?file directory name (no base component)]"
		"[+%H?command line argument under which file is found]"
		"[+%i?file inode number]"
		"[+%k?file size in kilobytes]"
		"[+%l?symbolic link text, empty if not symbolic link]"
		"[+%m?permission bits in octal]"
		"[+%n?number of hard links]"
		"[+%p?full path name]"
		"[+%P?file path with command line argument prefix deleted]"
		"[+%s?file size in bytes]"
		"[+%t?modify time in \bctime\b(3) format]"
		"[+%Tc?modify time is \bstrftime\b(3) %\ac\a format]"
		"[+%u?user name, or numeric user id if no name found]"
		"[+%U?numeric user id]"
		"[+%x?%p quoted for \bxargs\b(1)]"
		"[+%X?%P quoted for \bxargs\b(1)]",
	"Print format \aformat\a on the standard output, interpreting"
	" `\\' escapes and `%' directives. \bprintf\b(3) field width"
	" and precision are interpreted as usual, but the directive"
	" characters have special interpretation.",
"printx",	PRINTX,		Unary,		1,	0,	0,
	"Print the path name (including directory components) to the"
	" standard output, with \bxargs\b(1) special characters preceded"
	" by \b\\\b, followed by a newline.",
"prune",	PRUNE,		Unary,		0,	0,	0,
	"Ignored if \b-depth\b is given, otherwise do not descend the"
	" current directory.",
"regex",	REGEX,		Re,		0,	"pattern",	0,
	"Path name matches the anchored regular expression \apattern\a,"
	" i.e., leading ^ and traling $ are implied.",
"reverse",	REVERSE,	Unary,		0,	0,	0,
	"Reverse the \b-sort\b sense.",
"silent",	SILENT,		Unary,		0,	0,	0,
	"Do not warn about inaccessible directories or symbolic link loops.",
"size",		SIZE,		Num|Stat,	0,	"number[bcgkm]]", 0,
	"File size is \anumber\a units (b: 512 byte blocks, c: characters"
	" g: 1024*1024*1024 blocks, k: 1024 blocks, m: 1024*1024 blocks.)"
	" Sizes are rounded to the next unit.",
"sort",		SORT,		Str,		0,	"option",	0,
	"Search each directory in \a-option\a sort order, e.g., \b-name\b"
	" sorts by name, \b-size\b sorts by size.",
"test",		TEST,		Num,		0,	"seconds",	0,
	"Set the current time to \aseconds\a since the epoch. Other"
	" implementation defined test modes may also be enabled.",
"true",		CTRUE,		Unary,		0,	0,	0,
	"Always true.",
"type",		TYPE,		Str|Stat,	0,	"type",
		"[+b?block special]"
		"[+c?character special]"
		"[+d?directory]"
		"[+f?regular file]"
		"[+l?symbolic link]"
		"[+p?named pipe (FIFO)]"
		"[+s?socket]"
		"[+C?contiguous]"
		"[+D?door]",
	"File type matches \atype\a:",
"used",		USED,		Num|Stat,	0,	"days",	0,
	"File was accessed \adays\a days after its status changed.",
"user|uid",	USER,		Str|Stat,0,	"id",	0,
	"File user id matches the name or number \aid\a.",
"xargs",	XARGS,		Exec,		1,	"command ... \\;", 0,
	"Like \b-exec\b except as many file args as permitted are"
	" appended to \acommand ...\a which may be executed"
	" 0 or more times depending on the number of files found and"
	" local system \bexec\b(2) argument limits.",
"xtype",	XTYPE,		Str|Stat,	0,	"type",	0,
	"Like \b-type\b, except if symbolic links are followed, the test"
	" is applied to the symbolic link itself, otherwise the test is applied"
	" to the pointed to file. Equivalent to \b-type\b if no symbolic"
	" links are involved.",
	0,
};

union Item
{
	struct Node*	np;
	char*		cp;
	Cmdarg_t*	xp;
	regex_t*	re;
	Sfio_t*		fp;
	unsigned long	u;
	long		l;
	int		i;
};

struct Node
{
	struct Node*	next;
	const char*	name;
	enum Command	action;
	union Item	first;
	union Item	second;
	union Item	third;
};

struct Format
{
	Sffmt_t		fmt;
	Ftw_t*		ftw;
};

static unsigned int	minlevel = 0;
static unsigned int	maxlevel = ~0;
static int		walkflags = FTW_PHYSICAL|FTW_DELAY|FTW_SEEDOTDIR;
static char		buf[LS_W_LONG+LS_W_INUMBER+LS_W_BLOCKS+2*PATH_MAX+1];
static char		txt[PATH_MAX+1];

static char*		usage;
static struct Node*	cmd;
static Proc_t*		proc;
static struct Node*	topnode;
static struct Node*	lastnode;
static struct Node*	nextnode;
static char*		cpio[] = { "cpio", "-o", 0 };
static char*		ncpio[] = { "cpio", "-ocB", 0 };
static unsigned long	now;
static unsigned long	day;
static Sfio_t*		output;
static char*		dummyarg = (char*)-1;
static char*		codes;
static char*		fast;
static int		icase;
static int		lastval;
static int		primary;
static int		reverse;
static int		silent;
static int		(*sort)(Ftw_t*, Ftw_t*);
static enum Command	sortkey = IGNORE;
static Magic_t*		magic;
static Magicdisc_t	magicdisc;
static struct Format	fmt;
static Sfio_t*		tmp;

/*
 *  Table lookup routine
 */

static struct Args*
lookup(register char* word)
{
	register struct Args*	argp;
	register int		second;

	while (*word == '-')
		word++;
	if (*word)
	{
		second = word[1];
		for (argp = (struct Args*)commands; argp->name; argp++)
			if (second == argp->name[1] && streq(word, argp->name))
				return argp;
	}
	return 0;
}

/*
 * quote path component to sp for xargs(1)
 */

static void
quotex(register Sfio_t* sp, register const char* s, int term)
{
	register int	c;

	while (c = *s++)
	{
		if (isspace(c) || c == '\\' || c == '\'' || c == '"')
			sfputc(sp, '\\');
		sfputc(sp, c);
	}
	if (term >= 0)
		sfputc(sp, term);
}

/*
 * printf %! extension function
 */

static int
print(Sfio_t* sp, void* vp, Sffmt_t* dp)
{
	register struct Fmt*	fp = (struct Fmt*)dp;
	register Ftw_t*		ftw = fp->ftw;
	union Value*		value = (union Value*)vp;

	char*			s;

	if (dp->n_str > 0)
		sfsprintf(s = fp->tmp, sizeof(fp->tmp), "%.*s", dp->n_str, dp->t_str);
	else
		s = 0;
	switch (dp->fmt)
	{
	case 'A':
		dp->fmt = 's';
		dp->size = -1;
		value->s = fmttime(s, ftw->statb.st_atime);
		break;
	case 'b':
		dp->fmt = 'u';
		value->u = iblocks(&ftw->statb);
		break;
	case 'C':
		dp->fmt = 's';
		dp->size = -1;
		value->s = fmttime(s, ftw->statb.st_ctime);
		break;
	case 'd':
		dp->fmt = 'u';
		value->u = ftw->level;
		break;
	case 'H':
		while (ftw->level > 0)
			ftw = ftw->parent;
		/*FALLTHROUGH*/
	case 'f':
		dp->fmt = 's';
		dp->size = ftw->namelen;
		value->s = ftw->name;
		break;
	case 'F':
		dp->fmt = 's';
		dp->size = -1;
		value->s = fmtfs(&ftw->statb);
		break;
	case 'g':
		dp->fmt = 's';
		dp->size = -1;
		value->s = fmtgid(ftw->statb.st_gid);
		break;
	case 'G':
		dp->fmt = 'd';
		value->i = ftw->statb.st_gid;
		break;
	case 'i':
		dp->fmt = 'u';
		value->u = ftw->statb.st_ino;
		break;
	case 'k':
		dp->fmt = 'u';
		value->u = iblocks(&ftw->statb);
		break;
	case 'm':
		dp->fmt = 'o';
		value->i = ftw->statb.st_mode;
		break;
	case 'n':
		dp->fmt = 'u';
		value->u = ftw->statb.st_nlink;
		break;
	case 'p':
		dp->fmt = 's';
		dp->size = ftw->pathlen;
		value->s = ftw->path;
		break;
	case 'P':
		dp->fmt = 's';
		dp->size = -1;
		s = ftw->path;
		while (ftw->level > 0)
			ftw = ftw->parent;
		s += ftw->pathlen;
		if (*s == '/')
			s++;
		value->s = s;
		break;
	case 's':
		dp->fmt = 'u';
		value->u = ftw->statb.st_size;
		break;
	case 'T':
		dp->fmt = 's';
		dp->size = -1;
		value->s = fmttime(s, ftw->statb.st_mtime);
		break;
	case 'u':
		dp->fmt = 's';
		dp->size = -1;
		value->s = fmtgid(ftw->statb.st_uid);
		break;
	case 'U':
		dp->fmt = 'd';
		value->i = ftw->statb.st_uid;
		break;
	case 'x':
		dp->fmt = 's';
		quotex(tmp, ftw->path, -1);
		dp->size = sfstrtell(tmp);
		value->s = sfstruse(tmp);
		break;
	case 'X':
		dp->fmt = 's';
		s = ftw->path;
		while (ftw->level > 0)
			ftw = ftw->parent;
		s += ftw->pathlen;
		if (*s == '/')
			s++;
		quotex(tmp, s, -1);
		dp->size = sfstrtell(tmp);
		value->s = sfstruse(tmp);
		break;
	case 'Y':
		if (s)
		{
			switch (*s)
			{
			case 'H':
				dp->fmt = 's';
				dp->size = -1;
				value->s = "ERROR";
				break;
			case 'h':
				dp->fmt = 's';
				if (s = strrchr(ftw->path, '/'))
				{
					value->s = ftw->path;
					dp->size = s - ftw->path;
				}
				else
				{
					value->s = ".";
					dp->size = 1;
				}
				break;
			case 'l':
				dp->fmt = 's';
				dp->size = -1;
				value->s = S_ISLNK(ftw->statb.st_mode) && pathgetlink(PATH(ftw), fp->tmp, sizeof(fp->tmp)) > 0 ? fp->tmp : "";
				break;
			default:
				error(2, "%%(%s)Y: invalid %%Y argument", s);
				return -1;
			}
			break;
		}
		/*FALLTHROUGH*/
	default:
		error(2, "internal error: %%%c: unknown format", dp->fmt);
		return -1;
	case 'Z':
		dp->fmt = 'c';
		value->i = 0;
		break;
	}
	dp->flags |= SFFMT_VALUE;
	return 0;
}

/*
 * convert the gnu-style-find printf format string for sfio extension
 */

static char*
format(register char* s)
{
	register char*	t;
	register int	c;
	char*		b;

	stresc(s);
	c = strlen(s);
	if (!(t = newof(0, char, c * 2, 0)))
		error(ERROR_SYSTEM|3, "out of space [format]");
	b = t;
	while (c = *s++)
	{
		if (c == '%')
		{
			if (*s == '%')
			{
				*t++ = c;
				*t++ = *s++;
			}
			else
			{
				do
				{
					*t++ = c;
				} while ((c = *s++) && !isalpha(c));
				if (!c)
					break;
				switch (c)
				{
				case 'A':
				case 'C':
				case 'T':
					*t++ = '(';
					*t++ = '%';
					if ((*t++ = *s) && *s++ == '@')
						*(t - 1) = '@';
					*t++ = ')';
					break;
				case 'a':
				case 'c':
				case 't':
					c = toupper(c);
					break;
				case 'H':
				case 'h':
				case 'l':
					*t++ = '(';
					*t++ = c;
					*t++ = ')';
					c = 'Y';
					break;
				case 'b':
				case 'd':
				case 'f':
				case 'F':
				case 'g':
				case 'G':
				case 'i':
				case 'k':
				case 'm':
				case 'n':
				case 'p':
				case 'P':
				case 's':
				case 'u':
				case 'U':
				case 'x':
				case 'X':
				case 'Z':
					break;
				default:
					error(3, "%%%c: unknown format", c);
					break;
				}
			}
		}
		*t++ = c;
	}
	*t = 0;
	return b;
}

/*
 * compile the arguments
 */

static int
compile(char** argv, register struct Node* np)
{
	register char*			b;
	register struct Node*		oldnp = 0;
	register const struct Args*	argp;
	struct Node*			tp;
	char*				e;
	char**				com;
	int				index = opt_info.index;
	int				i;
	int				k;
	enum Command			oldop = PRINT;

	for (;;)
	{
		if ((i = optget(argv, usage)) > 0)
		{
			k = argv[opt_info.index-1][0];
			if (i == '?')
				error(ERROR_USAGE|4, "%s", opt_info.arg);
			if (i == ':')
				error(2, "%s", opt_info.arg);
			continue;
		}
		else if (i == 0)
		{
			if (e = argv[opt_info.index])
			{
				k = e[0];
				if (!e[1] || e[1] == k && !e[2])
					switch (k)
					{
					case '(':
						argv[opt_info.index] = "-begin";
						continue;
					case ')':
						argv[opt_info.index] = "-end";
						continue;
					case '!':
						argv[opt_info.index] = "-not";
						continue;
					case '&':
						argv[opt_info.index] = "-and";
						continue;
					case '|':
						argv[opt_info.index] = "-or";
						continue;
					}
			}
			oldop = PRINT;
			break;
		}
		argp = commands - (i + 10);
		primary |= argp->primary;
		np->next = 0;
		np->name = argp->name;
		np->action = argp->action;
		np->second.i = 0; 
		np->third.u = 0; 
		if (argp->type & Stat)
			walkflags &= ~FTW_DELAY;
		if (argp->type & Op)
		{
			if (oldop == NOT || np->action != NOT && (oldop != PRINT || !oldnp))
				error(3, "%s: operator syntax error", np->name);
			oldop = argp->action;
		}
		else
		{
			oldop = PRINT;
			if (!(argp->type & Unary))
			{
				b = opt_info.arg;
				switch (argp->type & ~Stat)
				{
				case File:
					if (streq(b, "/dev/stdout") || streq(b, "/dev/fd/1"))
						np->first.fp = output;
					else if (!(np->first.fp = sfopen(NiL, b, "w")))
						error(ERROR_SYSTEM|3, "%s: cannot write", b);
					break;
				case Num:
					if (*b == '+' || *b == '-')
					{
						np->second.i = *b; 
						b++;
					}
					np->first.u = strtoul(b, &e, 0);
					switch (*e++)
					{
					default:
						e--;
						/*FALLTHROUGH*/
					case 'b':
						np->third.u = 512;
						break;
					case 'c':
						break;
					case 'g':
						np->third.u = 1024 * 1024 * 1024;
						break;
					case 'k':
						np->third.u = 1024;
						break;
					case 'm':
						np->third.u = 1024 * 1024;
						break;
					case 'w':
						np->third.u = 2;
						break;
					}
					if (*e)
						error(1, "%s: invalid character%s after number", e, *(e + 1) ? "s" : "");
					break;
				default:
					np->first.cp = b;
					break;
				}
			}
		}
		switch (argp->action)
		{
		case AND:
			continue;
		case OR:
		case COMMA:
			np->first.np = topnode;
			topnode = np;
			oldnp->next = 0;
			break;
		case LPAREN:
			tp = topnode;
			topnode = np + 1;
			i = compile(argv, topnode);
			np->first.np = topnode;
			topnode = tp;
			oldnp = np;
			np->next = np + i;
			np += i;
			continue;
		case RPAREN:
			if (!oldnp)
				error(3, "(...) imbalance", np->name);
			oldnp->next = 0;
			return opt_info.index - index;
		case LOGIC:
			walkflags &= ~(FTW_META|FTW_PHYSICAL);
		ignore:
			np->action = IGNORE;
			continue;
		case META:
			walkflags |= FTW_META|FTW_PHYSICAL;
			goto ignore;
		case PHYS:
			walkflags &= ~FTW_META;
			walkflags |= FTW_PHYSICAL;
			goto ignore;
		case XDEV:
			walkflags |= FTW_MOUNT;
			goto ignore;
		case POST:
			walkflags |= FTW_TWICE;
			goto ignore;
		case CHECK:
			silent = 0;
			goto ignore;
		case NOLEAF:
			goto ignore;
		case REVERSE:
			reverse = 1;
			goto ignore;
		case SILENT:
			silent = 1;
			goto ignore;
		case CODES:
			codes = b;
			goto ignore;
		case FAST:
			fast = b;
			goto ignore;
		case ICASE:
			icase = 1;
			goto ignore;
		case LOCAL:
			np->first.l = 0;
			np->second.i = '-';
			break;
		case ATIME:
		case CTIME:
		case MTIME:
			switch (np->second.i)
			{
			case '+':
				np->second.u = day - np->first.u * DAY;
				np->first.u = 0;
				break;
			case '-':
				np->first.u = day - np->first.u * DAY;
				np->second.u = ~0;
				break;
			default:
				np->second.u = day - np->first.u * DAY;
				np->first.u = np->second.u - DAY;
				break;
			}
			break;
		case AMIN:
		case CMIN:
		case MMIN:
			np->action--;
			switch (np->second.i)
			{
			case '+':
				np->second.u = now - np->first.u * 60;
				np->first.u = 0;
				break;
			case '-':
				np->first.u = now - np->first.u * 60;
				np->second.u = ~0;
				break;
			default:
				np->second.u = now - np->first.u * 60;
				np->first.u = np->second.u - 60;
				break;
			}
			break;
		case USER:
			if ((np->first.l = struid(b)) < 0)
				error(3, "%s: invalid user name", np->name);
			break;
		case GROUP:
			if ((np->first.l = strgid(b)) < 0)
				error(3, "%s: invalid group name", np->name);
			break;
		case EXEC:
		case OK:
		case XARGS:
			walkflags |= FTW_DOT;
			com = argv + opt_info.index - 1;
			i = np->action == XARGS ? 0 : 1;
			k = np->action == OK ? CMD_QUERY : 0;
			for (;;)
			{
				if (!(b = argv[opt_info.index++]))
					error(3, "incomplete statement");
				if (streq(b, ";"))
					break;
				if (streq(b, "+"))
				{
					i = 0;
					break;
				}
				if (strmatch(b, "*{}*"))
					k |= CMD_INSERT;
			}
			argv[opt_info.index - 1] = 0;
			if (k & CMD_INSERT)
				i = 1;
			if (!(np->first.xp = cmdopen(com, i, 0, "{}", k)))
				error(ERROR_SYSTEM|3, "out of space [args]");
			np->second.np = cmd;
			cmd = np;
			break;
		case MAGIC:
		case MIME:
			if (!magic)
			{
				magicdisc.version = MAGIC_VERSION;
				magicdisc.flags = 0;
				magicdisc.errorf = errorf;
				if (!(magic = magicopen(&magicdisc)) || magicload(magic, NiL, 0))
					error(3, "%s: cannot load magic file", MAGIC_FILE);
			}
			break;
		case IREGEX:
		case REGEX:
			if (!(np->second.re = newof(0, regex_t, 1, 0)))
				error(ERROR_SYSTEM|3, "out of space [re]");
			i = REG_EXTENDED|REG_LENIENT|REG_NOSUB|REG_NULL|REG_LEFT|REG_RIGHT;
			if (argp->action == IREGEX)
			{
				i |= REG_ICASE;
				np->action = REGEX;
			}
			if (i = regcomp(np->second.re, b, i))
				regfatal(np->second.re, 3, i);
			break;
		case PERM:
			np->first.l = strperm(b, &e, 0);
			if (*e)
				error(3, "%s: invalid permission expression", e);
			break;
		case SORT:
			if (!(argp = lookup(b)))
				error(3, "%s: invalid sort key", b);
			sortkey = argp->action;
			goto ignore;
		case TYPE:
		case XTYPE:
			np->first.l = *b;
			break;
		case CPIO:
			com = cpio;
			goto common;
		case NCPIO:
			{
				long	ops[2];
				int	fd;

				com = ncpio;
			common:
				/*
				 * set up cpio
				 */

				if ((fd = open(b, O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)) < 0)
					error(3, "%s: cannot create", b);
				ops[0] = PROC_FD_DUP(fd, 1, PROC_FD_PARENT|PROC_FD_CHILD);
				ops[1] = 0;
				if (!(proc = procopen("cpio", com, NiL, ops, PROC_WRITE)))
					error(3, "cpio: cannot exec");
				if (!(output = sfnew(NiL, NiL, -1, proc->wfd, SF_WRITE)))
					error(3, "cpio: cannot write");
				walkflags |= FTW_TWICE;
				np->action = PRINT;
			}
			/*FALLTHROUGH*/
		case PRINT:
			np->first.fp = output;
			np->second.i = '\n';
			break;
		case PRINT0:
			np->first.fp = output;
			np->second.i = 0;
			np->action = PRINT;
			break;
		case PRINTF:
			np->second.cp = format(np->first.cp);
			np->first.fp = output;
			break;
		case PRINTX:
			np->first.fp = output;
			np->second.i = '\n';
			break;
		case FPRINT:
			np->second.i = '\n';
			np->action = PRINT;
			break;
		case FPRINT0:
			np->second.i = 0;
			np->action = PRINT;
			break;
		case FPRINTF:
			if (!(b = argv[opt_info.index++]))
				error(3, "incomplete statement");
			np->second.cp = format(b);
			break;
		case FPRINTX:
			np->second.i = '\n';
			np->action = PRINTX;
			break;
		case LS:
			np->first.fp = output;
			if (sortkey == IGNORE)
				sortkey = NAME;
			break;
		case FLS:
			if (sortkey == IGNORE)
				sortkey = NAME;
			np->action = LS;
			break;
		case NEWER:
		case ANEWER:
		case CNEWER:
			{
				struct stat	st;

				if (stat(b, &st))
					error(3, "%s: not found", b);
				np->first.l = st.st_mtime;
				np->second.i = '+';
			}
			break;
		case CHOP:
			walkflags |= FTW_NOSEEDOTDIR;
			goto ignore;
		case DAYSTART:
			{
				Tm_t*	tm;
				time_t	t;

				t = now;
				tm = tmmake(&t);
				tm->tm_hour = 0;
				tm->tm_min = 0;
				tm->tm_sec = 0;
				day = tmtime(tm, TM_LOCALZONE);
			}
			goto ignore;
		case MINDEPTH:
			minlevel = np->first.l;
			goto ignore;
		case MAXDEPTH:
			maxlevel = np->first.l;
			goto ignore;
		case TEST:
			day = np->first.u;
			goto ignore;
		}
		oldnp = np;
		oldnp->next = ++np;
	}
	if (oldop != PRINT)
		error(2, "%s: invalid argument", argv[opt_info.index - 1]);
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	nextnode = np;
	if (lastnode = oldnp)
		oldnp->next = 0;
	return opt_info.index - index;
}

/*
 * This is the function that gets executed at each node
 */

static int
execute(Ftw_t* ftw)
{
	register struct Node*	np = topnode;
	register int		val;
	register unsigned long	u;
	unsigned long		m;
	int			not = 1;
	Sfio_t*			fp;
	struct Node*		tp;
	struct stat		st;
	DIR*			dir;
	struct dirent*		ent;

	if (ftw->level > maxlevel)
	{
		ftw->status = FTW_SKIP;
		return 0;
	}
	switch (ftw->info)
	{
	case FTW_DP:
		if ((walkflags & FTW_DOT) && stat(PATH(ftw), &ftw->statb))
			return 0;
		break;
	case FTW_NS:
		if (!silent)
			error(2, "%s: not found", ftw->path);
		return 0;
	case FTW_DC:
		if (!silent)
			error(2, "%s: directory causes cycle", ftw->path);
		return 0;
	case FTW_DNR:
		if (!silent)
			error(2, "%s: cannot read directory", ftw->path);
		break;
	case FTW_DNX:
		if (!silent)
			error(2, "%s: cannot search directory", ftw->path);
		ftw->status = FTW_SKIP;
		break;
	case FTW_D:
		if (walkflags & FTW_TWICE)
			return 0;
		ftw->ignorecase = (icase || (!ftw->level || !ftw->parent->ignorecase) && strchr(astconf("PATH_ATTRIBUTES", ftw->name, NiL), 'c')) ? STR_ICASE : 0;
		break;
	default:
		ftw->ignorecase = ftw->level ? ftw->parent->ignorecase : (icase || strchr(astconf("PATH_ATTRIBUTES", ftw->name, NiL), 'c')) ? STR_ICASE : 0;
		break;
	}
	if (ftw->level < minlevel)
		return 0;
	while (np)
	{
		switch (np->action)
		{
		case NOT:
			not = !not;
			np = np->next;
			continue;
		case COMMA:
		case LPAREN:
		case OR:
			tp = topnode;
			topnode = np->first.np;
			execute(ftw);
			val = lastval;
			topnode = tp;
			switch (np->action)
			{
			case COMMA:
				val = 1;
				break;
			case OR:
				if (val)
					return 0;
				val = 1;
				break;
			}
			break;
		case LOCAL:
			u = ftwlocal(ftw);
			goto num;
		case XTYPE:
			val = ((walkflags & FTW_PHYSICAL) ? stat(PATH(ftw), &st) : lstat(PATH(ftw), &st)) ? 0 : st.st_mode;
			goto type;
		case TYPE:
			val = ftw->statb.st_mode;
		type:
			switch (np->first.l)
			{
			case 'b':
				val = S_ISBLK(val);
				break;
			case 'c':
				val = S_ISCHR(val);
				break;
			case 'd':
				val = S_ISDIR(val);
				break;
			case 'f':
				val = S_ISREG(val);
				break;
			case 'l':
				val = S_ISLNK(val);
				break;
			case 'p':
				val = S_ISFIFO(val);
				break;
#ifdef S_ISSOCK
			case 's':
				val = S_ISSOCK(val);
				break;
#endif
#ifdef S_ISCTG
			case 'C':
				val = S_ISCTG(val);
				break;
#endif
#ifdef S_ISDOOR
			case 'D':
				val = S_ISDOOR(val);
				break;
#endif
			default:
				val = 0;
				break;
			}
			break;
		case PERM:
			u = modex(ftw->statb.st_mode) & 07777;
			switch (np->second.i)
			{
			case '-':
				val = (u & np->first.u) == np->first.u;
				break;
			case '+':
				val = (u & np->first.u) != 0;
				break;
			default:
				val = u == np->first.u;
				break;
			}
			break;
		case INUM:
			u = ftw->statb.st_ino;
			goto num;
		case ATIME:
			u = ftw->statb.st_atime;
			goto tim;
		case CTIME:
			u = ftw->statb.st_ctime;
			goto tim;
		case MTIME:
			u = ftw->statb.st_mtime;
		tim:
			val = u >= np->first.u && u <= np->second.u;
			break;
		case NEWER:
			val = (unsigned long)ftw->statb.st_mtime > (unsigned long)np->first.u;
			break;
		case ANEWER:
			val = (unsigned long)ftw->statb.st_atime > (unsigned long)np->first.u;
			break;
		case CNEWER:
			val = (unsigned long)ftw->statb.st_ctime > (unsigned long)np->first.u;
			break;
		case SIZE:
			u = ftw->statb.st_size;
			goto num;
		case USER:
			u = ftw->statb.st_uid;
			goto num;
		case NOUSER:
			val = *fmtuid(ftw->statb.st_uid);
			val = isdigit(val);
			break;
		case GROUP:
			u = ftw->statb.st_gid;
			goto num;
		case NOGROUP:
			val = *fmtgid(ftw->statb.st_gid);
			val = isdigit(val);
			break;
		case LINKS:
			u = ftw->statb.st_nlink;
		num:
			if (m = np->third.u)
				u = (u + m - 1) / m;
			switch (np->second.i)
			{
			case '+':
				val = (u > np->first.u);
				break;
			case '-':
				val = (u < np->first.u);
				break;
			default:
				val = (u == np->first.u);
				break;
			}
			break;
		case EXEC:
		case OK:
		case XARGS:
			val = !cmdarg(np->first.xp, ftw->path, ftw->pathlen);
			break;
		case NAME:
			val = strgrpmatch(ftw->name, np->first.cp, NiL, 0, STR_MAXIMAL|STR_LEFT|STR_RIGHT|ftw->ignorecase) != 0;
			break;
		case INAME:
			val = strgrpmatch(ftw->name, np->first.cp, NiL, 0, STR_MAXIMAL|STR_LEFT|STR_RIGHT|STR_ICASE) != 0;
			break;
		case LNAME:
			val = S_ISLNK(ftw->statb.st_mode) && pathgetlink(PATH(ftw), txt, sizeof(txt)) > 0 && strgrpmatch(txt, np->first.cp, NiL, 0, STR_MAXIMAL|STR_LEFT|STR_RIGHT|ftw->ignorecase);
			break;
		case ILNAME:
			val = S_ISLNK(ftw->statb.st_mode) && pathgetlink(PATH(ftw), txt, sizeof(txt)) > 0 && strgrpmatch(txt, np->first.cp, NiL, 0, STR_MAXIMAL|STR_LEFT|STR_RIGHT|STR_ICASE);
			break;
		case PATH:
			val = strgrpmatch(ftw->path, np->first.cp, NiL, 0, STR_MAXIMAL|STR_LEFT|STR_RIGHT|ftw->ignorecase) != 0;
			break;
		case IPATH:
			val = strgrpmatch(ftw->path, np->first.cp, NiL, 0, STR_MAXIMAL|STR_LEFT|STR_RIGHT|STR_ICASE) != 0;
			break;
		case MAGIC:
			fp = sfopen(NiL, PATH(ftw), "r");
			val = strmatch(magictype(magic, fp, PATH(ftw), &ftw->statb), np->first.cp) != 0;
			if (fp)
				sfclose(fp);
			break;
		case MIME:
			fp = sfopen(NiL, PATH(ftw), "r");
			magicdisc.flags |= MAGIC_MIME;
			val = strmatch(magictype(magic, fp, PATH(ftw), &ftw->statb), np->first.cp) != 0;
			magicdisc.flags &= ~MAGIC_MIME;
			if (fp)
				sfclose(fp);
			break;
		case REGEX:
			if (!(val = regnexec(np->second.re, ftw->path, ftw->pathlen, NiL, 0, 0)))
				val = 1;
			else if (val == REG_NOMATCH)
				val = 0;
			else
				regfatal(np->second.re, 3, val);
			break;
		case PRINT:
			sfputr(np->first.fp, ftw->path, np->second.i);
			val = 1;
			break;
		case PRINTF:
			fmt.fmt.version = SFIO_VERSION;
			fmt.fmt.extf = print;
			fmt.fmt.form = np->second.cp;
			fmt.ftw = ftw;
			sfprintf(np->first.fp, "%!", &fmt);
			val = 1;
			break;
		case PRINTX:
			quotex(np->first.fp, ftw->path, np->second.i);
			val = 1;
			break;
		case PRUNE:
			ftw->status = FTW_SKIP;
			val = 1;
			break;
		case FSTYPE:
			val = strcmp(fmtfs(&ftw->statb), np->first.cp);
			break;
		case LS:
			fmtls(buf, ftw->path, &ftw->statb, NiL, S_ISLNK(ftw->statb.st_mode) && pathgetlink(PATH(ftw), txt, sizeof(txt)) > 0 ? txt : NiL, LS_LONG|LS_INUMBER|LS_BLOCKS);
			sfputr(np->first.fp, buf, '\n');
			val = 1;
			break;
		case EMPTY:
			if (S_ISREG(ftw->statb.st_mode))
				val = !ftw->statb.st_size;
			else if (!S_ISDIR(ftw->statb.st_mode))
				val = 0;
			else if (!ftw->statb.st_size)
				val = 1;
			else if (!(dir = opendir(ftw->path)))
			{
				if (!silent)
					error(2, "%s: cannot read directory", ftw->path);
				val = 0;
			}
			else
			{
				val = 1;
				while ((ent = readdir(dir)) && (ent->d_name[0] == '.' && (!ent->d_name[1] || ent->d_name[1] == '.' && !ent->d_name[2])));
				closedir(dir);
			}
			break;
		case CFALSE:
			val = 0;
			break;
		case CTRUE:
			val = 1;
			break;
		case LEVEL:
			u = ftw->level;
			goto num;
		default:
			error(3, "internal error: %s: action not implemented", np->name);
			break;
		}
		lastval = val;
		if (val ^ not)
			return 0;
		not = 1;
		np = np->next;
	}
	return 0;
}

/*
 * order child entries
 */

static int
order(register Ftw_t* f1, register Ftw_t* f2)
{
	register long	n1;
	register long	n2;
	int		n;

	switch (sortkey)
	{
	case ATIME:
		n2 = f1->statb.st_atime;
		n1 = f2->statb.st_atime;
		break;
	case CTIME:
		n2 = f1->statb.st_ctime;
		n1 = f2->statb.st_ctime;
		break;
	case MTIME:
		n2 = f1->statb.st_mtime;
		n1 = f2->statb.st_mtime;
		break;
	case SIZE:
		n2 = f1->statb.st_size;
		n1 = f2->statb.st_size;
		break;
	default:
		error(1, "invalid sort key -- name assumed");
		sortkey = NAME;
		/*FALLTHROUGH*/
	case NAME:
		return icase ? strcasecmp(f1->name, f2->name) : strcoll(f1->name, f2->name);
	}
	if (n1 < n2)
		n = -1;
	else if (n1 > n2)
		n = 1;
	else
		n = 0;
	if (reverse)
		n = -n;
	return n;
}

main(int argc, char** argv)
{
	register char*			cp;
	register char**			op;
	register Find_t*		fp;
	register const struct Args*	ap;
	Sfio_t*				sp;
	int				r;
	Finddisc_t			disc;

	static char*	defpath[] = { ".", 0 };
	static char*	defopts[] = { 0, "-print", 0 };

	setlocale(LC_ALL, "");
	error_info.id = "find";
	if (!(sp = sfstropen()) || !(tmp = sfstropen()))
		error(ERROR_SYSTEM|3, "out of space [usage]");
	sfputr(sp, usage1, -1);
	for (ap = commands; ap->name; ap++)
	{
		sfprintf(sp, "[%d:%s?%s]", ap - commands + 10, ap->name, ap->help);
		if (ap->arg)
			sfprintf(sp, "%c[%s]", (ap->type & Num) ? '#' : ':', ap->arg);
		if (ap->values)
			sfprintf(sp, "{%s}", ap->values);
		sfputc(sp, '\n');
	}
	sfputr(sp, usage2, -1);
	usage = sfstruse(sp);
	day = now = (unsigned long)time(NiL);
	output = sfstdout;
	if (!(topnode = newof(0, struct Node, argc + 1, 0)))
		error(3, "not enough space for expressions");
	compile(argv, topnode);
	op = argv + opt_info.index;
	for (;;)
	{
		if (!(cp = argv[opt_info.index]))
		{
			argv = defopts;
			opt_info.index = 0;
			break;
		}
		if (*cp == '-' || (*cp == '!' || *cp == '(' || *cp == ')' || *cp == ',') && *(cp + 1) == 0)
		{
			r = opt_info.index;
			compile(argv, topnode);
			argv[r] = 0;
			break;
		}
		opt_info.index++;
	}
	if (cp = argv[opt_info.index])
		error(3, "%s: invalid argument", cp);
	if (!*op)
		op = defpath;
	while (topnode && topnode->action == IGNORE)
		topnode = topnode->next;
	if (!(walkflags & FTW_PHYSICAL))
		walkflags &= ~FTW_DELAY;
	if (fast)
	{
		if (sortkey != IGNORE)
			error(1, "-sort ignored for -fast");
		memset(&disc, 0, sizeof(disc));
		disc.version = FIND_VERSION;
		disc.flags = icase ? FIND_ICASE : 0;
		disc.errorf = errorf;
		disc.dirs = op;
		walkflags |= FTW_TOP;
		if (fp = findopen(codes, fast, NiL, &disc))
			while (cp = findread(fp))
			{
				if (topnode)
					ftwalk(cp, execute, walkflags, NiL);
				else
					sfputr(sfstdout, cp, '\n');
			}
	}
	else
	{
		if (!primary)
		{
			if (lastnode)
				lastnode->next = nextnode;
			else if (topnode)
				nextnode = topnode;
			else
				topnode = nextnode;
			nextnode->action = PRINT;
			nextnode->first.fp = output;
			nextnode->second.i = '\n';
			nextnode->next = 0;
		}
		fp = 0;
		if (sortkey != IGNORE)
			sort = order;
		ftwalk((char*)op, execute, walkflags|FTW_MULTIPLE, sort);
	}
	while (cmd)
	{
		cmdflush(cmd->first.xp);
		cmd = cmd->second.np;
	}
	if (fp && findclose(fp))
		return 1;
	if (proc && (r = procclose(proc)))
		return r;
	if (sfsync(sfstdout))
		error(ERROR_SYSTEM|2, "write error");
	return error_info.errors;
}
