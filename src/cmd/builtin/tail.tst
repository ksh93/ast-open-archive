# regression tests for the od utilitiy

KEEP "*.dat"

function DATA
{
	typeset f
	integer n
	for f
	do	test -f $f && continue
		case $f in
		10.dat) for ((i = 1; i <= 10; i++))
			do	print $i
			done
			;;
		100.dat)for ((i = 1; i <= 100; i++))
			do	print $i
			done
			;;
		esac > $f
	done
}

TEST 01 'basic'
	DO	DATA 10.dat 100.dat
	EXEC	10.dat
		OUTPUT - $'1\n2\n3\n4\n5\n6\n7\n8\n9\n10'
	EXEC	+0 10.dat
	EXEC	+1 10.dat
	EXEC	+2 10.dat
		OUTPUT - $'2\n3\n4\n5\n6\n7\n8\n9\n10'
	EXEC	-9 10.dat
	EXEC	-1 10.dat
		OUTPUT - $'10'
	EXEC	+10 10.dat
	EXEC	100.dat
		OUTPUT - $'91\n92\n93\n94\n95\n96\n97\n98\n99\n100'
	EXEC	-1 100.dat
		OUTPUT - $'100'
	EXEC	+100 100.dat
	EXEC	-2 100.dat
		OUTPUT - $'99\n100'
	EXEC	+99 100.dat
	EXEC	-c 6 100.dat
		OUTPUT - $'9\n100'
	EXEC	-6c 100.dat
	EXEC	-c +287 100.dat
	EXEC	+287c 100.dat
