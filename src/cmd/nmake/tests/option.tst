# ast nmake option tests

INCLUDE test.def

TEST 01 'option basics'

	EXEC	-n -f - . 'print -- $(--:H:/ /$("\n")/G)'
		OUTPUT - $'--alias
--file=-
--intermediate
--jobs=1
--noaccept
--nobase
--nobelieve
--nobyname
--nocompatibility
--nocompile
--nocorrupt
--nocross
--nodebug
--nodefine
--noerrorid
--noexec
--noexpandview
--noexplain
--noforce
--noglobal
--noignore
--noignorelock
--noimport
--noinclude
--nokeepgoing
--nolist
--nomam
--nonever
--nooption
--nooverride
--nopreprocess
--noquestionable
--noreadonly
--noregress
--noreread
--noruledump
--nosilent
--nostrictview
--notargetcontext
--notest
--notolerance
--notouch
--noundef
--novardump
--nowarn
--readstate=32
--scan
--writeobject=-
--writestate=-'

	EXEC	-n -f - -A --writeobject=test.mo --nowritestate . 'print -- $(--:H:/ /$("\n")/G)'
		OUTPUT - $'--accept
--alias
--file=-
--intermediate
--jobs=1
--nobase
--nobelieve
--nobyname
--nocompatibility
--nocompile
--nocorrupt
--nocross
--nodebug
--nodefine
--noerrorid
--noexec
--noexpandview
--noexplain
--noforce
--noglobal
--noignore
--noignorelock
--noimport
--noinclude
--nokeepgoing
--nolist
--nomam
--nonever
--nooption
--nooverride
--nopreprocess
--noquestionable
--noreadonly
--noregress
--noreread
--noruledump
--nosilent
--nostrictview
--notargetcontext
--notest
--notolerance
--notouch
--noundef
--novardump
--nowarn
--nowritestate
--readstate=32
--scan
--writeobject=test.mo'

	EXEC	-n -f - . 'print -- $(--force)'
		OUTPUT - $''

	EXEC	-n -f - +F . 'print -- $(--force)'

	EXEC	-n -f - --noforce . 'print -- $(--force)'

	EXEC	-n -f - --force=0 . 'print -- $(--force)'

	EXEC	-n -f - --force . 'print -- $(--force)'
		OUTPUT - $'1'

	EXEC	-n -f - --force=1 . 'print -- $(--force)'

	EXEC	-n -f - -F . 'print -- $(--force)'

	EXEC	-n -f - . 'print -- $(++force)'
		OUTPUT - $'--noforce'

	EXEC	-n -f - +F . 'print -- $(++force)'

	EXEC	-n -f - --noforce . 'print -- $(++force)'

	EXEC	-n -f - --force=0 . 'print -- $(++force)'

	EXEC	-n -f - -o noforce . 'print -- $(++force)'

	EXEC	-n -f - -o force=0 . 'print -- $(++force)'

	EXEC	-n -f - -o +F . 'print -- $(++force)'

	EXEC	-n -f - --force . 'print -- $(++force)'
		OUTPUT - $'--force'

	EXEC	-n -f - --force=1 . 'print -- $(++force)'

	EXEC	-n -f - -F . 'print -- $(++force)'

	EXEC	-n -f - -o force . 'print -- $(++force)'

	EXEC	-n -f - -o force=1 . 'print -- $(++force)'

	EXEC	-n -f - -o -F . 'print -- $(++force)'

TEST 02 'undefined options'

	EXEC	--silent -f - --foobar
		ERROR - $'make: foobar: unknown option'
		EXIT 1

	EXEC	--silent -f - -o --foobar

	EXEC	--silent -f - -o foobar

	EXEC	--silent -f - ++foobar
		ERROR - $'make: ++foobar: section not found'

	EXEC	--silent -f - -Y
		ERROR - $'make: -Y: unknown option'

	EXEC	--silent -f - -o -Y

	EXEC	--silent -f - +Y
		ERROR - $'make: +Y: unknown option'

	EXEC	--silent -f - -o +Y

TEST 03 'user defined options'

	EXEC	--silent
		INPUT Makefile $'.set.unconditional : .FUNCTION
	set $(%:N=1:??no?)force
