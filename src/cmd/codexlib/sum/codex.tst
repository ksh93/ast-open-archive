# regression tests for the codex(1) sum methods

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

TEST 01 'old att'
	DO	DATA big.dat chars.dat xyz.dat zyx.dat zero.dat
	EXEC	'>sum-att'
		INPUT -
		IGNORE OUTPUT
		ERROR - 00000000
	EXEC	'>sum-att'
		SAME INPUT xyz.dat
		ERROR - 00000189
	EXEC	'>sum-att'
		SAME INPUT zyx.dat
		ERROR - 00000189
	EXEC	'>sum-att'
		SAME INPUT big.dat
		ERROR - 0000c2bb
	EXEC	'>sum-att'
		SAME INPUT chars.dat
		ERROR - 00007f80
	EXEC	'>sum-att'
		SAME INPUT zero.dat
		ERROR - 00000000

TEST 02 'old bsd'
	DO	DATA big.dat chars.dat xyz.dat zyx.dat zero.dat
	EXEC	'>sum-bsd'
		INPUT -
		IGNORE OUTPUT
		ERROR - 00000000
	EXEC	'>sum-bsd'
		SAME INPUT xyz.dat
		ERROR - 0000005d
	EXEC	'>sum-bsd'
		SAME INPUT zyx.dat
		ERROR - 0000105c
	EXEC	'>sum-bsd'
		SAME INPUT big.dat
		ERROR - 0000c5d7
	EXEC	'>sum-bsd'
		SAME INPUT chars.dat
		ERROR - 00000200
	EXEC	'>sum-bsd'
		SAME INPUT zero.dat
		ERROR - 00000000

TEST 03 'ast memsum'
	DO	DATA big.dat chars.dat xyz.dat zyx.dat zero.dat
	EXEC	'>sum-ast'
		INPUT -
		IGNORE OUTPUT
		ERROR - 00000000
	EXEC	'>sum-ast'
		SAME INPUT xyz.dat
		ERROR - 13e35657
	EXEC	'>sum-ast'
		SAME INPUT zyx.dat
		ERROR - 32559217
	EXEC	'>sum-ast'
		SAME INPUT big.dat
		ERROR - af81083b
	EXEC	'>sum-ast'
		SAME INPUT chars.dat
		ERROR - 9d02f880
	EXEC	'>sum-ast'
		SAME INPUT zero.dat
		ERROR - b119c100

TEST 04 'zip crc'
	DO	DATA big.dat chars.dat xyz.dat zyx.dat zero.dat
	EXEC	'>sum-zip'
		INPUT -
		IGNORE OUTPUT
		ERROR - 00000000
	EXEC	'>sum-zip'
		SAME INPUT xyz.dat
		ERROR - 32a6240c
	EXEC	'>sum-zip'
		SAME INPUT zyx.dat
		ERROR - 4d58e785
	EXEC	'>sum-zip'
		SAME INPUT big.dat
		ERROR - d9b25527
	EXEC	'>sum-zip'
		SAME INPUT chars.dat
		ERROR - 29058c73
	EXEC	'>sum-zip'
		SAME INPUT zero.dat
		ERROR - 0d968558

TEST 05 'posix cksum'
	DO	DATA big.dat chars.dat xyz.dat zyx.dat zero.dat
	EXEC	'>sum-cksum'
		INPUT -
		IGNORE OUTPUT
		ERROR - ffffffff
	EXEC	'>sum-cksum'
		SAME INPUT xyz.dat
		ERROR - 7f9d010f
	EXEC	'>sum-cksum'
		SAME INPUT zyx.dat
		ERROR - e2828823
	EXEC	'>sum-cksum'
		SAME INPUT big.dat
		ERROR - 1f7fa105
	EXEC	'>sum-cksum'
		SAME INPUT chars.dat
		ERROR - 4e4dc3a1
	EXEC	'>sum-cksum'
		SAME INPUT zero.dat
		ERROR - fb3ee248

TEST 06 'md5 message digest'
	DO	DATA big.dat chars.dat xyz.dat zyx.dat zero.dat
	EXEC	'>sum-md5'
		INPUT -
		IGNORE OUTPUT
		ERROR - d41d8cd98f00b204e9800998ecf8427e
	EXEC	'>sum-md5'
		SAME INPUT xyz.dat
		ERROR - 5c37d4d5cc8d74de8ed81fc394a56c0e
	EXEC	'>sum-md5'
		SAME INPUT zyx.dat
		ERROR - 30c4d234a30ae1665d3e63cbfac9ade9
	EXEC	'>sum-md5'
		SAME INPUT big.dat
		ERROR - 4633277f9842941660fbd0a681b1e656
	EXEC	'>sum-md5'
		SAME INPUT chars.dat
		ERROR - e2c865db4162bed963bfaa9ef6ac18f0
	EXEC	'>sum-md5'
		SAME INPUT zero.dat
		ERROR - 348a9791dc41b89796ec3808b5b5262f
	EXEC	'>sum-md5'
		INPUT -n - abc
		ERROR - 900150983cd24fb0d6963f7d28e17f72
	EXEC	'>sum-md5'
		INPUT -n - abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq
		ERROR - 8215ef0796a20bcaaae116d3876c664a

TEST 07 'SHA-1 secure hash algorithm 1'
	DO	DATA big.dat chars.dat xyz.dat zyx.dat zero.dat
	EXEC	'>sum-sha1'
		INPUT -
		IGNORE OUTPUT
		ERROR - da39a3ee5e6b4b0d3255bfef95601890afd80709
	EXEC	'>sum-sha1'
		SAME INPUT xyz.dat
		ERROR - 83305e292107a8d1955ac0c0047912ff62c5d6dc
	EXEC	'>sum-sha1'
		SAME INPUT zyx.dat
		ERROR - f1bac0f6f8e8d09b07cbc04c2e70b1b606fb9dd5
	EXEC	'>sum-sha1'
		SAME INPUT big.dat
		ERROR - d3e7a9584187f017342dd759bc8f3061b74c5faf
	EXEC	'>sum-sha1'
		SAME INPUT chars.dat
		ERROR - 4916d6bdb7f78e6803698cab32d1586ea457dfc8
	EXEC	'>sum-sha1'
		SAME INPUT zero.dat
		ERROR - b376885ac8452b6cbf9ced81b1080bfd570d9b91
	EXEC	'>sum-sha1'
		INPUT -n - abc
		ERROR - a9993e364706816aba3e25717850c26c9cd0d89d
	EXEC	'>sum-sha1'
		INPUT -n - abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq
		ERROR - 84983e441c3bd26ebaae4aa1f95129e5e54670f1
