# tests for the xargs utility

TEST 01 'basics'
	EXEC echo test
		INPUT -
		OUTPUT - $'test'
	EXEC echo test
		INPUT - $'first\nlast'
		OUTPUT - $'test first last'

TEST 02 'arg replacement'
	EXEC -i echo test {}
		INPUT - $'first\nlast'
		OUTPUT - $'test first\ntest last'
	EXEC -i{} echo test {}
	EXEC -iARG echo test ARG
	EXEC -i echo { {} }
		OUTPUT - $'{ first }\n{ last }'
	EXEC -i{} echo { {} }
	EXEC -iARG echo { ARG }
	EXEC -i echo {{}}
		OUTPUT - $'{first}\n{last}'
	EXEC -i echo A{}Z
		OUTPUT - $'AfirstZ\nAlastZ'
	EXEC -IARG echo AARGZ
		OUTPUT - $'AfirstZ\nAlastZ'
	EXEC -I ARG echo AARGZ
		OUTPUT - $'AfirstZ\nAlastZ'
	EXEC -i echo test {}
		INPUT - $'aaa\tbbb\nyyy zzz'
		OUTPUT - $'test aaa\tbbb\ntest yyy zzz'

TEST 03 'arg limit'
	EXEC -n2 echo
		INPUT - $'1\n2\n3\n4'
		OUTPUT - $'1 2\n3 4'
	EXEC -n 2 echo
	EXEC -n2 echo
		INPUT - $'1\n2\n3\n4\n5'
		OUTPUT - $'1 2\n3 4\n5'

TEST 04 'size limit'
	unset FOO
	min=$($COMMAND -s1 echo </dev/null 2>&1 | sed 's/[^0-9]//g')
	per=$(( $(FOO=BAR $COMMAND -s1 echo </dev/null 2>&1 | sed 's/[^0-9]//g') - min - 8 ))
	if	(( per == 4 ))
	then	dec=(0 6 10 18)
	else	dec=(0 7 21 29)
	fi
	min=$((min+4*(per+2)))
	EXEC -s$((min-${dec[0]})) echo
		INPUT - $'1\n2\n3\n4'
		OUTPUT - $'1 2 3 4'
	EXEC -s$((min-${dec[1]})) echo
		INPUT - $'1\n2\n3\n4'
		OUTPUT - $'1 2 3\n4'
	EXEC -s$((min-${dec[2]})) echo
		INPUT - $'1\n2\n3\n4'
		OUTPUT - $'1 2\n3 4'
	EXEC -s$((min-${dec[3]})) echo
		INPUT - $'1\n2\n3\n4'
		OUTPUT - $'1\n2\n3\n4'

TEST 05 'extensions'
	EXEC -z echo test
		INPUT -
		OUTPUT -