set option=u+unconditional+b+.set.unconditional+\'Equivalent to \b--force\b.\'
set unconditional
all : .MAKE
	print force=$(-force) unconditional=$(-unconditional) -F=$(-F) -u=$(-u) $(-)'
		OUTPUT - $'force=1 unconditional=1 -F=1 -u=1 option=u+unconditional+b+.set.unconditional+\'Equivalent to \b--force\b.\' --unconditional --silent --force'

	EXEC --silent

	EXEC --silent clobber
		OUTPUT -

	EXEC --silent -u
		OUTPUT - $'force=1 unconditional=1 -F=1 -u=1 option=u+unconditional+b+.set.unconditional+\'Equivalent to \b--force\b.\' --unconditional --silent --force'

	EXEC --silent -u

	EXEC --silent

	EXEC --silent +u
		OUTPUT - $'force= unconditional= -F= -u= option=u+unconditional+b+.set.unconditional+\'Equivalent to \b--force\b.\' --nounconditional --silent --noforce'

	EXEC --silent
		OUTPUT - $'force=1 unconditional=1 -F=1 -u=1 option=u+unconditional+b+.set.unconditional+\'Equivalent to \b--force\b.\' --unconditional --silent --force'

	EXEC --silent clobber
		OUTPUT -

	EXEC --silent +u
		OUTPUT - $'force= unconditional= -F= -u= option=u+unconditional+b+.set.unconditional+\'Equivalent to \b--force\b.\' --nounconditional --silent --noforce'

	EXEC --silent +u

	EXEC --silent

	EXEC --silent -u
		OUTPUT - $'force=1 unconditional=1 -F=1 -u=1 option=u+unconditional+b+.set.unconditional+\'Equivalent to \b--force\b.\' --unconditional --silent --force'

	EXEC --silent
		OUTPUT - $'force= unconditional= -F= -u= option=u+unconditional+b+.set.unconditional+\'Equivalent to \b--force\b.\' --nounconditional --silent --noforce'

	EXEC --silent clobber
		OUTPUT -

	EXEC --silent -o unconditional
		OUTPUT - $'force=1 unconditional=1 -F=1 -u=1 option=u+unconditional+b+.set.unconditional+\'Equivalent to \b--force\b.\' --unconditional --silent --force'

	EXEC --silent

	EXEC --silent -o nounconditional
		OUTPUT - $'force= unconditional= -F= -u= option=u+unconditional+b+.set.unconditional+\'Equivalent to \b--force\b.\' --nounconditional --silent --noforce'

	EXEC --silent
		OUTPUT - $'force=1 unconditional=1 -F=1 -u=1 option=u+unconditional+b+.set.unconditional+\'Equivalent to \b--force\b.\' --unconditional --silent --force'

	EXEC --silent clobber
		OUTPUT -

	EXEC --silent --unconditional
		OUTPUT - $'force=1 unconditional=1 -F=1 -u=1 option=u+unconditional+b+.set.unconditional+\'Equivalent to \b--force\b.\' --unconditional --silent --force'

	EXEC --silent

	EXEC --silent --nounconditional
		OUTPUT - $'force= unconditional= -F= -u= option=u+unconditional+b+.set.unconditional+\'Equivalent to \b--force\b.\' --nounconditional --silent --noforce'

	EXEC --silent
		OUTPUT - $'force=1 unconditional=1 -F=1 -u=1 option=u+unconditional+b+.set.unconditional+\'Equivalent to \b--force\b.\' --unconditional --silent --force'

TEST 04 'command line definition'

	EXEC	-n --option=Z+Ztest+b++"Z test." --Ztest
		INPUT Makefile $'tst :
	: $(-) : $(-Z) : $(-Ztest) :'
		OUTPUT - $'+ : --noexec option=Z+Ztest+b++\'Z test.\' --Ztest : 1 : 1 :'

	EXEC	-n --option=Z+Ztest+b++"Z test." -o Ztest

	EXEC	-n --option=Z+Ztest+b++"Z test." -o --Ztest

	EXEC	-n --option=Z+Ztest+b++"Z test." -o -Ztest

	EXEC	-n --option=Z+Ztest+b++"Z test." -Z

	EXEC	-n --option=Z+Ztest+b++"Z test." -o -Z

	EXEC	-n --option=Z+Ztest+b++"Z test." -o Z

	EXEC	-n --option=Z+Ztest+b++"Z test." --noZtest
		OUTPUT - $'+ : --noexec option=Z+Ztest+b++\'Z test.\' --noZtest :  :  :'

	EXEC	-n --option=Z+Ztest+b++"Z test." -o noZtest

	EXEC	-n --option=Z+Ztest+b++"Z test." -o --noZtest

	EXEC	-n --option=Z+Ztest+b++"Z test." -o -noZtest
		OUTPUT - $'+ : --noexec :  :  :'

	EXEC	-n --option=Z+Ztest+b++"Z test." +Z
		OUTPUT - $'+ : --noexec option=Z+Ztest+b++\'Z test.\' --noZtest :  :  :'

	EXEC	-n --option=Z+Ztest+b++"Z test." -o +Z

	EXEC	-n --option=Z+Ztest+b++"Z test." -o Z
		OUTPUT - $'+ : --noexec option=Z+Ztest+b++\'Z test.\' --Ztest : 1 : 1 :'

