/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1987-2003 AT&T Corp.                *
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
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * pax -- portable archive interchange
 *
 * test registry:
 *
 *	0010	dump option table
 *	0020	force DELTA_TEMP to file
 *	0040	pretend io device is char special
 *	0100	don't copy in holes
 */

static const char usage[] =
"[-?\n@(#)$Id: pax (AT&T Labs Research) 2003-09-11 $\n]"
USAGE_LICENSE
"[+NAME?pax - read, write, and list file archives]"
"[+DESCRIPTION?The pax command reads, writes, and lists archive files in"
"	various formats. There are four operation modes controlled by"
"	combinations of the -\br\b and -\bw\b options.]"
"[+?\bpax -w\b writes the files and directories named by the \apathname\a"
"	arguments to the standard output together with pathname and status"
"	information.  A directory \apathname\a argument refers to the files and"
"	(recursively) subdirectories of that directory.  If no \apathname\a"
"	arguments are given then the standard input is read to get a list of"
"	pathnames to copy, one pathname per line.  In this case only those"
"	pathnames appearing on the standard input are copied.]"
"[+?\bpax -r\b reads files from the standard input that is assumed to be"
"	the result of a previous \bpax -w\b command.  Only files with names"
"	that match any of the \apattern\a arguments are selected.  Matching"
"	is done before any \b-i\b or \b-s\b options are applied.  A"
"	\apattern\a is given in the name-generating notation of \bsh\b(1),"
"	except that the \b/\b character is also matched.  The default if no"
"	\apattern\a is given is \b*\b which selects all files.  The selected"
"	files are conditionally created and copied relative to the current"
"	directory tree, subject to the options described below.  By default the"
"	owner and group of selected files will be that of the current user, and"
"	the permissions and modify times will be the same as those in the"
"	archive.]"
"[+?\bpax -rw\b reads the files and directories named in the \apathname\a"
"	arguments and copies them to the destination \adirectory\a."
"	A directory \apathname\a argument refers to the files and (recursively)"
"	subdirectories of that directory.  If no \apathname\a arguments are"
"	given then the standard input is read to get a list of pathnames to"
"	copy, one pathname per line.  In this case only those pathnames"
"	appearing on the standard input are copied.  \adirectory\a must exist"
"	before the copy.]"
"[+?\bpax\b (\b-r\b and \b-w\b omitted) accepts \b-r\b operands and lists a"
"	table of contents of the selected files on the standard output.]"
"[+?The standard archive formats are automatically detected on input."
"	The default output archive format is implementation defined, but may be"
"	overridden by the \b-x\b option described below. \bpax\b archives may"
"	be concatenated to combine multiple volumes on a single tape or file."
"	This is accomplished by forcing any format prescribed pad data to be"
"	null bytes.  Hard links are not maintained between volumes, and delta"
"	and base archives cannot be multi-volume.]"
"[+?A single archive may span many files/devices.  The second and"
"	subsequent file names are prompted for on the terminal input.  The"
"	response may be:]{"
"		[+!command?Execute \acommand\a via \bsystem\b(3) and prompt"
"			again for file name.]"
"		[+EOF?Terminate processing and exit.]"
"		[+CR?An empty input line retains the previous file name.]"
"		[+pathname?The file name for the next archive part.]"
"}"
"[+?\bgetconf PATH_RESOLVE\b determines how symbolic links are handled. This"
"	can be explicitly overridden by the \b--logical\b, \b--metaphysical\b,"
"	and \b--physical\b options below. \bPATH_RESOLVE\b can be one of:]{"
"		[+logical?Follow all symbolic links.]"
"		[+metaphysical?Follow command argument symbolic links,"
"			otherwise don't follow.]"
"		[+physical?Don't follow symbolic links.]"
"}"
;

/* state.usage is generated at runtime from usage+options+usage2 */

static const char usage2[] =
"\n"
"[ pathname ... ]\n"
"[ pattern ... ]\n"
"[ pathname ... directory ]\n"
"\n"
"[+DIAGNOSTICS?The number of files, blocks, and optionally the number"
"	of volumes and media parts are listed on the standard error."
"	For -\bv\b the input archive formats are also listed on the"
"	standard error.]"
"[+EXAMPLES]{"
"	[+pax -w -t 1m .?Copies the contents of the current directory to"
"		tape drive 1, medium density.]"
"	[+mkdir newdir; cd olddir; pax -rw . newdir?Copies the"
"		\aolddir\a directory hierarchy to \anewdir\a.]"
"}"
"[+SEE ALSO?\bar\b(1), \bcpio\b(1), \bfind\b(1), \bgetconf\b(1), \bgzip\b(1),"
"	\bksh\b(1), \bratz\b(1), \btar\b(1), \btw\b(1), \blibdelta\b(3),"
"	\bcpio\b(5), \btar\b(5)]"
"[+BUGS?Special privileges may be required to copy special files."
"	Each archive format has a hard upper limit on member pathname sizes."
"	Device, user-id and group-id numbers larger than 65535 may cause"
"	additional header records to be output for some formats; these records"
"	are ignored by old versions of \bcpio\b(1) and \btar\b(1).]"
;

#include "pax.h"
#include "options.h"

#include <tm.h>

char*			definput = "/dev/stdin";
char*			defoutput = "/dev/stdout";
char*			eomprompt = "Change to part %d and hit RETURN: ";

char			alar_header[ALAR_LABEL];
Hdr_binary_t		binary_header;
Hdr_tar_t		tar_header_block;
char			zip_header[ZIP_HEADER];
State_t			state;

Format_t		format[] =
{

{ALAR_NAME,	ALAR_MATCH,	ALAR_DESC,	ALAR_REGULAR,	ALAR_SPECIAL,	ALAR_ALIGN,	ALAR_FLAGS},
{BINARY_NAME,	BINARY_MATCH,	BINARY_DESC,	BINARY_REGULAR,	BINARY_SPECIAL,	BINARY_ALIGN,	BINARY_FLAGS},
{CPIO_NAME,	CPIO_MATCH,	CPIO_DESC,	CPIO_REGULAR,	CPIO_SPECIAL,	CPIO_ALIGN,	CPIO_FLAGS},
{IBMAR_NAME,	IBMAR_MATCH,	IBMAR_DESC,	IBMAR_REGULAR,	IBMAR_SPECIAL,	IBMAR_ALIGN,	IBMAR_FLAGS},
{TAR_NAME,	TAR_MATCH,	TAR_DESC,	TAR_REGULAR,	TAR_SPECIAL,	TAR_ALIGN,	TAR_FLAGS},
{USTAR_NAME,	USTAR_MATCH,	USTAR_DESC,	USTAR_REGULAR,	USTAR_SPECIAL,	USTAR_ALIGN,	USTAR_FLAGS, },
{ASC_NAME,	ASC_MATCH,	ASC_DESC,	ASC_REGULAR,	ASC_SPECIAL,	ASC_ALIGN,	ASC_FLAGS},
{ASCHK_NAME,	ASCHK_MATCH,	ASCHK_DESC,	ASCHK_REGULAR,	ASCHK_SPECIAL,	ASCHK_ALIGN,	ASCHK_FLAGS},
{SAVESET_NAME,	SAVESET_MATCH,	SAVESET_DESC,	SAVESET_REGULAR,SAVESET_SPECIAL,SAVESET_ALIGN,	SAVESET_FLAGS},
{PAX_NAME,	PAX_MATCH,	PAX_DESC,	PAX_REGULAR,	PAX_SPECIAL,	PAX_ALIGN,	PAX_FLAGS},
{AR_NAME,	AR_MATCH,	AR_DESC,	AR_REGULAR,	AR_SPECIAL,	AR_ALIGN,	AR_FLAGS},
{VDB_NAME,	VDB_MATCH,	VDB_DESC,	VDB_REGULAR,	VDB_SPECIAL,	VDB_ALIGN,	VDB_FLAGS},
{ZIP_NAME,	ZIP_MATCH,	ZIP_DESC,	ZIP_REGULAR,	ZIP_SPECIAL,	ZIP_ALIGN,	ZIP_FLAGS},
{CAB_NAME,	CAB_MATCH,	CAB_DESC,	CAB_REGULAR,	CAB_SPECIAL,	CAB_ALIGN,	CAB_FLAGS},
{RPM_NAME,	RPM_MATCH,	RPM_DESC,	RPM_REGULAR,	RPM_SPECIAL,	RPM_ALIGN,	RPM_FLAGS},
{MIME_NAME,	MIME_MATCH,	MIME_DESC,	MIME_REGULAR,	MIME_SPECIAL,	MIME_ALIGN,	MIME_FLAGS},
{TNEF_NAME,	TNEF_MATCH,	TNEF_DESC,	TNEF_REGULAR,	TNEF_SPECIAL,	TNEF_ALIGN,	TNEF_FLAGS},

{COMPRESS_NAME,	COMPRESS_MATCH,	COMPRESS_DESC, COMPRESS_MAGIC_MASK, COMPRESS_MAGIC, 0, IN|OUT, COMPRESS_ALGORITHM, COMPRESS_UNDO},
{GZIP_NAME,	GZIP_MATCH,	GZIP_DESC,     GZIP_MAGIC_MASK, GZIP_MAGIC,         0, IN|OUT, GZIP_ALGORITHM,     GZIP_UNDO},
{BZIP_NAME,	BZIP_MATCH,	BZIP_DESC,     BZIP_MAGIC_MASK, BZIP_MAGIC,         0, IN|OUT, BZIP_ALGORITHM,     BZIP_UNDO},

{DELTA_NAME,		DELTA_MATCH,		DELTA_DESC,          0,0,0, IN|OUT, DELTA_ALGORITHM},
{DELTA_88_NAME,		DELTA_88_MATCH,		DELTA_88_DESC,       0,0,0, IN,     DELTA_88_ALGORITHM},
{DELTA_IGNORE_NAME,	DELTA_IGNORE_MATCH,	DELTA_IGNORE_DESC,   0,0,0, IN|OUT},
{DELTA_PATCH_NAME,	DELTA_PATCH_MATCH,	DELTA_PATCH_DESC,    0,0,0, IN|OUT},

{0}

};

