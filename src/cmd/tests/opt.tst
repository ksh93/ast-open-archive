# regression tests for the ast optget() and opthelp() functions

TEST 01 'compatibility'
	EXEC	cmd $'a file ...' -?
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: cmd [-a] file ...'
		EXIT 2
	EXEC	cmd $'a file ...' --??api
		OUTPUT - $'return=? option=-? name=--??api num=0'
		ERROR - $'.SH SYNOPSIS\ncmd [ options ] file ...\n.SH OPTIONS\n.OP a - flag -'
	EXEC	cmd $'[ab] file' -?
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: cmd [ -a | -b ] file'
	EXEC	cmd $'[ab] file' --??api
		OUTPUT - $'return=? option=-? name=--??api num=0'
		ERROR - $'.SH SYNOPSIS\ncmd [ options ] file\n.SH OPTIONS\n.OP - - oneof\n.OP a - flag -\n.OP b - flag -\n.OP - - anyof'
	EXEC	cmd $'[a:b] file' -?
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: cmd [ -a arg | -b ] file'
	EXEC	cmd $'[a:b] file' --??api
		OUTPUT - $'return=? option=-? name=--??api num=0'
		ERROR - $'.SH SYNOPSIS\ncmd [ options ] file\n.SH OPTIONS\n.OP - - oneof\n.OP a - string arg\n.OP b - flag -\n.OP - - anyof'
	EXEC	wc $'lw[cm] [ file ... ]' -?
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: wc [-lw] [ -c | -m ] [ file ... ]'
	EXEC	wc $'lw[cm] [ file ... ]' --??api
		OUTPUT - $'return=? option=-? name=--??api num=0'
		ERROR - $'.SH SYNOPSIS\nwc [ options ] [ file ... ]\n.SH OPTIONS\n.OP l - flag -\n.OP w - flag -\n.OP - - oneof\n.OP c - flag -\n.OP m - flag -\n.OP - - anyof'

TEST 02 'long options'
	usage=$'[-?\n@(#)testopt (AT&T Research) 1999-02-02\n][-author?Glenn Fowler <gsf@research.att.com>][--dictionary?tests:opt][x:method?One of]:?[algorithm:oneof:ignorecase]{[+a?method a][+b?method b][+c?method c]}[\n[y=10:yes?Yes.][20:no?No.]\n][d:database?File database path.]:[path][z:size?Important size.]#[sizeX][V:vernum?List program version and exit.][v:verbose?Enable verbose trace.][n:show?Show but don\'t execute.]\n\npattern ...'
	EXEC	cmd "$usage" -y
		OUTPUT - 'return=-10 option=-10 name=-y arg=(null) num=1'
	EXEC	cmd "$usage" --y
		OUTPUT - 'return=-10 option=-10 name=--yes arg=(null) num=1'
	EXEC	cmd "$usage" -x
		OUTPUT - 'return=x option=-x name=-x arg=(null) num=1'
	EXEC	cmd "$usage" -xabc
		OUTPUT - 'return=x option=-x name=-x arg=abc num=1'
	EXEC	cmd "$usage" -x abc
	EXEC	cmd "$usage" --method
		OUTPUT - 'return=x option=-x name=--method arg=(null) num=1'
	EXEC	cmd "$usage" --method=abc
		OUTPUT - 'return=x option=-x name=--method arg=abc num=1'
	EXEC	cmd "$usage" --method abc
		OUTPUT - $'return=x option=-x name=--method arg=(null) num=1\nargument=1 value="abc"'
	EXEC	cmd "$usage" -V -x --si 10k --size=1m
		OUTPUT - $'return=V option=-V name=-V arg=(null) num=1\nreturn=x option=-x name=-x arg=(null) num=1\nreturn=z option=-z name=--size arg=10k num=10240\nreturn=z option=-z name=--size arg=1m num=1048576'
	EXEC	cmd "$usage" --yes --no --noyes --no-yes --yes=1 --yes=0
		OUTPUT - $'return=-10 option=-10 name=--yes arg=(null) num=1\nreturn=-20 option=-20 name=--no arg=(null) num=1\nreturn=-10 option=-10 name=--yes arg=(null) num=0\nreturn=-10 option=-10 name=--yes arg=(null) num=0\nreturn=-10 option=-10 name=--yes arg=(null) num=1\nreturn=-10 option=-10 name=--yes arg=(null) num=0'
	EXEC	cmd "$usage" --vern
		OUTPUT - $'return=V option=-V name=--vernum arg=(null) num=1'
	EXEC	cmd "$usage" --ver
		OUTPUT - $'return=: option=-V name=--ver num=0'
		ERROR - $'cmd: --ver: ambiguous option'
		EXIT 1
	EXEC	cmd "$usage" --ve
		OUTPUT - $'return=: option=-V name=--ve num=0'
		ERROR - $'cmd: --ve: ambiguous option'
	EXEC	cmd "$usage" --v
		OUTPUT - $'return=: option=-V name=--v num=0'
		ERROR - $'cmd: --v: ambiguous option'
	EXEC	cmd "$usage" -z
		OUTPUT - $'return=: option=-z name=-z num=0'
		ERROR - $'cmd: -z: numeric sizeX argument expected'
	EXEC	cmd "$usage" --size
		OUTPUT - $'return=: option=-z name=--size num=0'
		ERROR - $'cmd: --size: numeric sizeX value expected'
	EXEC	cmd "$usage" --foo
		OUTPUT - $'return=: option= name=--foo num=0 str=--foo'
		ERROR - $'cmd: --foo: unknown option'
	EXEC	cmd "$usage" --foo=bar
		OUTPUT - $'return=: option= name=--foo num=0 str=--foo=bar'
		ERROR - $'cmd: --foo: unknown option'
	EXEC	cmd "$usage" --version
		OUTPUT - $'return=? option=- name=--version num=0'
		ERROR - $'  version         testopt (AT&T Research) 1999-02-02'
		EXIT 2
	EXEC	cmd "$usage" '--?-'
		OUTPUT - $'return=? option=-? name=--?- num=0'
		ERROR - $'  version         testopt (AT&T Research) 1999-02-02\n  author          Glenn Fowler <gsf@research.att.com>'
	EXEC	cmd "$usage" '--??about'
		OUTPUT - $'return=? option=-? name=--??about num=0'
	EXEC	cmd "$usage" '--about'
		OUTPUT - $'return=? option=- name=--about num=0'
	EXEC	cmd "$usage" '--?--'
		OUTPUT - $'return=? option=-? name=--?-- num=0'
		ERROR - $'  version         testopt (AT&T Research) 1999-02-02\n  author          Glenn Fowler <gsf@research.att.com>\n  dictionary      tests:opt'
	EXEC	cmd "$usage" -?
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: cmd [-Vvn] [-x[algorithm]] [ -y ] [-d path] [-z sizeX] pattern ...'
	EXEC	cmd "$usage" --?
		OUTPUT - $'return=? option=-? name=--? num=0'
		ERROR - $'Usage: cmd [ options ] pattern ...
OPTIONS
  -x, --method[=algorithm]
                  One of
                    a     method a
                    b     method b
                    c     method c
                  The option value may be omitted.
  -y, --yes       Yes.
  --no            No.
  -d, --database=path
                  File database path.
  -z, --size=sizeX
                  Important size.
  -V, --vernum    List program version and exit.
  -v, --verbose   Enable verbose trace.
  -n, --show      Show but don\'t execute.'
	EXEC	cmd "$usage" --??api
		OUTPUT - $'return=? option=-? name=--??api num=0'
		ERROR - $'.SH SYNOPSIS\ncmd [ options ] pattern ...\n.SH OPTIONS\n.OP x method string:ignorecase:oneof:optional algorithm\nOne of\n.H3 a\nmethod a\n.H3 b\nmethod b\n.H3 c\nmethod c\n.PP\nThe option value may be omitted.\n.OP - - oneof\n.OP y yes flag -\nYes.\n.OP - no flag -\nNo.\n.OP - - anyof\n.OP d database string path\nFile database path.\n.OP z size number sizeX\nImportant size.\n.OP V vernum flag -\nList program version and exit.\n.OP v verbose flag -\nEnable verbose trace.\n.OP n show flag -\nShow but don\'t execute.\n.SH IMPLEMENTATION\n.H1 version\ntestopt (AT&T Research) 1999-02-02\n.H1 author\nGlenn Fowler <gsf@research.att.com>'
	EXEC	cmd "$usage" --?help
		OUTPUT - $'return=? option=-? name=--?help num=0'
		ERROR - $'Usage: cmd [--??help] [--help=?help]
OPTIONS
  --??help        List detailed help option info.'
	EXEC	cmd "$usage" --??help
		OUTPUT - $'return=? option=-? name=--??help num=0'
		ERROR - $'Usage: cmd [-?] [--?[item]] [--help[=item]]
OPTIONS
  -? and --?* options are the same for all ast commands. For any item below, if
  --item is not supported by a given command then it is equivalent to --??item.
  All output is written to the standard error.
    --??about     List all implementation info.
    --??api       List detailed info in program readable form.
    --??help      List detailed help option info.
    --??html      List detailed info in html.
    --??keys      List the usage translation key strings with C style escapes.
    --??long      List long option usage.
    --??man       List detailed info in displayed man page form.
    --??nroff     List detailed info in nroff.
    --??options   List short and long option details.
    --??short     List short option usage.
    --??usage     List the usage string with C style escapes.
    --?-label     List implementation info matching label*.
    --?name       List descriptions for long options matching name*.
    --?           Equivalent to --??options.
    --??          Equivalent to --??man.
    --???         Equivalent to --??help.
    -?            --??long if any long options otherwise --??short.
    --???item     Print version=n: n>0 if --??item is supported, 0 otherwise.
    --???ESC      Emit escape codes even if output is not a terminal.
    --???TEST     Massage output for regression testing.'
	EXEC	cmd "$usage" --???
		OUTPUT - $'return=? option=-? name=--??? num=0'
	EXEC	cmd "$usage" --??usage
		OUTPUT - $'return=? option=-? name=--??usage num=0'
		ERROR - $'[-?\\n@(#)testopt (AT&T Research) 1999-02-02\\n][-author?Glenn Fowler <gsf@research.att.com>][--dictionary?tests:opt][x:method?One of]:?[algorithm:oneof:ignorecase]{[+a?method a][+b?method b][+c?method c]}[\\n[y=10:yes?Yes.][20:no?No.]\\n][d:database?File database path.]:[path][z:size?Important size.]#[sizeX][V:vernum?List program version and exit.][v:verbose?Enable verbose trace.][n:show?Show but don\\\'t execute.]\\n\\npattern ...'

TEST 03 'info callback'
	usage=$'[+][x:method?One of \fmethods\f.]:?[algorithm][m:meta?Enable metachars.]\n\npattern ...'
	EXEC	cmd "$usage" --?met
		OUTPUT - $'return=? option=-? name=--?met num=0'
		ERROR - $'Usage: cmd [ options ] pattern ...
OPTIONS
  -x, --method[=algorithm]
                  One of <* methods info ok *>. The option value may be
                  omitted.
  -m, --meta      Enable metachars.'
		EXIT 2
	usage=$'[-][+NAME?stty][a:aaa?AAA][z:zzz?ZZZ]:[path][+\fone\f?\ftwo\f]{[+?\fthree\f]}'
	EXEC	cmd "$usage" --man
		OUTPUT - $'return=? option=- name=--man num=0'
		ERROR - $'NAME
  stty

