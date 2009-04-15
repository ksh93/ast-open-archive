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

TEST 01 'explicit file operands'
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

SET pipe-input

TEST 04 'standard input'
	DO	DATA 10.dat 100.dat
	EXEC
		SAME INPUT 10.dat
		OUTPUT - $'1\n2\n3\n4\n5\n6\n7\n8\n9\n10'
	EXEC	-
	EXEC	+0
	EXEC	-n +0
	EXEC	+1
	EXEC	-n +1
	EXEC	+2
		OUTPUT - $'2\n3\n4\n5\n6\n7\n8\n9\n10'
	EXEC	-n +2
	EXEC	-9
	EXEC	-n -9
	EXEC	-1
		OUTPUT - $'10'
	EXEC	-n -1
	EXEC	+10
	EXEC	-n +10
	EXEC	--
		SAME INPUT 100.dat
		OUTPUT - $'91\n92\n93\n94\n95\n96\n97\n98\n99\n100'
	EXEC	-
	EXEC	-1
		OUTPUT - $'100'
	EXEC	-n -1
	EXEC	+100
	EXEC	-n +100
	EXEC	-2
		OUTPUT - $'99\n100'
	EXEC	-n -2
	EXEC	+99
	EXEC	-n +99
	EXEC	-c 6
		OUTPUT - $'9\n100'
	EXEC	-6c
	EXEC	-c +287
	EXEC	+287c

# the remainder converted from the gnu tail Test.pm

TEST 05	chars

	EXEC	+2c
		INPUT -n - $'abcd'
		OUTPUT -n - $'bcd'

TEST 06	obs-c

	EXEC	+8c
		INPUT -n - $'abcd'
		OUTPUT - 

	EXEC	-1c
		INPUT -n - $'abcd'
		OUTPUT -n - $'d'

	EXEC	-9c
		INPUT -n - $'abcd'
		OUTPUT -n - $'abcd'

	EXEC	-12c
		INPUT -n - $'xyyyyyyyyyyyyz'
		OUTPUT -n - $'yyyyyyyyyyyz'

	EXEC	-1l
		INPUT - $'x'
		OUTPUT - $'x'

TEST 07	obs-l

	EXEC	-1l
		INPUT - $'x\ny'
		OUTPUT - $'y'

	EXEC	+1l
		INPUT - $'x\ny'
		OUTPUT - $'x\ny'

	EXEC	+2l
		INPUT - $'x\ny'
		OUTPUT - $'y'

	EXEC	-1
		INPUT - $'x'
		OUTPUT - $'x'

TEST 08	obs

	EXEC	-1
		INPUT - $'x\ny'
		OUTPUT - $'y'

	EXEC	+1
		INPUT - $'x\ny'
		OUTPUT - $'x\ny'

	EXEC	+2
		INPUT - $'x\ny'
		OUTPUT - $'y'

	EXEC	+c
		INPUT - $'xyyyyyyyyyyz'
		OUTPUT - $'yyz'

	EXEC	-c
		OUTPUT - $'yyyyyyyyz'

	EXEC	+c
		INPUT -n - $'xyyyyyyyyyyz'
		OUTPUT -n - $'yyz'

	EXEC	-c
		OUTPUT -n - $'yyyyyyyyyz'

TEST 09	obsx

	EXEC	+l
		INPUT -n - $'x\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\nz'
		OUTPUT -n - $'y\ny\nz'

	EXEC	+cl

	EXEC	-l
		INPUT - $'x\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\nz'
		OUTPUT - $'y\ny\ny\ny\ny\ny\ny\ny\ny\nz'

	EXEC	-cl

TEST 10	empty

	EXEC	-

	EXEC	-c

TEST 11 err

	EXEC	+2cz
		ERROR - $'tail: z: invalid suffix
Usage: tail [-bfhlLqrsv] [-n lines] [-c[chars]] [-t timeout] [file ...]'
	    	EXIT 2

	EXEC	-2cX
		ERROR - $'tail: X: invalid suffix
Usage: tail [-bfhlLqrsv] [-n lines] [-c[chars]] [-t timeout] [file ...]'

	EXEC	-c99999999999999999999
		ERROR - $'tail: -c: 99999999999999999999: invalid numeric argument -- out of range
Usage: tail [-bfhlLqrsv] [-n lines] [-c[chars]] [-t timeout] [file ...]'

TEST 12	minus

	EXEC	-
		INPUT - $'x\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\nz'
		OUTPUT - $'y\ny\ny\ny\ny\ny\ny\ny\ny\nz'

	EXEC	-n 10

TEST 13	n

	EXEC	-n -10
		INPUT - $'x\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\nz'
		OUTPUT - $'y\ny\ny\ny\ny\ny\ny\ny\ny\nz'

	EXEC	-n +10
		INPUT - $'x\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\nz'
		OUTPUT - $'y\ny\nz'

	EXEC	-n +0
		INPUT - $'y\ny\ny\ny\ny'
		OUTPUT - $'y\ny\ny\ny\ny'

	EXEC	-n +1
		INPUT - $'y\ny\ny\ny\ny'
		OUTPUT - $'y\ny\ny\ny\ny'

	EXEC	-n -0
		INPUT - $'y\ny\ny\ny\ny'
		OUTPUT - 

	EXEC	-n -1
		INPUT - $'y\ny\ny\ny\ny'
		OUTPUT - $'y'

	EXEC	-n  0
		INPUT - $'y\ny\ny\ny\ny'
		OUTPUT - 

