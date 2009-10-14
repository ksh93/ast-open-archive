# tests for the dss tst query

TITLE + tst

TEST 01 'basics'

	EXEC -x text:'%(number)d' '{tst::even}'
		INPUT - $'1\n2\n3\n4\n5\n6\n7\n8\n9'
		OUTPUT - $'even_beg
even_sel 2
even_act 2
even_sel 4
even_act 4
even_sel 6
even_act 6
even_sel 8
even_act 8
even_end 9 4 4'

	EXEC -x text:'%(number)d' -ltst '{even}'

	EXEC -x text:'%(number)d' '{tst::odd}'
		OUTPUT - $'odd_beg
odd_sel 1
odd_act 1
odd_sel 3
odd_act 3
odd_sel 5
odd_act 5
odd_sel 7
odd_act 7
odd_sel 9
odd_act 9
odd_end 9 5 5'

	EXEC -x text:'%(number)d' '{tst::even};{tst::odd}'
		OUTPUT - $'even_beg
odd_beg
odd_sel 1
odd_act 1
even_sel 2
even_act 2
odd_sel 3
odd_act 3
even_sel 4
even_act 4
odd_sel 5
odd_act 5
even_sel 6
even_act 6
odd_sel 7
odd_act 7
even_sel 8
even_act 8
odd_sel 9
odd_act 9
even_end 9 4 4
odd_end 9 5 5'

	EXEC -x text:'%(number)d' -ltst '{tst::even};{tst::odd}'

	EXEC -x text:'%(number)d' -ltst '{tst::even};{odd}'

	EXEC -x text:'%(number)d' -ltst '{even};{tst::odd}'

	EXEC -x text:'%(number)d' -ltst '{even};{odd}'

	EXEC -x text:'%(number)d' '{tst::odd};{tst::even}'
		OUTPUT - $'odd_beg
even_beg
odd_sel 1
odd_act 1
even_sel 2
even_act 2
odd_sel 3
odd_act 3
even_sel 4
even_act 4
odd_sel 5
odd_act 5
even_sel 6
even_act 6
odd_sel 7
odd_act 7
even_sel 8
even_act 8
odd_sel 9
odd_act 9
odd_end 9 5 5
even_end 9 4 4'

	EXEC -x text:'%(number)d' '{tst::even}|{tst::odd}'
		OUTPUT - $'odd_beg
even_beg
even_sel 2
even_act 2
odd_sel 1
odd_act 1
even_sel 4
even_act 4
even_sel 6
even_act 6
odd_sel 3
odd_act 3
even_sel 8
even_act 8
even_end 9 4 4
odd_end 4 2 2'

	EXEC -x text:'%(number)d' '{tst::odd}|{tst::even}'
		OUTPUT - $'even_beg
odd_beg
odd_sel 1
odd_act 1
odd_sel 3
odd_act 3
even_sel 2
even_act 2
odd_sel 5
odd_act 5
odd_sel 7
odd_act 7
even_sel 4
even_act 4
odd_sel 9
odd_act 9
odd_end 9 5 5
even_end 5 2 2'
