# ast nmake edit operator tests

INCLUDE test.def

TEST 01 ':H: sort'

	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H)'
		OUTPUT - $'a a b c'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=)'
		OUTPUT - $'a a b c'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H<)'
		OUTPUT - $'a a b c'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H>)'
		OUTPUT - $'c b a a'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H!)'
		OUTPUT - $'a b c'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H!=)'
		OUTPUT - $'a b c'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=F)'
		OUTPUT - $'a'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=I)'
		OUTPUT - $'c b a a'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=N)'
		OUTPUT - $'a a b c'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=U)'
		OUTPUT - $'a b c'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=V)'
		OUTPUT - $'a a b c'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=NF)'
		OUTPUT - $'a'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=NI)'
		OUTPUT - $'c b a a'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=NU)'
		OUTPUT - $'a b c'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=NV)'
		OUTPUT - $'a a b c'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=VF)'
		OUTPUT - $'a'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=VI)'
		OUTPUT - $'c b a a'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=VU)'
		OUTPUT - $'a b c'

	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H)'
		OUTPUT - $'02 1 10 2'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=)'
		OUTPUT - $'1 02 2 10'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H<)'
		OUTPUT - $'02 1 10 2'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H>)'
		OUTPUT - $'2 10 1 02'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H!)'
		OUTPUT - $'02 1 10 2'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H!=)'
		OUTPUT - $'1 02 10'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=F)'
		OUTPUT - $'02'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=I)'
		OUTPUT - $'2 10 1 02'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=N)'
		OUTPUT - $'1 02 2 10'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=U)'
		OUTPUT - $'02 1 10 2'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=V)'
		OUTPUT - $'10 2 02 1'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=NF)'
		OUTPUT - $'1'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=NI)'
		OUTPUT - $'10 2 02 1'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=NU)'
		OUTPUT - $'1 02 10'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=NV)'
		OUTPUT - $'10 2 02 1'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=VF)'
		OUTPUT - $'10'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=VI)'
		OUTPUT - $'1 02 2 10'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=VU)'
		OUTPUT - $'10 2 1'

	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H)'
		OUTPUT - $'a-01.2.3 a-02.3.4 a-1.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=)'
		OUTPUT - $'a-01.2.3 a-02.3.4 a-1.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H<)'
		OUTPUT - $'a-01.2.3 a-02.3.4 a-1.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H>)'
		OUTPUT - $'a-1.2.3 a-02.3.4 a-01.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H!)'
		OUTPUT - $'a-01.2.3 a-02.3.4 a-1.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=F)'
		OUTPUT - $'a-01.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=I)'
		OUTPUT - $'a-1.2.3 a-02.3.4 a-01.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=N)'
		OUTPUT - $'a-01.2.3 a-02.3.4 a-1.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=U)'
		OUTPUT - $'a-01.2.3 a-02.3.4 a-1.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=V)'
		OUTPUT - $'a-02.3.4 a-1.2.3 a-01.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=NF)'
		OUTPUT - $'a-01.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=NI)'
		OUTPUT - $'a-1.2.3 a-02.3.4 a-01.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=NU)'
		OUTPUT - $'a-01.2.3 a-02.3.4 a-1.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=NV)'
		OUTPUT - $'a-02.3.4 a-1.2.3 a-01.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=VF)'
		OUTPUT - $'a-02.3.4'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=VI)'
		OUTPUT - $'a-1.2.3 a-01.2.3 a-02.3.4'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=VU)'
		OUTPUT - $'a-02.3.4 a-1.2.3'

TEST 02 ':I: intersection'

	EXEC	-n -f - . 'A = c 2 b 10 a 02 1 a' 'print -- $(A:I)'
		OUTPUT - $''
	EXEC	-n -f - . 'A = c 2 b 10 a 02 1 a' 'print -- $(A:I<)'
		OUTPUT - $''

TEST 03 ':L: glob list'

	EXEC	-n -f - . 'print -- $("L.dir":L)'
		INPUT L.dir/001
		INPUT L.dir/002
		INPUT L.dir/003
		INPUT L.dir/004
		OUTPUT - $'001 002 003 004'
	EXEC	-n -f - . 'print -- $("L.dir":L>)'
		OUTPUT - $'004'
	EXEC	-n -f - . 'print -- $("L.dir":L<)'
		OUTPUT - $'001'
	EXEC	-n -f - . 'print -- $("L.dir":L=)'
		OUTPUT - $'001 002 003 004'
	EXEC	-n -f - . 'print -- $("L.dir":L!)'
		OUTPUT - $'L.dir/001 L.dir/002 L.dir/003 L.dir/004'

