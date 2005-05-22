# regression tests for the ast floating point strto*() routines

TEST 01 'min/max'
	EXEC	1.17549435E-38 1.17549436E-38 1.17549437E-38
		OUTPUT - $'strtod   "1.17549435E-38" "" 1.175494350000000e-38 OK
strtold  "1.17549435E-38" "" 1.1754943500000000000000000000000e-38 OK

strtod   "1.17549436E-38" "" 1.175494360000000e-38 OK
strtold  "1.17549436E-38" "" 1.1754943600000000000000000000000e-38 OK

strtod   "1.17549437E-38" "" 1.175494370000000e-38 OK
strtold  "1.17549437E-38" "" 1.1754943700000000000000000000000e-38 OK'
	EXEC	3.40282347E+38 3.40282348E+38 3.40282349E+38
		OUTPUT - $'strtod   "3.40282347E+38" "" 3.402823470000000e+38 OK
strtold  "3.40282347E+38" "" 3.4028234700000000000000000000000e+38 OK

strtod   "3.40282348E+38" "" 3.402823480000000e+38 OK
strtold  "3.40282348E+38" "" 3.4028234800000000000000000000000e+38 OK

strtod   "3.40282349E+38" "" 3.402823490000000e+38 OK
strtold  "3.40282349E+38" "" 3.4028234900000000000000000000000e+38 OK'
	EXEC	2.2250738585072014E-308 2.2250738585072015E-308 2.2250738585072016E-308
		OUTPUT - $'strtod   "2.2250738585072014E-308" "" 2.225073858507200e-308 OK
strtold  "2.2250738585072014E-308" "" 2.2250738585072013830902327173320e-308 OK

strtod   "2.2250738585072015E-308" "" 2.225073858507200e-308 OK
strtold  "2.2250738585072015E-308" "" 2.2250738585072013830902327173320e-308 OK

strtod   "2.2250738585072016E-308" "" 2.225073858507200e-308 OK
strtold  "2.2250738585072016E-308" "" 2.2250738585072013830902327173320e-308 OK'
	EXEC	1.7976931348623157E+308 1.7976931348623158E+308 1.7976931348623159E+308
		OUTPUT - $'strtod   "1.7976931348623157E+308" "" 1.797693134862320e+308 OK
strtold  "1.7976931348623157E+308" "" 1.7976931348623157000000000000000e+308 OK

strtod   "1.7976931348623158E+308" "" Inf ERANGE
strtold  "1.7976931348623158E+308" "" 1.7976931348623158000000000000000e+308 OK

strtod   "1.7976931348623159E+308" "" Inf ERANGE
strtold  "1.7976931348623159E+308" "" Inf ERANGE'
	EXEC	2.225073858507201383090232717332404E-308 2.225073858507201383090232717332405E-308 2.225073858507201383090232717332406E-308
		OUTPUT - $'strtod   "2.225073858507201383090232717332404E-308" "" 2.225073858507200e-308 OK
strtold  "2.225073858507201383090232717332404E-308" "" 2.2250738585072013830902327173320e-308 OK

strtod   "2.225073858507201383090232717332405E-308" "" 2.225073858507200e-308 OK
strtold  "2.225073858507201383090232717332405E-308" "" 2.2250738585072013830902327173320e-308 OK

strtod   "2.225073858507201383090232717332406E-308" "" 2.225073858507200e-308 OK
strtold  "2.225073858507201383090232717332406E-308" "" 2.2250738585072013830902327173320e-308 OK'
	EXEC	1.797693134862315807937289714053023E+308 1.797693134862315807937289714053024E+308 1.797693134862315807937289714053025E+308
		OUTPUT - $'strtod   "1.797693134862315807937289714053023E+308" "" Inf ERANGE
strtold  "1.797693134862315807937289714053023E+308" "" Inf ERANGE

strtod   "1.797693134862315807937289714053024E+308" "" Inf ERANGE
strtold  "1.797693134862315807937289714053024E+308" "" Inf ERANGE

strtod   "1.797693134862315807937289714053025E+308" "" Inf ERANGE
strtold  "1.797693134862315807937289714053025E+308" "" Inf ERANGE'

TEST 02 'optional suffix'
	EXEC	1.17549435E-38F
		OUTPUT - $'strtod   "1.17549435E-38F" "" 1.175494350000000e-38 OK