Option_t		options[] =
{
{
	0,
},
{
	"append",
	'a',
	OPT_append,
	"Append to end of archive.",
},
{
	"atime",
	0,
	OPT_atime,
	"Preserve or set file access times.",
	"time",
	0,
	OPT_HEADER|OPT_OPTIONAL,
},
{
	"base",
	'z',
	OPT_base,
	"Two archives are required for delta operations. \b--file\b names\
	the delta archive and \b--base\b names the delta base archive.\
	If \aarchive\a is \b-\b then the base is ignored (the actual delta\
	sizes are reported and restored) on input and the delta is\
	compressed on output.",
	"archive",
},
{
	"blocksize",
	'b',
	OPT_blocksize,
	"Input/output block size. The default is format specific.",
	"size",
	0,
	OPT_NUMBER,
},
{
	"blok",
	0,
	OPT_blok,
	"Input/output BLOK format for tapes on file.",
	"i|o",
	0,
	OPT_OPTIONAL,
},
{
	"charset",
	0,
	OPT_charset,
	"Header data character set name.",
	"name",
	0,
	OPT_HEADER,
},
{
	"checksum",
	0,
	OPT_checksum,
	"Generate a \amethod\a checksum file for each archive member and add\
	the resulting file as the archive member \apath\a. See \bsum\b(1) for\
	supported checksum methods; \bmd5\b is a good candidate. The generated\
	file is suitable for input to \bsum --check --permissions\b.",
	"method::path",
},
{
	"chksum",
	0,
	OPT_chksum,
	"The header checksum string; empty if not supported.",
	0,
	0,
	OPT_READONLY,
},
{
	"clobber",
	'k',
	OPT_clobber,
	"Overwrite output files.",
	0,
	0,
	OPT_INVERT,
},
{
	"comment",
	0,
	OPT_comment,
	"Header comment text.",
	"text",
	0,
	OPT_HEADER,
},
{
	"complete",
	0,
	OPT_complete,
	"Complete archive must fit in one media part.",
},
{
	"crossdevice",
	'X',
	OPT_crossdevice,
	"Directory traversal may cross device boundaries.",
	0,
	0,
	OPT_INVERT,
},
{
	"ctime",
	0,
	OPT_ctime,
	"Preserve or set file change times.",
	"time",
	0,
	OPT_HEADER|OPT_OPTIONAL,
},
{
	"debug",
	'D',
	OPT_debug,
	"Set debug trace level. Higher levels produce more output.",
	"level",
	0,
	OPT_NUMBER,
},
{
	"delete",
	0,
	OPT_delete,
	"\bdelete\b=\apattern\a ignores all global and extended header keywords\
	matching the \bksh\b(1) \apattern\a.",
	"pattern",
},
{
	"delta",
	0,
	OPT_delta,
	"The header delta operation; empty if not a delta archive member.",
	0,
	0,
	OPT_READONLY,
},
{
	"descend",
	'd',
	OPT_descend,
	"Command line directories name themselves and descendents.",
	0,
	0,
	OPT_INVERT,
},
{
	"device",
	0,
	OPT_device,
	"The device major and minor number string, empty if not a device file.",
	0,
	0,
	OPT_READONLY,
},
{
	"devmajor",
	0,
	OPT_devmajor,
	"The major device number, 0 if not supported.",
	0,
	0,
	OPT_READONLY,
},
{
	"devminor",
	0,
	OPT_devminor,
	"The minor device number, 0 if not supported.",
	0,
	0,
	OPT_READONLY,
},
{
	"dir",
	0,
	OPT_dir,
	"File directory name (base elided).",
	0,
	0,
	OPT_READONLY,
},
{
	"dots",
	0,
	OPT_dots,
	"Print a dot on the standard error for each block transferred.",
},
{
	"edit",
	's',
	OPT_edit,
	"Pathname substitution from \aold\a to \anew\a.\
	The first character is the expression delimiter.\
	There may be more than one edit expression;\
	each is applied in order from left to right.",
	",old,new,[glsu]]",
	"[+g?All \aold\a patterns.]\
	[+l?Convert \anew\a to lower case.]\
	[+p?Print the edit result on the standard error.]\
	[+s?Stop edits on path if this edit succeeds.]\
	[+u?Convert \anew\a to upper case.]",
},
{
	"entry",
	0,
	OPT_entry,
	"File entry ordinal.",
	0,
	0,
	OPT_HEADER,
},
{
	"eom",
	0,
	OPT_eom,
	"End of media prompt or !\acommand\a. Processing terminates if !\acommand\a returns non-zero exit status.",
	"[!]]prompt",
},
{
	"exact",
	'n',
	OPT_exact,
	"Exit after each file arg matches exactly once.",
},
{
	"exthdr.name",
	0,
	OPT_extended_name,
	"Equivalent to \bheader\b:=\aformat\a.",
	"format",
},
{
	"file",
	'f',
	OPT_file,
	"The main archive file name.",
	"path",
},
{
	"filter",
	0,
	OPT_filter,
	"Input/output file filter command. \acommand\a is applied to each\
	file as it is read from or written to the archive. \acommand\a is\
	split into space separated arguments, and is executed with the\
	pathname of the file to be processed as the last argument.\
	The standard output of the resulting command is read by \bpax\b.\
	\b--nodescend\b is implied by \b--filter\b. If \acommand\a is \b-\b\
	and the archive is being written and there are no command line\
	\afile\a arguments, then each line on the standard input is\
	interpreted as a delimiter separated command:\
	\bX\b\aoptions\a\bX\b\acommand\a\bX\b\aphysical\a\bX\b\alogical\a,\
	where:",
	"command",
	"[+X?A delimiter character that does not appear outside quotes.]\
	[+options?\b,\b separated [\bno\b]]\aname\a[=\avalue\a]] options:]{\
		[+logical?Override the command line \b--logical\b and\
			\b--physical\b options for this file.]\
		[+physical?Override the command line \b--logical\b and\
			\b--physical\b options for this file.]\
	}\
	[+command?A shell command that reads the physical file and writes\
		the filtered contents to the standard output. If \acommand\a\
		is empty then the file contents are copied unchanged.]\
	[+physical?The actual file path, used for archive status.]\
	[+logical?The file path as it will appear in the archive. If\
		\alogical\a is empty then the \aphysical\a path is used. The\
		resulting path is still subject to any \b--edit\b options.]"
},
{
	"format",
	'x',
	OPT_format,
	"Archive format. The default ouput format is \bustar\b. Formats are\
	automatically detected on read. A basic and compress format may be\
	combined, separated by \b:\b. The supported formats are:",
	"format",
},
{
	"from",
	0,
	OPT_from,
	"File data input character set name.\
	Only files that have no control characters in the first 256 bytes\
	are converted.",
	"name",
	"[+NATIVE?Local system default.]\
	[+ASCII?Common US ASCII.]\
	[+ISO8859-1?8-bit ASCII.]\
	[+EBCDIC?One of the many flavors.]\
	[+IBM-1047?Another EBCDIC flavor.]",
},
{
	"gid",
	0,
	OPT_gid,
	"Header group id. The default is the group id of the invoking process.",
	"group",
	0,
	OPT_HEADER|OPT_NUMBER,
},
{
	"globexthdr.name",
	0,
	OPT_global_name,
	"Equivalent to \bheader\b=\aformat\a.",
	"format",
},
{
	"gname",
	0,
	OPT_gname,
	"Header group name. The default is the group name of the invoking process..",
	"group",
	0,
	OPT_HEADER,
},
{
	"header",
	0,
	OPT_header,
	"\bheader\b=\aformat\a sets the global header path name format to the\
	\blistopt\b \aformat\a. The default is \b" HEADER_EXTENDED_STD "\b\
	when strict conformance is in effect (see \bgetconf\b(1) CONFORMANCE)\
	and \b" HEADER_EXTENDED "\b otherwise. \bheader\b:=\aformat\a\
	sets the extended header path name format to the \blistopt\b\
	\aformat\a. The default is \b" HEADER_GLOBAL_STD "\b when strict\
	conformance is in effect and \b" HEADER_GLOBAL "\b otherwise.\
	The strict conformance defaults are prone to global header filename\
	collisions and are ill-defined when extended header names exceed the\
	underlying header format limits.",
	"format",
},
{
	"ignore",
	0,
	OPT_ignore,
	"\bignore\b=\apattern\a ignores all global and extended header keywords\
	matching the \bksh\b(1) \apattern\a; \bignore\b:=\apattern\a ignores\
	all extended header keywords matching \apattern\a.",
	"pattern",
},
{
	"ino",
	0,
	OPT_ino,
	"The file serial number, 0 if not supported.",
	0,
	0,
	OPT_READONLY,
},
{
	"install",
	0,
	OPT_install,
	"Generate an installation \bsh\b(1) script file that contains\
	\bchmod\b(1), \bchgrp\b(1) and \bchown\b(1) commands to restore\
	file modes and ownership not supported by standard \bcpio\b(1)\
	and \btar\b(1). Only files with user or group specific\
	read/execute/setuid permissions are included in the script.\
	The script is added to the archive with member name \apath\a.",
	"path",
},
{
	"intermediate",
	'I',
	OPT_intermediate,
	"Copy each file to an intermediate name and rename to the\
	real name if the intermediate copy succeeds; otherwise retain\
	the original file and issue a diagnostic.",
},
{
	"invalid",
	0,
	OPT_invalid,
	"Invalid path action:",
	"action",
	"[i:bypass|ignore?Silently ignore.]\
	[p:rename|prompt?Prompt for new name.]\
	[t:write|translate?Automatically translate and/or truncate\
		to local specifications.]\
	[u:utf-8|UTF-8?Convert to UTF-8.]",
},
{
	"invert",
	'c',
	OPT_invert,
	"Invert pattern match sense. The !(...) construct is more general.",
},
{
	"keepgoing",
	'K',
	OPT_keepgoing,
	"Attempt to skip over damaged input archive data.",
},
{
	"label",
	0,
	OPT_label,
	"Append \astring\a to the volume label; \blabel\b:=\astring\a\
	prepends \astring\a.",
	"string",
},
{
	"link",
	0,
	OPT_link,
	"Hard link files on output if possible.",
},
{
	"linkdata",
	0,
	OPT_linkdata,
	"Output data with each hard link. The default outputs the\
	data only with the first link in the archive.",
},
{
	"linkop",
	0,
	OPT_linkop,
	"The link operation string, \b==\b for hard links, \b->\b for\
	symbolic links, otherwise empty.",
	0,
	0,
	OPT_READONLY,
},
{
	"linkpath",
	0,
	OPT_linkpath,
	"Header symbolic link pathname.",
	"path",
	0,
	OPT_HEADER,
},
{
	"listformat|listopt",
	0,
	OPT_listformat,
	"Append to the member listing format string. \aformat\a follows\
	\bprintf\b(3) conventions, except that \bsfio\b(3) inline ids\
	are used instead of arguments:\
	%[-+]][\awidth\a[.\aprecis\a[.\abase\a]]]]]](\aid\a[:\asubformat\a]])\achar\a.\
	If \achar\a is \bs\b then the string form of the item is listed,\
	otherwise the corresponding numeric form is listed. \asubformat\a\
	overrides the default formatting for \aid\a. All of the file related\
	options are supported as \aid\as, along with the following:",
	"format",
},
{
	"listmacro",
	0,
	OPT_listmacro,
	"Define a \b--listformat\b macro.",
	"name[=value]]",
},
{
	"local",
	'l',
	OPT_local,
	"Reject files and links that traverse outside the current directory.",
},
{
	"logical|follow",
	'L',
	OPT_logical,
	"Follow symbolic links. The default is determined by\
	\bgetconf PATH_RESOLVE\b.",
},
{
	"magic",
	0,
	OPT_magic,
	"The header magic string; empty if not supported.",
	0,
	0,
	OPT_READONLY,
},
{
	"mark",
	0,
	OPT_mark,
	"The file type mark character string:",
	0,
	"[+=?hard link]\
	[+@?symbolic link]\
	[+/?directory]\
	[+|?fifo]\
	[+=?socket]\
	[+$?block or character special]\
	[+*?executable]",
	OPT_READONLY,
},
{
	"maxout",
	0,
	OPT_maxout,
	"Output media size limit.",
	"size",
	0,
	OPT_NUMBER,
},
{
	"metaphysical",
	'H',
	OPT_metaphysical,
	"Follow command argument symbolic links, otherwise don't follow.\
	The default is determined by \bgetconf PATH_RESOLVE\b.",
},
{
	"meter",
	'm',
	OPT_meter,
	"Display a one line text meter showing archive read progress.\
	The input must be seekable. \bcompress\b and \bbzip\b uncompressed\
	input size is estimated.",
},
{
	"mkdir",
	0,
	OPT_mkdir,
	"Create intermediate directories on output. On by default.",
},
{
	"mode",
	0,
	OPT_mode,
	"The file type and acces mode.",
	0,
	0,
	OPT_READONLY,
},
{
	"mtime",
	0,
	OPT_mtime,
	"Preserve or set file modify times.",
	"time",
	0,
	OPT_HEADER|OPT_OPTIONAL,
},
{
	"name",
	0,
	OPT_name,
	"File base name (directory elided).",
	0,
	0,
	OPT_HEADER,
},
{
	"nlink",
	0,
	OPT_nlink,
	"The hard link count.",
	0,
	0,
	OPT_READONLY,
},
{
	"options",
	'o',
	OPT_options,
	"Set options without leading -- from \astring\a.",
	"string",
},
{
	"ordered",
	'O',
	OPT_ordered,
	"Input files and base are ordered by name.",
},
{
	"owner",
	0,
	OPT_owner,
	"Change output file owner to current user or to \auid\a\
	if specified.",
	"uid",
},
{
	"path",
	0,
	OPT_path,
	"File path name.",
	"path",
	0,
	OPT_HEADER,
},
{
	"physical",
	'P',
	OPT_physical,
	"Don't follow symbolic links.\
	The default is determined by \bgetconf PATH_RESOLVE\b.",
},
{
	"pid",
	0,
	OPT_pid,
	"\bpax\b process id.",
},
{
	"preserve",
	'p',
	OPT_preserve,
	"Preserve selected file attributes:",
	"aemops",
	"[+a?Don't preserve access time.]\
	[+e?Preserve everything permissible.]\
	[+m?Don't preserve modify time.]\
	[+o?Preserve user, group, setuid and setgid.]\
	[+p?Preserve mode.]\
	[+s?Preserve setuid and setgid.]",
	OPT_OPTIONAL,
},
{
	"read",
	'r',
	OPT_read,
	"Read files from the archive.",
},
{
	"record-charset",
	0,
	OPT_record_charset,
	"Enable character set translation. On by default.",
},
{
	"record-delimiter",
	0,
	OPT_record_delimiter,
	"\bvdb\b format record delimiter character.\
	No delimiter if omitted. The default is ; .",
	"char",
},
{
	"record-format",
	0,
	OPT_record_format,
	"Labeled tape record format:",
	"DFSUV",
	"[+D?decimal variable]\
	[+F?fixed length]\
	[+S?spanned]\
	[+U?input block size]\
	[+B?binary variable]",
},
{
	"record-header",
	0,
	OPT_record_header,
	"Member header, NULL if omitted. The default value is format specific.",
},
{
	"record-line",
	0,
	OPT_record_line,
	"Records are lines. The default is format specific.",
},
{
	"record-match",
	0,
	OPT_record_match,
	"Select record formats that match \apattern\a.",
	"pattern",
},
{
	"record-pad",
	0,
	OPT_record_pad,
	"Pad records. The default is format specific.",
},
{
	"record-size",
	0,
	OPT_record_size,
	"Fixed length record size. The default is format specific.",
	"size",
	0,
	OPT_NUMBER,
},
{
	"record-trailer",
	0,
	OPT_record_trailer,
	"Member trailer, NULL if omitted. The default is format specific.",
	"string",
},
{
	"release",
	0,
	OPT_release,
	"The \bpax\b implementation release stamp.",
	"string",
	0,
	OPT_GLOBAL|OPT_IGNORE|OPT_VENDOR,
},
{
	"reset-atime",
	't',
	OPT_reset_atime,
	"Reset the file access times of copied files.",
},
{
	"sequence",
	0,
	OPT_sequence,
	"The archive member sequence number. Numbers for volumes >1 are of the\
	form \avolume\a.\asequence\a.",
	0,
	0,
	OPT_READONLY,
},
{
	"size",
	0,
	OPT_size,
	"File size.",
	"size",
	0,
	OPT_HEADER|OPT_OPTIONAL,
},
{
	"strict",
	'S',
	OPT_strict,
	"Disable non-standard \b--write\b mode extensions. The default is\
	determined by the \bgetconf\b(1) CONFORMANCE setting.",
},
{
	"summary",
	0,
	OPT_summary,
	"List summary information for each archive. On by default.",
},
{
	"symlink",
	0,
	OPT_symlink,
	"symlink files if possible.",
},
{
	"tape",
	0,
	OPT_tape,
	"Canonical tape unit name and operations.",
	"[#]][lmhcu]][n]][bv]][s[#]]]][k[#]]]]",
	"[+#?unit number [0-9]]]\
	[+l?low density]\
	[+m?medium density]\
	[+h?high density]\
	[+c?compressed]\
	[+u?uncompressed]\
	[+n?don't rewind on close]\
	[+b?bsd behavior]\
	[+v?system V behavior]\
	[+s[#]]?skip all [#]] volumes]\
	[+k[#]]?keep all [#]] volumes after skip]",
},
{
	"test",
	'T',
	OPT_test,
	"Test mask for debugging. Multiple values are or'ed together.",
	"mask",
	0,
	OPT_NUMBER,
},
{
	"testdate",
	0,
	OPT_testdate,
	"\b--listformat\b time values newer than \adate\a will be printed\
	as \adate\a. Used for regression testing.",
	"date",
},
{
	"times",
	0,
	OPT_times,
	"Preserve \batime\b, \bctime\b, and \bmtime\b.",
},
{
	"tmp",
	0,
	OPT_tmp,
	"The value of the \bTMPDIR\b environment variable if defined,\
	otherwise \b/tmp\b.",
},
{
	"to",
	0,
	OPT_to,
	"Output character set. See \b--from\b above for possible values.",
	"name",
},
{
	"typeflag",
	0,
	OPT_typeflag,
	"The header type flag string; empty if not supported.",
	0,
	0,
	OPT_READONLY,
},
{
	"uid",
	0,
	OPT_uid,
	"Header user id. The default is the user id of the invoking process.",
	"user",
	0,
	OPT_HEADER|OPT_NUMBER,
},
{
	"uname",
	0,
	OPT_uname,
	"Header user name. The default is the user name of the invoking process.",
	"user",
	0,
	OPT_HEADER,
},
{
	"unblocked",
	0,
	OPT_unblocked,
	"Force unblocked input/output. The default is format specific.\
	Both input and output are unblocked if the value is omitted.",
	"i|o",
},
{
	"update",
	'u',
	OPT_update,
	"Only copy archive members that are newer than the target files.",
},
{
	"verbose",
	'v',
	OPT_verbose,
	"Produce long listings or list each file name as it is processed.",
},
{
	"verify",
	'i',
	OPT_verify,
	"Prompt for verification and/or rename.",
},
{
	"version",
	0,
	OPT_version,
	"The header version string; empty if not supported.",
	0,
	0,
	OPT_READONLY,
},
{
	"write",
	'w',
	OPT_write,
	"Write files to the archive.",
},
{
	"yes",
	'y',
	OPT_yes,
	"Prompt for yes/no file verification.",
},
};

