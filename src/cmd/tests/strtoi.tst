# regression tests for the ast integral strto*() routines

TEST 01 'base 10'
	EXEC	32767 -32767 32768 -32768 32769 -32769
		OUTPUT - $'strtol   "32767" "" 32767 OK
strton   "32767" "" 32767 OK 0
strtoul  "32767" "" 32767 OK
strtoll  "32767" "" 32767 OK
strtonll "32767" "" 32767 OK 0
strtoull "32767" "" 32767 OK

strtol   "-32767" "" -32767 OK
strton   "-32767" "" -32767 OK 0
strtoul  "-32767" "" 4294934529 OK
strtoll  "-32767" "" -32767 OK
strtonll "-32767" "" -32767 OK 0
strtoull "-32767" "" 18446744073709518849 OK

strtol   "32768" "" 32768 OK
strton   "32768" "" 32768 OK 0
strtoul  "32768" "" 32768 OK
strtoll  "32768" "" 32768 OK
strtonll "32768" "" 32768 OK 0
strtoull "32768" "" 32768 OK

strtol   "-32768" "" -32768 OK
strton   "-32768" "" -32768 OK 0
strtoul  "-32768" "" 4294934528 OK
strtoll  "-32768" "" -32768 OK
strtonll "-32768" "" -32768 OK 0
strtoull "-32768" "" 18446744073709518848 OK

strtol   "32769" "" 32769 OK
strton   "32769" "" 32769 OK 0
strtoul  "32769" "" 32769 OK
strtoll  "32769" "" 32769 OK
strtonll "32769" "" 32769 OK 0
strtoull "32769" "" 32769 OK

strtol   "-32769" "" -32769 OK
strton   "-32769" "" -32769 OK 0
strtoul  "-32769" "" 4294934527 OK
strtoll  "-32769" "" -32769 OK
strtonll "-32769" "" -32769 OK 0
strtoull "-32769" "" 18446744073709518847 OK'
	EXEC	65535 -65535 65536 -65536 65537 -65537
		OUTPUT - $'strtol   "65535" "" 65535 OK
strton   "65535" "" 65535 OK 0
strtoul  "65535" "" 65535 OK
strtoll  "65535" "" 65535 OK
strtonll "65535" "" 65535 OK 0
strtoull "65535" "" 65535 OK

strtol   "-65535" "" -65535 OK
strton   "-65535" "" -65535 OK 0
strtoul  "-65535" "" 4294901761 OK
strtoll  "-65535" "" -65535 OK
strtonll "-65535" "" -65535 OK 0
strtoull "-65535" "" 18446744073709486081 OK

strtol   "65536" "" 65536 OK
strton   "65536" "" 65536 OK 0
strtoul  "65536" "" 65536 OK
strtoll  "65536" "" 65536 OK
strtonll "65536" "" 65536 OK 0
strtoull "65536" "" 65536 OK

strtol   "-65536" "" -65536 OK
strton   "-65536" "" -65536 OK 0
strtoul  "-65536" "" 4294901760 OK
strtoll  "-65536" "" -65536 OK
strtonll "-65536" "" -65536 OK 0
strtoull "-65536" "" 18446744073709486080 OK

strtol   "65537" "" 65537 OK
strton   "65537" "" 65537 OK 0
strtoul  "65537" "" 65537 OK
strtoll  "65537" "" 65537 OK
strtonll "65537" "" 65537 OK 0
strtoull "65537" "" 65537 OK

strtol   "-65537" "" -65537 OK
strton   "-65537" "" -65537 OK 0
strtoul  "-65537" "" 4294901759 OK
strtoll  "-65537" "" -65537 OK
strtonll "-65537" "" -65537 OK 0
strtoull "-65537" "" 18446744073709486079 OK'
	EXEC	2147483647 -2147483647 2147483648 -2147483648 2147483649 -2147483649
		OUTPUT - $'strtol   "2147483647" "" 2147483647 OK
