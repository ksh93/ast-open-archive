# ast nmake state tests

INCLUDE cc.def

TEST 01 'state views'

	EXEC	--nojobs --regress one code=2
		INPUT Makefile $'all : one two
one : .ALWAYS .VIRTUAL .FORCE
	cd $(<)
	MAKEPATH=.. ../$(PWD:P=R=$(MAKE)) $(-) -f main.mk CODE==$(code)
two : .ALWAYS .VIRTUAL .FORCE
	$(PWD:P=R=$(MAKE)) $(-) -f main.mk CODE==$(code)'
		INPUT cc.probe
		INPUT one/
		INPUT main.mk $'%.o : %.c
	: $(CCFLAGS) > $(<)
.COMMAND.o :
	: > $(<)
app :: a.c b.c c.c'
		INPUT a.c $'extern int b(); main() { return b() + CODE; }'
		INPUT b.c $'extern int c(); int b() { return c() + CODE; }'
		INPUT c.c $'int c() { return CODE; }'
		ERROR - $'+ cd one
+ ../../nmake --nojobs --regress -f main.mk CODE==2
+ MAKEPATH=..
+ : -DCODE=2
+ 1> a.o
+ : -DCODE=2
+ 1> b.o
+ : -DCODE=2
+ 1> c.o
+ :
+ 1> app'

	EXEC	--nojobs --regress one code=2
		ERROR - $'+ cd one
+ ../../nmake --nojobs --regress -f main.mk CODE==2
+ MAKEPATH=..'

	EXEC	--nojobs --regress one code=2

	EXEC	--nojobs --regress two code=0
		ERROR - $'+ ../nmake --nojobs --regress -f main.mk CODE==0
+ : -DCODE=0
+ 1> a.o
+ : -DCODE=0
+ 1> b.o
+ : -DCODE=0
+ 1> c.o
+ :
+ 1> app'

	EXEC	--nojobs --regress two code=0
		INPUT two/
		ERROR - $'+ ../nmake --nojobs --regress -f main.mk CODE==0'

	EXEC	--nojobs --regress one code=2
		ERROR - $'+ cd one
+ ../../nmake --nojobs --regress -f main.mk CODE==2
+ MAKEPATH=..
+ : -DCODE=2
+ 1> a.o
+ : -DCODE=2
+ 1> b.o
+ : -DCODE=2
+ 1> c.o
+ :
+ 1> app'

TEST 02 'state actions'

	EXEC
		INPUT Makefile $'all : (XX) (YY)
XX = xx
(XX) : Makefile
	: XX is $(;)
YY = yy
(YY) :
	: YY is $(;)'
		ERROR - $'+ : XX is xx
+ : YY is yy'

	EXEC	--regress
		ERROR -

	EXEC	--regress XX=aa
		ERROR - $'+ : XX is aa'

	EXEC	--regress
		ERROR - $'+ : XX is xx'

	EXEC	--regress YY=zz
		ERROR - $'+ : YY is zz'

	EXEC	--regress
		ERROR - $'+ : YY is yy'

TEST 03 'error status'

	EXEC	--regress -NFMstatic
		INPUT Makefile $'a : b y z
	echo aha $(*)
b :
	true > $(<)
y :
	false > $(<)
z :
	true > $(<)'
		OUTPUT - $'info mam static 00000
setv INSTALLROOT ${INSTALLROOT}
setv PACKAGEROOT ${PACKAGEROOT}
setv AR ar
setv ARFLAGS cr
setv AS as
setv ASFLAGS
setv CC cc
setv mam_cc_FLAGS
setv CCFLAGS ${debug?1?${mam_cc_DEBUG} -D_BLD_DEBUG?${mam_cc_OPTIMIZE}?}
setv CCLDFLAGS  ${strip?1?${mam_cc_LD_STRIP}??}
setv COTEMP $$
setv CPIO cpio
setv CPIOFLAGS
setv CPP "${CC} -E"
setv F77 f77
setv HOSTCC ${CC}
setv IGNORE
setv LD ld
setv LDFLAGS
setv LEX lex
setv LEXFLAGS
setv LPR lpr
setv LPRFLAGS
setv M4FLAGS
setv NMAKE nmake
setv NMAKEFLAGS
setv PR pr
setv PRFLAGS
setv SHELL /bin/sh
setv SILENT
setv TAR tar
setv YACC yacc
setv YACCFLAGS -d
make a
make b
exec - true > b
done b virtual
make y
exec - false > y
done y virtual
make z
exec - true > z
done z virtual
exec - echo aha b y z
done a virtual'

	EXEC	--regress
		OUTPUT -
		ERROR - $'+ true
+ 1> b
+ false
+ 1> y
make: *** exit code 1 making y'
		EXIT 1

	EXEC	--regress
		ERROR - $'+ false
+ 1> y
make: *** exit code 1 making y'