SYNOPSIS
  cmd [ options ]

OPTIONS
  -a, --aaa       AAA
  -z, --zzz=path  ZZZ

<* one info ok *>
  <* two info ok *>
          <* three info ok *>'
	usage=$'[-][+NAME?stty][z:zzz?ZZZ]:[path]{\fzero\f}[a:aaa?AAA][+\fone\f?\ftwo\f]{[+?\fthree\f]}'
	EXEC	cmd "$usage" --man
		ERROR - $'NAME
  stty

SYNOPSIS
  cmd [ options ]

OPTIONS
  -z, --zzz=path  ZZZ
                    yabba dabba
                    doo   aroni
  -a, --aaa       AAA

<* one info ok *>
  <* two info ok *>
          <* three info ok *>'

TEST 04 'oh man'
	usage=$'
	[-?@(#)sum (AT&T Labs Research) 1999-01-23\n]
	[-author?Glenn Fowler <gsf@research.att.com>]
	[-author?David Korn <dgk@research.att.com>]
	[-copyright?Copyright (c) 1989-1999 AT&T Corp.]
	[-license?http://www.research.att.com/sw/license/ast-open.html]
	[+NAME?\f?\f - write file checksums and sizes]
	[+DESCRIPTION?\b\f?\f\b calculates and writes to the standard
		output a checksum and the number of bytes in each file.
		Alternate algorithms can be specified as an option.
		The default algorithm is POSIX CRC.]
	[+?This should start another paragraph under DESCRIPTION.]
	[v:v*erbose?List verbose information on the algorithm. This
		can get really long and really wordy.]
	[V:vernum?List the program version and exit.]
	[a:each?List sum for each file when used with --total.]
	[l:file-list?Each file is a list of file names to sum.]
	[r:recursive?Recursively sum the files in directories.]
	[10:furby?Use fuzzy logic.]
	[t:total?Print the sum of the concatenation of all files.]
	[m:magic?Magic incantation.]#?[yyxzy]
	[x:algorithm|method?The checksum algorithm to apply.]:
		[algorithm:=att:oneof:ignorecase]
		{
			[+att|sys5|s5?The traditional system 5 sum.]
			{
				[+one?Test sub sub 1.]
				[+two?Test sub sub 2.]
			}
			[+ast?The ast pseudo-random number generator sum.]
			[+md5?The message digest checksum.]
		}
	[:woo?Short test 0.]:[junk-0]
	[A?Short test 1.]:[junk-1]
	[B:?Short test 2.]:[junk-2]
	[C:junk?Short test 3.]:[junk-3]
	[D:?Short test 4.]:[junk-4]
	[E]:[junk-5]
	[F:]:[junk-6]
	[G?]:[junk-7]
	[H:?]:[junk-8]
	[Q:test?Enable test code -- could be buggy.]

	file file
	file [ ... dir ]

	[+STANDARD OUTPUT FORMAT?"%u %d %s\\n" <\achecksum\a> <\apathname\a>]
	[+?If no \afile\a operand is specified, the pathname and its leading
		space are omitted.]
	[+FILES?These files are referenced whether [you]] like it [or not.]]]
	{
		[+lib/file/magic?Magic number table.]
		[+lib/find/find.codes?Fast find database.]
	}
	[+JUNK]
	{
		[+lib/file/magic?Magic number table.]
		[+lib/locate/locate.db?GNU fast find database.]
	}
	[+AND ALSO MORE JUNK TOO?First paragraph.]
	[+?Next paragraph.]
'
	EXEC	sum "$usage" '-?'
		EXIT 2
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: sum [-vValrtQ] [-m[yyxzy]] [-x algorithm] [-A junk-1] [-B junk-2]
           [-C junk-3] [-D junk-4] [-E junk-5] [-F junk-6] [-G junk-7]
           [-H junk-8] file file
   Or: sum [ options ] file [ ... dir ]'
	EXEC	sum "$usage" '--?'
		OUTPUT - $'return=? option=-? name=--? num=0'
		ERROR - $'Usage: sum [ options ] file file
   Or: sum [ options ] file [ ... dir ]
OPTIONS
  -v, --v*erbose  List verbose information on the algorithm. This can get
                  really long and really wordy.
  -V, --vernum    List the program version and exit.
  -a, --each      List sum for each file when used with --total.
  -l, --file-list Each file is a list of file names to sum.
  -r, --recursive Recursively sum the files in directories.
  --furby         Use fuzzy logic.
  -t, --total     Print the sum of the concatenation of all files.
  -m, --magic[=yyxzy]
                  Magic incantation. The option value may be omitted.
  -x, --algorithm|method=algorithm
                  The checksum algorithm to apply.
                    att|sys5|s5
                          The traditional system 5 sum.
                            one   Test sub sub 1.
                            two   Test sub sub 2.
                    ast   The ast pseudo-random number generator sum.
                    md5   The message digest checksum.
                  The default value is att.
  --woo=junk-0    Short test 0.
  -A junk-1       Short test 1.
  -B junk-2       Short test 2.
  -C, --junk=junk-3
                  Short test 3.
  -D junk-4       Short test 4.
  -E junk-5
  -F junk-6
  -G junk-7
  -H junk-8
  -Q, --test      Enable test code -- could be buggy.'
	EXEC	sum "$usage" '--??'
		OUTPUT - $'return=? option=-? name=--?? num=0'
		ERROR - $'NAME
  sum - write file checksums and sizes

SYNOPSIS
  sum [ options ] file file
  sum [ options ] file [ ... dir ]

DESCRIPTION
  sum calculates and writes to the standard output a checksum and the number of
  bytes in each file. Alternate algorithms can be specified as an option. The
  default algorithm is POSIX CRC.

  This should start another paragraph under DESCRIPTION.

OPTIONS
  -v, --v*erbose  List verbose information on the algorithm. This can get
                  really long and really wordy.
  -V, --vernum    List the program version and exit.
  -a, --each      List sum for each file when used with --total.
  -l, --file-list Each file is a list of file names to sum.
  -r, --recursive Recursively sum the files in directories.
  --furby         Use fuzzy logic.
  -t, --total     Print the sum of the concatenation of all files.
  -m, --magic[=yyxzy]
                  Magic incantation. The option value may be omitted.
  -x, --algorithm|method=algorithm
                  The checksum algorithm to apply.
                    att|sys5|s5
                          The traditional system 5 sum.
                            one   Test sub sub 1.
                            two   Test sub sub 2.
                    ast   The ast pseudo-random number generator sum.
                    md5   The message digest checksum.
                  The default value is att.
  --woo=junk-0    Short test 0.
  -A junk-1       Short test 1.
  -B junk-2       Short test 2.
  -C, --junk=junk-3
                  Short test 3.
  -D junk-4       Short test 4.
  -E junk-5
  -F junk-6
  -G junk-7
  -H junk-8
  -Q, --test      Enable test code -- could be buggy.

STANDARD OUTPUT FORMAT
  "%u %d %s\\n" <checksum> <pathname>

  If no file operand is specified, the pathname and its leading space are
  omitted.

FILES
  These files are referenced whether [you] like it [or not.]
    lib/file/magic
          Magic number table.
    lib/find/find.codes
          Fast find database.

JUNK
    lib/file/magic
          Magic number table.
    lib/locate/locate.db
          GNU fast find database.

AND ALSO MORE JUNK TOO
  First paragraph.

  Next paragraph.

IMPLEMENTATION
  version         sum (AT&T Labs Research) 1999-01-23
  author          Glenn Fowler <gsf@research.att.com>
  author          David Korn <dgk@research.att.com>
  copyright       Copyright (c) 1989-1999 AT&T Corp.
  license         http://www.research.att.com/sw/license/ast-open.html'
	EXEC	sum "$usage" '--?' -
		OUTPUT - $'return=? option=-? name=--? num=0'
		ERROR - $'  version         sum (AT&T Labs Research) 1999-01-23
  author          Glenn Fowler <gsf@research.att.com>
  author          David Korn <dgk@research.att.com>
  copyright       Copyright (c) 1989-1999 AT&T Corp.
  license         http://www.research.att.com/sw/license/ast-open.html'
	EXEC	sum "$usage" '--?' -ver
		OUTPUT - $'return=? option=-? name=--? num=0'
		ERROR - $'  version         sum (AT&T Labs Research) 1999-01-23'
	EXEC	sum "$usage" '--?' -copyright
		OUTPUT - $'return=? option=-? name=--? num=0'
		ERROR - $'  copyright       Copyright (c) 1989-1999 AT&T Corp.'
	EXEC	sum "$usage" '--?' -license
		OUTPUT - $'return=? option=-? name=--? num=0'
		ERROR - $'  license         http://www.research.att.com/sw/license/ast-open.html'
	EXEC	sum "$usage" '--?' -author
		OUTPUT - $'return=? option=-? name=--? num=0'
		ERROR - $'  author          Glenn Fowler <gsf@research.att.com>
  author          David Korn <dgk@research.att.com>'
	EXEC	sum "$usage" '--?' -foo
		EXIT 1
		OUTPUT - $'return=: option=-? name=--? num=0'
		ERROR - $'sum: -foo: unknown option'

TEST 05 'should at least generate its own man page'
	usage=$'[-?@(#)getopts (AT&T Research) 1999-02-02\n]
[+NAME?\f?\f - parse utility options]
a:[name?Use \aname\a instead of the command name in usage messages.]

opstring name [args...]

[+DESCRIPTION?The \bgetopts\b utility can be used to retrieve options and
arguments from a list of arguments give by \aargs\a or the positional
parameters if \aargs\a is omitted.  It can also generate usage messages
and a man page for the command based on the information in \aoptstring\a.]
[+?The \aoptstring\a string consists of alpha-numeric characters,
the special characters +, -, ?, :, and <space>, or character groups
enclosed in [...]].  Character groups may be nested in {...}.
Outside of a [...]] group, a single new-line followed by zero or
more blanks is ignored.  One or more blank lines separates the
options from the command argument synopsis.]
[+?Each [...]] group consists of an optional label,
optional attributes separated by :, and an
optional description string following ?.  The characters from the ?
to the end of the next ]] are ignored for option parsing and short
usage messages.  They are used for generating verbose help or man pages.
The : character may not appear in the label
The ?? character must be specified as ???? in label and the ]] character
must be spcified as ]]]] in the description string.
Text between two \\b (backspace) characters indicates
that the text should be emboldened when displayed.
Text between two \\a (bell) characters indicates that the text should
be emphasised or italicised when displayed.]
[+?There are four types of groups:]
{
	[+1.?An option specifiation of the form \aoption\a:\alongname\a.
	In this case the first field is the option character.  If there
	is no option character, then a two digit number should be specified
	that corresponds to the long options.  This negative of this number
	will be returned as the value of \aname\a by \bgetopts\b if the long
	option is matched. A longname is matched with \b--\b\alongname\a.  A
	* in the \alongname\a field indicates that only characters up that
	point need to match provided any additional characters match the option.
	The [ and ]] can be omitted for options that don\'t have longnames
	or descriptive text.]
	[+2.?A string option argument specification.
	Options that take arguments can be followed by : or # and an option
	group specification.  An option group specification consists
	of a name for the option argument as field 1.   The remaining
	fields are a typename and zero or more of the special attribute words
	\blistof\b, \boneof\b, and \bignorecase\b.
	The option specification can be followed
	by a list of option value descriptions enclosed in parenthesis.]
	[+3.?A option value description.]
	[+4.?A argument specification. A list of valid option argument values
		can be specified by enclosing them inside a {...} following
		the option argument specification.  Each of the permitted
		values can be specified with a [...]] containing the
		value followed by a description.]
}
[+?If the leading character of \aoptstring\a is +, then arguments
beginning with + will also be considered options.]
[+?A leading : character or a : following a leading + in \aoptstring\a
affects the way errors are handled.  If an option character or longname
argument not specified in \aoptstring\a is encountered when processing
options, the shell variable whose name is \aname\a will be set to the ?
character.  The shell variable \bOPTARG\b will be set to
the character found.  If an option argument is missing or has an invalid
value, then \aname\a will be set to the : character and the shell variable
\bOPTARG\b will be set to the option character found.
Without the leading :, \aname\a will be set to the ? character, \bOPTARG\b
will be unset, and an error message will be written to standard error
when errors are encountered.]
[+?The end of options occurs when:]
{
	[+1.?The special argument \b--\b.]
	[+2.?An argument that does not beging with a \b-\b.]
	[+3.?A help argument is specified.]
	[+4.?An error is encountered.]
}
[+?If \bOPTARG\b is set to the value \b1\b, a new set of arguments
can be used.]
[+?\bgetopts\b can also be used to generate help messages containing command
usage and detailed descriptions.  Specify \aargs\a as:]
{
	[+-???To generate a usage synopsis.]
	[+--?????To generate a verbose usage message.]
	[+--????man?To generate a formatted man page.]
	[+--????api?To generate an easy to parse usage message.]
	[+--????html?To generate a man page in \bhtml\b format.]
}
[+?When the end of options is encountered, \bgetopts\b exits with a
non-zero return value and the variable \bOPTIND\b is set to the
index of the first non-option argument.]
[+EXIT STATUS]
{
	[+0?An option specified was found.]
	[+1?An end of options was encountered.]
	[+2?A usage or information message was generated.]
}
'
	EXEC	getopts "$usage" '-?'
		EXIT 2
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: getopts [-a name] opstring name [args...]'
	EXEC	getopts "$usage" '--?'
		OUTPUT - $'return=? option=-? name=--? num=0'
		ERROR - $'Usage: getopts [ options ] opstring name [args...]