strton   "2147483647" "" 2147483647 OK 0
strtoul  "2147483647" "" 2147483647 OK
strtoll  "2147483647" "" 2147483647 OK
strtonll "2147483647" "" 2147483647 OK 0
strtoull "2147483647" "" 2147483647 OK

strtol   "-2147483647" "" -2147483647 OK
strton   "-2147483647" "" -2147483647 OK 0
strtoul  "-2147483647" "" 2147483649 OK
strtoll  "-2147483647" "" -2147483647 OK
strtonll "-2147483647" "" -2147483647 OK 0
strtoull "-2147483647" "" 18446744071562067969 OK

strtol   "2147483648" "" 2147483647 ERANGE
strton   "2147483648" "" 2147483647 ERANGE 0
strtoul  "2147483648" "" 2147483648 OK
strtoll  "2147483648" "" 2147483648 OK
strtonll "2147483648" "" 2147483648 OK 0
strtoull "2147483648" "" 2147483648 OK

strtol   "-2147483648" "" -2147483648 OK
strton   "-2147483648" "" -2147483648 OK 0
strtoul  "-2147483648" "" 2147483648 OK
strtoll  "-2147483648" "" -2147483648 OK
strtonll "-2147483648" "" -2147483648 OK 0
strtoull "-2147483648" "" 18446744071562067968 OK

strtol   "2147483649" "" 2147483647 ERANGE
strton   "2147483649" "" 2147483647 ERANGE 0
strtoul  "2147483649" "" 2147483649 OK
strtoll  "2147483649" "" 2147483649 OK
strtonll "2147483649" "" 2147483649 OK 0
strtoull "2147483649" "" 2147483649 OK

strtol   "-2147483649" "" -2147483648 ERANGE
strton   "-2147483649" "" -2147483648 ERANGE 0
strtoul  "-2147483649" "" 2147483647 OK
strtoll  "-2147483649" "" -2147483649 OK
strtonll "-2147483649" "" -2147483649 OK 0
strtoull "-2147483649" "" 18446744071562067967 OK'
	EXEC	4294967295 -4294967295 4294967296 -4294967296 4294967297 -4294967297
		OUTPUT - $'strtol   "4294967295" "" 2147483647 ERANGE
strton   "4294967295" "" 2147483647 ERANGE 0
strtoul  "4294967295" "" 4294967295 OK
strtoll  "4294967295" "" 4294967295 OK
strtonll "4294967295" "" 4294967295 OK 0
strtoull "4294967295" "" 4294967295 OK

strtol   "-4294967295" "" -2147483648 ERANGE
strton   "-4294967295" "" -2147483648 ERANGE 0
strtoul  "-4294967295" "" 4294967295 ERANGE
strtoll  "-4294967295" "" -4294967295 OK
strtonll "-4294967295" "" -4294967295 OK 0
strtoull "-4294967295" "" 18446744069414584321 OK

strtol   "4294967296" "" 2147483647 ERANGE
strton   "4294967296" "" 2147483647 ERANGE 0
strtoul  "4294967296" "" 4294967295 ERANGE
strtoll  "4294967296" "" 4294967296 OK
strtonll "4294967296" "" 4294967296 OK 0
strtoull "4294967296" "" 4294967296 OK

strtol   "-4294967296" "" -2147483648 ERANGE
strton   "-4294967296" "" -2147483648 ERANGE 0
strtoul  "-4294967296" "" 4294967295 ERANGE
strtoll  "-4294967296" "" -4294967296 OK
strtonll "-4294967296" "" -4294967296 OK 0
strtoull "-4294967296" "" 18446744069414584320 OK

strtol   "4294967297" "" 2147483647 ERANGE
strton   "4294967297" "" 2147483647 ERANGE 0
strtoul  "4294967297" "" 4294967295 ERANGE
strtoll  "4294967297" "" 4294967297 OK
strtonll "4294967297" "" 4294967297 OK 0
strtoull "4294967297" "" 4294967297 OK

