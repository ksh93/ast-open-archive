# regression tests for the POSIX ed utility

KEEP "*.dat"

function DATA
{
	typeset f x=0123456789
	integer i
	typeset -i8 n
	for f
	do	test -f $f && continue
		case $f in
		big.dat)for ((i = 0; i <= 10000; i++))
			do	print $i:$x$x$x$x$x$x$x$x$x$x
			done
			;;
		esac > $f
	done
}

TEST 01 'file args'
	DO	DATA big.dat
	EXEC
		NOTE 'no input file'
	EXEC file
		NOTE 'empty input file'
		INPUT file
		OUTPUT - 0
	EXEC big.dat
		NOTE 'big input file'
		INPUT - $'v/^10*:/d\n1,$s/:.*//\n,p\nQ'
		OUTPUT - $'1058997\n1\n10\n100\n1000\n10000'
	EXEC not_a_file
		NOTE 'non-existent input file'
		OUTPUT - '?'
		EXIT '[12]'

TEST 02 'commands'
	EXEC file
		NOTE '='
		INPUT - g/./.=
		INPUT file $'a\nb\nc'
		OUTPUT - $'6\n1\n2\n3'
	EXEC file
		NOTE '# comments'
		INPUT - $'#n\ng/./.=\n#comment'
	EXEC file
		NOTE 'a top'
		INPUT - $'0a\nz\n.\nw\nq'
		OUTPUT - $'6\n8'
		OUTPUT file $'z\na\nb\nc'
	EXEC file
		NOTE 'a mid'
		INPUT - $'2a\nz\n.\nw\nq'
		OUTPUT file $'a\nb\nz\nc'
	EXEC file
		NOTE 'a bot'
		INPUT - $'$a\nz\n.\nw\nq'
		OUTPUT file $'a\nb\nc\nz'
	EXEC file
		NOTE 'c top'
		INPUT - $'1c\nz\n.\nw\nq'
		OUTPUT - $'6\n6'
		OUTPUT file $'z\nb\nc'
	EXEC file
		NOTE 'c mid'
		INPUT - $'2c\nz\n.\nw\nq'
		OUTPUT file $'a\nz\nc'
	EXEC file
		NOTE 'c bot'
		INPUT - $'$c\nz\n.\nw\nq'
		OUTPUT file $'a\nb\nz'

TEST 03 'commands'
	EXEC file
		NOTE 'substitute'
		INPUT file $'a\nb\nc'
		INPUT - $'1,$s/./&&/\n1,$s/./&x/\n1,$s/./&y/2\n1,$s/./&z/4\nw\nq'
		OUTPUT file $'axyaz\nbxybz\ncxycz'
		OUTPUT - $'6\n18'
	EXEC file
		NOTE 'substitute g'
		INPUT file $'abcabcabc\nabcabcabc\nabcabcabc\nabcabcabc\nabcabcabc\nabcabcabc\nabcabcabc\nabcabcabc\nabcabcabc'
		INPUT - $'1s/a//g\n2s/b//g\n3s/c//g\n4s/a/X/g\n5s/b/X/g\n6s/c/X/g\n7s/a/XX/g\n8s/b/XX/g\n9s/c/XX/g\nw\nq'
		OUTPUT file $'bcbcbc\nacacac\nababab\nXbcXbcXbc\naXcaXcaXc\nabXabXabX\nXXbcXXbcXXbc\naXXcaXXcaXXc\nabXXabXXabXX'
		OUTPUT - $'90\n90'
	EXEC file
		NOTE 'substitute count'
		INPUT - $'1s/a//2\n2s/b//2\n3s/c//2\n4s/a/X/2\n5s/b/X/2\n6s/c/X/2\n7s/a/XX/2\n8s/b/XX/2\n9s/c/XX/2\nw\nq'
		OUTPUT file $'abcbcabc\nabcacabc\nabcababc\nabcXbcabc\nabcaXcabc\nabcabXabc\nabcXXbcabc\nabcaXXcabc\nabcabXXabc'
		OUTPUT - $'90\n90'