static int		signals[] =	/* signals caught by interrupt() */
{
	SIGHUP,
	SIGINT,
#if !DEBUG
	SIGQUIT,
#endif
	SIGALRM,
	SIGTERM,
};

static struct
{
	char*		arg0;
	Sfio_t*		ignore_all;
	Sfio_t*		ignore_ext;
	Map_t*		lastmap;
	Sfio_t*		listformat;
	char*		owner;
} opt;

/*
 * clean up dir info before exit
 */

static void
interrupt(int sig)
{
	signal(sig, SIG_IGN);
	switch (sig)
	{
	case SIGINT:
	case SIGQUIT:
		sfprintf(sfstderr, "\n");
		break;
	}
	state.interrupt = sig;
	finish(1);
}

/*
 * enter new substitute expression(s)
 */

static void
substitute(Map_t** lastmap, register char* s)
{
	register Map_t*	mp;
	int		c;

	for (;;)
	{
		while (isspace(*s))
			s++;
		if (!*s)
			break;
		if (!(mp = newof(0, Map_t, 1, 0)))
			error(3, "no space [substitution]");
		if (!(c = regcomp(&mp->re, s, REG_DELIMITED|REG_LENIENT|REG_NULL)))
		{
			s += mp->re.re_npat;
			if (!(c = regsubcomp(&mp->re, s, NiL, 0, 0)))
				s += mp->re.re_npat;
		}
		if (c)
			regfatal(&mp->re, 3, c);
		if (*s && !isspace(*s))
			error(1, "invalid character after substitution: %s", s);
		if (*lastmap)
			*lastmap = (*lastmap)->next = mp;
		else
			state.maps = *lastmap = mp;
	}
}

