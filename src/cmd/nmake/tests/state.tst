# ast nmake state tests

INCLUDE cc.def

TEST 01 'state views'

	EXEC	--nojobs one code=2
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
+ ../../nmake --nojobs -f main.mk CODE==2
+ MAKEPATH=..
+ : -DCODE=2
+ 1> a.o
+ : -DCODE=2
+ 1> b.o
+ : -DCODE=2
+ 1> c.o
+ :
+ 1> app'

	EXEC	--nojobs one code=2
		ERROR - $'+ cd one
+ ../../nmake --nojobs -f main.mk CODE==2
+ MAKEPATH=..'

	EXEC	--nojobs one code=2

	EXEC	--nojobs two code=0
		ERROR - $'+ ../nmake --nojobs -f main.mk CODE==0
+ : -DCODE=0
+ 1> a.o
+ : -DCODE=0
+ 1> b.o
+ : -DCODE=0
+ 1> c.o
+ :
+ 1> app'

	EXEC	--nojobs two code=0
		INPUT two/
		ERROR - $'+ ../nmake --nojobs -f main.mk CODE==0'

	EXEC	--nojobs one code=2
		ERROR - $'+ cd one
+ ../../nmake --nojobs -f main.mk CODE==2
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

	EXEC	--
		ERROR -

	EXEC	XX=aa
		ERROR - $'+ : XX is aa'

	EXEC	--
		ERROR - $'+ : XX is xx'

	EXEC	YY=zz
		ERROR - $'+ : YY is zz'

	EXEC	--
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
setv CCFLAGS ${-debug-symbols?1?${mam_cc_DEBUG} -D_BLD_DEBUG?${mam_cc_OPTIMIZE}?}
setv CCLDFLAGS  ${-strip-symbols?1?${mam_cc_LD_STRIP}??}
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

	EXEC	--
		OUTPUT -
		ERROR - $'+ true
+ 1> b
+ false
+ 1> y
make: *** exit code 1 making y'
		EXIT 1

	EXEC
		ERROR - $'+ false
+ 1> y
make: *** exit code 1 making y'

TEST 04 'cancelled error status'

	EXEC
		INPUT Makefile $'%.z : %.x (DISABLE.%)
	$(DISABLE.$(%):?false?touch $(<)?)
z : a.z b.z c.z d.z
	touch $(<)'
		INPUT a.x
		INPUT b.x
		INPUT c.x
		INPUT d.x
		ERROR - $'+ touch a.z
+ touch b.z
+ touch c.z
+ touch d.z
+ touch z'
	
	EXEC	DISABLE.a=1 DISABLE.b=1 DISABLE.c=1 DISABLE.d=1 -k
		EXIT 1
		ERROR - $'+ false
make: *** exit code 1 making a.z
+ false
make: *** exit code 1 making b.z
+ false
make: *** exit code 1 making c.z
+ false
make: *** exit code 1 making d.z
make: *** 4 actions failed'
	
	EXEC	DISABLE.a=1 DISABLE.b=1 DISABLE.c=1 DISABLE.d=1
		ERROR - $'+ false
make: *** exit code 1 making a.z'
	
	EXEC	DISABLE.a=1 DISABLE.b=1 DISABLE.c=1 DISABLE.d=1 -k
		ERROR - $'+ false
make: *** exit code 1 making a.z
+ false
make: *** exit code 1 making b.z
+ false
make: *** exit code 1 making c.z
+ false
make: *** exit code 1 making d.z
make: *** 4 actions failed'
	
	EXEC	DISABLE.a=  DISABLE.b=  DISABLE.c=1 DISABLE.d=1
		ERROR - $'+ touch a.z
+ touch b.z
+ false
make: *** exit code 1 making c.z'
	
	EXEC	DISABLE.a=  DISABLE.b=  DISABLE.c=1 DISABLE.d=1
		ERROR - $'+ false
make: *** exit code 1 making c.z'
	
	EXEC	DISABLE.a=  DISABLE.b=  DISABLE.c=  DISABLE.d=1
		ERROR - $'+ touch c.z
+ false
make: *** exit code 1 making d.z'

	EXEC	DISABLE.a=  DISABLE.b=  DISABLE.c=  DISABLE.d=1
		ERROR - $'+ false
make: *** exit code 1 making d.z'
	
	EXEC	DISABLE.a=  DISABLE.b=  DISABLE.c=  DISABLE.d=
		EXIT 0
		ERROR - $'+ touch d.z
+ touch z'
	
	EXEC	DISABLE.a=  DISABLE.b=  DISABLE.c=  DISABLE.d=
		ERROR -
