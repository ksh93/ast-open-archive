# ast nmake assignment tests

INCLUDE test.def

TEST 01 'delayed eval'

	EXEC	-n
		INPUT Makefile $'VAR1 = 1
VAR2 = $(VAR1)
VAR1 = 2
tst:
	: VAR1 : $(VAR1) : VAR2 : $(VAR2) :'
		OUTPUT - $'+ : VAR1 : 2 : VAR2 : 2 :'

	EXEC	-n VAR1=5
		OUTPUT - $'+ : VAR1 : 5 : VAR2 : 5 :'

	EXEC	-n VAR1+=5
		OUTPUT - $'+ : VAR1 : 2 5 : VAR2 : 2 5 :'

	EXEC	-n VAR2=5
		OUTPUT - $'+ : VAR1 : 2 : VAR2 : 5 :'

	EXEC	-n VAR2+=5
		OUTPUT - $'+ : VAR1 : 2 : VAR2 : 2 5 :'

TEST 02 'immediate eval'

	EXEC	-n
		INPUT Makefile $'VAR1 = 1
VAR2 := $(VAR1)
VAR1 = 2
tst:
	: VAR1 : $(VAR1) : VAR2 : $(VAR2) :'
		OUTPUT - $'+ : VAR1 : 2 : VAR2 : 1 :'

	EXEC	-n VAR1=5
		OUTPUT - $'+ : VAR1 : 5 : VAR2 : 5 :'

	EXEC	-n VAR1+=5
		OUTPUT - $'+ : VAR1 : 2 5 : VAR2 : 1 5 :'

	EXEC	-n VAR2=5
		OUTPUT - $'+ : VAR1 : 2 : VAR2 : 5 :'

	EXEC	-n VAR2+=5
		OUTPUT - $'+ : VAR1 : 2 : VAR2 : 1 5 :'

TEST 03 'append'

	EXEC	-n
		INPUT Makefile $'VAR1 = 1
VAR2 += $(VAR1)
VAR2 += 2
VAR1 += 3
tst:
	: VAR1 : $(VAR1) : VAR2 : $(VAR2) :'
		OUTPUT - $'+ : VAR1 : 1 3 : VAR2 : 1 2 :'

	EXEC	-n VAR1=5
		OUTPUT - $'+ : VAR1 : 5 : VAR2 : 5 2 :'

	EXEC	-n VAR1+=5
		OUTPUT - $'+ : VAR1 : 1 3 5 : VAR2 : 1 5 2 :'

	EXEC	-n VAR2=5
		OUTPUT - $'+ : VAR1 : 1 3 : VAR2 : 5 :'

	EXEC	-n VAR2+=5
		OUTPUT - $'+ : VAR1 : 1 3 : VAR2 : 1 2 5 :'

TEST 04 'auxilliary'

	EXEC	-n
		INPUT Makefile $'VAR1 = 1
VAR2 = 2
VAR1 &= $(VAR2)
tst:
	: VAR1 : $(VAR1) : VAR2 : $(VAR2) :'
		OUTPUT - $'+ : VAR1 : 1 2 : VAR2 : 2 :'

	EXEC	-n VAR1=5
		OUTPUT - $'+ : VAR1 : 5 2 : VAR2 : 2 :'

	EXEC	-n VAR1+=5
		OUTPUT - $'+ : VAR1 : 1 5 2 : VAR2 : 2 :'

	EXEC	-n VAR2=5
		OUTPUT - $'+ : VAR1 : 1 5 : VAR2 : 5 :'

	EXEC	-n VAR2+=5
		OUTPUT - $'+ : VAR1 : 1 2 5 : VAR2 : 2 5 :'

TEST 05 'delayed eval combinations'

	EXEC	-n
		INPUT src/file.t $'t'
		INPUT Makefile $'FILE = file.t
.SOURCE$(FILE:S) : src
.INIT : .MAKE
	if "$(DELAY)"
	LIB = $(FILE:T=F)
	else
	LIB := $(FILE:T=F)
	end
	DIR = $(LIB:D)
	error 0 LIB is $(LIB)
	error 0 LIB dir is $(LIB:D)
$(FILE) : .FORCE
	: $(LIB) $(LIB:T=F) $(<)
	: $(LIB:D) $(LIB:T=F:D) $(<:D) $(DIR)'
		OUTPUT - $'+ : src/file.t src/file.t file.t
+ : src src . src'
		ERROR - $'LIB is src/file.t
LIB dir is src'

	EXEC	-n DELAY=1
		OUTPUT - $'+ : file.t file.t file.t
+ : . . . .'

TEST 06 'defined in .mo'

	EXEC	--silent WHO=there
		INPUT Makefile $'WHO = world
all :
	echo hello $(WHO)'
		OUTPUT - $'hello there'

	EXEC	--silent WHO=there

	EXEC	--silent
		OUTPUT - $'hello world'

	EXEC	--silent

TEST 07 'frozen in .mo'

	EXEC	--silent WHO=there
		INPUT Makefile $'WHO = world
who := $(WHO)
all :
	echo hello $(who)'
		OUTPUT - $'hello there'

	EXEC	--silent WHO=there

	EXEC	--silent
		OUTPUT - $'hello world'

	EXEC	--silent
