# regression tests for the sort utility
# generated from the gnu perl tests

TEST 01 'simple checks'
	EXEC	-c f1
		INPUT f1 $'A\nB\nC'
	EXEC	f1
		OUTPUT - $'A\nB\nC'
	EXEC	-cu f1
		INPUT f1 $'A\nB'
		OUTPUT - 

TEST 02 'two fields'
	EXEC	-k1.1,1.0 f1
		INPUT f1
	EXEC	-k1.1,1 f1
	EXEC	-k1,1 f1
	EXEC	-k1 f1
		INPUT f1 $'B\nA'
		OUTPUT - $'A\nB'
	EXEC	-k1,1 f1
	EXEC	-k1 -k2 f1
		INPUT f1 $'A b\nA a'
		OUTPUT - $'A a\nA b'

TEST 03 'numeric fields'
	EXEC	-nc f1
		INPUT f1 $'2\n11'
	EXEC	-n f1
		INPUT f1 $'11\n2'
		OUTPUT - $'2\n11'
	EXEC	-k1n f1
	EXEC	-k1 f1
		OUTPUT - $'11\n2'
	EXEC	-k2 f1
		INPUT f1 $'ignored B\nz-ig A'
		OUTPUT - $'z-ig A\nignored B'

TEST 04 'two fields from three, option spacing'
	EXEC	-k1,2 f1
		INPUT f1 $'A B\nA A'
		OUTPUT - $'A A\nA B'
	EXEC	-k 1,2 f1
	EXEC	-k1,2 f1
		INPUT f1 $'A B A\nA A Z'
		OUTPUT - $'A A Z\nA B A'
	EXEC	-k 1,2 f1
	EXEC	-k1 -k2 f1
	EXEC	-k 1 -k 2 f1
	EXEC	-k2,2 f1
	EXEC	-k 2,2 f1
	EXEC	-k2,2 f1
		INPUT f1 $'A B Z\nA A A'
		OUTPUT - $'A A A\nA B Z'
	EXEC	-k 2,2 f1
	EXEC	-k2,2 f1
		INPUT f1 $'A B A\nA A Z'
		OUTPUT - $'A A Z\nA B A'
	EXEC	-k 2,2 f1

TEST 05 'three on three'
	EXEC	-k 2,3 f1
		INPUT f1 $'9 a b\n7 a a'
		OUTPUT - $'7 a a\n9 a b'
	EXEC	-k 2,3 f1
		INPUT f1 $'a a b\nz a a'
		OUTPUT - $'z a a\na a b'
	EXEC	-k 2,3 f1
		INPUT f1 $'y k b\nz k a'
		OUTPUT - $'z k a\ny k b'
	EXEC	+1 -3 f1

TEST 06 'floating point'
	EXEC	-g f1
		INPUT f1 $'1e2\n2e1'
		OUTPUT - $'2e1\n1e2'
	EXEC	-n f1
		OUTPUT - $'1e2\n2e1'
	EXEC	-n f1
		INPUT f1 $'2e1\n1e2'
		OUTPUT - $'1e2\n2e1'
	EXEC	-k2g f1
		INPUT f1 $'a 1e2\nb 2e1'
		OUTPUT - $'b 2e1\na 1e2'

TEST 07	'character offsets in field'
	EXEC	-t : -k 2.2,2.2 f1
		INPUT f1 $':ba\n:ab'
		OUTPUT - $':ba\n:ab'
	EXEC	-t : +1.1 -1.2 f1
	EXEC	-t : -k 2.2,2.2 f1
		INPUT f1 $':ab\n:ba'
		OUTPUT - $':ba\n:ab'
	EXEC	-t : +1.1 -1.2 f1
	EXEC	-t : -k 1.3,1.3 f1
	EXEC	-k 2.3,2.3 f1
		INPUT f1 $'z ba\nz ab'
		OUTPUT - $'z ba\nz ab'
	EXEC	-b -k 2.2,2.2 f1
	EXEC	-k 1.2,1.2 f1
		INPUT f1 $'ba\nab'
		OUTPUT - $'ba\nab'
	EXEC	-k 1.2,1.2 f1
		INPUT f1 $'ab\nba'
	EXEC	-k 1.4,1.4 f1
		INPUT f1 $'a ab\nb ba'
		OUTPUT - $'b ba\na ab'