/*
 * clear meter line for each error message
 */

static ssize_t
meterror(int fd, const void* buf, size_t n)
{
	if (state.meter.last)
	{
		sfprintf(sfstderr, "%*s", state.meter.last, "\r");
		state.meter.last = 0;
	}
	return write(fd, buf, n);
}

/*
 * set options from line if != 0 or argv according to usage
 * type: 0:command EXTTYPE:extended GLBTYPE:global
 */

void
setoptions(char* line, char** argv, char* usage, Archive_t* ap, int type)
{
	_ast_intmax_t	n;
	int		c;
	int		y;
	int		cvt;
	int		index;
	int		offset;
	int		from;
	int		to;
	int		last;
	char*		e;
	char*		s;
	char*		v;
	Option_t*	op;
	Value_t*	vp;

	cvt = 0;
	index = opt_info.index;
	offset = opt_info.offset;
	for (last = 0; c = line ? optstr(line, usage) : optget(argv, usage); last = opt_info.offset)
	{
		if (c > 0)
		{
			if (c == '?')
				error(ERROR_USAGE|4, "%s", opt_info.arg);
			if (c == ':')
			{
				/*
				 * NOTE: numeric options are ignored so
				 *	 extended headers can be passed
				 *	 as a single unit to setoptions()
				 */

				if (type && strneq(opt_info.name, VENDOR, sizeof(VENDOR) - 1) && opt_info.arg[sizeof(VENDOR) - 1] == '.')
				{
					while (isspace(line[last]))
						last++;
					opt_info.offset = last + sizeof(VENDOR); 
					continue;
				}
				if (!type || islower(*opt_info.name) && !strmatch(opt_info.name, "+([0-9])|*.*"))
					error(2, "%s", opt_info.arg);
			}
			continue;
		}
		y = (n = opt_info.number) != 0;
		if (!(v = opt_info.arg))
			v = "";
		else if (!n)
			y = 1;
		op = options - c;

		/*
		 * option precedence levels
		 *
		 *	8	ignore all
		 *	7	command:=
		 *	6	ignore extended
		 *	5	extended:=
		 *	4	extended=
		 *	3	command=
		 *	2	global:=
		 *	1	global=
		 */

		switch (type)
		{
		case EXTTYPE:
			c = (opt_info.assignment == ':') ? 5 : 4;
			vp = &op->temp;
			break;
		case GLBTYPE:
			c = (opt_info.assignment == ':') ? 2 : 1;
			vp = &op->perm;
			break;
		default:
			c = (opt_info.assignment == ':') ? 7 : 3;
			vp = &op->perm;
			break;
		}
		if (op->level > c)
			continue;
		if (y && (op->flags & (OPT_HEADER|OPT_READONLY)) == OPT_HEADER)
		{
			if (vp == &op->temp)
				op->entry = ap->entry;
			else
				op->level = c;
			if (*v)
			{
				op->flags |= OPT_SET;
				if (op->flags & OPT_NUMBER)
					vp->number = n;
				stash(vp, v, 0);
			}
			else
				vp = 0;
		}
		else
			vp = 0;
		message((-4, "option: %s%s%-.1s=%s type=%c entry=%d level=%d number=%I*d", y ? "" : "no", op->name, &opt_info.assignment, v, type ? type : '-', op->entry, op->level, sizeof(opt_info.num), opt_info.num));
		switch (op->index)
		{
		case OPT_append:
			state.append = y;
			break;
		case OPT_atime:
			if (vp)
			{
			settime:
				vp->number = strtoul(vp->string, &e, 10);
				vp->fraction = 0;
				if (*e)
				{
					if (*e != '.')
						vp->number = tmdate(vp->string, &e, NiL);
					if (*e == '.')
						vp->fraction = strtoul(s = e + 1, &e, 10);
					if (*e)
						error(2, "%s: invalid %s date string", vp->string, options[op->index].name);
					else if (vp->fraction)
					{
						y = e - s;
						for (y = e - s; y < 9; y++)
							vp->fraction *= 10;
						for (; y > 9; y--)
							vp->fraction /= 10;
					}
				}
			}
			break;
		case OPT_base:
			ap = getarchive(state.operation);
			if (ap->delta)
				error(3, "base archive already specified");
			if (y)
			{
				initdelta(ap);
				if (!*v || streq(v, "-"))
				{
					state.delta2delta++;
					if (!(state.operation & OUT))
					{
						ap->delta->format = DELTA_IGNORE;
						break;
					}
					v = "/dev/null";
				}
				ap->delta->base = initarchive(strdup(v), O_RDONLY);
			}
			break;
		case OPT_blocksize:
			if (y)
			{
				state.blocksize = n;
				if (*e) error(3, "%s: invalid block size", v);
				if (state.blocksize < MINBLOCK)
					error(3, "block size must be at least %d", MINBLOCK);
				if (state.blocksize & (BLOCKSIZE - 1))
					error(1, "block size should probably be a multiple of %d", BLOCKSIZE);
			}
			else state.blocksize = DEFBLOCKS * BLOCKSIZE;
			break;
		case OPT_blok:
			if (!*v) getarchive(IN)->io->blok = getarchive(OUT)->io->blok = n;
			else while (*v) switch (*v++)
			{
			case 'i':
				getarchive(IN)->io->blok = 1;
				break;
			case 'o':
				getarchive(OUT)->io->blok = 1;
				break;
			default:
				error(3, "%s: [io] expected", op->name);
				break;
			}
			break;
		case OPT_checksum:
			if (y)
			{
				if (e = strchr(v, ':'))
					*e++ = 0;
				else
				{
					e = v;
					v = "md5";
				}
				state.checksum.name = strdup(e);
				if (!(state.checksum.sum = sumopen(v)))
					error(3, "%s: %s: unknown checksum algorithm", e, v);
			}
			else
				state.checksum.name = 0;
			break;
		case OPT_clobber:
			state.clobber = y;
			break;
		case OPT_comment:
			state.header.comment = y ? strdup(v) : (char*)0;
			break;
		case OPT_complete:
			state.complete = y;
			break;
		case OPT_crossdevice:
			if (!y) state.ftwflags |= FTW_MOUNT;
			else state.ftwflags &= ~FTW_MOUNT;
			break;
		case OPT_ctime:
			if (vp) goto settime;
			break;
		case OPT_debug:
			if (y)
			{
				y = error_info.trace;
				error_info.trace = -(int)n;
				if (!y) message((-10, "usage %s", usage));
			}
			else error_info.trace = 0;
			break;
		case OPT_delete:
			if (y && *v)
				sfprintf(opt.ignore_all, "%s(%s)", sfstrtell(opt.ignore_all) ? "|" : "", v);
			break;
		case OPT_descend:
			state.descend = y;
			break;
		case OPT_dots:
			state.drop = y;
			break;
		case OPT_edit:
			substitute(&opt.lastmap, (char*)v);
			break;
		case OPT_eom:
			eomprompt = y ? strdup(v) : (char*)0;
			break;
		case OPT_exact:
			state.exact = y;
			break;
		case OPT_extended_name:
			state.header.extended = y ? strdup(v) : (char*)0;
			break;
		case OPT_file:
			ap = getarchive(state.operation);
			if (ap->name)
				error(3, "%s: %s: archive name already specified", v, ap->name);
			ap->name = strdup(v);
			break;
		case OPT_filter:
			if (y && *v)
			{
				state.filter.command = v;
				state.descend = 0;
			}
			else
				state.filter.command = 0;
			break;
		case OPT_format:
			ap = getarchive(state.operation);
			if (!y)
				ap->format = -1;
			else
			{
				if (streq(v, "tgz"))
					v = "tar:gzip";
				else if (streq(v, "tbz"))
					v = "tar:bzip2";
				if (s = strdup(v))
					do
					{
						for (e = s;;)
						{
							switch (*e++)
							{
							case 0:
								e = 0;
								break;
							case ' ':
							case '\t':
							case '\n':
							case ':':
							case ',':
							case '.':
								*(e - 1) = 0;
								if (*s) break;
								s = e;
								continue;
							default:
								continue;
							}
							break;
						}
						if (s[0] == 'g' && s[1] == 'z')
							s = "gzip";
						else if (s[0] == 'b' && s[1] == 'z')
							s = "bzip2";
						if ((y = getformat(s)) >= DELTA)
						{
							initdelta(ap);
							switch (y)
							{
							case DELTA_IGNORE:
							case DELTA_PATCH:
								ap->delta->format = y;
								break;
							default:
								ap->delta->version = y;
								break;
							}
						}
						else if (y >= COMPRESS)
							ap->compress = y;
						else if ((ap->format = y) < 0)
						{
							Sfio_t*	sp;

							if (!pathpath(tar_block, "lib/pax", opt.arg0, PATH_EXECUTE) || sfsprintf(alar_header, sizeof(alar_header) - 1, "%s/%s.fmt", tar_block, s) <= 0 || !(sp = sfopen(NiL, alar_header, "r")))
								error(3, "%s: unknown archive format", s);
							while (e = sfgetr(sp, '\n', 1))
								if (*e != '#')
								{
									setoptions(e, NiL, state.usage, ap, type);
									line += opt_info.offset;
								}
							sfclose(sp);
						}
					} while (s = e);
			}
			ap->expected = ap->format;
			if (!state.operation)
				state.format = ap->format;
			break;
		case OPT_from:
		case OPT_to:
			if (!cvt)
			{
				cvt = 1;
				from = to = CC_NATIVE;
			}
			ap = getarchive(state.operation);
			if ((y = ccmapid(v)) < 0)
				error(3, "%s: unknown character code set", v);
			switch (op->index)
			{
			case OPT_from:
				from = y;
				break;
			case OPT_to:
				to = y;
				break;
			}
			break;
		case OPT_global_name:
			state.header.global = y ? strdup(v) : (char*)0;
			break;
		case OPT_header:
			v = y ? strdup(v) : (char*)0;
			if (opt_info.assignment == ':')
				state.header.extended = v;
			else
				state.header.global = v;
			break;
		case OPT_ignore:
			if (y && *v)
			{
				if (opt_info.assignment == ':')
					sfprintf(opt.ignore_ext, "%s(%s)", sfstrtell(opt.ignore_ext) ? "|" : "", v);
				else
					sfprintf(opt.ignore_all, "%s(%s)", sfstrtell(opt.ignore_all) ? "|" : "", v);
			}
			break;
		case OPT_install:
			state.install.name = y ? strdup(v) : (char*)0;
			break;
		case OPT_intermediate:
			state.intermediate = y;
			break;
		case OPT_invalid:
			switch (opt_info.num)
			{
			case 'i':
				state.header.invalid = INVALID_ignore;
				break;
			case 'p':
				state.header.invalid = INVALID_prompt;
				break;
			case 't':
				state.header.invalid = INVALID_translate;
				break;
			case 'u':
				state.header.invalid = INVALID_UTF8;
				break;
			}
			break;
		case OPT_invert:
			state.matchsense = !y;
			break;
		case OPT_keepgoing:
			state.keepgoing = y;
			break;
		case OPT_label:
			if (*state.id.volume)
			{
				if (opt_info.assignment == ':')
					sfsprintf(tar_block, sizeof(tar_block), "%s %s", v, state.id.volume);
				else
					sfsprintf(tar_block, sizeof(tar_block), "%s %s", state.id.volume, v);
				v = tar_block;
			}
			strncpy(state.id.volume, v, sizeof(state.id.volume) - 2);
			break;
		case OPT_link:
			if (y)
				state.linkf = link;
			else
				state.linkf = 0;
			break;
		case OPT_linkdata:
			state.header.linkdata = y;
			break;
		case OPT_listformat:
			if (y && *v)
				sfputr(opt.listformat, v, ' ');
			break;
		case OPT_listmacro:
			if (y && *v)
			{
				if (s = strchr(v, '='))
					*s++ = 0;
				if (!(op = (Option_t*)hashget(state.options, v)))
				{
					if (!s) break;
					if (!(op = newof(0, Option_t, 1, 0)))
						nospace();
					op->name = hashput(state.options, 0, op);
				}
				if (s)
				{
					op->macro = strdup(s);
					*(s - 1) = 0;
				}
				else op->macro = 0;
			}
			break;
		case OPT_local:
			state.local = 1;
			break;
		case OPT_logical:
			if (y) state.ftwflags &= ~FTW_PHYSICAL;
			else state.ftwflags |= FTW_PHYSICAL;
			break;
		case OPT_maxout:
			state.maxout = n;
			break;
		case OPT_metaphysical:
			if (y) state.ftwflags |= FTW_META|FTW_PHYSICAL;
			else state.ftwflags &= ~(FTW_META|FTW_PHYSICAL);
			break;
		case OPT_meter:
			if ((state.meter.on = y) && (state.meter.fancy = isatty(sffileno(sfstderr))))
			{
				error_info.write = meterror;
				astwinsize(1, NiL, &state.meter.width);
				if (state.meter.width < 2 * (METER_parts + 1))
					state.meter.width = 2 * (METER_parts + 1);
			}
			break;
		case OPT_mkdir:
			state.mkdir = y;
			break;
		case OPT_mtime:
			if (vp) goto settime;
			break;
		case OPT_options:
			if (v)
			{
				setoptions(v, NiL, usage, ap, type);
				line += opt_info.offset;
			}
			break;
		case OPT_ordered:
			state.ordered = y;
			break;
		case OPT_owner:
			if (!(state.owner = y)) opt.owner = 0;
			else if (*v) opt.owner = strdup(v);
			break;
		case OPT_physical:
			if (y)
			{
				state.ftwflags &= ~FTW_META;
				state.ftwflags |= FTW_PHYSICAL;
			}
			else state.ftwflags &= ~FTW_PHYSICAL;
			break;
		case OPT_preserve:
			for (;;)
			{
				switch (*v++)
				{
				case 0:
					break;
				case 'a':
					state.acctime = 0;
					continue;
				case 'e':
					state.acctime = 1;
					state.modtime = 1;
					state.owner = 1;
					state.modemask = 0;
					state.chmod = 1;
					continue;
				case 'm':
					state.modtime = 0;
					continue;
				case 'o':
					state.owner = 1;
					continue;
				case 'p':
					state.modemask = 0;
					state.chmod = 1;
					continue;
				case 's':
					state.modemask &= ~(S_ISUID|S_ISGID);
					continue;
				default:
					error(1, "%s=%c: unknown flag", op->name, *(v - 1));
					continue;
				}
				break;
			}
			break;
		case OPT_read:
			if (y) state.operation |= IN;
			else state.operation &= ~IN;
			break;
		case OPT_record_charset:
			state.record.charset = y;
			break;
		case OPT_record_delimiter:
			if (!y) state.record.delimiter = 0;
			else if (!*v) state.record.delimiter = VDB_DELIMITER;
			else state.record.delimiter = *v;
			break;
		case OPT_record_format:
			state.record.format = y ? *v : 0;
			break;
		case OPT_record_header:
			if (!y)
			{
				state.record.header = 0;
				state.record.headerlen = 0;
			}
			else if (!(state.record.headerlen = stresc(state.record.header = strdup(v))))
				state.record.headerlen = 1;
			break;
		case OPT_record_line:
			state.record.line = y;
			break;
		case OPT_record_match:
			state.record.pattern = y ? strdup(v) : (char*)0;
			break;
		case OPT_record_pad:
			state.record.pad = y;
			break;
		case OPT_record_size:
			state.record.size = n;
			break;
		case OPT_record_trailer:
			if (!y)
			{
				state.record.trailer = 0;
				state.record.trailerlen = 0;
			}
			else if (!(state.record.trailerlen = stresc(state.record.trailer = strdup(v))))
				state.record.trailerlen = 1;
			break;
		case OPT_reset_atime:
			state.acctime = y;
			break;
		case OPT_size:
			break;
		case OPT_strict:
			state.strict = y;
			break;
		case OPT_summary:
			state.summary = y;
			break;
		case OPT_symlink:
			if (y) state.linkf = pathsetlink;
			else state.linkf = 0;
			break;
		case OPT_tape:
			ap = getarchive(state.operation);
			if (ap->name)
				error(3, "%s: %s: archive name already specified", v, ap->name);
			s = strtape(v, &e);
			if (*s) ap->name = s;
			for (;;)
			{
				switch (*e++)
				{
				case 'k':
					if (!(n = strtonll(e, &e, 0, 1))) n = -1;
					ap->io->keep = n;
					ap->io->mode = O_RDWR;
					continue;
				case 's':
					if (!(n = strtonll(e, &e, 0, 1))) n = -1;
					ap->io->skip = n;
					ap->io->mode = O_RDWR;
					continue;
				}
				e--;
				break;
			}
			if (*e) error(3, "%s: invalid tape unit specification [%s]", v, e);
			break;
		case OPT_test:
			if (y) state.test |= (unsigned long)n;
			else state.test = 0;
			break;
		case OPT_testdate:
			if (y)
			{
				state.testdate = tmdate(v, &e, NiL);
				if (*e)
					error(3, "%s: invalid %s date string", v, options[op->index].name);
			}
			else state.testdate = ~0;
			break;
		case OPT_times:
			setoptions("atime:= ctime:= mtime:=", NiL, usage, ap, type);
			line += opt_info.offset;
			break;
		case OPT_unblocked:
			if (!*v) getarchive(IN)->io->unblocked = getarchive(OUT)->io->unblocked = y;
			else while (*v) switch (*v++)
			{
			case 'i':
				getarchive(IN)->io->unblocked = 1;
				break;
			case 'o':
				getarchive(OUT)->io->unblocked = 1;
				break;
			default:
				error(3, "%s: [io] expected", op->name);
				break;
			}
			break;
		case OPT_update:
			state.update = y;
			break;
		case OPT_verbose:
			state.verbose = y;
			break;
		case OPT_verify:
			state.verify = y;
			break;
		case OPT_write:
			if (y) state.operation |= OUT;
			else state.operation &= ~OUT;
			if (!(state.operation & IN) && state.in && !state.out)
			{
				state.out = state.in;
				state.in = 0;
				state.out->io->mode = O_CREAT|O_TRUNC|O_WRONLY;
			}
			break;
		case OPT_yes:
			state.verify = state.yesno = y;
			break;
		default:
			if (!type && !(op->flags & OPT_HEADER) || !(op->flags & (OPT_IGNORE|OPT_SET)))
				error(1, "%s: option ignored [index=%d]", op->name, op->index);
			break;
		}
	}
	if (!argv)
	{
		opt_info.index = index;
		opt_info.offset = offset;
	}
	if (cvt)
	{
		ap->convert[0].on = 1;
		convert(ap, SECTION_DATA, from, to);
	}
}