strtol   "-4294967297" "" -2147483648 ERANGE
strton   "-4294967297" "" -2147483648 ERANGE 0
strtoul  "-4294967297" "" 4294967295 ERANGE
strtoll  "-4294967297" "" -4294967297 OK
strtonll "-4294967297" "" -4294967297 OK 0
strtoull "-4294967297" "" 18446744069414584319 OK'
	EXEC	9223372036854775807 -9223372036854775807 9223372036854775808 -9223372036854775808 9223372036854775809 -9223372036854775809
		OUTPUT - $'strtol   "9223372036854775807" "" 2147483647 ERANGE
strton   "9223372036854775807" "" 2147483647 ERANGE 0
strtoul  "9223372036854775807" "" 4294967295 ERANGE
strtoll  "9223372036854775807" "" 9223372036854775807 OK
strtonll "9223372036854775807" "" 9223372036854775807 OK 0
strtoull "9223372036854775807" "" 9223372036854775807 OK

strtol   "-9223372036854775807" "" -2147483648 ERANGE
strton   "-9223372036854775807" "" -2147483648 ERANGE 0
strtoul  "-9223372036854775807" "" 4294967295 ERANGE
strtoll  "-9223372036854775807" "" -9223372036854775807 OK
strtonll "-9223372036854775807" "" -9223372036854775807 OK 0
strtoull "-9223372036854775807" "" 9223372036854775809 OK

strtol   "9223372036854775808" "" 2147483647 ERANGE
strton   "9223372036854775808" "" 2147483647 ERANGE 0
strtoul  "9223372036854775808" "" 4294967295 ERANGE
strtoll  "9223372036854775808" "" 9223372036854775807 ERANGE
strtonll "9223372036854775808" "" 9223372036854775807 ERANGE 0
strtoull "9223372036854775808" "" 9223372036854775808 OK

strtol   "-9223372036854775808" "" -2147483648 ERANGE
strton   "-9223372036854775808" "" -2147483648 ERANGE 0
strtoul  "-9223372036854775808" "" 4294967295 ERANGE
strtoll  "-9223372036854775808" "" -9223372036854775808 OK
strtonll "-9223372036854775808" "" -9223372036854775808 OK 0
strtoull "-9223372036854775808" "" 9223372036854775808 OK

strtol   "9223372036854775809" "" 2147483647 ERANGE
strton   "9223372036854775809" "" 2147483647 ERANGE 0
strtoul  "9223372036854775809" "" 4294967295 ERANGE
strtoll  "9223372036854775809" "" 9223372036854775807 ERANGE
strtonll "9223372036854775809" "" 9223372036854775807 ERANGE 0
strtoull "9223372036854775809" "" 9223372036854775809 OK

strtol   "-9223372036854775809" "" -2147483648 ERANGE
strton   "-9223372036854775809" "" -2147483648 ERANGE 0
strtoul  "-9223372036854775809" "" 4294967295 ERANGE
strtoll  "-9223372036854775809" "" -9223372036854775808 ERANGE
strtonll "-9223372036854775809" "" -9223372036854775808 ERANGE 0
strtoull "-9223372036854775809" "" 9223372036854775807 OK'
	EXEC	18446744073709551615 -18446744073709551615 18446744073709551616 -18446744073709551616 18446744073709551617 -18446744073709551617
		OUTPUT - $'strtol   "18446744073709551615" "" 2147483647 ERANGE
strton   "18446744073709551615" "" 2147483647 ERANGE 0
strtoul  "18446744073709551615" "" 4294967295 ERANGE
strtoll  "18446744073709551615" "" 9223372036854775807 ERANGE
strtonll "18446744073709551615" "" 9223372036854775807 ERANGE 0
strtoull "18446744073709551615" "" 18446744073709551615 OK

strtol   "-18446744073709551615" "" -2147483648 ERANGE
strton   "-18446744073709551615" "" -2147483648 ERANGE 0
strtoul  "-18446744073709551615" "" 4294967295 ERANGE
strtoll  "-18446744073709551615" "" -9223372036854775808 ERANGE
strtonll "-18446744073709551615" "" -9223372036854775808 ERANGE 0
strtoull "-18446744073709551615" "" 18446744073709551615 ERANGE