TEST 08 'delimiters'
	EXEC	-t: -k1,1b -k2,2 f1
		INPUT f1 $'a\t:a\na :b'
		OUTPUT - $'a\t:a\na :b'
	EXEC	-t: -k1,1b -k2,2 f1
		INPUT f1 $'a :b\na\t:a'
		OUTPUT - $'a\t:a\na :b'
	EXEC	-t: -k2,2b -k3,3 f1
		INPUT f1 $'z:a\t:a\na :b'
		OUTPUT - $'z:a\t:a\na :b'
	EXEC	-t: -k2,2b -k3,3 f1
		INPUT f1 $'z:a :b\na\t:a'
		OUTPUT - $'a\t:a\nz:a :b'

TEST 09	'numeric with delimiters'
	EXEC	-n -t: +1 f1
		INPUT f1 $'a:1\nb:-'
		OUTPUT - $'b:-\na:1'
	EXEC	-n -t: +1 f1
		INPUT f1 $'b:-\na:1'
	EXEC	-n -t: +1 f1
		INPUT f1 $'a:1\nb:X'
		OUTPUT - $'b:X\na:1'
	EXEC	-n -t: +1 f1
		INPUT f1 $'b:X\na:1'
	EXEC	+0.1n f1
		INPUT f1 $'axx\nb-1'
		OUTPUT - $'b-1\naxx'
	EXEC	+0.1n f1
		INPUT f1 $'b-1\naxx'

TEST 18	'unique with control chars'
	EXEC	-d -u f1
		INPUT f1 $'mal\nmal-\nmala'
		OUTPUT - $'mal\nmala'
	EXEC	-f -d -u f1
	EXEC	-i -u f1
		INPUT f1 $'a\na\1'
		OUTPUT - $'a'
	EXEC	-i -u f1
		INPUT f1 $'a\n\1a'
	EXEC	-i -u f1
		INPUT f1 $'a\1\na'
		OUTPUT - $'a\1'
	EXEC	-i -u f1
		INPUT f1 $'\1a\na'
		OUTPUT - $'\1a'
	EXEC	-i -u f1
		INPUT f1 $'a\n\1\1\1\1\1a\1\1\1\1'
		OUTPUT - $'a'

TEST 19	'i18n'
	EXEC	-f f1
		INPUT f1 $'\351minence\n\374berhaupt\n\'s-Gravenhage\na\353roclub\nAag\naagtappels'
		OUTPUT - $'\'s-Gravenhage\nAag\naagtappels\na\353roclub\n\351minence\n\374berhaupt'

TEST 20	'huh'
	EXEC	-c f1
		INPUT f1 $'xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx'

TEST 21	'analyze this'
	EXEC	-n -k1.1,1.2 f1
		INPUT f1 $' 901\n100'
		OUTPUT - $' 901\n100'
	EXEC	-k1.1n,1.2 f1
	EXEC	-k1.1n,1.2n f1
	EXEC	-k1.1,1.2n f1
	EXEC	-b -k1.1n,1.2 f1
	EXEC	-b -k1.1n,1.2n f1
	EXEC	-b -k1.1,1.2n f1
	EXEC	-k1.1nb,1.2 f1
	EXEC	-k1.1b,1.2n f1
	EXEC	-k1 -k2n f1
	EXEC	-k1.1n,1.2b f1
		OUTPUT - $'100\n 901'
	EXEC	-k1.1n,1.2nb f1
	EXEC	-nb -k1.1,1.2 f1
	EXEC	-n -k1 f1
	EXEC	-k1n f1
	EXEC	-n -k1 -k2 f1
	EXEC	-k1n -k2 f1

TEST 22 'errors'
	DIAGNOSTICS
	EXEC	-c f1
		INPUT f1 $'A\nC\nB'
		EXIT [12]
	EXEC	-cu f1
		INPUT f1 $'A\nB\nB'
	EXEC	-cu f1
		INPUT f1 $'A\nA'
	EXEC	-cu f1
		INPUT f1 $'B\nA\nB'
	EXEC	-k0 f1
		INPUT f1
	EXEC	-k1.0 f1
	EXEC	-k1.1,-k0 f1
	EXEC	-k 2.,3 f1
	EXEC	-k 2, f1
