# regression tests for the ast floating point strnto*() routines

TEST 01 'min/max'
	EXEC	14 1.17549435E-38 14 1.17549436E-38 14 1.17549437E-38
		OUTPUT - $'strntod   "1.17549435E-38" "" 1.175494350000000e-38 OK
strntold  "1.17549435E-38" "" 1.1754943500000000000000000000000e-38 OK

strntod   "1.17549436E-38" "" 1.175494360000000e-38 OK
strntold  "1.17549436E-38" "" 1.1754943600000000000000000000000e-38 OK

strntod   "1.17549437E-38" "" 1.175494370000000e-38 OK
strntold  "1.17549437E-38" "" 1.1754943700000000000000000000000e-38 OK'
	EXEC	14 3.40282347E+38 14 3.40282348E+38 14 3.40282349E+38
		OUTPUT - $'strntod   "3.40282347E+38" "" 3.402823470000000e+38 OK
strntold  "3.40282347E+38" "" 3.4028234700000000000000000000000e+38 OK

strntod   "3.40282348E+38" "" 3.402823480000000e+38 OK
strntold  "3.40282348E+38" "" 3.4028234800000000000000000000000e+38 OK

strntod   "3.40282349E+38" "" 3.402823490000000e+38 OK
strntold  "3.40282349E+38" "" 3.4028234900000000000000000000000e+38 OK'
	EXEC	23 2.2250738585072014E-308 23 2.2250738585072015E-308 23 2.2250738585072016E-308
		OUTPUT - $'strntod   "2.2250738585072014E-308" "" 2.225073858507200e-308 OK
strntold  "2.2250738585072014E-308" "" 2.2250738585072013830902327173320e-308 OK

strntod   "2.2250738585072015E-308" "" 2.225073858507200e-308 OK
strntold  "2.2250738585072015E-308" "" 2.2250738585072013830902327173320e-308 OK

strntod   "2.2250738585072016E-308" "" 2.225073858507200e-308 OK
strntold  "2.2250738585072016E-308" "" 2.2250738585072013830902327173320e-308 OK'
	EXEC	23 1.7976931348623157E+308 23 1.7976931348623158E+308 23 1.7976931348623159E+308
		OUTPUT - $'strntod   "1.7976931348623157E+308" "" 1.797693134862320e+308 OK
strntold  "1.7976931348623157E+308" "" 1.7976931348623157000000000000000e+308 OK

strntod   "1.7976931348623158E+308" "" Inf ERANGE
strntold  "1.7976931348623158E+308" "" 1.7976931348623158000000000000000e+308 OK

strntod   "1.7976931348623159E+308" "" Inf ERANGE
strntold  "1.7976931348623159E+308" "" Inf ERANGE'
	EXEC	40 2.225073858507201383090232717332404E-308 40 2.225073858507201383090232717332405E-308 40 2.225073858507201383090232717332406E-308
		OUTPUT - $'strntod   "2.225073858507201383090232717332404E-308" "" 2.225073858507200e-308 OK
strntold  "2.225073858507201383090232717332404E-308" "" 2.2250738585072013830902327173320e-308 OK

strntod   "2.225073858507201383090232717332405E-308" "" 2.225073858507200e-308 OK
strntold  "2.225073858507201383090232717332405E-308" "" 2.2250738585072013830902327173320e-308 OK

strntod   "2.225073858507201383090232717332406E-308" "" 2.225073858507200e-308 OK
strntold  "2.225073858507201383090232717332406E-308" "" 2.2250738585072013830902327173320e-308 OK'
	EXEC	40 1.797693134862315807937289714053023E+308 40 1.797693134862315807937289714053024E+308 40 1.797693134862315807937289714053025E+308
		OUTPUT - $'strntod   "1.797693134862315807937289714053023E+308" "" Inf ERANGE
strntold  "1.797693134862315807937289714053023E+308" "" Inf ERANGE