strtold  "1.17549435E-38F" "" 1.1754943500000000000000000000000e-38 OK'
	EXEC	2.225073858507201383090232717332404E-308L
		OUTPUT - $'strtod   "2.225073858507201383090232717332404E-308L" "" 2.225073858507200e-308 OK
strtold  "2.225073858507201383090232717332404E-308L" "" 2.2250738585072013830902327173320e-308 OK'

TEST 03 'hexadecimal floating point'
	EXEC	0x1.fffffffffffff7p1023 0x1.fffffffffffff8p1023
		OUTPUT - $'strtod   "0x1.fffffffffffff7p1023" "" 1.797693134862320e+308 OK
strtold  "0x1.fffffffffffff7p1023" "" 1.7976931348623157954632877794610e+308 OK

strtod   "0x1.fffffffffffff8p1023" "" Inf ERANGE
strtold  "0x1.fffffffffffff8p1023" "" Inf ERANGE'
	EXEC	0x1p+1023 -0x1p+1023 0x1p-1021 -0x1p-1021
		OUTPUT - $'strtod   "0x1p+1023" "" 8.988465674311580e+307 OK
strtold  "0x1p+1023" "" 8.9884656743115795386465259539450e+307 OK

strtod   "-0x1p+1023" "" -8.988465674311580e+307 OK
strtold  "-0x1p+1023" "" -8.9884656743115795386465259539450e+307 OK

strtod   "0x1p-1021" "" 4.450147717014400e-308 OK
strtold  "0x1p-1021" "" 4.4501477170144027661804654346650e-308 OK

strtod   "-0x1p-1021" "" -4.450147717014400e-308 OK
strtold  "-0x1p-1021" "" -4.4501477170144027661804654346650e-308 OK'
	EXEC	0x1p+1023 -0x1p+1023 0x1p+1024 -0x1p+1024 0x1p-1022 -0x1p-1022 0x1p-1023 -0x1p-1023
		OUTPUT - $'strtod   "0x1p+1023" "" 8.988465674311580e+307 OK
strtold  "0x1p+1023" "" 8.9884656743115795386465259539450e+307 OK

strtod   "-0x1p+1023" "" -8.988465674311580e+307 OK
strtold  "-0x1p+1023" "" -8.9884656743115795386465259539450e+307 OK

strtod   "0x1p+1024" "" Inf ERANGE
strtold  "0x1p+1024" "" Inf ERANGE

strtod   "-0x1p+1024" "" -Inf ERANGE
strtold  "-0x1p+1024" "" -Inf ERANGE

strtod   "0x1p-1022" "" 2.225073858507200e-308 OK
strtold  "0x1p-1022" "" 2.2250738585072013830902327173320e-308 OK

strtod   "-0x1p-1022" "" -2.225073858507200e-308 OK
strtold  "-0x1p-1022" "" -2.2250738585072013830902327173320e-308 OK

strtod   "0x1p-1023" "" 0.000000000000000e+00 ERANGE
strtold  "0x1p-1023" "" 0.0000000000000000000000000000000e+00 ERANGE

strtod   "-0x1p-1023" "" 0.000000000000000e+00 ERANGE
strtold  "-0x1p-1023" "" 0.0000000000000000000000000000000e+00 ERANGE'
	EXEC	0x1p127 0x1.p127 0x1.0p127 0x.1p131 0x0.1p131 0x0.10p131
		OUTPUT - $'strtod   "0x1p127" "" 1.701411834604690e+38 OK
strtold  "0x1p127" "" 1.7014118346046923173168730371590e+38 OK

strtod   "0x1.p127" "" 1.701411834604690e+38 OK
strtold  "0x1.p127" "" 1.7014118346046923173168730371590e+38 OK

strtod   "0x1.0p127" "" 1.701411834604690e+38 OK
strtold  "0x1.0p127" "" 1.7014118346046923173168730371590e+38 OK

strtod   "0x.1p131" "" 1.701411834604690e+38 OK
strtold  "0x.1p131" "" 1.7014118346046923173168730371590e+38 OK

strtod   "0x0.1p131" "" 1.701411834604690e+38 OK
strtold  "0x0.1p131" "" 1.7014118346046923173168730371590e+38 OK