strtol   "18446744073709551616" "" 2147483647 ERANGE
strton   "18446744073709551616" "" 2147483647 ERANGE 0
strtoul  "18446744073709551616" "" 4294967295 ERANGE
strtoll  "18446744073709551616" "" 9223372036854775807 ERANGE
strtonll "18446744073709551616" "" 9223372036854775807 ERANGE 0
strtoull "18446744073709551616" "" 18446744073709551615 ERANGE

strtol   "-18446744073709551616" "" -2147483648 ERANGE
strton   "-18446744073709551616" "" -2147483648 ERANGE 0
strtoul  "-18446744073709551616" "" 4294967295 ERANGE
strtoll  "-18446744073709551616" "" -9223372036854775808 ERANGE
strtonll "-18446744073709551616" "" -9223372036854775808 ERANGE 0
strtoull "-18446744073709551616" "" 18446744073709551615 ERANGE

strtol   "18446744073709551617" "" 2147483647 ERANGE
strton   "18446744073709551617" "" 2147483647 ERANGE 0
strtoul  "18446744073709551617" "" 4294967295 ERANGE
strtoll  "18446744073709551617" "" 9223372036854775807 ERANGE
strtonll "18446744073709551617" "" 9223372036854775807 ERANGE 0
strtoull "18446744073709551617" "" 18446744073709551615 ERANGE

strtol   "-18446744073709551617" "" -2147483648 ERANGE
strton   "-18446744073709551617" "" -2147483648 ERANGE 0
strtoul  "-18446744073709551617" "" 4294967295 ERANGE
strtoll  "-18446744073709551617" "" -9223372036854775808 ERANGE
strtonll "-18446744073709551617" "" -9223372036854775808 ERANGE 0
strtoull "-18446744073709551617" "" 18446744073709551615 ERANGE'

TEST 02 'hex'
	EXEC	0xffffffff0 0xfffffffff 0x17fffffff
		OUTPUT - $'strtol   "0xffffffff0" "" 2147483647 ERANGE
strton   "0xffffffff0" "" 2147483647 ERANGE 16
strtoul  "0xffffffff0" "" 4294967295 ERANGE
strtoll  "0xffffffff0" "" 68719476720 OK
strtonll "0xffffffff0" "" 68719476720 OK 16
strtoull "0xffffffff0" "" 68719476720 OK

strtol   "0xfffffffff" "" 2147483647 ERANGE
strton   "0xfffffffff" "" 2147483647 ERANGE 16
strtoul  "0xfffffffff" "" 4294967295 ERANGE
strtoll  "0xfffffffff" "" 68719476735 OK
strtonll "0xfffffffff" "" 68719476735 OK 16
strtoull "0xfffffffff" "" 68719476735 OK

strtol   "0x17fffffff" "" 2147483647 ERANGE
strton   "0x17fffffff" "" 2147483647 ERANGE 16
strtoul  "0x17fffffff" "" 4294967295 ERANGE
strtoll  "0x17fffffff" "" 6442450943 OK
strtonll "0x17fffffff" "" 6442450943 OK 16
strtoull "0x17fffffff" "" 6442450943 OK'

TEST 03 'thousands separator'
	EXEC	LC_ALL=debug 12345678 12.345.678 12.345678 12.345.67 1.234 123.456
		OUTPUT - $'strtol   "12345678" "" 12345678 OK
strton   "12345678" "" 12345678 OK 0
strtoul  "12345678" "" 12345678 OK
strtoll  "12345678" "" 12345678 OK
strtonll "12345678" "" 12345678 OK 0
strtoull "12345678" "" 12345678 OK

strtol   "12.345.678" "" 12345678 OK
strton   "12.345.678" "" 12345678 OK 0
strtoul  "12.345.678" "" 12345678 OK
strtoll  "12.345.678" "" 12345678 OK
strtonll "12.345.678" "" 12345678 OK 0
strtoull "12.345.678" "" 12345678 OK