strntod   "1.797693134862315807937289714053024E+308" "" Inf ERANGE
strntold  "1.797693134862315807937289714053024E+308" "" Inf ERANGE

strntod   "1.797693134862315807937289714053025E+308" "" Inf ERANGE
strntold  "1.797693134862315807937289714053025E+308" "" Inf ERANGE'

TEST 02 'optional suffix'
	EXEC	15 1.17549435E-38F
		OUTPUT - $'strntod   "1.17549435E-38F" "" 1.175494350000000e-38 OK
strntold  "1.17549435E-38F" "" 1.1754943500000000000000000000000e-38 OK'
	EXEC	41 2.225073858507201383090232717332404E-308L
		OUTPUT - $'strntod   "2.225073858507201383090232717332404E-308L" "" 2.225073858507200e-308 OK
strntold  "2.225073858507201383090232717332404E-308L" "" 2.2250738585072013830902327173320e-308 OK'

TEST 03 'hexadecimal floating point'
	EXEC	23 0x1.fffffffffffff7p1023 23 0x1.fffffffffffff8p1023
		OUTPUT - $'strntod   "0x1.fffffffffffff7p1023" "" 1.797693134862320e+308 OK
strntold  "0x1.fffffffffffff7p1023" "" 1.7976931348623157954632877794610e+308 OK

strntod   "0x1.fffffffffffff8p1023" "" Inf ERANGE
strntold  "0x1.fffffffffffff8p1023" "" Inf ERANGE'
	EXEC	9 0x1p+1023 10 -0x1p+1023 9 0x1p-1021 10 -0x1p-1021
		OUTPUT - $'strntod   "0x1p+1023" "" 8.988465674311580e+307 OK
strntold  "0x1p+1023" "" 8.9884656743115795386465259539450e+307 OK

strntod   "-0x1p+1023" "" -8.988465674311580e+307 OK
strntold  "-0x1p+1023" "" -8.9884656743115795386465259539450e+307 OK

strntod   "0x1p-1021" "" 4.450147717014400e-308 OK
strntold  "0x1p-1021" "" 4.4501477170144027661804654346650e-308 OK

strntod   "-0x1p-1021" "" -4.450147717014400e-308 OK
strntold  "-0x1p-1021" "" -4.4501477170144027661804654346650e-308 OK'
	EXEC	9 0x1p+1023 10 -0x1p+1023 9 0x1p+1024 10 -0x1p+1024 9 0x1p-1022 10 -0x1p-1022 9 0x1p-1023 10 -0x1p-1023
		OUTPUT - $'strntod   "0x1p+1023" "" 8.988465674311580e+307 OK
strntold  "0x1p+1023" "" 8.9884656743115795386465259539450e+307 OK

strntod   "-0x1p+1023" "" -8.988465674311580e+307 OK
strntold  "-0x1p+1023" "" -8.9884656743115795386465259539450e+307 OK

strntod   "0x1p+1024" "" Inf ERANGE
strntold  "0x1p+1024" "" Inf ERANGE

strntod   "-0x1p+1024" "" -Inf ERANGE
strntold  "-0x1p+1024" "" -Inf ERANGE

strntod   "0x1p-1022" "" 2.225073858507200e-308 OK
strntold  "0x1p-1022" "" 2.2250738585072013830902327173320e-308 OK

strntod   "-0x1p-1022" "" -2.225073858507200e-308 OK
strntold  "-0x1p-1022" "" -2.2250738585072013830902327173320e-308 OK

strntod   "0x1p-1023" "" 0.000000000000000e+00 ERANGE
strntold  "0x1p-1023" "" 0.0000000000000000000000000000000e+00 ERANGE

