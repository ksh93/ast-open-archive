# regression tests for the sum utilitiy

KEEP "*.dat"

function DATA
{
	typeset f
	integer i
	typeset -i8 n
	for f
	do	test -f $f && continue
		case $f in
		big.dat)for ((i = 0; i <= 10000; i++))
			do	print $i
			done
			;;
		chars.dat)
			typeset -i8 o
			for ((o = 0; o < 256; o++))
			do	print -f "\\${o#8#}"
			done
			;;
		xyz.dat)print x
			print y
			print z
			;;
		zero.dat)
			for ((n = 0; n < 256; n++))
			do	print -f "\\0"
			done
			;;
		zyx.dat)print z
			print y
			print x
			;;
		esac > $f
	done
}

TEST 01 'old att algorithm'
	DO	DATA big.dat chars.dat xyz.dat zyx.dat zero.dat
	EXEC	-x att
		OUTPUT - $'0 0'
	EXEC	-x att /dev/null
		OUTPUT - $'0 0 /dev/null'
	EXEC	-x att xyz.dat zyx.dat
		OUTPUT - $'393 1 xyz.dat\n393 1 zyx.dat'
	EXEC	-x att -t xyz.dat zyx.dat
		OUTPUT - $'0 1 2'
	EXEC	-x att -t xyz.dat chars.dat
		OUTPUT - $'32265 1 2'
	EXEC	-x att big.dat
		OUTPUT - $'49851 96 big.dat'
	EXEC	-x att chars.dat
		OUTPUT - $'32640 1 chars.dat'
	EXEC	-x att zero.dat
		OUTPUT - $'0 1 zero.dat'

TEST 02 'old bsd algorithm'
	DO	DATA big.dat chars.dat xyz.dat zyx.dat zero.dat
	EXEC	-x bsd
		OUTPUT - $'0 0'
	EXEC	-x bsd /dev/null
		OUTPUT - $'0 0 /dev/null'
	EXEC	-x bsd xyz.dat zyx.dat
		OUTPUT - $'93 1 xyz.dat\n4188 1 zyx.dat'
	EXEC	-x bsd -t xyz.dat zyx.dat
		OUTPUT - $'4097 1 2'
	EXEC	-x bsd -t xyz.dat chars.dat
		OUTPUT - $'605 1 2'
	EXEC	-x bsd big.dat
		OUTPUT - $'50647 48 big.dat'
	EXEC	-x bsd chars.dat
		OUTPUT - $'512 1 chars.dat'
	EXEC	-x bsd zero.dat
		OUTPUT - $'0 1 zero.dat'

TEST 03 'ast memsum algorithm'
	DO	DATA big.dat chars.dat xyz.dat zyx.dat zero.dat
	EXEC	-x ast
		OUTPUT - $'0 0'
	EXEC	-x ast /dev/null
		OUTPUT - $'0 0 /dev/null'
	EXEC	-x ast xyz.dat zyx.dat
		OUTPUT - $'333665879 6 xyz.dat\n844468759 6 zyx.dat'
	EXEC	-x ast -t xyz.dat zyx.dat
		OUTPUT - $'565625920 12 2'
	EXEC	-x ast big.dat
		OUTPUT - $'2944469051 48896 big.dat'
	EXEC	-x ast chars.dat
		OUTPUT - $'2634217600 256 chars.dat'
	EXEC	-x ast zero.dat
		OUTPUT - $'2971255040 256 zero.dat'

TEST 04 'zip crc algorithm'
	DO	DATA big.dat chars.dat xyz.dat zyx.dat zero.dat
	EXEC	-x zip
		OUTPUT - $'0 0'
	EXEC	-x zip /dev/null
		OUTPUT - $'0 0 /dev/null'
	EXEC	-x zip xyz.dat zyx.dat
		OUTPUT - $'849749004 6 xyz.dat\n1297672069 6 zyx.dat'
	EXEC	-x zip -t xyz.dat zyx.dat
		OUTPUT - $'2147402633 12 2'
	EXEC	-x zip big.dat
		OUTPUT - $'3652343079 48896 big.dat'
	EXEC	-x zip chars.dat
		OUTPUT - $'688229491 256 chars.dat'
	EXEC	-x zip zero.dat
		OUTPUT - $'227968344 256 zero.dat'

TEST 05 'posix cksum algorithm'
	DO	DATA big.dat chars.dat xyz.dat zyx.dat zero.dat
	EXEC	-x cksum
		OUTPUT - $'4294967295 0'
	EXEC	-x cksum /dev/null
		OUTPUT - $'4294967295 0 /dev/null'
	EXEC	-x cksum xyz.dat zyx.dat
		OUTPUT - $'2140995855 6 xyz.dat\n3800205347 6 zyx.dat'
	EXEC	-x cksum -t xyz.dat zyx.dat
		OUTPUT - $'2636089644 12 2'
	EXEC	-x cksum big.dat
		OUTPUT - $'528457989 48896 big.dat'
	EXEC	-x cksum chars.dat
		OUTPUT - $'1313719201 256 chars.dat'
	EXEC	-x cksum zero.dat
		OUTPUT - $'4215202376 256 zero.dat'

TEST 06 'md5 message digest algorithm'
	DO	DATA big.dat chars.dat xyz.dat zyx.dat zero.dat
	EXEC	-x md5
		OUTPUT - $'d41d8cd98f00b204e9800998ecf8427e'
	EXEC	-x md5 /dev/null
		OUTPUT - $'d41d8cd98f00b204e9800998ecf8427e /dev/null'
	EXEC	-x md5 xyz.dat zyx.dat
		OUTPUT - $'5c37d4d5cc8d74de8ed81fc394a56c0e xyz.dat\n30c4d234a30ae1665d3e63cbfac9ade9 zyx.dat'
	EXEC	-x md5 -t xyz.dat zyx.dat
		OUTPUT - $'6cf306e16f8795b8d3e67c086e6cc1e7'
	EXEC	-x md5 big.dat
		OUTPUT - $'4633277f9842941660fbd0a681b1e656 big.dat'
	EXEC	-x md5 chars.dat
		OUTPUT - $'e2c865db4162bed963bfaa9ef6ac18f0 chars.dat'
	EXEC	-x md5 zero.dat
		OUTPUT - $'348a9791dc41b89796ec3808b5b5262f zero.dat'

TEST 07 'text vs binary mode'
	EXEC	-x posix
		INPUT -n - $'\r'
		OUTPUT - $'3863691631 1'
	EXEC	-x posix -T
	EXEC	-x posix
		INPUT -n - $'\n'
		OUTPUT - $'3515105045 1'
	EXEC	-x posix -T
	EXEC	-x posix -T
		INPUT -n - $'\r\n'
	EXEC	-x posix
		OUTPUT - $'3568853871 2'
	EXEC	-x posix
		INPUT -n - $'\r\r\n'
		OUTPUT - $'2299211872 3'
	EXEC	-x posix -T
		OUTPUT - $'3568853871 2'
	EXEC	-x posix
		INPUT -n - $'\n\n'
		OUTPUT - $'3518178554 2'
	EXEC	-x posix -T
		INPUT -n - $'\r\n\r\n'
