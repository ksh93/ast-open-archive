# regression tests for the cut utility
# from the gnu textutils

TEST 01 '-d -f'
	EXEC	-d: -f1,3- f1
		INPUT f1 $'a:b:c'
		OUTPUT - $'a:c'
	EXEC	-d: -f1,3- f1
	EXEC	-d: -f2- f1
		OUTPUT - $'b:c'
	EXEC	-d: -f4 f1
		OUTPUT - $''
	EXEC	-d: -f4 f1
		INPUT f1
		OUTPUT -

TEST 02 '-c'
	EXEC	-c4 f1
		INPUT f1
	EXEC	-c4 f1
		INPUT f1 $'123'
		OUTPUT - $''
	EXEC	-c4 f1
	EXEC	-c4 f1
		INPUT f1 $'123\n1'
		OUTPUT - $'\n'

TEST 03	'-d -f -s'
	EXEC	-s -d: -f2,3 f1
		INPUT f1 $'abc'
	EXEC	-s -d: -f3- f1
		INPUT f1 $'a:b:c'
		OUTPUT - $'c'
	EXEC	-s -d: -f3-4 f1
	EXEC	-s -d: -f3,4 f1
	EXEC	-s -d: -f2,3 f1
		OUTPUT - $'b:c'
	EXEC	-s -d: -f1,3 f1
		OUTPUT - $'a:c'

TEST 04 '-d -f'
	EXEC	-d: -f1-3 f1
		INPUT f1 $':::'
		OUTPUT - $'::'
	EXEC	-d: -f1-4 f1
		INPUT f1 $':::'
		OUTPUT - $':::'
	EXEC	-d: -f2-3 f1
		OUTPUT - $':'
	EXEC	-d: -f2-4 f1
		OUTPUT - $'::'

TEST 05 '-d -f -s'
	EXEC	-s -d: -f1-3 f1
		INPUT f1 $':::'
		OUTPUT - $'::'
	EXEC	-s -d: -f1-4 f1
		INPUT f1 $':::'
		OUTPUT - $':::'
	EXEC	-s -d: -f2-3 f1
		INPUT f1 $':::'
		OUTPUT - $':'
	EXEC	-s -d: -f2-4 f1
		INPUT f1 $':::'
		OUTPUT - $'::'
	EXEC	-s -d: -f2-4 f1
		INPUT f1 $':::\n:'
		OUTPUT - $'::\n'
	EXEC	-s -d: -f2-4 f1
		INPUT f1 $':::\n:1'
		OUTPUT - $'::\n1'
	EXEC	-s -d: -f1-4 f1
		INPUT f1 $':::\n:a'
		OUTPUT - $':::\n:a'
	EXEC	-s -d: -f3- f1
		INPUT f1 $':::\n:1'
		OUTPUT - $':\n'

TEST 07 'no fields'
	EXEC	-f3- f1
		INPUT f1
	EXEC	-s -f3- f1
	EXEC	-b 1 f1

TEST 08 'two empty fields'
	EXEC	-s -d: -f2-4 f1
		INPUT f1 $':'
		OUTPUT - $''

TEST 09 'fixed length records'
	EXEC	 -r5 -b2-4 f1
		INPUT -n f1 $'abcdefghij'
		OUTPUT - $'bcd\nghi'
	EXEC	 -N -r5 -b2-4 f1
		OUTPUT -n - $'bcdghi'

TEST 10 'misc errors'
	DIAGNOSTICS
	EXEC
		EXIT [12]
	EXEC	-b
	EXEC	-f
	EXEC	f1
		INPUT f1 $':'
	EXEC	-b '' f1
	EXEC	-f '' f1
	EXEC	-s -b4 f1