/*
 * option match with VENDOR check
 */

static int
matchopt(const char* name, const char* pattern, Option_t* op)
{
	return strmatch(name, pattern) || (op->flags & OPT_VENDOR) && strmatch(sfprints("%s.%s", VENDOR, name), pattern);
}

/*
 * mark ignored header keywords
 */

static void
ignore(void)
{
	register Option_t*	op;
	Hash_position_t*	pos;
	char*			all;
	char*			ext;
	int			lev;

	all = sfstrtell(opt.ignore_all) ? sfstruse(opt.ignore_all) : (char*)0;
	ext = sfstrtell(opt.ignore_ext) ? sfstruse(opt.ignore_ext) : (char*)0;
	if ((all || ext) && (pos = hashscan(state.options, 0)))
	{
		while (hashnext(pos))
		{
			op = (Option_t*)pos->bucket->value;
			if (!(op->flags & OPT_READONLY) && (all && matchopt(pos->bucket->name, all, op) && (lev = 8) || ext && matchopt(pos->bucket->name, ext, op) && (lev = 6)) && op->level < lev)
				op->level = lev;
		}
		hashdone(pos);
	}
	sfstrclose(opt.ignore_all);
	sfstrclose(opt.ignore_ext);
}

int
main(int argc, char** argv)
{
	register int		i;
	register int		j;
	register char*		s;
	register Archive_t*	ap;
	char*			p;
	Hash_position_t*	pos;
	Option_t*		op;
	int			n;
	int			pass = 0;
	unsigned long		blocksize;
	struct stat		st;

	setlocale(LC_ALL, "");
	error_info.id = "pax";
	state.strict = !strcmp(astconf("CONFORMANCE", NiL, NiL), "standard");
	state.gid = getegid();
	state.uid = geteuid();
	state.pid = getpid();
	umask(state.modemask = umask(0));
	state.modemask |= S_ISUID|S_ISGID;
	state.ftwflags = ftwflags()|FTW_DOT;
	state.buffersize = DEFBUFFER * DEFBLOCKS;
	state.clobber = 1;
	state.delta.buffersize = DELTA_WINDOW >> 1;
	state.descend = RESETABLE;
	state.format = OUT_DEFAULT;
	state.header.extended = state.strict ? HEADER_EXTENDED_STD : HEADER_EXTENDED;
	state.header.global = state.strict ? HEADER_GLOBAL_STD : HEADER_GLOBAL;
	state.map.a2n = ccmap(CC_ASCII, CC_NATIVE);
	state.map.e2n = ccmap(CC_EBCDIC, CC_NATIVE);
	state.map.n2e = ccmap(CC_NATIVE, CC_EBCDIC);
	if (!(opt.ignore_all = sfstropen()) || !(opt.ignore_ext = sfstropen()))
		nospace();
	if (!(opt.listformat = sfstropen()))
		nospace();
	state.matchsense = 1;
	state.mkdir = 1;
	state.modtime = 1;
	if (!(state.tmp.fmt = sfstropen()) || !(state.tmp.lst = sfstropen()) || !(state.tmp.str = sfstropen()))
		nospace();
	stash(&options[OPT_release].perm, release(), 0);
	options[OPT_release].flags |= OPT_SET;
	if (!(state.options = hashalloc(NiL, HASH_name, "options", 0)))
		nospace();
	for (i = 1; i < elementsof(options); i++)
	{
		p = options[i].name;
		if (strchr(p, '|'))
			p = strdup(p);
		do
		{
			if (s = strchr(p, '|'))
				*s++ = 0;
			hashput(state.options, p, &options[options[i].index]);
		} while (p = s);
	}
	hashset(state.options, HASH_ALLOCATE);
	state.record.charset = 1;
	state.record.line = 1;
	state.summary = 1;
	state.testdate = ~0;
	if (!(state.tmp.file = pathtemp(NiL, 0, NiL, error_info.id, NiL)))
		nospace();
	sfputr(state.tmp.str, usage, -1);
	for (i = 1; i < elementsof(options); i++)
		if (!(options[i].flags & OPT_READONLY))
		{
			sfputc(state.tmp.str, '[');
			if (options[i].flag)
			{
				sfputc(state.tmp.str, options[i].flag);
				if (options[i].flags & OPT_INVERT)
					sfputc(state.tmp.str, '!');
			}
			sfprintf(state.tmp.str, "=%d:%s", options[i].index, options[i].name);
			sfputc(state.tmp.str, '?');
			sfputr(state.tmp.str, options[i].description, -1);
			sfputc(state.tmp.str, ']');
			if (options[i].argument)
			{
				sfputc(state.tmp.str, (options[i].flags & OPT_NUMBER) ? '#' : ':');
				if (options[i].flags & OPT_OPTIONAL)
					sfputc(state.tmp.str, '?');
				sfprintf(state.tmp.str, "[%s]", options[i].argument);
			}
			if (options[i].details)
				sfprintf(state.tmp.str, "\n{%s}", options[i].details);
			else if (i == OPT_format)
			{
				sfputc(state.tmp.str, '\n');
				sfputc(state.tmp.str, '{');
				for (n = 0; format[n].name; n++)
				{
					if (n == COMPRESS)
					{
						Ardirmeth_t*	ar = 0;

						while (ar = ardirlist(ar))
							sfprintf(state.tmp.str, "[+%s?%s; for input only]", ar->name, ar->description);
						sfprintf(state.tmp.str, "[+----?compression methods ----]");
					}
					else if (n == DELTA)
						sfprintf(state.tmp.str, "[+----?delta methods ----]");
					sfprintf(state.tmp.str, "[+%s%s%s?%s%s]", format[n].name, format[n].match ? "|" : "", format[n].match ? format[n].match : "", format[n].desc, (format[n].flags & OUT) ? "" : "; for input only");
				}
				sfputc(state.tmp.str, '}');
			}
			else if (i == OPT_listformat)
			{
				sfputc(state.tmp.str, '\n');
				sfputc(state.tmp.str, '{');
				for (j = 1; j < elementsof(options); j++)
					if (options[j].flags & OPT_READONLY)
					{
						sfprintf(state.tmp.str, "[+%s?%s]\n", options[j].name, options[j].description);
						if (options[j].details)
							sfprintf(state.tmp.str, "{\n%s\n}", options[j].details);
					}
				sfprintf(state.tmp.str, "%s",
"	[+----?subformats ----]"
"	[+case\b::\bp\b\a1\a::\bs\b\a1\a::...::\bp\b\an\a::\bs\b\an\a?Expands"
"		to \bs\b\ai\a if the value of \aid\a matches the shell"
"		pattern \bp\b\ai\a, or the empty string if there is no"
"		match.]"
"	[+mode?The integral value as a \bfmtmode\b(3) string.]"
"	[+perm?The integral value as a \bfmtperm\b(3) string.]"
"	[+time[=\aformat\a]]?The integral value as a \bstrftime\b(3)"
"		string. For example,"
"		\b--format=\"%8(mtime)u %(ctime:time=%H:%M:%S)s\"\b"
"		lists the mtime in seconds since the epoch and the"
"		ctime as hours:minutes:seconds.]");
				sfputc(state.tmp.str, '}');
			}
			sfputc(state.tmp.str, '\n');
		}
	sfputr(state.tmp.str, usage2, -1);
	state.usage = sfstruse(state.tmp.str);
	opt.arg0 = argv[0];
	setoptions(NiL, argv, state.usage, NiL, 0);
	argv += opt_info.index;
	argc -= opt_info.index;
	if (error_info.errors)
		error(ERROR_USAGE|4, optusage(NiL));
	if (!state.operation)
	{
		state.operation = IN;
		state.list = 1;
	}
	if (!sfstrtell(opt.listformat))
		sfputr(opt.listformat, (state.list && state.verbose) ? "%(mode)s %2(nlink)d %-8(uname)s %-8(gname)s%8(device:case::%(size)llu:*:%(device)s)s %(mtime)s %(delta:case:?*:%(delta)s )s%(path)s%(linkop:case:?*: %(linkop)s %(linkpath)s)s" : "%(delta:case:?*:%(delta)s )s%(path)s%(linkop:case:?*: %(linkop)s %(linkpath)s)s", ' ');
	sfstrrel(opt.listformat, -1);
	if (!state.meter.on)
		sfputc(opt.listformat, '\n');
	if (!(state.listformat = strdup(sfstruse(opt.listformat))))
		nospace();
	sfstrclose(opt.listformat);
	ignore();
	if (s = state.filter.command)
	{
		if (streq(s, "-"))
		{
			state.filter.line = -1;
			s = "sh -c";
		}
		if (!(s = strdup(s)))
			nospace();
		p = s;
		n = 3;
		while (i = *s++)
			if (isspace(i))
			{
				n++;
				while (isspace(*s))
					s++;
			}
		if (!(state.filter.argv = newof(0, char*, n, 0)))
			error(3, "no space [filter]");
		s = p;
		state.filter.argv[0] = s;
		if (n > 3)
		{
			n = 1;
			while (i = *s++)
				if (isspace(i))
				{
					*(s - 1) = 0;
					while (isspace(*s))
						s++;
					if (*s)
						state.filter.argv[n++] = s;
				}
		}
		else
			n = 1;
		state.filter.patharg = state.filter.argv + n;
	}
	state.statf = (state.ftwflags & FTW_PHYSICAL) ? lstat : pathstat;

	/*
	 * determine the buffer sizes
	 */

	switch (state.operation)
	{
	case IN|OUT:
		if (!state.in)
			break;
		/*FALLTHROUGH*/
	case IN:
	case OUT:
		getarchive(state.operation);
		break;
	}
	blocksize = state.blocksize;
	if (ap = state.out)
	{
		if (ap->format < 0) ap->format = state.format;
		else if (state.operation == (IN|OUT)) pass = 1;
		if (state.operation == OUT)
		{
			if (state.files)
				state.ftwflags |= FTW_POST;
			if (state.update)
				state.append = 1;
		}
		if (state.append)
			ap->io->mode = O_CREAT|O_RDWR;
		ap->io->fd = 1;
		if (!ap->name || streq(ap->name, "-"))
			ap->name = defoutput;
		else
		{
			close(1);
			if (open(ap->name, ap->io->mode|O_BINARY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH) != 1)
				error(ERROR_SYSTEM|3, "%s: cannot write", ap->name);
		}
		if (!state.blocksize)
		{
			if (fstat(ap->io->fd, &st))
				error(ERROR_SYSTEM|3, "%s: cannot stat", ap->name);
			st.st_mode = modex(st.st_mode);
			if (state.test & 040) st.st_mode = X_IFCHR;
			if (X_ITYPE(st.st_mode) == X_IFREG)
			{
				state.blocksize = format[ap->format].regular;
				ap->io->unblocked = 1;
			}
			else state.blocksize = format[ap->format].special;
			state.buffersize = state.blocksize *= BLOCKSIZE;
		}
		switch (ap->format)
		{
		case ALAR:
		case IBMAR:
			if (!state.record.format)
				state.record.format = ALAR_RECFORMAT;
			if (state.record.size <= 0) switch (state.record.format)
			{
			case 'D':
			case 'U':
				state.record.size = state.blocksize;
				break;
			case 'F':
				state.record.size = ALAR_RECSIZE;
				break;
			case 'S':
			case 'V':
				state.record.size = 0;
				break;
			default:
				error(3, "%s %c record format not supported on output", format[ap->format].name, state.record.format);
			}
			if (state.blocksize < state.record.size)
				error(3, "block size (%d) must be >= record size (%d)", state.blocksize, state.record.size);
			if (state.record.size && (state.blocksize / state.record.size) * state.record.size != state.blocksize)
				error(1, "block size should be a multiple of record size");
			break;
		default:
			if (state.record.format || state.record.size)
				error(1, "record format and size ignored for %s format", format[ap->format].name);
			break;
		}
	}
	else
	{
		if (state.blocksize) state.buffersize = state.blocksize;
		else state.blocksize = (state.operation == (IN|OUT) ? FILBLOCKS : DEFBLOCKS) * BLOCKSIZE;
		if (state.record.size)
			error(1, "record size automatically determined on archive read");
	}
	if (ap = state.in)
	{
		if (!ap->name || streq(ap->name, "-"))
			ap->name = definput;
		else
		{
			close(0);
			if (open(ap->name, ap->io->mode|O_BINARY))
				error(ERROR_SYSTEM|3, "%s: cannot read", ap->name);
		}
		if (!fstat(ap->io->fd, &st) && S_ISREG(st.st_mode) && st.st_size > 0)
		{
			ap->io->seekable = 1;
			ap->io->size = st.st_size;
		}
		if (state.meter.on && !(state.meter.size = ap->io->size))
			state.meter.on = 0;
	}
	if (!blocksize && (blocksize = bblock(!state.in)))
		state.blocksize = blocksize;
	if (state.buffersize < state.blocksize)
		state.buffersize = state.blocksize;
	state.tmp.buffersize = state.buffersize;
	if (!(state.tmp.buffer = newof(0, char, state.tmp.buffersize, 0)))
		nospace();
	message((-1, "blocksize=%d buffersize=%d recordsize=%d", state.blocksize, state.buffersize, state.record.size));

	/*
	 * initialize the main io
	 */

	switch (state.operation)
	{
	case IN:
	case OUT:
		getarchive(state.operation);
		break;
	}
	if (ap = state.in)
	{
		binit(ap);
		if (state.append)
		{
			error(1, "append ignored for archive read");
			state.append = 0;
		}
	}
	if (ap = state.out)
	{
		if (ap->format < 0)
			ap->format = state.format;
		binit(ap);
		if (ap->compress)
		{
			Proc_t*	proc;
			List_t*	p;
			char*	cmd[3];

			cmd[0] = format[ap->compress].name;
			cmd[1] = format[ap->compress].algorithm;
			cmd[2] = 0;
			if (!(proc = procopen(*cmd, cmd, NiL, NiL, PROC_WRITE)))
				error(3, "%s: cannot execute %s filter", ap->name, format[ap->compress].name);
			n = proc->wfd;
			proc->wfd = 1;
			close(1);
			if (dup(n) != 1)
				error(3, "%s: cannot redirect %s filter output", ap->name, format[ap->compress].name);
			close(n);
			if (!(p = newof(0, List_t, 1, 0)))
				nospace();
			p->item = (void*)proc;
			p->next = state.proc;
			state.proc = p;
		}
		if (state.checksum.name)
		{
			if (!(state.checksum.path = pathtemp(NiL, 0, NiL, error_info.id, NiL)))
				nospace();
			if (!(state.checksum.sp = sfopen(NiL, state.checksum.path, "w")))
				error(3, "%s: cannot write checksum temporary", state.checksum.path);
			sfprintf(state.checksum.sp, "method=%s\n", state.checksum.sum->name);
			sfprintf(state.checksum.sp, "permissions\n");
		}
		if (state.install.name)
		{
			if (!(state.install.path = pathtemp(NiL, 0, NiL, error_info.id, NiL)))
				nospace();
			if (!(state.install.sp = sfopen(NiL, state.install.path, "w")))
				error(3, "%s: cannot write install temporary", state.install.path);
		}
	}
	if (!(state.linktab = hashalloc(NiL, HASH_set, HASH_ALLOCATE, HASH_namesize, sizeof(Fileid_t), HASH_name, "links", 0)))
		error(3, "cannot allocate hard link table");
	if ((state.operation & IN) && !state.list && !(state.restore = hashalloc(NiL, HASH_set, HASH_ALLOCATE, HASH_name, "restore", 0)))
		error(3, "cannot allocate directory table");
	if (state.owner)
	{
		if (state.operation & IN)
		{
			state.modemask = 0;
			if (opt.owner)
			{
				if ((state.setuid = struid(opt.owner)) < 0 || (state.setgid = strgid(opt.owner)) < 0)
					error(3, "%s: invalid user name", opt.owner);
				state.flags |= SETIDS;
			}
		}
		else error(1, "ownership assignment ignored on archive write");
	}
	if (state.verify)
		interactive();
	if (!(state.modemask &= (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)))
		umask(0);
	state.modemask = ~state.modemask;
#if DEBUG
	if ((state.test & 010) && (pos = hashscan(state.options, 0)))
	{
		while (hashnext(pos))
		{
			op = (Option_t*)pos->bucket->value;
			if (op->name == pos->bucket->name)
				sfprintf(sfstderr, "%-16s %c %2d %d perm=%ld:%s temp=%ld:%s%s%s\n", op->name, op->flag ? op->flag : '-', op->index, op->level, op->perm.number, op->perm.string, op->temp.number, op->temp.string, (op->flags & OPT_HEADER) ? " HEADER" : "", (op->flags & OPT_READONLY) ? " READONLY" : "");
		}
		hashdone(pos);
	}
#endif
	for (i = 0; i < elementsof(signals); i++)
		if (signal(signals[i], interrupt) == SIG_IGN)
			signal(signals[i], SIG_IGN);
	switch (state.operation)
	{
	case IN:
		if (*argv)
		{
			state.patterns = initmatch(argv);
			if (state.exact) state.matchsense = 1;
		}
		else if (state.exact)
			error(3, "file arguments expected");
		getcwd(state.pwd, PATH_MAX);
		state.pwdlen = strlen(state.pwd);
		if (state.pwdlen > 1)
			state.pwd[state.pwdlen++] = '/';
		copyin(state.in);
		if (state.exact)
		{
			argv = state.patterns;
			while (s = *argv++)
				if (*s) error(2, "%s: %s: file not found in archive", state.in->name, s);
		}
		break;

	case OUT:
		if (*argv)
			state.files = argv;
		if (!state.maxout && state.complete)
			error(3, "maximum block count required");
		copy(state.out, copyout);
		break;

	case (IN|OUT):
		if (pass || state.in || state.out)
		{
			state.pass = 1;
			if (*argv)
				state.patterns = initmatch(argv);
			deltapass(getarchive(IN), getarchive(OUT));
		}
		else
		{
			if (--argc < 0)
			{
				error(2, "destination directory required for pass mode");
				error(ERROR_USAGE|4, optusage(NiL));
			}
			state.destination = argv[argc];
			argv[argc] = 0;
			if (*argv)
				state.files = argv;
			if (state.record.size)
				error(1, "record size ignored in pass mode");

			/*
			 * initialize destination dir
			 */

			pathcanon(state.destination, 0);
			if (stat(state.destination, &st) || !S_ISDIR(st.st_mode))
				error(3, "%s: destination must be a directory", state.destination);
			state.dev = st.st_dev;
			strcpy(state.pwd, state.destination);
			state.pwdlen = strlen(state.pwd);
			if (state.pwdlen > 1)
				state.pwd[state.pwdlen++] = '/';
			getarchive(OUT);
			copy(NiL, copyinout);
		}
		break;
	}
	finish(0);
}

