# ast nmake attribute tests

INCLUDE cc.def

TEST 01 '.ACCEPT'

	EXEC	--silent
		INPUT Makefile $'all : target1 target2 target3
target1 : prereq1 
	echo making $(<)
	: > $(<)
target2 : prereq2 .VIRTUAL
	echo making $(<)
target3 : .VIRTUAL
	echo making $(<)
prereq1 : prereq3 .ACCEPT
	echo making $(<)
	: > $(<)
prereq2 :
	echo making $(<)
	: > $(<)
prereq3 :
	echo making $(<)
	: > $(<)'
		OUTPUT - $'making prereq3
making prereq1
making target1
making prereq2
making target2
making target3'

	EXEC	--silent
		OUTPUT -

	DO sleep 1
	DO touch prereq1 prereq2

	EXEC	--silent
		OUTPUT - $'making target1
making prereq2
making target2'

	DO sleep 1
	DO touch prereq3

	EXEC	--silent
		OUTPUT - $'making prereq3
making prereq1
making target1'

	EXEC	--silent
		OUTPUT -

TEST 02 '.ACCEPT'

	EXEC	--silent
		INPUT Makefile $'prog : prog.o
	echo "link $(*) > $(<)"
	echo link $(*) > $(<)
prog.o : .ACCEPT'
		INPUT prog.o '<!obj>'
		OUTPUT - $'link prog.o > prog'

	EXEC	--silent
		OUTPUT -

	DO	sleep 1
	DO	touch prog.c

	EXEC	--silent

TEST 03 '.ACCEPT'

	EXEC	--silent -f a.mk
		INPUT a.mk $'SFILES = main.c x.c y.c z.c
objs : $(SFILES:B:S=.o)
	: $(*)
	cat <<-EOD > $(<)
	$(*)
	EOD'
		INPUT main.o '<!obj>main'
		INPUT x.o '<!obj>x'
		INPUT y.o '<!obj>y'
		INPUT z.o '<!obj>z'
		OUTPUT objs 'main.o x.o y.o z.o'

	EXEC	--silent -f b.mk
		INPUT b.mk $'OFILES :COMMAND:
	cat objs
.MAIN : xeq
$(OFILES) : .ACCEPT
xeq : $(OFILES)
	echo "link $(*) > $(<)"
	echo link $(*) > $(<)'
		OUTPUT - $'link main.o x.o y.o z.o > xeq'

	EXEC	--silent -f a.mk
		OUTPUT -

	EXEC	--silent -f b.mk

TEST 04 '.PARAMETER'

	EXEC	-n
		INPUT param.G $'#define aaa 1
#define zzz 2'
		INPUT Makefile $'.ATTRIBUTE.%.G : .TERMINAL
.MAKEINIT : .init
.init : .MAKE .VIRTUAL .FORCE
	param.G : .SCAN.c .PARAMETER .SPECIAL
all : .MAKE
	local v
	print PARAMETER : $(...:A=.PARAMETER:A=.REGULAR)
	for v $(...:A=.STATEVAR:A=.SCAN:P=L:H)
		print $(v) = $($(v))
	end'
		OUTPUT - $'PARAMETER : param.G
(aaa) = 1
(zzz) = 2'

TEST 05 '.DONTCARE'

	EXEC	-n RET=1
		INPUT Makefile $'all : yes.bb no.xx
no.xx : no.aa
	cp $(*) $(<)
%.bb : %.aa
	cp $(>) $(<)
if "$(RET)"
.DONTCARE.%.aa : .FUNCTION
	return $(RET)
end'
		INPUT yes.aa
		OUTPUT - $'+ cp yes.aa yes.bb
+ cp no.aa no.xx'

	EXEC	-n
		OUTPUT - $'+ cp yes.aa yes.bb'
		ERROR - $'make: don\'t know how to make all : no.xx : no.aa'
		EXIT 1