strtod   "0x0.10p131" "" 1.701411834604690e+38 OK
strtold  "0x0.10p131" "" 1.7014118346046923173168730371590e+38 OK'
	EXEC	0x12345p127 0x12345.6789ap127 1.26866461572665980e+43
		OUTPUT - $'strtod   "0x12345p127" "" 1.268657734472990e+43 OK
strtold  "0x12345p127" "" 1.2686577344729888264073263801570e+43 OK

strtod   "0x12345.6789ap127" "" 1.268664615726660e+43 OK
strtold  "0x12345.6789ap127" "" 1.2686646157266598765168544535890e+43 OK

strtod   "1.26866461572665980e+43" "" 1.268664615726660e+43 OK
strtold  "1.26866461572665980e+43" "" 1.2686646157266598000000000000000e+43 OK'

TEST 04 'to infinity and beyond'
	EXEC	inf +inf -inf
		OUTPUT - $'strtod   "inf" "" Inf OK
strtold  "inf" "" Inf OK

strtod   "+inf" "" Inf OK
strtold  "+inf" "" Inf OK

strtod   "-inf" "" -Inf OK
strtold  "-inf" "" -Inf OK'
	EXEC	Inf +Inf -Inf
		OUTPUT - $'strtod   "Inf" "" Inf OK
strtold  "Inf" "" Inf OK

strtod   "+Inf" "" Inf OK
strtold  "+Inf" "" Inf OK

strtod   "-Inf" "" -Inf OK
strtold  "-Inf" "" -Inf OK'
	EXEC	InFiNiTy +InFiNiTy -InFiNiTy
		OUTPUT - $'strtod   "InFiNiTy" "" Inf OK
strtold  "InFiNiTy" "" Inf OK

strtod   "+InFiNiTy" "" Inf OK
strtold  "+InFiNiTy" "" Inf OK

strtod   "-InFiNiTy" "" -Inf OK
strtold  "-InFiNiTy" "" -Inf OK'
	EXEC	infi +infi -infi
		OUTPUT - $'strtod   "infi" "i" Inf OK
strtold  "infi" "i" Inf OK

strtod   "+infi" "i" Inf OK
strtold  "+infi" "i" Inf OK

strtod   "-infi" "i" -Inf OK
strtold  "-infi" "i" -Inf OK'
	EXEC	in +in -in
		OUTPUT - $'strtod   "in" "in" 0.000000000000000e+00 OK
strtold  "in" "in" 0.0000000000000000000000000000000e+00 OK

strtod   "+in" "+in" 0.000000000000000e+00 OK
strtold  "+in" "+in" 0.0000000000000000000000000000000e+00 OK

strtod   "-in" "-in" 0.000000000000000e+00 OK
strtold  "-in" "-in" 0.0000000000000000000000000000000e+00 OK'
	EXEC	NaN +NaN -NaN
		OUTPUT - $'strtod   "NaN" "" NaN OK
strtold  "NaN" "" NaN OK

strtod   "+NaN" "" NaN OK
strtold  "+NaN" "" NaN OK

strtod   "-NaN" "" NaN OK
strtold  "-NaN" "" NaN OK'
	EXEC	NaN12-34abc.def +NaN12-34abc.def -NaN12-34abc.def
		OUTPUT - $'strtod   "NaN12-34abc.def" "" NaN OK
strtold  "NaN12-34abc.def" "" NaN OK

strtod   "+NaN12-34abc.def" "" NaN OK
strtold  "+NaN12-34abc.def" "" NaN OK

strtod   "-NaN12-34abc.def" "" NaN OK
strtold  "-NaN12-34abc.def" "" NaN OK'

TEST 05 'simple, right?'
	EXEC	1 12 1.2 1.2.3
		OUTPUT - $'strtod   "1" "" 1.000000000000000e+00 OK
strtold  "1" "" 1.0000000000000000000000000000000e+00 OK

strtod   "12" "" 1.200000000000000e+01 OK
strtold  "12" "" 1.2000000000000000000000000000000e+01 OK

strtod   "1.2" "" 1.200000000000000e+00 OK
strtold  "1.2" "" 1.2000000000000000000000000000000e+00 OK

strtod   "1.2.3" ".3" 1.200000000000000e+00 OK
strtold  "1.2.3" ".3" 1.2000000000000000000000000000000e+00 OK'