strtol   "12.345678" ".345678" 12 OK
strton   "12.345678" ".345678" 12 OK 0
strtoul  "12.345678" ".345678" 12 OK
strtoll  "12.345678" ".345678" 12 OK
strtonll "12.345678" ".345678" 12 OK 0
strtoull "12.345678" ".345678" 12 OK

strtol   "12.345.67" ".67" 12345 OK
strton   "12.345.67" ".67" 12345 OK 0
strtoul  "12.345.67" ".67" 12345 OK
strtoll  "12.345.67" ".67" 12345 OK
strtonll "12.345.67" ".67" 12345 OK 0
strtoull "12.345.67" ".67" 12345 OK

strtol   "1.234" "" 1234 OK
strton   "1.234" "" 1234 OK 0
strtoul  "1.234" "" 1234 OK
strtoll  "1.234" "" 1234 OK
strtonll "1.234" "" 1234 OK 0
strtoull "1.234" "" 1234 OK

strtol   "123.456" "" 123456 OK
strton   "123.456" "" 123456 OK 0
strtoul  "123.456" "" 123456 OK
strtoll  "123.456" "" 123456 OK
strtonll "123.456" "" 123456 OK 0
strtoull "123.456" "" 123456 OK'

TEST 04 'malformations'
	EXEC - + 0x -0x +0x 11# -11# +11# 11#A 11#B
		OUTPUT - $'strtol   "-" "-" 0 OK
strton   "-" "-" 0 OK 10
strtoul  "-" "-" 0 OK
strtoll  "-" "-" 0 OK
strtonll "-" "-" 0 OK 10
strtoull "-" "-" 0 OK

strtol   "+" "+" 0 OK
strton   "+" "+" 0 OK 10
strtoul  "+" "+" 0 OK
strtoll  "+" "+" 0 OK
strtonll "+" "+" 0 OK 10
strtoull "+" "+" 0 OK

strtol   "0x" "x" 0 OK
strton   "0x" "x" 0 OK 10
strtoul  "0x" "x" 0 OK
strtoll  "0x" "x" 0 OK
strtonll "0x" "x" 0 OK 10
strtoull "0x" "x" 0 OK

strtol   "-0x" "x" 0 OK
strton   "-0x" "x" 0 OK 10
strtoul  "-0x" "x" 0 OK
strtoll  "-0x" "x" 0 OK
strtonll "-0x" "x" 0 OK 10
strtoull "-0x" "x" 0 OK

strtol   "+0x" "x" 0 OK
strton   "+0x" "x" 0 OK 10
strtoul  "+0x" "x" 0 OK
strtoll  "+0x" "x" 0 OK
strtonll "+0x" "x" 0 OK 10
strtoull "+0x" "x" 0 OK

strtol   "11#" "#" 0 OK
strton   "11#" "#" 0 OK 10
strtoul  "11#" "#" 0 OK
strtoll  "11#" "#" 0 OK
strtonll "11#" "#" 0 OK 10
strtoull "11#" "#" 0 OK

strtol   "-11#" "#" 0 OK
strton   "-11#" "#" 0 OK 10
strtoul  "-11#" "#" 0 OK
strtoll  "-11#" "#" 0 OK
strtonll "-11#" "#" 0 OK 10
strtoull "-11#" "#" 0 OK

strtol   "+11#" "#" 0 OK
strton   "+11#" "#" 0 OK 10
strtoul  "+11#" "#" 0 OK
strtoll  "+11#" "#" 0 OK
strtonll "+11#" "#" 0 OK 10
strtoull "+11#" "#" 0 OK

strtol   "11#A" "" 10 OK
strton   "11#A" "" 10 OK 11
strtoul  "11#A" "" 10 OK
strtoll  "11#A" "" 10 OK
strtonll "11#A" "" 10 OK 11
strtoull "11#A" "" 10 OK

strtol   "11#B" "#B" 0 OK
strton   "11#B" "#B" 0 OK 10
strtoul  "11#B" "#B" 0 OK
strtoll  "11#B" "#B" 0 OK
strtonll "11#B" "#B" 0 OK 10
strtoull "11#B" "#B" 0 OK'