TEST 06 'change => clash'

	EXEC	old=1
		INPUT Makefile $'.ATTR.aaa : .ATTRIBUTE
if old
.ATTR.xxx : .ATTRIBUTE
.SCAN.xxx : .SCAN
	OM
end
.ATTR.yyy : .ATTRIBUTE
.SCAN.yyy : .SCAN
	OM
.ATTR.zzz : .ATTRIBUTE
.SCAN.zzz : .SCAN
	OM
all : x y z
	: $(*)
x : .ATTR.aaa
if old
x : .ATTR.xxx .SCAN.xxx
end
y : .ATTR.aaa .ATTR.yyy .SCAN.yyy
z : .ATTR.aaa .ATTR.zzz .SCAN.zzz'
		INPUT x
		INPUT y
		INPUT z
		ERROR - $'+ : x y z'

	EXEC	-n old=0
		OUTPUT - $'+ : x y z'
		ERROR - $'make: warning: Makefile.mo: frozen command argument variable old changed
make: warning: Makefile.mo: recompiling
make: warning: Makefile.ms: .ATTR.xxx .ATTRIBUTE definition clashes with .ATTR.yyy
make: warning: Makefile.ms: .ATTR.yyy .ATTRIBUTE definition changed
make: warning: Makefile.ms: .ATTR.zzz .ATTRIBUTE definition changed
make: warning: Makefile.ms: .SCAN.xxx .SCAN definition clashes with .SCAN.yyy
make: warning: Makefile.ms: .SCAN.yyy .SCAN definition changed
make: warning: Makefile.ms: .SCAN.zzz .SCAN definition changed'

	EXEC	old=0
		OUTPUT -
		ERROR - $'+ : x y z'

	EXEC	-n old=1
		OUTPUT - $'+ : x y z'
		ERROR - $'make: warning: Makefile.mo: frozen command argument variable old changed
make: warning: Makefile.mo: recompiling
make: warning: Makefile.ms: .ATTR.yyy .ATTRIBUTE definition changed
make: warning: Makefile.ms: .ATTR.zzz .ATTRIBUTE definition changed
make: warning: Makefile.ms: .SCAN.yyy .SCAN definition changed
make: warning: Makefile.ms: .SCAN.zzz .SCAN definition changed'

	EXEC	old=1
		OUTPUT -
		ERROR - $'+ : x y z'

TEST 07 '.READ'

	EXEC	--silent
		INPUT Makefile $'all : .READ
	echo result = the result is $( echo M* )
.INIT .DONE :
	echo $(<) : $(result)'
		OUTPUT - $'.INIT :
.DONE : the result is Makefile Makefile.ml Makefile.mo'

TEST 08 '.VIRTUAL'

	EXEC
		INPUT Makefile $'all : huh/yes huh/no
huh/yes : .VIRTUAL yes.src
	cp $(>) $(<)
huh/no : .VIRTUAL no.src
	: no override necessary'
		INPUT no.src
		INPUT yes.src
		INPUT huh/
		ERROR - $'+ cp yes.src huh/yes
+ : no override necessary'

	EXEC
		ERROR -

TEST 09 '.VIRTUAL state'

	EXEC
		INPUT Makefile $'.MAIN : .FOO
.FOO : .VIRTUAL a b
	echo AHA $(>)
a  b :
	echo $(<) > $(<)'
		OUTPUT - $'AHA a b'
		ERROR - $'+ echo a
+ 1> a
+ echo b
+ 1> b
+ echo AHA a b'

	EXEC
		OUTPUT -
		ERROR -

	EXEC

TEST 10 '.PARAMETER file'

	EXEC
		INPUT Makefile $'h :: m.c h.c
h.h : .PARAMETER'
		INPUT m.c $'#include "h.h"
extern int h();
int main()
{
	return h();
}'
		INPUT h.c $'#include "h.h"