OPTIONS
  -a name         Use name instead of the command name in usage messages.'
	EXEC	getopts "$usage" '--??'
		OUTPUT - $'return=? option=-? name=--?? num=0'
		ERROR - $'NAME
  getopts - parse utility options

SYNOPSIS
  getopts [ options ] opstring name [args...]

OPTIONS
  -a name         Use name instead of the command name in usage messages.

DESCRIPTION
  The getopts utility can be used to retrieve options and arguments from a list
  of arguments give by args or the positional parameters if args is omitted. It
  can also generate usage messages and a man page for the command based on the
  information in optstring.

  The optstring string consists of alpha-numeric characters, the special
  characters +, -, ?, :, and <space>, or character groups enclosed in [...].
  Character groups may be nested in {...}. Outside of a [...] group, a single
  new-line followed by zero or more blanks is ignored. One or more blank lines
  separates the options from the command argument synopsis.

  Each [...] group consists of an optional label, optional attributes separated
  by :, and an optional description string following ?. The characters from the
  ? to the end of the next ] are ignored for option parsing and short usage
  messages. They are used for generating verbose help or man pages. The :
  character may not appear in the label The ?? character must be specified as
  ???? in label and the ] character must be spcified as ]] in the description
  string. Text between two \\b (backspace) characters indicates that the text
  should be emboldened when displayed. Text between two \\a (bell) characters
  indicates that the text should be emphasised or italicised when displayed.

  There are four types of groups:
    1.    An option specifiation of the form option:longname. In this case the
          first field is the option character. If there is no option character,
          then a two digit number should be specified that corresponds to the
          long options. This negative of this number will be returned as the
          value of name by getopts if the long option is matched. A longname is
          matched with --longname. A * in the longname field indicates that
          only characters up that point need to match provided any additional
          characters match the option. The [ and ] can be omitted for options
          that don\'t have longnames or descriptive text.
    2.    A string option argument specification. Options that take arguments
          can be followed by : or # and an option group specification. An
          option group specification consists of a name for the option argument
          as field 1. The remaining fields are a typename and zero or more of
          the special attribute words listof, oneof, and ignorecase. The option
          specification can be followed by a list of option value descriptions
          enclosed in parenthesis.
    3.    A option value description.
    4.    A argument specification. A list of valid option argument values can
          be specified by enclosing them inside a {...} following the option
          argument specification. Each of the permitted values can be specified
          with a [...] containing the value followed by a description.

  If the leading character of optstring is +, then arguments beginning with +
  will also be considered options.

  A leading : character or a : following a leading + in optstring affects the
  way errors are handled. If an option character or longname argument not
  specified in optstring is encountered when processing options, the shell
  variable whose name is name will be set to the ? character. The shell
  variable OPTARG will be set to the character found. If an option argument is
  missing or has an invalid value, then name will be set to the : character and
  the shell variable OPTARG will be set to the option character found. Without
  the leading :, name will be set to the ? character, OPTARG will be unset, and
  an error message will be written to standard error when errors are
  encountered.

  The end of options occurs when:
    1.    The special argument --.
    2.    An argument that does not beging with a -.
    3.    A help argument is specified.
    4.    An error is encountered.

  If OPTARG is set to the value 1, a new set of arguments can be used.

  getopts can also be used to generate help messages containing command usage
  and detailed descriptions. Specify args as:
    -?    To generate a usage synopsis.
    --??  To generate a verbose usage message.
    --??man
          To generate a formatted man page.
    --??api
          To generate an easy to parse usage message.
    --??html
          To generate a man page in html format.

  When the end of options is encountered, getopts exits with a non-zero return
  value and the variable OPTIND is set to the index of the first non-option
  argument.

EXIT STATUS
    0     An option specified was found.
    1     An end of options was encountered.
    2     A usage or information message was generated.

IMPLEMENTATION
  version         getopts (AT&T Research) 1999-02-02'
	EXEC	getopts "$usage" '--??api'
		OUTPUT - $'return=? option=-? name=--??api num=0'
		ERROR - $'.SH NAME
getopts - parse utility options
.SH SYNOPSIS
getopts [ options ] opstring name [args...]
.SH OPTIONS
.OP a - string name
Use name instead of the command name in usage messages.
.SH DESCRIPTION
The getopts utility can be used to retrieve options and arguments from a list
of arguments give by args or the positional parameters if args is omitted. It
can also generate usage messages and a man page for the command based on the
information in optstring.
.PP
The optstring string consists of alpha-numeric characters, the special
characters +, -, ?, :, and <space>, or character groups enclosed in [...].
Character groups may be nested in {...}. Outside of a [...] group, a single
new-line followed by zero or more blanks is ignored. One or more blank lines
separates the options from the command argument synopsis.
.PP
Each [...] group consists of an optional label, optional attributes separated
by :, and an optional description string following ?. The characters from the ?
to the end of the next ] are ignored for option parsing and short usage
messages. They are used for generating verbose help or man pages. The :
character may not appear in the label The ?? character must be specified as
???? in label and the ] character must be spcified as ]] in the description
string. Text between two \\b (backspace) characters indicates that the text
should be emboldened when displayed. Text between two \\a (bell) characters
indicates that the text should be emphasised or italicised when displayed.
.PP
There are four types of groups:
.H1 1.
An option specifiation of the form option:longname. In this case the first
field is the option character. If there is no option character, then a two
digit number should be specified that corresponds to the long options. This
negative of this number will be returned as the value of name by getopts if the
long option is matched. A longname is matched with --longname. A * in the
longname field indicates that only characters up that point need to match
provided any additional characters match the option. The [ and ] can be omitted
for options that don\'t have longnames or descriptive text.
.H1 2.
A string option argument specification. Options that take arguments can be
followed by : or # and an option group specification. An option group
specification consists of a name for the option argument as field 1. The
remaining fields are a typename and zero or more of the special attribute words
listof, oneof, and ignorecase. The option specification can be followed by a
list of option value descriptions enclosed in parenthesis.
.H1 3.
A option value description.
.H1 4.
A argument specification. A list of valid option argument values can be
specified by enclosing them inside a {...} following the option argument
specification. Each of the permitted values can be specified with a [...]
containing the value followed by a description.
.PP
If the leading character of optstring is +, then arguments beginning with +
will also be considered options.
.PP
A leading : character or a : following a leading + in optstring affects the way
errors are handled. If an option character or longname argument not specified
in optstring is encountered when processing options, the shell variable whose
name is name will be set to the ? character. The shell variable OPTARG will be
set to the character found. If an option argument is missing or has an invalid
value, then name will be set to the : character and the shell variable OPTARG
will be set to the option character found. Without the leading :, name will be
set to the ? character, OPTARG will be unset, and an error message will be
written to standard error when errors are encountered.
.PP
The end of options occurs when:
.H1 1.
The special argument --.
.H1 2.
An argument that does not beging with a -.
.H1 3.
A help argument is specified.
.H1 4.
An error is encountered.
.PP
If OPTARG is set to the value 1, a new set of arguments can be used.
.PP
getopts can also be used to generate help messages containing command usage and
detailed descriptions. Specify args as:
.H1 -?
To generate a usage synopsis.
.H1 --??
To generate a verbose usage message.
.H1 --??man
To generate a formatted man page.
.H1 --??api
To generate an easy to parse usage message.
.H1 --??html
To generate a man page in html format.
.PP
When the end of options is encountered, getopts exits with a non-zero return
value and the variable OPTIND is set to the index of the first non-option
argument.
.SH EXIT STATUS
.H1 0
An option specified was found.
.H1 1
An end of options was encountered.
.H1 2
A usage or information message was generated.
.SH IMPLEMENTATION
.H1 version
getopts (AT&T Research) 1999-02-02'
	EXEC	getopts "$usage" '--??html'
		OUTPUT - $'return=? option=-? name=--??html num=0'
		ERROR - $'<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML//EN">