TEST 05 '.mo interaction'

	EXEC	-n
		INPUT Makefile $'all : status
status : .MAKE .VIRTUAL .FORCE .REPEAT
	print : $(-) : keepgoing=$(-keepgoing)'
		OUTPUT - $': --noexec : keepgoing='

	EXEC	-n -k
		OUTPUT - $': --noexec --keepgoing : keepgoing=1'

	EXEC	-k
		OUTPUT - $': --keepgoing : keepgoing=1'

	EXEC	--
		OUTPUT - $':  : keepgoing='

TEST 06 'jobs'

	EXEC	--silent --jobs=4
		INPUT Makefile $'SLEEP = sleep
LONG =
test : a b c d
	echo $(<) done
a :
	$(SLEEP) 4$(LONG)
	echo $(<) done
b :
	$(SLEEP) 3$(LONG)
	echo $(<) done
c :
	$(SLEEP) 2$(LONG)
	echo $(<) done
d :
	$(SLEEP) 1$(LONG)
	echo $(<) done'
		OUTPUT - $'d done
c done
b done
a done
test done'

TEST 07 'old jobs deadlock' && {

	mkdir -p root && cd root || FATAL cannot cd root

	EXEC	--jobs=4
		INPUT Makefile $'
a : d
	: making $(<)
	sleep 5
	echo $(<) : *
	touch $(<)
b : c a
	: making $(<)
	sleep 1
	echo $(<) : *
	touch $(<)
c : c.x
	: making $(<)
	sleep 1
	echo $(<) : *
	touch $(<)
d : b d.x
	: making $(<)
	sleep 5
	echo $(<) : *
	touch $(<)'
		INPUT c.x
		INPUT d.x
		OUTPUT - $'c : Makefile Makefile.ml Makefile.mo c.x d.x
a : Makefile Makefile.ml Makefile.mo c c.x d.x
b : Makefile Makefile.ml Makefile.mo a c c.x d.x
d : Makefile Makefile.ml Makefile.mo a b c c.x d.x'
		ERROR - $'+ : making c
+ sleep 1
+ echo c : Makefile Makefile.ml Makefile.mo c.x d.x
+ touch c
+ : making a
+ sleep 5
+ echo a : Makefile Makefile.ml Makefile.mo c c.x d.x
+ touch a
+ : making b
+ sleep 1
+ echo b : Makefile Makefile.ml Makefile.mo a c c.x d.x
+ touch b
+ : making d
+ sleep 5
+ echo d : Makefile Makefile.ml Makefile.mo a b c c.x d.x
+ touch d'

}

TEST 08 'concurrency vs. errors'

	EXEC	--jobs=1 --silent
		INPUT Makefile $'all : 1 2 3 4
1 3 :
	sleep $(<); echo $(<) >&2; false
2 4 :
	sleep $(<); echo $(<) >&2'
		ERROR - $'1\nmake: *** exit code 1 making 1'
		EXIT 1

	EXEC	--jobs=2 --silent
		ERROR - $'1\nmake: *** exit code 1 making 1\n2'

	EXEC	--jobs=3 --silent
		ERROR - $'1\nmake: *** exit code 1 making 1\n2\n3\nmake: *** exit code 1 making 3'

	EXEC	--jobs=4 --silent
		ERROR - $'1\nmake: *** exit code 1 making 1\n2\n3\nmake: *** exit code 1 making 3\n4'

TEST 09 '--writeobject and --writestate with readonly pwd'

	DO	{ mkdir ro wr || FATAL cannot initialize directories ;}

	CD	ro

	EXEC	--silent
		INPUT Makefile $'all :\n\tls ../wr'
		OUTPUT -

	DO	chmod ugo-w .

	EXEC	--silent --nowriteobject --nowritestate

	EXEC	--silent --writeobject=../wr --nowritestate
		OUTPUT - $'Makefile.mo'

	EXEC	--silent --writeobject=../wr --writestate=../wr
		OUTPUT - $'Makefile.ml\nMakefile.mo'

	EXEC	--silent --writeobject=../wr/Testfile.mo --nowritestate
		OUTPUT - $'Makefile.mo\nMakefile.ms\nTestfile.mo'

	EXEC	--silent --writeobject=../wr/Testfile.mo --writestate=../wr/Testfile.ms
		OUTPUT - $'Makefile.mo\nMakefile.ms\nTestfile.ml\nTestfile.mo'

	EXEC	--silent --writeobject=../wr/Testfile.mo --writestate=../wr/Testfile.ms
		OUTPUT - $'Makefile.mo\nMakefile.ms\nTestfile.ml\nTestfile.mo\nTestfile.ms'