strntod   "-0x1p-1023" "" 0.000000000000000e+00 ERANGE
strntold  "-0x1p-1023" "" 0.0000000000000000000000000000000e+00 ERANGE'
	EXEC	7 0x1p127 8 0x1.p127 9 0x1.0p127 10 0x.1p131 11 0x0.1p131 12 0x0.10p131
		OUTPUT - $'strntod   "0x1p127" "" 1.701411834604690e+38 OK
strntold  "0x1p127" "" 1.7014118346046923173168730371590e+38 OK

strntod   "0x1.p127" "" 1.701411834604690e+38 OK
strntold  "0x1.p127" "" 1.7014118346046923173168730371590e+38 OK

strntod   "0x1.0p127" "" 1.701411834604690e+38 OK
strntold  "0x1.0p127" "" 1.7014118346046923173168730371590e+38 OK

strntod   "0x.1p131" "" 1.701411834604690e+38 OK
strntold  "0x.1p131" "" 1.7014118346046923173168730371590e+38 OK

strntod   "0x0.1p131" "" 1.701411834604690e+38 OK
strntold  "0x0.1p131" "" 1.7014118346046923173168730371590e+38 OK

strntod   "0x0.10p131" "" 1.701411834604690e+38 OK
strntold  "0x0.10p131" "" 1.7014118346046923173168730371590e+38 OK'
	EXEC	11 0x12345p127 17 0x12345.6789ap127 23 1.26866461572665980e+43
		OUTPUT - $'strntod   "0x12345p127" "" 1.268657734472990e+43 OK
strntold  "0x12345p127" "" 1.2686577344729888264073263801570e+43 OK

strntod   "0x12345.6789ap127" "" 1.268664615726660e+43 OK
strntold  "0x12345.6789ap127" "" 1.2686646157266598765168544535890e+43 OK

strntod   "1.26866461572665980e+43" "" 1.268664615726660e+43 OK
strntold  "1.26866461572665980e+43" "" 1.2686646157266598000000000000000e+43 OK'

TEST 04 'to infinity and beyond'
	EXEC	3 inf 4 +inf 4 -inf
		OUTPUT - $'strntod   "inf" "" Inf OK
strntold  "inf" "" Inf OK

strntod   "+inf" "" Inf OK
strntold  "+inf" "" Inf OK

strntod   "-inf" "" -Inf OK
strntold  "-inf" "" -Inf OK'
	EXEC	3 Inf 4 +Inf 4 -Inf
		OUTPUT - $'strntod   "Inf" "" Inf OK
strntold  "Inf" "" Inf OK

strntod   "+Inf" "" Inf OK
strntold  "+Inf" "" Inf OK

strntod   "-Inf" "" -Inf OK
strntold  "-Inf" "" -Inf OK'
	EXEC	8 InFiNiTy 9 +InFiNiTy 9 -InFiNiTy
		OUTPUT - $'strntod   "InFiNiTy" "" Inf OK
strntold  "InFiNiTy" "" Inf OK

strntod   "+InFiNiTy" "" Inf OK
strntold  "+InFiNiTy" "" Inf OK

strntod   "-InFiNiTy" "" -Inf OK
strntold  "-InFiNiTy" "" -Inf OK'
	EXEC	4 infi 5 +infi 5 -infi
		OUTPUT - $'strntod   "infi" "i" Inf OK
strntold  "infi" "i" Inf OK

strntod   "+infi" "i" Inf OK
strntold  "+infi" "i" Inf OK

strntod   "-infi" "i" -Inf OK
strntold  "-infi" "i" -Inf OK'
	EXEC	3 infi 4 +infi 4 -infi
		OUTPUT - $'strntod   "infi" "i" Inf OK
strntold  "infi" "i" Inf OK

strntod   "+infi" "i" Inf OK
strntold  "+infi" "i" Inf OK

strntod   "-infi" "i" -Inf OK
strntold  "-infi" "i" -Inf OK'
	EXEC	5 infin 6 +infin 6 -infin
		OUTPUT - $'strntod   "infin" "in" Inf OK