<HTML>
<HEAD>
<META NAME="generator" CONTENT="optget (AT&T Labs Research) 1999-08-11">
<TITLE>getopts man document</TITLE>
</HEAD>
<BODY bgcolor=\'#ffffff\'>
<H4><TABLE WIDTH=100%><TR><TH ALIGN=LEFT>&nbsp;getopts&nbsp;(&nbsp;1&nbsp;)&nbsp;<TH ALIGN=CENTER><A HREF="" TITLE="Index">USER COMMANDS</A><TH ALIGN=RIGHT>getopts&nbsp;(&nbsp;1&nbsp;)</TR></TABLE></H4>
<HR>
<DL COMPACT>
<DT><H4><A NAME="NAME">NAME</A></H4>
<DL COMPACT>
<DT>getopts - parse utility options
<P>
</DL>
<DT><H4><A NAME="SYNOPSIS">SYNOPSIS</A></H4>
<DL COMPACT>
<DT><B>getopts</B> &#0091; <I>options</I> &#0093; opstring name &#0091;args...&#0093;
<P>
</DL>
<DT><H4><A NAME="OPTIONS">OPTIONS</A></H4>
<DL COMPACT>
<DT>-<B>a</B> <I>name</I><DD>Use <I>name</I> instead of the command name in
usage messages.
</DL>
<DT><H4><A NAME="DESCRIPTION">DESCRIPTION</A></H4>
<DL COMPACT>
<DT>The <B>getopts</B> utility can be used to retrieve options and arguments
from a list of arguments give by <I>args</I> or the positional parameters if <I>args
</I> is omitted. It can also generate usage messages and a man page for the
command based on the information in <I>optstring</I>.
<P>
<DT>The <I>optstring</I> string consists of alpha-numeric characters, the
special characters +, -, ?, :, and &lt;space&gt;, or character groups enclosed
in &#0091;...&#0093;. Character groups may be nested in {...}. Outside of a &#0091;...&#0093; group, a
single new-line followed by zero or more blanks is ignored. One or more blank
lines separates the options from the command argument synopsis.
<P>
<DT>Each &#0091;...&#0093; group consists of an optional label, optional attributes
separated by :, and an optional description string following ?. The characters
from the ? to the end of the next &#0093; are ignored for option parsing and short
usage messages. They are used for generating verbose help or man pages. The :
character may not appear in the label The ?? character must be specified as
???? in label and the &#0093; character must be spcified as &#0093;&#0093; in the description
string. Text between two \\b (backspace) characters indicates that the text
should be emboldened when displayed. Text between two \\a (bell) characters
indicates that the text should be emphasised or italicised when displayed.
<P>
<DT>There are four types of groups:
<DL COMPACT>
<DL COMPACT>
<DT><A NAME="1."><B>1.</B></A><DD>An option specifiation of the form <I>option
</I>:<I>longname</I>. In this case the first field is the option character. If
there is no option character, then a two digit number should be specified that
corresponds to the long options. This negative of this number will be returned
as the value of <I>name</I> by <B>getopts</B> if the long option is matched. A
longname is matched with <B>--</B><I>longname</I>. A * in the <I>longname</I>
field indicates that only characters up that point need to match provided any
additional characters match the option. The &#0091; and &#0093; can be omitted for options
that don\'t have longnames or descriptive text.
<DT><A NAME="2."><B>2.</B></A><DD>A string option argument specification.
Options that take arguments can be followed by : or # and an option group
specification. An option group specification consists of a name for the option
argument as field 1. The remaining fields are a typename and zero or more of
the special attribute words <B>listof</B>, <B>oneof</B>, and <B>ignorecase</B>.
The option specification can be followed by a list of option value descriptions
enclosed in parenthesis.
<DT><A NAME="3."><B>3.</B></A><DD>A option value description.
<DT><A NAME="4."><B>4.</B></A><DD>A argument specification. A list of valid
option argument values can be specified by enclosing them inside a {...}
following the option argument specification. Each of the permitted values can
be specified with a &#0091;...&#0093; containing the value followed by a description.
<P>
</DL>
</DL>
<DT>If the leading character of <I>optstring</I> is +, then arguments beginning
with + will also be considered options.
<P>
<DT>A leading : character or a : following a leading + in <I>optstring</I>
affects the way errors are handled. If an option character or longname argument
not specified in <I>optstring</I> is encountered when processing options, the
shell variable whose name is <I>name</I> will be set to the ? character. The
shell variable <B>OPTARG</B> will be set to the character found. If an option
argument is missing or has an invalid value, then <I>name</I> will be set to
the : character and the shell variable <B>OPTARG</B> will be set to the option
character found. Without the leading :, <I>name</I> will be set to the ?
character, <B>OPTARG</B> will be unset, and an error message will be written to
standard error when errors are encountered.
<P>
<DT>The end of options occurs when:
<DL COMPACT>
<DL COMPACT>
<DT><A NAME="1."><B>1.</B></A><DD>The special argument <B>--</B>.
<DT><A NAME="2."><B>2.</B></A><DD>An argument that does not beging with a <B>-
</B>.
<DT><A NAME="3."><B>3.</B></A><DD>A help argument is specified.
<DT><A NAME="4."><B>4.</B></A><DD>An error is encountered.
<P>
</DL>
</DL>
<DT>If <B>OPTARG</B> is set to the value <B>1</B>, a new set of arguments can
be used.
<P>
<DT><B>getopts</B> can also be used to generate help messages containing
command usage and detailed descriptions. Specify <I>args</I> as:
<DL COMPACT>
<DL COMPACT>
<DT><A NAME="-?"><B>-?</B></A><DD>To generate a usage synopsis.
<DT><A NAME="--??"><B>--??</B></A><DD>To generate a verbose usage message.
<DT><A NAME="--??man"><B>--??man</B></A><DD>To generate a formatted man page.
<DT><A NAME="--??api"><B>--??api</B></A><DD>To generate an easy to parse usage
message.
<DT><A NAME="--??html"><B>--??html</B></A><DD>To generate a man page in <B>html
</B> format.
<P>
</DL>
</DL>
<DT>When the end of options is encountered, <B>getopts</B> exits with a
non-zero return value and the variable <B>OPTIND</B> is set to the index of the
first non-option argument.
</DL>
<DT><H4><A NAME="EXIT STATUS">EXIT STATUS</A></H4>
<DL COMPACT>
<DT><A NAME="0"><B>0</B></A><DD>An option specified was found.
<DT><A NAME="1"><B>1</B></A><DD>An end of options was encountered.
<DT><A NAME="2"><B>2</B></A><DD>A usage or information message was generated.
</DL>
<DT><H4><A NAME="IMPLEMENTATION">IMPLEMENTATION</A></H4>
<DL COMPACT>
<DT><A NAME="version"><B>version</B></A><DD>getopts (AT&amp;T Research)
1999-02-02
</DL></DL>
</BODY>
</HTML>'
	EXEC	getopts "$usage" '--version'
		OUTPUT - $'return=? option=- name=--version num=0'
		ERROR - $'  version         getopts (AT&T Research) 1999-02-02'

TEST 06 'bugs of omission'
	usage=$'[-][+NAME?locate][a:again?Look again.][b:noback?Don\'t look back.][n:notnow?Don\'t look now.][t:twice?Look twice.][+FOO]{[+foo?aha]}[+BAR?bahah]'
	EXEC	locate "$usage" '-?'
		EXIT 2
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: locate [-abnt]'
	EXEC	locate "$usage" '--?'
		OUTPUT - $'return=? option=-? name=--? num=0'
		ERROR - $'Usage: locate [ options ]
OPTIONS
  -a, --again     Look again.
  -b, --noback    Don\'t look back.
  -n, --notnow    Don\'t look now.
  -t, --twice     Look twice.'
	EXEC	locate "$usage" '--man'
		OUTPUT - $'return=? option=- name=--man num=0'
		ERROR - $'NAME
  locate

SYNOPSIS
  locate [ options ]

OPTIONS
  -a, --again     Look again.
  -b, --noback    Don\'t look back.
  -n, --notnow    Don\'t look now.
  -t, --twice     Look twice.

FOO
    foo   aha

BAR
  bahah'

TEST 07 'return value tests'
	usage=$'[-][a=1111:aaa]\t[b=2:bbb]\t[=3:ccc][44:ddd][i!:iii][j!:jjj?Yada yada.]'
	EXEC tst "$usage" -a
		OUTPUT - $'return=-1111 option=-1111 name=-a arg=(null) num=1'
	EXEC tst "$usage" --a
		OUTPUT - $'return=-1111 option=-1111 name=--aaa arg=(null) num=1'
	EXEC tst "$usage" -b
		OUTPUT - $'return=-2 option=-2 name=-b arg=(null) num=1'
	EXEC tst "$usage" --b
		OUTPUT - $'return=-2 option=-2 name=--bbb arg=(null) num=1'
	EXEC tst "$usage" --c
		OUTPUT - $'return=-3 option=-3 name=--ccc arg=(null) num=1'
	EXEC tst "$usage" --d
		OUTPUT - $'return=-44 option=-44 name=--ddd arg=(null) num=1'
	EXEC tst "$usage" -a -b --a --b --c --d -i --i --noi
		OUTPUT - $'return=-1111 option=-1111 name=-a arg=(null) num=1
return=-2 option=-2 name=-b arg=(null) num=1
return=-1111 option=-1111 name=--aaa arg=(null) num=1
return=-2 option=-2 name=--bbb arg=(null) num=1
return=-3 option=-3 name=--ccc arg=(null) num=1
return=-44 option=-44 name=--ddd arg=(null) num=1
return=i option=-i name=-i arg=(null) num=0
return=i option=-i name=--iii arg=(null) num=1
return=i option=-i name=--iii arg=(null) num=0'
	EXEC tst "$usage" '-?'
		EXIT 2
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: tst [-abij]'
	EXEC tst "$usage" '--?'
		OUTPUT - $'return=? option=-? name=--? num=0'
		ERROR - $'Usage: tst [ options ]\nOPTIONS\n  -a, --aaa\n  -b, --bbb\n  --ccc\n  --ddd\n  -i, --iii\n  -j, --jjj       Yada yada. On by default; -j means --nojjj.'
	EXEC tst "$usage" '--??api'
		OUTPUT - $'return=? option=-? name=--??api num=0'
		ERROR - $'.SH SYNOPSIS
tst [ options ]
.SH OPTIONS
.OP a aaa flag -
.OP b bbb flag -
.OP - ccc flag -
.OP - ddd flag -
.OP i iii flag:invert -
.OP j jjj flag:invert -
Yada yada.
 On by default; -j means --nojjj.
.SH IMPLEMENTATION
.PP'

TEST 08 'optstr() tests'
	usage=$'[-?@(#)pax (AT&T Labs Research) 1999-02-14\n]
[a:append?Append to end of archive.]
[101:atime?Preserve or set access time.]:?[time]
[z:base?Delta base archive name. - ignores base on input, compresses on output.]:[archive]
[b:blocksize?Input/output block size. The default is format specific.]#[size]
[102:blok?Input/output BLOK format for tapes on file.]:?[i|o]
[103:charset?Header data character set name.]:[name]

[ file ... ]

