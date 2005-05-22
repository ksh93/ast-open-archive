# regression tests for the tail utilitiy

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
		a.dat)	print a
			;;
		b.dat)	print b
			;;
		esac > $f
	done
}

TEST 01 'basic'
	DO	DATA 10.dat 100.dat
	EXEC	10.dat
		OUTPUT - $'1\n2\n3\n4\n5\n6\n7\n8\n9\n10'
	EXEC	+0 10.dat
	EXEC	-n +0 10.dat
	EXEC	+1 10.dat
	EXEC	-n +1 10.dat
	EXEC	+2 10.dat
		OUTPUT - $'2\n3\n4\n5\n6\n7\n8\n9\n10'
	EXEC	-n +2 10.dat
	EXEC	-9 10.dat
	EXEC	-n -9 10.dat
	EXEC	-1 10.dat
		OUTPUT - $'10'
	EXEC	-n -1 10.dat
	EXEC	+10 10.dat
	EXEC	-n +10 10.dat
	EXEC	100.dat
		OUTPUT - $'91\n92\n93\n94\n95\n96\n97\n98\n99\n100'
	EXEC	-1 100.dat
		OUTPUT - $'100'
	EXEC	-n -1 100.dat
	EXEC	+100 100.dat
	EXEC	-n +100 100.dat
	EXEC	-2 100.dat
		OUTPUT - $'99\n100'
	EXEC	-n -2 100.dat
	EXEC	+99 100.dat
	EXEC	-n +99 100.dat
	EXEC	-c 6 100.dat
		OUTPUT - $'9\n100'
	EXEC	-6c 100.dat
	EXEC	-c +287 100.dat
	EXEC	+287c 100.dat

TEST 02 'headers'
	DO	DATA a.dat b.dat
	EXEC	a.dat
		OUTPUT - $'a'
	EXEC	-v a.dat
		OUTPUT - $'==> a.dat <==\na'
	EXEC	a.dat b.dat
		OUTPUT - $'==> a.dat <==\na\n\n==> b.dat <==\nb'
	EXEC	-v a.dat b.dat
	EXEC	-h a.dat
		OUTPUT - $'a'
	EXEC	-h a.dat b.dat
		OUTPUT - $'a\nb'

TEST 03 'timeouts'
	DO	DATA a.dat b.dat
	EXEC	-f -t 2 a.dat
		OUTPUT - $'a'
		ERROR - $'tail: warning: a.dat: 2.00s timeout'
	EXEC	-f -t 2 a.dat b.dat
		OUTPUT - $'==> a.dat <==\na\n\n==> b.dat <==\nb'
		ERROR - $'tail: warning: a.dat: 2.00s timeout\ntail: warning: b.dat: 2.00s timeout'
	EXEC	-s -f -t 2 a.dat
		OUTPUT - $'a'
		ERROR -
	EXEC	-s -f -t 2 a.dat b.dat
		OUTPUT - $'==> a.dat <==\na\n\n==> b.dat <==\nb'
	EXEC	-h -s -f -t 2 a.dat
		OUTPUT - $'a'
	EXEC	-h -s -f -t 2 a.dat b.dat
		OUTPUT - $'a\nb'
	EXEC	-h -f -t 2 a.dat
		OUTPUT - $'a'
		ERROR - $'tail: warning: a.dat: 2.00s timeout'
	EXEC	-h -f -t 2 a.dat b.dat
		OUTPUT - $'a\nb'
		ERROR - $'tail: warning: a.dat: 2.00s timeout\ntail: warning: b.dat: 2.00s timeout'
