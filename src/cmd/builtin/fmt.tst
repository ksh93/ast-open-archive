# regression tests for the fmt utilitiy

function DATA
{
	for f
	do	if	[[ ! -f $f ]]
		then	case $f in
			big.in)	for ((i = 0; i < 1024; i++))
				do	print -n "abcdefghijklmnop "
				done
				print
				;;
			big.out)for ((i = 0; i < 256; i++))
				do	print "abcdefghijklmnop abcdefghijklmnop abcdefghijklmnop abcdefghijklmnop"
				done
				;;
			esac > $f
		fi
	done
}

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
	EXEC
		INPUT -n - $'aaa'
		OUTPUT - $'aaa'
	EXEC
		INPUT -n - $'aaa\n\nzzz'
		OUTPUT - $'aaa\n\nzzz'

TEST 02 'line buffer stress'
	DO	DATA big.in big.out
	EXEC	big.in
		SAME OUTPUT big.out