[-author?Glenn Fowler <gsf@research.att.com>]
[-license?http://www.research.att.com/sw/tools/reuse]
'
	EXEC	- pax "$usage" 'append base="aaa zzz" charset=us'
		OUTPUT - $'return=a option=-a name=append arg=(null) num=1
return=z option=-z name=base arg=aaa zzz num=1
return=-103 option=-103 name=charset arg=us num=1'
	EXEC	- pax "$usage" '14 foo'
		EXIT 1
		OUTPUT - $'return=: option= name=14 num=0 str=14 foo\nreturn=: option= name=foo num=0 str=14 foo'
		ERROR - $'pax: 14: unknown option\npax: foo: unknown option'

TEST 09 'hidden options'
	usage=$'ab?cd [ file ... ]'
	EXEC tst "$usage" -a -b -c -d
		OUTPUT - $'return=a option=-a name=-a arg=(null) num=1
return=b option=-b name=-b arg=(null) num=1
return=c option=-c name=-c arg=(null) num=1
return=d option=-d name=-d arg=(null) num=1'
	EXEC tst "$usage" -?
		EXIT 2
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: tst [-ab] [ file ... ]'
	EXEC tst "$usage" --?
		OUTPUT - $'return=? option=-? name=--? num=0'
	EXEC tst "$usage" --api
		OUTPUT - $'return=? option=- name=--api num=0'
		ERROR - $'.SH SYNOPSIS
tst [ options ] [ file ... ]
.SH OPTIONS
.OP a - flag -
.OP b - flag -
.OP c - flag:hidden -
.OP d - flag:hidden -'
	usage=$'ab:[arg]?c'
	EXEC tst "$usage" -a -b z -c
		EXIT 0
		OUTPUT - $'return=a option=-a name=-a arg=(null) num=1
return=b option=-b name=-b arg=z num=1
return=c option=-c name=-c arg=(null) num=1'
		ERROR -
	EXEC tst "$usage" -?
		EXIT 2
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: tst [-a] [-b arg]'
	EXEC tst "$usage" --?
		OUTPUT - $'return=? option=-? name=--? num=0'
	EXEC tst "$usage" --api
		OUTPUT - $'return=? option=- name=--api num=0'
		ERROR - $'.SH SYNOPSIS
tst [ options ]
.SH OPTIONS
.OP a - flag -
.OP b - string arg
.OP c - flag:hidden -'

TEST 10 'numeric options'
	old=$'bK#[old-key]'
	new=$'[-][b:ignoreblanks][K:oldkey?Obsolete]#[old-key]'
	alt=$'[-][b:ignoreblanks][K|X:oldkey?Obsolete]#[old-key]'
	EXEC sort "$old" -1 +2 file
		OUTPUT - $'return=K option=-K name=-1 arg=1 num=1\nreturn=K option=+K name=+2 arg=2 num=2\nargument=1 value="file"'
	EXEC sort "$new" -1 +2 file
	EXEC sort "$alt" -1 +2 file

TEST 11 'find style!'
	find=$'[-1p1][+NAME?\bfind\b - find files][13:amin?File was last accessed \aminutes\a minutes ago.]#[minutes][17:chop?Chop leading \b./\b from printed pathnames.]\n\n[ path ... ] [ option ]\n\n[+SEE ALSO?cpio(1), file(1), ls(1), sh(1), test(1), tw(1), stat(2)]'
	EXEC	find "$find" -amin 1 -chop
		OUTPUT - $'return=-13 option=-13 name=-amin arg=1 num=1\nreturn=-17 option=-17 name=-chop arg=(null) num=1'
	EXEC	find "$find" -amin=1 --chop
	EXEC	find "$find" --amin 1 --chop
	EXEC	find "$find" -amin
		OUTPUT - $'return=: option=-13 name=-amin num=0'
		ERROR - $'find: -amin: numeric minutes value expected'
		EXIT 1
	EXEC	find "$find" -foo
		OUTPUT - $'return=: option= name=-foo num=0 str=-foo'
		ERROR - $'find: -foo: unknown option'
	EXEC	find "$find" --foo
		OUTPUT - $'return=: option= name=-foo num=0 str=--foo'
		ERROR - $'find: -foo: unknown option'
	EXEC	find "$find" -foo=bar
		OUTPUT - $'return=: option= name=-foo num=0 str=-foo=bar'
		ERROR - $'find: -foo: unknown option'
	EXEC	find "$find" --foo=bar
		OUTPUT - $'return=: option= name=-foo num=0 str=--foo=bar'
		ERROR - $'find: -foo: unknown option'
	EXEC	find "$find" -?
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: find [-amin minutes] [-chop] [ path ... ] [ option ]'
		EXIT 2
	EXEC	find "$find" --?
		ERROR - $'Usage: find [ options ] [ path ... ] [ option ]\nOPTIONS\n  -amin minutes   File was last accessed minutes minutes ago.\n  -chop           Chop leading ./ from printed pathnames.'
	EXEC	find "$find" --man
		OUTPUT - $'return=? option=- name=-man num=0'
		ERROR - $'NAME\n  find - find files\n\nSYNOPSIS\n  find [ options ] [ path ... ] [ option ]\n\nOPTIONS\n  -amin minutes   File was last accessed minutes minutes ago.\n  -chop           Chop leading ./ from printed pathnames.\n\nSEE ALSO\n  cpio(1), file(1), ls(1), sh(1), test(1), tw(1), stat(2)'

TEST 12 'dd style!'
	dd=$'[-1p0][+NAME?\bdd\b - copy and convert file][10:if?Input file name.]:[file][11:conv?Conversion option.]:[conversion][+SEE ALSO?cp(1), pax(1), tr(1), seek(2)]'
	EXEC	dd "$dd" if=foo conv=ascii
		OUTPUT - $'return=-10 option=-10 name=if arg=foo num=1\nreturn=-11 option=-11 name=conv arg=ascii num=1'
	EXEC	dd "$dd" if foo conv ascii
	EXEC	dd "$dd" -if=foo --conv=ascii
	EXEC	dd "$dd" if
		OUTPUT - $'return=: option=-10 name=if num=0'
		ERROR - $'dd: if: file argument expected'
		EXIT 1
	EXEC	dd "$dd" --if=bar foo
		OUTPUT - $'return=-10 option=-10 name=if arg=bar num=1\nreturn=: option= name=foo num=0 str=foo'
		ERROR - $'dd: foo: unknown option'
	EXEC	dd "$dd" -foo
		OUTPUT - $'return=: option= name=foo num=0 str=-foo'
	EXEC	dd "$dd" --foo
		OUTPUT - $'return=: option= name=foo num=0 str=--foo'
	EXEC	dd "$dd" foo=bar
		OUTPUT - $'return=: option= name=foo num=0 str=foo=bar'
	EXEC	dd "$dd" -foo=bar
		OUTPUT - $'return=: option= name=foo num=0 str=-foo=bar'
	EXEC	dd "$dd" --foo=bar
		OUTPUT - $'return=: option= name=foo num=0 str=--foo=bar'
	EXEC	dd "$dd" -?
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: dd [if=file] [conv=conversion]'
		EXIT 2
	EXEC	dd "$dd" --?
		OUTPUT - $'return=? option=-? name=? num=0'
		ERROR - $'Usage: dd [ options ]\nOPTIONS\n  if=file         Input file name.\n  conv=conversion Conversion option.'
	EXEC	dd "$dd" --man
		OUTPUT - $'return=? option=- name=man num=0'
		ERROR - $'NAME\n  dd - copy and convert file\n\nSYNOPSIS\n  dd [ options ]\n\nOPTIONS\n  if=file         Input file name.\n  conv=conversion Conversion option.\n\nSEE ALSO\n  cp(1), pax(1), tr(1), seek(2)'

TEST 13 'unknown options'
	usage=$'[-][l:library]:[dll][G!:gzip] [ file ]'
	EXEC	pzip "$usage" --library=mps --checksum --nogzip data.db
		OUTPUT - $'return=l option=-l name=--library arg=mps num=1\nreturn=: option= name=--checksum num=0 str=--checksum\nreturn=G option=-G name=--gzip arg=(null) num=0\nargument=1 value="data.db"'
		ERROR - $'pzip: --checksum: unknown option'
		EXIT 1
	EXEC	pzip "$usage" --library=mps --checksum=1 --nogzip data.db
		OUTPUT - $'return=l option=-l name=--library arg=mps num=1\nreturn=: option= name=--checksum num=0 str=--checksum=1\nreturn=G option=-G name=--gzip arg=(null) num=0\nargument=1 value="data.db"'
	EXEC	pzip "$usage" --library=mps --nochecksum --nogzip data.db
		OUTPUT - $'return=l option=-l name=--library arg=mps num=1\nreturn=: option= name=--nochecksum num=0 str=--nochecksum\nreturn=G option=-G name=--gzip arg=(null) num=0\nargument=1 value="data.db"'
		ERROR - $'pzip: --nochecksum: unknown option'

TEST 14 'interface queries'
	usage=$'[-][l:library]:[dll][G!:gzip] [ file ]'
	EXEC	command "$usage" --???api
		OUTPUT - $'return=? option=-? name=--???api num=0'
		ERROR - $'version=1'
		EXIT 2
	EXEC	command "$usage" --???html
		OUTPUT - $'return=? option=-? name=--???html num=0'
	EXEC	command "$usage" --???man
		OUTPUT - $'return=? option=-? name=--???man num=0'
	usage=$'[-p2][l:library]:[dll][G!:gzip] [ file ]'
	EXEC	command "$usage" --???api
		OUTPUT - $'return=? option=-? name=--???api num=0'
		ERROR - $'version=1'
		EXIT 2
	usage=$'[-2p2][l:library]:[dll][G!:gzip] [ file ]'
	EXEC	command "$usage" --???api
		OUTPUT - $'return=? option=-? name=--???api num=0'
		ERROR - $'version=2'
		EXIT 2
	usage=$'l:[dll]G [ file ]'
	EXEC	command "$usage" --???api
		OUTPUT - $'return=? option=-? name=--???api num=0'
		ERROR - $'version=0'
		EXIT 2
	EXEC	command "$usage" --???html
		OUTPUT - $'return=? option=-? name=--???html num=0'
	EXEC	command "$usage" --???man
		OUTPUT - $'return=? option=-? name=--???man num=0'

TEST 15 'required vs. optional arguments'
	usage=$'[-][r:required]:[value][o:optional]:?[value][2:aha]'
	EXEC	cmd "$usage" --req=1 --req -2 --req 3 4 5
		OUTPUT - $'return=r option=-r name=--required arg=1 num=1\nreturn=r option=-r name=--required arg=-2 num=1\nreturn=r option=-r name=--required arg=3 num=1\nargument=1 value="4"\nargument=2 value="5"'
	EXEC	cmd "$usage" -r1 -r -2 -r 3 4 5
		OUTPUT - $'return=r option=-r name=-r arg=1 num=1\nreturn=r option=-r name=-r arg=-2 num=1\nreturn=r option=-r name=-r arg=3 num=1\nargument=1 value="4"\nargument=2 value="5"'
	EXEC	cmd "$usage" --opt=1 --opt -2 --opt 3 4 5
		OUTPUT - $'return=o option=-o name=--optional arg=1 num=1\nreturn=o option=-o name=--optional arg=(null) num=1\nreturn=2 option=-2 name=-2 arg=(null) num=1\nreturn=o option=-o name=--optional arg=(null) num=1\nargument=1 value="3"\nargument=2 value="4"\nargument=3 value="5"'
	EXEC	cmd "$usage" -o1 -o -2 -o 3 4 5
		OUTPUT - $'return=o option=-o name=-o arg=1 num=1\nreturn=o option=-o name=-o arg=(null) num=1\nreturn=2 option=-2 name=-2 arg=(null) num=1\nreturn=o option=-o name=-o arg=3 num=1\nargument=1 value="4"\nargument=2 value="5"'

TEST 16 'detailed man'
	usage=$'[-][+NAME?ah][+DESCRIPTION?\abla\a does bla and bla. The blas are:]{[+\aaha\a?bla bla aha][+\bbwaha?not bold][+bold?yes it is]}[+?Next paragraph]{[+aaa?aaa][+bbb?bbb]}'
	EXEC	cmd "$usage" --html
		OUTPUT - $'return=? option=- name=--html num=0'
		ERROR - $'<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML//EN">
<HTML>
<HEAD>
<META NAME="generator" CONTENT="optget (AT&T Labs Research) 1999-08-11">
<TITLE>cmd man document</TITLE>
</HEAD>
<BODY bgcolor=\'#ffffff\'>
<H4><TABLE WIDTH=100%><TR><TH ALIGN=LEFT>&nbsp;cmd&nbsp;(&nbsp;1&nbsp;)&nbsp;<TH ALIGN=CENTER><A HREF="" TITLE="Index">USER COMMANDS</A><TH ALIGN=RIGHT>cmd&nbsp;(&nbsp;1&nbsp;)</TR></TABLE></H4>
<HR>
<DL COMPACT>
<DT><H4><A NAME="NAME">NAME</A></H4>
<DL COMPACT>
<DT>ah
<P>
</DL>
<DT><H4><A NAME="SYNOPSIS">SYNOPSIS</A></H4>
<DL COMPACT>
<DT><B>cmd</B> &#0091; <I>options</I> &#0093;
</DL>
<DT><H4><A NAME="DESCRIPTION">DESCRIPTION</A></H4>
<DL COMPACT>
<DT><I>bla</I> does bla and bla. The blas are:
<DL COMPACT>
<DL COMPACT>
<DT><A NAME="aha"><I>aha</I></A><DD>bla bla aha
<DT><A NAME="bwaha">bwaha</A><DD>not bold
<DT><A NAME="bold"><B>bold</B></A><DD>yes it is
<P>
</DL>
</DL>
<DT>Next paragraph
<DL COMPACT>
<DL COMPACT>
<DT><A NAME="aaa"><B>aaa</B></A><DD>aaa
<DT><A NAME="bbb"><B>bbb</B></A><DD>bbb
</DL>
</DL>
</DL></DL>
</BODY>
</HTML>'
		EXIT 2

TEST 17 'literal : ? ] makess you ssee double'
	usage=$'[-][a:aha?magic\\sesame]:[yyzzy][x?foo]:[bar]{[+a[b::c[d[,e]]]]??f[::g]]]]?A?B:C[]]][+p \'d\' "q"?duh]}[z:zoom]'
	EXEC	zwei "$usage" -a o
		OUTPUT - $'return=a option=-a name=-a arg=o num=1'
	EXEC	zwei "$usage" -?
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: zwei [-z] [-a yyzzy] [-x bar]'
		EXIT 2
	EXEC	zwei "$usage" --api
		OUTPUT - $'return=? option=- name=--api num=0'
		ERROR - $'.SH SYNOPSIS
zwei [ options ]
.SH OPTIONS
.OP a aha string yyzzy
magic\\sesame
.OP x - string bar
foo
.H3 a[b:c[d[,e]]?f[:g]]
A?B:C[]
.H3 p \'d\' "q"
duh
.OP z zoom flag -
.SH IMPLEMENTATION
.PP'
	EXEC	zwei "$usage" --nroff
		OUTPUT - $'return=? option=- name=--nroff num=0'
		ERROR - $'.\\" format with nroff|troff|groff -man
.nr mI 0
.de H1
.if \\\\n(mI!=0 \\{
.nr mI 0
.RE
.\\}
.TP
\\fB\\\\$1\\fP
..
.de H3
.if \\\\n(mI=0 \\{
.nr mI 1
.RS
.\\}
.TP
\\fB     \\\\$1\\fP
..
.de OP
.if \\\\n(mI!=0 \\{
.nr mI 0
.RE
.\\}
.ie !\'\\\\$1\'-\' \\{
.ds mO \\\\fB\\\\-\\\\$1\\\\fP
.ds mS ,\\\\0
.\\}
.el \\{
.ds mO \\\\&
.ds mS \\\\&
.\\}
.ie \'\\\\$2\'-\' \\{
.if !\'\\\\$4\'-\' .as mO \\\\0\\\\fI\\\\$4\\\\fP
.\\}
.el \\{
.as mO \\\\*(mS\\\\fB\\\\-\\\\-\\\\$2\\\\fP
.if !\'\\\\$4\'-\' .as mO =\\\\fI\\\\$4\\\\fP
.\\}
.TP
\\\\*(mO
..
.de FN
.if \\\\n(mI!=0 \\{
.nr mI 0
.RE
.\\}
.TP
\\\\$1 \\\\$2
..
.TH zwei 1
.SH SYNOPSIS
\\fBzwei\\fP\\ [\\ \\fIoptions\\fP\\ ]
.SH OPTIONS
.OP a aha string yyzzy
magic\\\\sesame
.OP x - string bar
foo
.H3 a[b:c[d[,e]]?f[:g]]
A?B:C[]
.H3 p\\ \'d\'\\ "q"
duh
.OP z zoom flag -
.SH IMPLEMENTATION
.PP'

TEST 18 'more compatibility'
	usage=$'CD:[macro[=value]]EI:?[dir]MPU:[macro]V?A:[assertion]HTX:[dialect]Y:[stdinclude] [input [output]]'
	EXEC	cpp "$usage" -Da=b -Ic -I -Id
		OUTPUT - $'return=D option=-D name=-D arg=a=b num=1\nreturn=I option=-I name=-I arg=c num=1\nreturn=I option=-I name=-I arg=(null) num=1\nreturn=I option=-I name=-I arg=d num=1'
	EXEC	cpp "$usage" -?
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: cpp [-CEMPV] [-D macro[=value]] [-I[dir]] [-U macro] [input [output]]'
		EXIT 2
	usage=$'abc'
	EXEC	typeset "$usage" -
		OUTPUT - $'argument=1 value="-"'
		ERROR -
		EXIT 0
	EXEC	typeset "+$usage" -
	EXEC	typeset "$usage" +
		OUTPUT - $'argument=1 value="+"'
	EXEC	typeset "+$usage" +
	usage=$' file'
	EXEC	none "$usage" -?
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: none [ options ] file'
		EXIT 2
	EXEC	none "$usage" --api
		OUTPUT - $'return=? option=- name=--api num=0'
		ERROR - $'.SH SYNOPSIS\nnone [ options ] file'
	usage=$'[-]\n\nfile'
	EXEC	none "$usage" -?
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: none [ options ] file'
	EXEC	none "$usage" --api
		OUTPUT - $'return=? option=- name=--api num=0'
		ERROR - $'.SH SYNOPSIS\nnone [ options ] file\n.SH IMPLEMENTATION\n.PP'

TEST 19 'mutual exclusion'
	usage=$'[-][\t[a:A][b:B][c:C]\t][d:D]\n\n[ file ... ]'
	EXEC	mutex "$usage" -?
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: mutex [-d] [ -a | -b | -c ] [ file ... ]'
		EXIT 2
	usage=$'[-] [ [a:A][b:B][c:C] ] [d:D]\n\n[ file ... ]'
	EXEC	mutex "$usage" -?
	usage=$'[abc]d [ file ... ]'
	EXEC	mutex "$usage" -?

TEST 20 'flag alternation'
	usage=$'[-][a|A:aha][b|q|z:foo][n|m=10:meth]:[meth][x:bar]\n\nfile ...'
	EXEC	alt "$usage" -a
		OUTPUT - $'return=a option=-a name=-a arg=(null) num=1'
	EXEC	alt "$usage" -A
		OUTPUT - $'return=a option=-A name=-A arg=(null) num=1'
	EXEC	alt "$usage" --aha
		OUTPUT - $'return=a option=-a name=--aha arg=(null) num=1'
	EXEC	alt "$usage" -b
		OUTPUT - $'return=b option=-b name=-b arg=(null) num=1'
	EXEC	alt "$usage" -q
		OUTPUT - $'return=b option=-q name=-q arg=(null) num=1'
	EXEC	alt "$usage" -z
		OUTPUT - $'return=b option=-z name=-z arg=(null) num=1'
	EXEC	alt "$usage" --foo
		OUTPUT - $'return=b option=-b name=--foo arg=(null) num=1'
	EXEC	alt "$usage" -n nnn
		OUTPUT - $'return=-10 option=-10 name=-n arg=nnn num=1'
	EXEC	alt "$usage" -m mmm
		OUTPUT - $'return=-10 option=-10 name=-m arg=mmm num=1'
	EXEC	alt "$usage" --meth=zzz
		OUTPUT - $'return=-10 option=-10 name=--meth arg=zzz num=1'
	EXEC	alt "$usage" -x
		OUTPUT - $'return=x option=-x name=-x arg=(null) num=1'
	EXEC	alt "$usage" --bar
		OUTPUT - $'return=x option=-x name=--bar arg=(null) num=1'
	EXEC	alt "$usage" -?
		OUTPUT - $'return=? option=-? name=-? num=0'
		ERROR - $'Usage: alt [-aAbqzx] [-n|m meth] file ...'
		EXIT 2
	EXEC	alt "$usage" --api
		OUTPUT - $'return=? option=- name=--api num=0'
		ERROR - $'.SH SYNOPSIS
alt [ options ] file ...
.SH OPTIONS
.OP a|A aha flag -
.OP b|q|z foo flag -
.OP n|m meth string meth
.OP x bar flag -
.SH IMPLEMENTATION
.PP'

TEST 21 'justification and emphasis'
	usage=$'[-][w!:warn?Warn about invalid \b--check\b lines.][f:format?hours:minutes:seconds. \aid\a may be followed by \b:case:\b\ap1\a:\as1\a:...:\apn\a:\asn\a which expands to \asi\a if'
	EXEC esc "$usage" --???TEST --man
		OUTPUT - $'return=? option=-? name=--man num=0'
		ERROR - $'\E[1mSYNOPSIS\E[0m
  \E[1mesc\E[0m [ \E[1;4moptions\E[0m ]

\E[1mOPTIONS\E[0m
  -\E[1mw\E[0m, --\E[1mwarn\E[0m      Warn about invalid \E[1m--check\E[0m lines. On by default; -\E[1mw\E[0m means
                  --\E[1mnowarn\E[0m.
  -\E[1mf\E[0m, --\E[1mformat\E[0m    hours:minutes:seconds. \E[1;4mid\E[0m may be followed by
                  \E[1m:case:\E[0m\E[1;4mp1\E[0m:\E[1;4ms1\E[0m:...:\E[1;4mpn\E[0m:\E[1;4msn\E[0m which expands to \E[1;4msi\E[0m if'
		EXIT 2
	EXEC esc "$usage" --keys
		OUTPUT - $'return=? option=- name=--keys num=0'
		ERROR - $'"Warn about invalid \\b--check\\b lines."
"hours:minutes:seconds. \\aid\\a may be followed by \\b:case:\\b\\ap1\\a:\\as1\\a:...:\\apn\\a:\\asn\\a which expands to \\asi\\a if"'

TEST 22 'no with values'
	usage=$'[-][j:jobs?Job concurrency level.]#[level]'
	extra=$'[n!:exec?Execute shell actions.]'
	EXEC	make "$usage" -j1 -j 2 --jobs=3 --jobs 4 --nojobs 5 6
		OUTPUT - $'return=j option=-j name=-j arg=1 num=1
return=j option=-j name=-j arg=2 num=2
return=j option=-j name=--jobs arg=3 num=3
return=j option=-j name=--jobs arg=4 num=4
return=j option=-j name=--jobs arg=(null) num=0
argument=1 value="5"
argument=2 value="6"'
	EXEC	make "$usage$extra" -j1 -j 2 --jobs=3 --jobs 4 --nojobs 5 6
	EXEC	make "$usage" --nojobs=1 2
		OUTPUT - $'return=: option=-j name=--jobs num=0\nargument=1 value="2"'
		ERROR - $'make: --nojobs: value not expected'
		EXIT 1
	EXEC	make "$usage$extra" --nojobs=1 2

TEST 23 'weird help'
	usage=$'[-][j:jobs?Job concurrency level.]#[level]'
	EXEC	test "$usage" --man=-
		OUTPUT - $'return=? option=- name=--man num=0'
		ERROR - ''
		EXIT 2
	EXEC	test "$usage" --man -
		ERROR - $'SYNOPSIS
  test [ options ]

OPTIONS
  -j, --jobs=level
                  Job concurrency level.'
	EXEC	test "$usage" --man --
	EXEC	test "$usage" --man
	usage=$'[-][!:expand?Compress to 32 byte record format.][!:sort?Sort detail records.]'
	EXEC	pzip "$usage" --man
		OUTPUT - $'return=? option=- name=--man num=0'
		ERROR - 'SYNOPSIS
  pzip [ options ]

OPTIONS
  --expand        Compress to 32 byte record format. On by default; use
                  --noexpand to turn off.
  --sort          Sort detail records. On by default; use --nosort to turn off.'
		EXIT 2

TEST 24 'detailed html'
	usage=$'[-][-author?Glenn Fowler <gsf@research.att.com>][-copyright?Copyright (c) 1989-1999 AT&T Corp.][-license?http://www.research.att.com/sw/license/ast-open.html][+NAME?\bdd\b - copy and convert file][10:if?Input file name (see \aintro\a(2)).]:[file\a (see \bstat\b(2))][11:conv?Conversion option \abegin[-end]]=value\a passed to \bmain\b().]:[conversion][+SEE ALSO?\bcp\b(1), \bpax\b(1), \btr\b(1), \bseek\b(2)]'
	EXEC	test "$usage" --html
		EXIT 2
		OUTPUT - $'return=? option=- name=--html num=0'
		ERROR - $'<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML//EN">
<HTML>
<HEAD>
<META NAME="generator" CONTENT="optget (AT&T Labs Research) 1999-08-11">
<TITLE>test man document</TITLE>
</HEAD>
<BODY bgcolor=\'#ffffff\'>
<H4><TABLE WIDTH=100%><TR><TH ALIGN=LEFT>&nbsp;test&nbsp;(&nbsp;1&nbsp;)&nbsp;<TH ALIGN=CENTER><A HREF="" TITLE="Index">USER COMMANDS</A><TH ALIGN=RIGHT>test&nbsp;(&nbsp;1&nbsp;)</TR></TABLE></H4>
<HR>
<DL COMPACT>
<DT><H4><A NAME="NAME">NAME</A></H4>
<DL COMPACT>
<DT><B>dd</B> - copy and convert file
<P>
</DL>
<DT><H4><A NAME="SYNOPSIS">SYNOPSIS</A></H4>
<DL COMPACT>
<DT><B>test</B> &#0091; <I>options</I> &#0093;
<P>
</DL>
<DT><H4><A NAME="OPTIONS">OPTIONS</A></H4>
<DL COMPACT>
<DT>--<B>if</B>=<I>file</I> (see <NOBR><A HREF="../man2/stat.html"><B>stat</B></A>(2))</NOBR>
<DD>Input file name (see <NOBR><A HREF="../man2/intro.html"><I>intro</I></A>(2)).</NOBR>
<DT>--<B>conv</B>=<I>conversion</I><DD>Conversion option <I>begin&#0091;-end&#0093;=value
</I> passed to <B>main</B>().
</DL>
<DT><H4><A NAME="SEE ALSO">SEE ALSO</A></H4>
<DL COMPACT>
<DT><NOBR><A HREF="../man1/cp.html"><B>cp</B></A>(1),</NOBR> <NOBR><A HREF="../man1/pax.html"><B>pax</B></A>(1),</NOBR>
<NOBR><A HREF="../man1/tr.html"><B>tr</B></A>(1),</NOBR> <NOBR><A HREF="../man2/seek.html"><B>seek</B></A>(2)</NOBR>
</DL>
<DT><H4><A NAME="IMPLEMENTATION">IMPLEMENTATION</A></H4>

<DL COMPACT>
<DT><A NAME="author"><B>author</B></A><DD>Glenn Fowler
&lt;gsf@research.att.com&gt;
<DT><A NAME="copyright"><B>copyright</B></A><DD>Copyright &copy; 1989-1999 AT&amp;T Corp.
<DT><A NAME="license"><B>license</B></A><DD><A HREF="http://www.research.att.com/sw/license/ast-open.html">http://www.research.att.com/sw/license/ast-open.html</A>
</DL></DL>
</BODY>
</HTML>'

TEST 25 'extra args after help options'
	usage=$'[-][a:aha][b:foo][n=10:meth]:[meth][x:bar]\n\nfile ...'
	EXEC	extra "$usage" xxx
		OUTPUT - $'argument=1 value="xxx"'
	EXEC	extra "$usage" --short
		OUTPUT - $'return=? option=- name=--short num=0'
		ERROR - $'Usage: extra [-abx] [-n meth] file ...'
		EXIT 2
	EXEC	extra "$usage" --short xxx
	EXEC	extra "$usage" --??short
		OUTPUT - $'return=? option=-? name=--??short num=0'
	EXEC	extra "$usage" --??short xxx

TEST 26 'usage combinations'
	lib1=$'[-1l?library][-author?Dewey Cheatham][+LIBRARY?\b-loser\b - user library][b:bbb?BBB]'
	lib2=$'[-1i?implicit][-author?Andy Howe][+LIBRARY?implicit library][z:zzz?ZZZ]'
	usage=$'[-?main][-author?Bea Taylor][+NAME?\baha\b - bwoohahahahah][a:aha?AHA] [ file ... ]'
	EXEC	+ "$lib1" + "$lib2" combo "$usage" --aha
		OUTPUT - $'return=a option=-a name=--aha arg=(null) num=1'
	EXEC	+ "$lib1" + "$lib2" combo "$usage" --bbb
		EXIT 1
		OUTPUT - $'return=: option= name=--bbb num=0 str=--bbb'
		ERROR - $'combo: --bbb: unknown option'
	EXEC	+ "$lib1" + "$lib2" combo "$usage" --aha --man
		EXIT 2
		OUTPUT - $'return=a option=-a name=--aha arg=(null) num=1\nreturn=? option=-a name=--man num=1'
		ERROR - $'NAME
  aha - bwoohahahahah

SYNOPSIS
  combo [ options ]

OPTIONS
  -a, --aha       AHA

IMPLEMENTATION
  version         main
  author          Bea Taylor

LIBRARY
  -loser - user library

OPTIONS
  --bbb           BBB

IMPLEMENTATION
  version         library
  author          Dewey Cheatham'
	EXEC	combo "$lib1" --bbb --?bbb
		OUTPUT - $'return=b option=-b name=--bbb arg=(null) num=1\nreturn=? option=-? name=--?bbb num=1'
		ERROR - $'Usage: combo [ options ]\nOPTIONS\n  --bbb           BBB'
	EXEC	+ "$lib1" + "$lib2" combo "$usage" --aha --?bbb
		OUTPUT - $'return=a option=-a name=--aha arg=(null) num=1\nreturn=? option=-? name=--?bbb num=1'
	EXEC	+ "$lib1" + "$lib2" combo "$usage" --aha --usage
		OUTPUT - $'return=a option=-a name=--aha arg=(null) num=1\nreturn=? option=-a name=--usage num=1'
		ERROR - $'[-?main][-author?Bea Taylor][+NAME?\\baha\\b - bwoohahahahah][a:aha?AHA] [ file ... ]\n[-1l?library][-author?Dewey Cheatham][+LIBRARY?\\b-loser\\b - user library][b:bbb?BBB]'
	EXEC	+ "$lib1" + "$lib2" combo "$usage" --aha --keys
		OUTPUT - $'return=a option=-a name=--aha arg=(null) num=1\nreturn=? option=-a name=--keys num=1'
		ERROR - $'"\\baha\\b - bwoohahahahah"
"AHA"
"[ file ... ]"
"\\b-loser\\b - user library"
"BBB"'

TEST 27 'opt_info.num with opt_info.arg'
	usage=$'[-][f:flag][m:must]:[yes][o:optional]:?[maybe]'
	EXEC	huh "$usage" --noflag
		OUTPUT - $'return=f option=-f name=--flag arg=(null) num=0'
	EXEC	huh "$usage" --flag
		OUTPUT - $'return=f option=-f name=--flag arg=(null) num=1'
	EXEC	huh "$usage" -f
		OUTPUT - $'return=f option=-f name=-f arg=(null) num=1'
	EXEC	huh "$usage" --nomust
		OUTPUT - $'return=m option=-m name=--must arg=(null) num=0'
	EXEC	huh "$usage" --must=ok
		OUTPUT - $'return=m option=-m name=--must arg=ok num=1'
	EXEC	huh "$usage" --must ok
	EXEC	huh "$usage" -mok
		OUTPUT - $'return=m option=-m name=-m arg=ok num=1'
	EXEC	huh "$usage" -m ok
		OUTPUT - $'return=m option=-m name=-m arg=ok num=1'
	EXEC	huh "$usage" --nooptional
		OUTPUT - $'return=o option=-o name=--optional arg=(null) num=0'
	EXEC	huh "$usage" --optional
		OUTPUT - $'return=o option=-o name=--optional arg=(null) num=1'
	EXEC	huh "$usage" --optional -f
		OUTPUT - $'return=o option=-o name=--optional arg=(null) num=1\nreturn=f option=-f name=-f arg=(null) num=1'
	EXEC	huh "$usage" -o
		OUTPUT - $'return=o option=-o name=-o arg=(null) num=1'
	EXEC	huh "$usage" -o -f
		OUTPUT - $'return=o option=-o name=-o arg=(null) num=1\nreturn=f option=-f name=-f arg=(null) num=1'
	EXEC	huh "$usage" --optional=ok
		OUTPUT - $'return=o option=-o name=--optional arg=ok num=1'
	EXEC	huh "$usage" --optional ok
		OUTPUT - $'return=o option=-o name=--optional arg=(null) num=1\nargument=1 value="ok"'
	EXEC	huh "$usage" -ook
		OUTPUT - $'return=o option=-o name=-o arg=ok num=1'
	EXEC	huh "$usage" -o ok
		OUTPUT - $'return=o option=-o name=-o arg=ok num=1'
	EXEC	huh "$usage" --must
		OUTPUT - $'return=: option=-m name=--must num=0'
		ERROR - $'huh: --must: yes value expected'
		EXIT 1

TEST 28 'user defined optget return value'
	usage=$'[-][n:count]#[number][234:ZZZ]'
	EXEC	num "$usage" -123
		OUTPUT - $'return=n option=-n name=-1 arg=123 num=123'
	EXEC	num "$usage" -234
		OUTPUT - $'return=n option=-n name=-2 arg=234 num=234'

TEST 29 'usage stack'
	usage=$'[-?main][-author?Barney Fife][+NAME?\baha\b - bwoohahahahah][a:aha?AHA.]\foptions\f[q:what?Explain in detail.] [ file ... ]'
	EXEC	info "$usage" --aha --zoom --boom=junk --what
		OUTPUT - $'return=a option=-a name=--aha arg=(null) num=1
return=Z option=-Z name=--zoom arg=(null) num=1
return=B option=-B name=--boom arg=junk num=1
return=q option=-q name=--what arg=(null) num=1'
	EXEC	info "$usage" --man
		EXIT 2
		OUTPUT - $'return=? option=- name=--man num=0'
		ERROR - $'NAME
  aha - bwoohahahahah

SYNOPSIS
  info [ options ]

OPTIONS
  -a, --aha       AHA.
  -Z, --zoom      Do it as fast as possible.
  -C, --cram      Cram as much as possible.
  -K, --kill      kill all processes.
  -F, --fudge     Fudge the statistics to satisfy everyone.
  -D, --dump      Dump as much as possible.
  -B, --boom=file Dump into file.
  -q, --what      Explain in detail.

IMPLEMENTATION
  version         main
  author          Barney Fife'

TEST 30 'library interfaces'
	USAGE_LICENSE="[-author?Glenn Fowler <gsf@research.att.com>][-copyright?Copyright (c) 1995-1999 AT&T Corp.][-license?http://www.research.att.com/sw/license/ast-open.html]"
	usage=$'
[-1s3?@(#)sum (AT&T Labs Research) 1999-12-11]'$USAGE_LICENSE$'
[+NAME?sum - checksum library]
[+DESCRIPTION?\bsum\b is a checksum library.]
[Sum_t*:sumopen(const char* \amethod\a)?Open a sum handle for \amethod\a.]
[int:sumclose(Sum_t* \asum\a)?Close a sum handle \asum\a previously returned
	by \bsumopen\b.]

#include <sum.h>

[+SEE ALSO?\bcksum\b(1)]
'
	EXEC	sum "$usage" --html
		EXIT 2
		OUTPUT - $'return=? option=- name=html num=0'
		ERROR - $'<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML//EN">
<HTML>
<HEAD>
<META NAME="generator" CONTENT="optget (AT&T Labs Research) 1999-08-11">
<TITLE>sum man document</TITLE>
</HEAD>
<BODY bgcolor=\'#ffffff\'>
<H4><TABLE WIDTH=100%><TR><TH ALIGN=LEFT>&nbsp;sum&nbsp;(&nbsp;3&nbsp;)&nbsp;<TH ALIGN=CENTER><A HREF="" TITLE="Index">USER LIBRARY</A><TH ALIGN=RIGHT>sum&nbsp;(&nbsp;3&nbsp;)</TR></TABLE></H4>
<HR>
<DL COMPACT>
<DT><H4><A NAME="NAME">NAME</A></H4>
<DL COMPACT>
<DT>sum - checksum library
<P>
</DL>
<DT><H4><A NAME="SYNOPSIS">SYNOPSIS</A></H4>
<DL COMPACT>
<DT>#include &lt;sum.h&gt;
</DL>
<DT><H4><A NAME="DESCRIPTION">DESCRIPTION</A></H4>
<DL COMPACT>
<DT><B>sum</B> is a checksum library.
<P>
</DL>
<DT><H4><A NAME="FUNCTIONS">FUNCTIONS</A></H4>
<DL COMPACT>
<DT>Sum_t* <B>sumopen</B>(const char* <I>method</I>)<DD>Open a sum handle for
<I>method</I>.
<DT>int <B>sumclose</B>(Sum_t* <I>sum</I>)<DD>Close a sum handle <I>sum</I>
previously returned by <B>sumopen</B>.
</DL>
<DT><H4><A NAME="SEE ALSO">SEE ALSO</A></H4>
<DL COMPACT>
<DT><NOBR><A HREF="../man1/cksum.html"><B>cksum</B></A>(1)</NOBR>
</DL>
<DT><H4><A NAME="IMPLEMENTATION">IMPLEMENTATION</A></H4>
<DL COMPACT>
<DT><A NAME="version"><B>version</B></A><DD>sum (AT&amp;T Labs Research)
1999-12-11
<DT><A NAME="author"><B>author</B></A><DD>Glenn Fowler
&lt;gsf@research.att.com&gt;
<DT><A NAME="copyright"><B>copyright</B></A><DD>Copyright &copy; 1995-1999 AT&amp;T Corp.
<DT><A NAME="license"><B>license</B></A><DD><A HREF="http://www.research.att.com/sw/license/ast-open.html">http://www.research.att.com/sw/license/ast-open.html</A>
</DL></DL>
</BODY>
</HTML>'
	EXEC	sum "$usage" --nroff
		OUTPUT - $'return=? option=- name=nroff num=0'
		ERROR - $'.\\" format with nroff|troff|groff -man
.nr mI 0
.de H1
.if \\\\n(mI!=0 \\{
.nr mI 0
.RE
.\\}
.TP
\\fB\\\\$1\\fP
..
.de H3
.if \\\\n(mI=0 \\{
.nr mI 1
.RS
.\\}
.TP
\\fB     \\\\$1\\fP
..
.de OP
.if \\\\n(mI!=0 \\{
.nr mI 0
.RE
.\\}
.ie !\'\\\\$1\'-\' \\{
.ds mO \\\\fB\\\\-\\\\$1\\\\fP
.ds mS ,\\\\0
.\\}
.el \\{
.ds mO \\\\&
.ds mS \\\\&
.\\}
.ie \'\\\\$2\'-\' \\{
.if !\'\\\\$4\'-\' .as mO \\\\0\\\\fI\\\\$4\\\\fP
.\\}
.el \\{
.as mO \\\\*(mS\\\\fB\\\\-\\\\-\\\\$2\\\\fP
.if !\'\\\\$4\'-\' .as mO =\\\\fI\\\\$4\\\\fP
.\\}
.TP
\\\\*(mO
..
.de FN
.if \\\\n(mI!=0 \\{
.nr mI 0
.RE
.\\}
.TP
\\\\$1 \\\\$2
..
.TH sum 3
.SH NAME
sum - checksum library
.SH SYNOPSIS
#include\\ <sum.h>
.SH DESCRIPTION
\\fBsum\\fP is a checksum library.
.SH FUNCTIONS
.FN Sum_t*\\ \\fBsumopen\\fP(const\\ char*\\ \\fImethod\\fP)
Open a sum handle for \\fImethod\\fP.
.FN int\\ \\fBsumclose\\fP(Sum_t*\\ \\fIsum\\fP)
Close a sum handle \\fIsum\\fP previously returned by \\fBsumopen\\fP.
.SH SEE\\ ALSO
\\fBcksum\\fP(1)
.SH IMPLEMENTATION
.H1 version
sum (AT&T Labs Research) 1999-12-11
.H1 author
Glenn Fowler <gsf@research.att.com>
.H1 copyright
Copyright (c) 1995-1999 AT&T Corp.
.H1 license
http://www.research.att.com/sw/license/ast-open.html'

	EXEC	sum "$usage" --api
		OUTPUT - $'return=? option=- name=api num=0'
		ERROR - $'.SH NAME
sum - checksum library
.SH SYNOPSIS
#include <sum.h>
.SH DESCRIPTION
sum is a checksum library.
.SH FUNCTIONS
.FN Sum_t* sumopen(const char* method)
Open a sum handle for method.
.FN int sumclose(Sum_t* sum)
Close a sum handle sum previously returned by sumopen.
.SH SEE ALSO
cksum(1)
.SH IMPLEMENTATION
.H1 version
sum (AT&T Labs Research) 1999-12-11
.H1 author
Glenn Fowler <gsf@research.att.com>
.H1 copyright
Copyright (c) 1995-1999 AT&T Corp.
.H1 license
http://www.research.att.com/sw/license/ast-open.html'

TEST 31 'off by one -- my epitaph'
	short_usage=$'[-][n]#[s]:\n\npid ...'
	long_usage=$'[-][n:number]#[s:name]:\n\npid ...'
	EXEC	kill "$short_usage" -0 123
		OUTPUT - $'return=n option=-n name=-0 arg=0 num=0\nargument=1 value="123"'
	EXEC	kill "$long_usage" -0 123
	EXEC	kill "$short_usage" -T 123
		OUTPUT - $'return=: option= name=-T num=0 str=[-][n]#[s]:\n\npid ...\nargument=1 value="123"'
		ERROR - $'kill: -T: unknown option'
		EXIT 1
	EXEC	kill "$long_usage" -T 123
		OUTPUT - $'return=: option= name=-T num=0 str=[-][n:number]#[s:name]:\n\npid ...\nargument=1 value="123"'

TEST 32 'miscellaneous'
	usage=$'[-][+NAME?wow - zowee][a|b:aORb?A or B.][y|z:yORz?Y or Z.]\n\n[ file ... ]'
	EXEC	wow "$usage" --nroff
		EXIT 2
		OUTPUT - $'return=? option=- name=--nroff num=0'
		ERROR - $'.\\" format with nroff|troff|groff -man
.nr mI 0
.de H1
.if \\\\n(mI!=0 \\{
.nr mI 0
.RE
.\\}
.TP
\\fB\\\\$1\\fP
..
.de H3
.if \\\\n(mI=0 \\{
.nr mI 1
.RS
.\\}
.TP
\\fB     \\\\$1\\fP
..
.de OP
.if \\\\n(mI!=0 \\{
.nr mI 0
.RE
.\\}
.ie !\'\\\\$1\'-\' \\{
.ds mO \\\\fB\\\\-\\\\$1\\\\fP
.ds mS ,\\\\0
.\\}
.el \\{
.ds mO \\\\&
.ds mS \\\\&
.\\}
.ie \'\\\\$2\'-\' \\{
.if !\'\\\\$4\'-\' .as mO \\\\0\\\\fI\\\\$4\\\\fP
.\\}
.el \\{
.as mO \\\\*(mS\\\\fB\\\\-\\\\-\\\\$2\\\\fP
.if !\'\\\\$4\'-\' .as mO =\\\\fI\\\\$4\\\\fP
.\\}
.TP
\\\\*(mO
..
.de FN
.if \\\\n(mI!=0 \\{
.nr mI 0
.RE
.\\}
.TP
\\\\$1 \\\\$2
..
.TH wow 1
.SH NAME
wow - zowee
.SH SYNOPSIS
\\fBwow\\fP\\ [\\ \\fIoptions\\fP\\ ]\\ [\\ file\\ ...\\ ]
.SH OPTIONS
.OP a|b aORb flag -
A or B.
.OP y|z yORz flag -
Y or Z.
.SH IMPLEMENTATION
.PP'
	usage=$'[-][+NAME?wow - zowee][n:new-test?New test.][o:old-test?Old test.]\n\n[ file ... ]'
	EXEC	wow "$usage" --?new
		EXIT 2
		OUTPUT - $'return=? option=-? name=--?new num=0'
		ERROR - $'Usage: wow [ options ] [ file ... ]
OPTIONS
  -n, --new-test  New test.'
	EXEC	wow "$usage" --?new-test
		OUTPUT - $'return=? option=-? name=--?new-test num=0'
	EXEC	wow "$usage" --?newtest
		OUTPUT - $'return=? option=-? name=--?newtest num=0'
