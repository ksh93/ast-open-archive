# regression tests for the expr utility

TEST 01 'string op output'
	EXEC	fred : '.*'
		OUTPUT - 4
	EXEC	fred/fred : '.*/\(.*\)'
		OUTPUT - fred
	EXEC	fred : '.*/\(.*\)'
		OUTPUT -
		EXIT 1

TEST 02 'numeric output'
	EXEC	10
		OUTPUT - 10
	EXEC	010 + 0
	EXEC	1 + 9
	EXEC	11 - 1
	EXEC	010
		OUTPUT - 010
	EXEC	+010
		OUTPUT - +010
	EXEC	1+2
		OUTPUT - 1+2
	EXEC	1 + 2
		OUTPUT - 3