TEST 04 ':N: file match'

	EXEC	-n -f - . 'A = c 2 b 10 a 02 1 a' 'print -- $(A:N)'
		OUTPUT - $''

TEST 05 ':O: ordinal'

	EXEC	-n -f - . 'A = c 2 b 10 a 02 1 a' 'print -- $(A:O)'
		OUTPUT - $'8'
	EXEC	-n -f - . 'A = c 2 b 10 a 02 1 a' 'print -- $(A:O=)'
		OUTPUT - $'8'

TEST 06 ':Q: quoting'

	EXEC	-n -f - . 'A = "a z"' 'print -- $(A:Q)'
		OUTPUT - $'\\""a z"\\"'
	EXEC	-n -f - . "A = 'a z'" 'print -- $(A:Q)'
		OUTPUT - $'"\'a z\'"'
	EXEC	-n -f - . 'A = a z' 'print -- $(A:Q)'
		OUTPUT - $'a z'
	EXEC	-n -f - . 'A = a\z' 'print -- $(A:Q)'
		OUTPUT - $'\'a\\z\''
	EXEC	-n -f - . 'A = a$z' 'print -- $(A:Q)'
		OUTPUT - $'\'a\$z\''
	EXEC	-n -f - . 'A = "a z"' 'print -- $(A:@Q)'
		OUTPUT - $'\\""a z"\\"'
	EXEC	-n -f - . "A = 'a z'" 'print -- $(A:@Q)'
		OUTPUT - $'"\'a z\'"'
	EXEC	-n -f - . 'A = a z' 'print -- $(A:@Q)'
		OUTPUT - $'\'a z\''
	EXEC	-n -f - . 'A = a\z' 'print -- $(A:@Q)'
		OUTPUT - $'\'a\\z\''
	EXEC	-n -f - . 'A = a$z' 'print -- $(A:@Q)'
		OUTPUT - $'\'a\$z\''

TEST 07 ':T=D: quoting'

	EXEC	-n -f - . 'A == "a z"' 'print -- $("(A)":T=D)'
		OUTPUT - -DA=$'\\""a z"\\"'
	EXEC	-n -f - . "A == 'a z'" 'print -- $("(A)":T=D)'
		OUTPUT - -DA=$'"\'a z\'"'
	EXEC	-n -f - . 'A == a z' 'print -- $("(A)":T=D)'
		OUTPUT - -DA=$'\'a z\''
	EXEC	-n -f - . 'A == a\z' 'print -- $("(A)":T=D)'
		OUTPUT - -DA=$'\'a\\z\''
	EXEC	-n -f - . 'A == a$z' 'print -- $("(A)":T=D)'
		OUTPUT - -DA=$'\'a\$z\''

TEST 08 ':T=E: quoting'

	EXEC	-n -f - . 'A == "a z"' 'print -- $("(A)":T=E)'
		OUTPUT - A=$'"a z"'
	EXEC	-n -f - . "A == 'a z'" 'print -- $("(A)":T=E)'
		OUTPUT - A=$'\'a z\''
	EXEC	-n -f - . 'A == a z' 'print -- $("(A)":T=E)'
	EXEC	-n -f - . 'A == a\z' 'print -- $("(A)":T=E)'
		OUTPUT - A=$'\'a\\z\''
	EXEC	-n -f - . 'A == a$z' 'print -- $("(A)":T=E)'
		OUTPUT - A=$'\'a\$z\''

TEST 09 ':C:'

	EXEC	-n
		INPUT Makefile $'A = ../../hdr
t :
	: $(X)
	: $(Y)
	: $(Z)
X : .FUNCTION
	return $(A:/^/-I)
Y : .FUNCTION
	.Y : $(A:/^/-I)
	return $(~.Y)
Z : .FUNCTION
	.Z : $(A:/^/-I)
	return $(*.Z)'
		OUTPUT - $'+ : -I../../hdr
+ : -I../../hdr
+ : -I../../hdr'

TEST 10 ':F:'

	EXEC -n
		INPUT Makefile $'LOWERCASE = GO LOWER CASE