TEST 05 'multiplier suffixes'
	EXEC 1b 1k 1m 1g 1t 1. 1.2 1.23 1.234 1.k 1.2k 1.23k 1.234k
		OUTPUT - $'strtol   "1b" "b" 1 OK
strton   "1b" "" 512 OK 0
strtoul  "1b" "b" 1 OK
strtoll  "1b" "b" 1 OK
strtonll "1b" "" 512 OK 0
strtoull "1b" "b" 1 OK

strtol   "1k" "k" 1 OK
strton   "1k" "" 1024 OK 0
strtoul  "1k" "k" 1 OK
strtoll  "1k" "k" 1 OK
strtonll "1k" "" 1024 OK 0
strtoull "1k" "k" 1 OK

strtol   "1m" "m" 1 OK
strton   "1m" "" 1048576 OK 0
strtoul  "1m" "m" 1 OK
strtoll  "1m" "m" 1 OK
strtonll "1m" "" 1048576 OK 0
strtoull "1m" "m" 1 OK

strtol   "1g" "g" 1 OK
strton   "1g" "" 1073741824 OK 0
strtoul  "1g" "g" 1 OK
strtoll  "1g" "g" 1 OK
strtonll "1g" "" 1073741824 OK 0
strtoull "1g" "g" 1 OK

strtol   "1t" "t" 1 OK
strton   "1t" "" 2147483647 ERANGE 0
strtoul  "1t" "t" 1 OK
strtoll  "1t" "t" 1 OK
strtonll "1t" "" 1099511627776 OK 0
strtoull "1t" "t" 1 OK

strtol   "1." "." 1 OK
strton   "1." "." 1 OK 0
strtoul  "1." "." 1 OK
strtoll  "1." "." 1 OK
strtonll "1." "." 1 OK 0
strtoull "1." "." 1 OK

strtol   "1.2" ".2" 1 OK
strton   "1.2" "" 120 OK 0
strtoul  "1.2" ".2" 1 OK
strtoll  "1.2" ".2" 1 OK
strtonll "1.2" "" 120 OK 0
strtoull "1.2" ".2" 1 OK

strtol   "1.23" ".23" 1 OK
strton   "1.23" "" 123 OK 0
strtoul  "1.23" ".23" 1 OK
strtoll  "1.23" ".23" 1 OK
strtonll "1.23" "" 123 OK 0
strtoull "1.23" ".23" 1 OK

strtol   "1.234" ".234" 1 OK
strton   "1.234" "" 123 OK 0
strtoul  "1.234" ".234" 1 OK
strtoll  "1.234" ".234" 1 OK
strtonll "1.234" "" 123 OK 0
strtoull "1.234" ".234" 1 OK

strtol   "1.k" ".k" 1 OK
strton   "1.k" ".k" 1 OK 0
strtoul  "1.k" ".k" 1 OK
strtoll  "1.k" ".k" 1 OK
strtonll "1.k" ".k" 1 OK 0
strtoull "1.k" ".k" 1 OK

strtol   "1.2k" ".2k" 1 OK
strton   "1.2k" "k" 120 OK 0
strtoul  "1.2k" ".2k" 1 OK
strtoll  "1.2k" ".2k" 1 OK
strtonll "1.2k" "k" 120 OK 0
strtoull "1.2k" ".2k" 1 OK

strtol   "1.23k" ".23k" 1 OK
strton   "1.23k" "k" 123 OK 0
strtoul  "1.23k" ".23k" 1 OK
strtoll  "1.23k" ".23k" 1 OK
strtonll "1.23k" "k" 123 OK 0
strtoull "1.23k" ".23k" 1 OK

strtol   "1.234k" ".234k" 1 OK
strton   "1.234k" "k" 123 OK 0
strtoul  "1.234k" ".234k" 1 OK
strtoll  "1.234k" ".234k" 1 OK
strtonll "1.234k" "k" 123 OK 0
strtoull "1.234k" ".234k" 1 OK'