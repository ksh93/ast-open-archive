This directory contains AT&T test harnesses and data for the X/Open pattern
matching routines:

	HARNESS			HEADER		FUNCTIONS
	-------			------		---------
	testfnmatch.c		<fnmatch.h>	fnmatch()
	testglob.c		<glob.h>	glob()
	testmatch.c		<ast.h>		ksh93 strmatch(),strgrpmatch()
	testre.c		<regex.h>	regcomp(),regexec()

Each test*.c file is a main program driven by these data files:

	testfnmatch,testmatch,testre	testre.dat,testmatch.dat
	testglob			testglob.dat

For example, to build the fnmatch test harness and run the tests:

	cc -o testfnmatch testfnmatch.c
	testfnmatch < testre.dat
	testfnmatch < testmatch.dat

If the local implementation hangs or dumps on some tests then run with
the -c option.

There are two input formats, described in testglob.dat and testre.dat.
The input files exercise all features; the harnesses detect and ignore
features not supported by the local implementation.

strmatch() is the ksh93 augmented pattern matcher; testmatch.c will only
compile with the AT&T <ast.h> and -last.

Extensions to the standard terminology the derived from the AT&T RE
implementation, unified under <regex.h> with these modes:

	MODE	FLAGS			
	----	-----
	BRE	0			basic RE
	ERE	REG_EXTENDED		egrep RE
	ARE	REG_AUGMENTED		ERE with ! negation, <> word boundaries
	SRE	REG_SHELL		sh patterns
	KRE	REG_SHELL|REG_AUGMENTED	ksh93 patterns: ! @ ( | & ) { }

and some additional flags to handle fnmatch():

	REG_SHELL_ESCAPED	FNM_NOESCAPE
	REG_SHELL_PATH		FNM_PATHNAME
	REG_SHELL_DOT		FNM_PERIOD

The original testre.c was done by Doug McIlroy at Bell Labs.
The current implementation is maintained by

	Glenn Fowler <gsf@research.att.com>

I'd like to make the regression tests as comprehensive as possible.
Send any new tests to me and I'll roll them into the open source
distribution at http://www.research.att.com/sw/download/ with proper
attribution.

Please note that the regression tests nail down unspecified standard behavior.
These should be noted in the test data with 'u' but currently are not.
Experience with other implementations will help clean this up.