TEST 14 VSC#4,5,14,15,1001,1003

	EXEC	- more
		INPUT more $'111\n222\n333\n444\n555\n666\n777\n888\n999\naaa\nbbb\nccc'
		OUTPUT - $'333\n444\n555\n666\n777\n888\n999\naaa\nbbb\nccc'

	EXEC	-10 more

	EXEC	-l more

	EXEC	-10l more

	EXEC	+ more
		OUTPUT - $'aaa\nbbb\nccc'

	EXEC	+10 more

	EXEC	+l more

	EXEC	+10l more

	EXEC	- less
		INPUT less $'111\n222\n333\n444\n555'
		OUTPUT - $'111\n222\n333\n444\n555'

	EXEC	-l less

	EXEC	+c less
		OUTPUT - $'33\n444\n555'

	EXEC	+c mini
		INPUT mini $'111\n222'
		OUTPUT -

	EXEC	+512c big
		INPUT big $'1bcdefgh2bcdefgh3bcdefgh4bcdefgh5bcdefgh6bcdefgh7bcdefgh8bcdefgh
2bcdefgh2bcdefgh3bcdefgh4bcdefgh5bcdefgh6bcdefgh7bcdefgh8bcdefgh
3bcdefgh2bcdefgh3bcdefgh4bcdefgh5bcdefgh6bcdefgh7bcdefgh8bcdefgh
4bcdefgh2bcdefgh3bcdefgh4bcdefgh5bcdefgh6bcdefgh7bcdefgh8bcdefgh
5bcdefgh2bcdefgh3bcdefgh4bcdefgh5bcdefgh6bcdefgh7bcdefgh8bcdefgh
6bcdefgh2bcdefgh3bcdefgh4bcdefgh5bcdefgh6bcdefgh7bcdefgh8bcdefgh
7bcdefgh2bcdefgh3bcdefgh4bcdefgh5bcdefgh6bcdefgh7bcdefgh8bcdefgh
8bcdefgh2bcdefgh3bcdefgh4bcdefgh5bcdefgh6bcdefgh7bcdefgh8bcdefgh'
		OUTPUT - $'8bcdefgh'

	EXEC	+1b big

	EXEC	-512c big
		OUTPUT - $'2bcdefgh3bcdefgh4bcdefgh5bcdefgh6bcdefgh7bcdefgh8bcdefgh
2bcdefgh2bcdefgh3bcdefgh4bcdefgh5bcdefgh6bcdefgh7bcdefgh8bcdefgh
3bcdefgh2bcdefgh3bcdefgh4bcdefgh5bcdefgh6bcdefgh7bcdefgh8bcdefgh
4bcdefgh2bcdefgh3bcdefgh4bcdefgh5bcdefgh6bcdefgh7bcdefgh8bcdefgh
5bcdefgh2bcdefgh3bcdefgh4bcdefgh5bcdefgh6bcdefgh7bcdefgh8bcdefgh
6bcdefgh2bcdefgh3bcdefgh4bcdefgh5bcdefgh6bcdefgh7bcdefgh8bcdefgh
7bcdefgh2bcdefgh3bcdefgh4bcdefgh5bcdefgh6bcdefgh7bcdefgh8bcdefgh
8bcdefgh2bcdefgh3bcdefgh4bcdefgh5bcdefgh6bcdefgh7bcdefgh8bcdefgh'

	EXEC	-1b big

TEST 15 VSC#20

	EXEC	-t1s -s -f -c -7 more
		INPUT more $'111\n222\n333\n444\n555\n666\n777\n888\n999\naaa\nbbb\nccc'
		OUTPUT - $'bb\nccc'

	EXEC	-c -7 more #21

	EXEC	-t1s -s -f -n +10 more
		OUTPUT - $'aaa\nbbb\nccc'

	EXEC	-n +10 more

	EXEC	-t1s -s -f -n -4 more
		OUTPUT - $'999\naaa\nbbb\nccc'

	EXEC	-n -4 more

	EXEC	-t1s -s -f -n +4 more
		OUTPUT - $'444\n555\n666\n777\n888\n999\naaa\nbbb\nccc'

	EXEC	-n +4 more

	EXEC	-t1s -s -7lf more
		OUTPUT - $'666\n777\n888\n999\naaa\nbbb\nccc'

	EXEC	-7l more

	EXEC	-t1s -s -7cf more
		OUTPUT - $'bb\nccc'

	EXEC	-7c more

	EXEC	-t1s -s +33cf more
		OUTPUT - $'999\naaa\nbbb\nccc'

	EXEC	+33c more

	EXEC	-t1s -s +12lf more
		OUTPUT - $'ccc'

	EXEC	+12l more

TEST 16 xpg4

	EXEC	-t1s -s -f1 more
		INPUT more $'111\n222\n333\n444\n555\n666\n777\n888\n999\naaa\nbbb\nccc'
		OUTPUT - $'ccc'

	EXEC	-t1s -s -f10 more
		OUTPUT - $'333\n444\n555\n666\n777\n888\n999\naaa\nbbb\nccc'

	EXEC	-t1s -s -f more
