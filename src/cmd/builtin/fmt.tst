# regression tests for the fmt utilitiy

TEST 01 'basics'
	EXEC
		INPUT - $'aaa\n\nzzz'
		OUTPUT - $'aaa\n\nzzz'
	EXEC
		INPUT - $'aaa \n\nzzz'
	EXEC
		INPUT - $'aaa\n\nzzz '
	EXEC
		INPUT - $'aaa\n\n zzz'
		OUTPUT - $'aaa\n\n zzz'
