# regression tests for the paste utilitiy

TEST 01 'columnize'
	EXEC	- - -
		INPUT - $'1\n2\n3\n4\n'
		OUTPUT - $'1\t2\t3\n4\t\t'
	EXEC	-d: - - -
		OUTPUT - $'1:2:3\n4::'