strntold  "infin" "in" Inf OK

strntod   "+infin" "in" Inf OK
strntold  "+infin" "in" Inf OK

strntod   "-infin" "in" -Inf OK
strntold  "-infin" "in" -Inf OK'
	EXEC	3 infin 4 +infin 4 -infin
		OUTPUT - $'strntod   "infin" "in" Inf OK
strntold  "infin" "in" Inf OK

strntod   "+infin" "in" Inf OK
strntold  "+infin" "in" Inf OK

strntod   "-infin" "in" -Inf OK
strntold  "-infin" "in" -Inf OK'
	EXEC	2 in 3 +in 3 -in
		OUTPUT - $'strntod   "in" "in" 0.000000000000000e+00 OK
strntold  "in" "in" 0.0000000000000000000000000000000e+00 OK

strntod   "+in" "+in" 0.000000000000000e+00 OK
strntold  "+in" "+in" 0.0000000000000000000000000000000e+00 OK

strntod   "-in" "-in" 0.000000000000000e+00 OK
strntold  "-in" "-in" 0.0000000000000000000000000000000e+00 OK'
	EXEC	3 in0 4 +in0 4 -in0
		OUTPUT - $'strntod   "in0" "in0" 0.000000000000000e+00 OK
strntold  "in0" "in0" 0.0000000000000000000000000000000e+00 OK

strntod   "+in0" "+in0" 0.000000000000000e+00 OK
strntold  "+in0" "+in0" 0.0000000000000000000000000000000e+00 OK

strntod   "-in0" "-in0" 0.000000000000000e+00 OK
strntold  "-in0" "-in0" 0.0000000000000000000000000000000e+00 OK'
	EXEC	3 NaN 4 +NaN 4 -NaN
		OUTPUT - $'strntod   "NaN" "" NaN OK
strntold  "NaN" "" NaN OK

strntod   "+NaN" "" NaN OK
strntold  "+NaN" "" NaN OK

strntod   "-NaN" "" NaN OK
strntold  "-NaN" "" NaN OK'
	EXEC	15 NaN12-34abc.def 16 +NaN12-34abc.def 16 -NaN12-34abc.def
		OUTPUT - $'strntod   "NaN12-34abc.def" "" NaN OK
strntold  "NaN12-34abc.def" "" NaN OK

strntod   "+NaN12-34abc.def" "" NaN OK
strntold  "+NaN12-34abc.def" "" NaN OK

strntod   "-NaN12-34abc.def" "" NaN OK
strntold  "-NaN12-34abc.def" "" NaN OK'
	EXEC	3 NaN12-34abc.def 4 +NaN12-34abc.def 4 -NaN12-34abc.def
		OUTPUT - $'strntod   "NaN12-34abc.def" "12-34abc.def" NaN OK
strntold  "NaN12-34abc.def" "12-34abc.def" NaN OK

strntod   "+NaN12-34abc.def" "12-34abc.def" NaN OK
strntold  "+NaN12-34abc.def" "12-34abc.def" NaN OK

strntod   "-NaN12-34abc.def" "12-34abc.def" NaN OK
strntold  "-NaN12-34abc.def" "12-34abc.def" NaN OK'

TEST 05 'simple, right?'
	EXEC	1 1 2 12 3 1.2 5 1.2.3
		OUTPUT - $'strntod   "1" "" 1.000000000000000e+00 OK
strntold  "1" "" 1.0000000000000000000000000000000e+00 OK

strntod   "12" "" 1.200000000000000e+01 OK
strntold  "12" "" 1.2000000000000000000000000000000e+01 OK

strntod   "1.2" "" 1.200000000000000e+00 OK
strntold  "1.2" "" 1.2000000000000000000000000000000e+00 OK

strntod   "1.2.3" ".3" 1.200000000000000e+00 OK
strntold  "1.2.3" ".3" 1.2000000000000000000000000000000e+00 OK'