UPPERCASE = go upper case
FILENAME = $$*/usr/people/login
COUNT = 23
STRING = "this is a string"
all :
	: obsolete lower :$(LOWERCASE:F=L):
	: lower :$(LOWERCASE:F=%(lower)s):
	: obsolete upper :$(UPPERCASE:F=U):
	: upper :$(UPPERCASE:F=%(upper)s):
	: string :$(STRING:F=%s):
	: hex :$(COUNT:F=%x):
	: dec :$(COUNT:F=%10.5d):
	: oct :$(COUNT:F=%o):
	: right justify :$("var1 var2":F=%20.10s):
	: left justify :$("var1 var2":F=%-20.10s):
	: obsolete variable :$(FILENAME:F=V):
	: variable :$(FILENAME:F=%(variable)s):'
		OUTPUT - $'+ : obsolete lower :go lower case:
+ : lower :go lower case:
+ : obsolete upper :GO UPPER CASE:
+ : upper :GO UPPER CASE:
+ : string :"this is a string":
+ : hex :17:
+ : dec :     00023:
+ : oct :27:
+ : right justify :                var1                 var2:
+ : left justify :var1                var2                :
+ : obsolete variable :....usr.people.login:
+ : variable :....usr.people.login:'

TEST 11 'empty op values'

	EXEC	-n
		INPUT Makefile $'all : .MAKE
	A = a b c
	print $(A:A)
	print $(A:I)'
		OUTPUT - $'
'

TEST 12 ':P=G:'

	EXPORT VPATH=$TWD/dev:$TWD/ofc

	CD ofc

	EXEC	-n
		INPUT Makefile $'all : .MAKE
	X = *.c
	print :P=G: $(X:P=G)
	print :P=G:T=F: $(X:P=G:T=F)'
		INPUT ofc.c
		INPUT dup.c
		OUTPUT - $':P=G: dup.c ofc.c
:P=G:T=F: dup.c ofc.c'

	CD ../dev

	EXEC	-n
		INPUT dev.c
		INPUT dup.c
		OUTPUT - $':P=G: dev.c dup.c ofc.c
:P=G:T=F: dev.c dup.c '$TWD$'/ofc/ofc.c'

TEST 13 ':P=S:'

	EXPORT VPATH=$TWD/dev:$TWD/ofc

	CD ofc

	EXEC	-ns
		INPUT Makefile $'FILES = a.h ../a.h
all : $(FILES)
../a.h : a.h
	cp $(*) $(<)
.DONE : .done
.done : .MAKE
	print :P=S: $(FILES:T=F:P=S)
	print :P!=S: $(FILES:T=F:P!=S)'
		INPUT a.h
		INPUT $TWD/a.h
		OUTPUT - $':P=S: a.h
:P!=S: ../a.h'

	CD ../dev

	EXEC	-ns
		OUTPUT - $':P=S: '$TWD$'/ofc/a.h
:P!=S: ../a.h'

TEST 14 'rebind variants'

	EXEC
		INPUT Makefile $'set nowriteobject nowritestate
all : tst
	rm -f notfound $(*.SOURCE:L=e*)
tst : .VIRTUAL .FORCE
	touch exists'
		ERROR - $'+ touch exists\n+ rm -f notfound'

	EXEC
		ERROR - $'+ touch exists\n+ rm -f notfound exists'

	EXEC
		INPUT Makefile $'set nowriteobject nowritestate
all : tst
	rm -f notfound $("rebind $(*.SOURCE)":R)$(*.SOURCE:L=e*)
tst : .VIRTUAL .FORCE
	touch exists'
		ERROR - $'+ touch exists\n+ rm -f notfound exists'

	EXEC

	EXEC
		INPUT Makefile $'set nowriteobject nowritestate
all : tst
	rm -f notfound $(*.SOURCE:T=B:L=e*)
tst : .VIRTUAL .FORCE
	touch exists'
		ERROR - $'+ touch exists\n+ rm -f notfound exists'

	EXEC

	EXEC
		INPUT Makefile $'set nowriteobject nowritestate
all : tst
	rm -f notfound $(*.SOURCE:L^=e*)
tst : .VIRTUAL .FORCE
	touch exists'
		ERROR - $'+ touch exists\n+ rm -f notfound exists'

	EXEC
