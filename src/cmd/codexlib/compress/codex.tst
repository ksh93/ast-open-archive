# regression tests for the codex(1) compress method

TITLE + compress

KEEP '*.dat'

TEST 01 'basics'
	EXEC	'>compress'
		INPUT - 'aaaa bbbb cccc aaaa cccc bbbb zzzz aaaa'
		MOVE OUTPUT c.dat
	EXEC	-i
		SAME INPUT c.dat
		OUTPUT - compress
	EXEC	'<compress'
		OUTPUT - 'aaaa bbbb cccc aaaa cccc bbbb zzzz aaaa'
	EXEC	-d