/*
 * print number of blocks actually copied and exit
 */

void
finish(int code)
{
	register Archive_t*	ap;
	register char*		x1 = &state.tmp.buffer[0];
	register char*		x2 = &state.tmp.buffer[state.buffersize / 2];
	register off_t		n;

	while (state.proc)
	{
		procclose((Proc_t*)state.proc->item);
		state.proc = state.proc->next;
	}
	remove(state.tmp.file);
	if (state.checksum.path)
		remove(state.checksum.path);
	if (state.install.path)
		remove(state.install.path);
	if (state.restore)
		hashwalk(state.restore, 0, restore, NiL);
	sfsync(sfstdout);
	if (state.meter.last)
	{
		sfprintf(sfstderr, "%*s", state.meter.last, "\r");
		state.meter.last = 0;
	}
	else if (state.dropcount)
	{
		sfprintf(sfstderr, "\n");
		sfsync(sfstderr);
	}
	if (state.summary)
	{
		ap = getarchive(state.operation);
		n = ap->io->count + ap->io->offset;
		message((-1, "%s totals entries=%d count=%I*d offset=%I*d BLOCKSIZE=%I*d n=%I*d blocks=%I*d", ap->name, ap->entries, sizeof(ap->io->count), ap->io->count, sizeof(ap->io->offset), ap->io->offset, sizeof(BLOCKSIZE), BLOCKSIZE, sizeof(n), n, sizeof(n), (n + BLOCKSIZE - 1) / BLOCKSIZE));
		if (ap->entries)
		{
			if (ap->volume > 1)
				sfsprintf(x1, state.tmp.buffersize / 2, ", %d volumes", ap->volume);
			else
				*x1 = 0;
			if (ap->volume > 0 && ap->part > ap->volume)
				sfsprintf(x2, state.tmp.buffersize / 2, ", %d parts", ap->part - ap->volume + 1);
			else
				*x2 = 0;
			n = (n + BLOCKSIZE - 1) / BLOCKSIZE;
			if (state.verbose)
				sfprintf(sfstderr, "%d file%s, %I*d block%s%s%s\n", ap->selected, ap->selected == 1 ? "" : "s", sizeof(n), n, n == 1 ? "" : "s", x1, x2);
			else
				sfprintf(sfstderr, "%I*d block%s%s%s\n", sizeof(n), n, n == 1 ? "" : "s", x1, x2);
		}
	}
	sfsync(sfstderr);
	if (state.interrupt)
	{
		signal(state.interrupt, SIG_DFL);
		kill(getpid(), state.interrupt);
		pause();
	}
	exit(code ? code : error_info.errors != 0);
}

/*
 * return release stamp
 */

char*
release(void)
{
	register char*	b;
	register char*	s;
	register char*	t;

	if ((s = strchr(usage, '@')) && (t = strchr(s, '\n')) && (b = fmtbuf(t - s + 1)))
	{
		memcpy(b, s, t - s);
		b[t - s] = 0;
	}
	else
		b = fmtident(usage);
	return b;
}