#include <stdio.h>
int h()
{
#if GERMAN
	printf("Guten Tag\\n");
#elif SPANISH
	printf("Buenos Dias\\n");
#else
	printf("hey\\n");
#endif
	return 0;
}'
		INPUT h.h $'#define ENGLISH 1'
		ERROR - $'+ cc -O -I. -c m.c
+ cc -O -I. -c h.c
+ cc -O -o h m.o h.o'

	EXEC	--regress
		INPUT h.h $'#define YODA 1'
		ERROR -

	EXEC	--regress
		INPUT h.h $'#define GERMAN 1'
		ERROR - $'+ cc -O -I. -c h.c
+ cc -O -o h m.o h.o'

TEST 11 'attribute propagation'

	EXEC	--regress -n
		INPUT Makefile $'all : foo.bar
.SRC : .ATTRIBUTE
.SOURCE.%.SRC : src
foo : .SRC
%.bar : src/%
	cat $(>) > $(<)
.DONE : .show
.show : .MAKE
	query .SRC foo src/foo'
		INPUT src/foo stuff
		OUTPUT - $'+ cat src/foo > foo.bar'
		ERROR - $'
.SRC : [not found] attribute=0x00000020 index unbound

src/foo==foo : [recent] .SRC alias regular 

src/foo : [current] .SRC must=1 regular EXISTS

()src/foo : [recent] .SRC event=[current] force compiled state

src/foo : [current] .SRC must=1 regular EXISTS

()src/foo : [recent] .SRC event=[current] force compiled state
'

TEST 12 'target status'

	EXEC	--silent
		INPUT Makefile $'all : foo
foo : bar
bar :
	false
.DONE : .report
.report : .MAKE
	print FAILED : $(...:A=.FAILED:H)'
		OUTPUT - $'FAILED : all bar foo'
		ERROR - $'make: *** exit code 1 making bar'
		EXIT 1

TEST 13 '.MEMBER'

	EXEC	--regress
		INPUT Makefile $'x :: main.c -ltst
tst :LIBRARY: a.c b.c
.DONE : .done
.done : .MAKE .VIRTUAL .FORCE .REPEAT
	print : $(...:A=.MEMBER:H) :'
		INPUT a.c $'int a(){return 0;}'
		INPUT b.c $'int b(){return 0;}'
		INPUT main.c $'int main(){return 0;}'
		OUTPUT - $':  :'
		ERROR - $'+ cc -O -c main.c
+ echo \'\' -ltst
+ 1> tst.req
+ cc -O -c a.c
+ cc -O -c b.c
+ ar cr libtst.a a.o b.o
+ ignore ranlib libtst.a
+ rm -f a.o b.o
+ cc -O -o x main.o libtst.a'

	EXEC	--regress
		OUTPUT - $': a.o b.o :'
		ERROR -

TEST 14 '.PARAMETER + state + views'

	DO	{ mkdir v0 v1 v2 || FATAL cannot initialize views ;}

	CD	v2

	EXEC	--regress
		INPUT Makefile $'.ATTRIBUTE.%.G : .TERMINAL
.MAKEINIT : .init
.init : .MAKE .VIRTUAL .FORCE
	param.G : .SCAN.c .PARAMETER .SPECIAL
all : .MAKE .VIRTUAL .FORCE
	local v
	print PARAMETER : $(...:A=.PARAMETER:A=.REGULAR:H)
	for v $(...:A=.STATEVAR:A=.SCAN:P=L:H)
		print $(v) = $($(v))
	end'
		INPUT param.G $'#define aaa 1
#define zzz 2'
		OUTPUT - $'PARAMETER : param.G
(aaa) = 1
(zzz) = 2'

	EXEC

	CD	../v0

	EXPORT	VPATH=$TWD/v0:$TWD/v1:$TWD/v2

	EXEC	--regress
		INPUT param.G $'#define aaa NEW
#define zzz 2'
		OUTPUT - $'PARAMETER : param.G
(aaa) = NEW
(zzz) = 2'

	EXEC
