# : : generated from strtof-6.128-15.1024-33.16384.rt by mktest : : #

TEST 01 'min/max boundaries'

	EXEC	2.225073858507201e-308 2.225073858507202e-308 2.225073858507203e-308
		OUTPUT - $'strtod   "2.225073858507201e-308" "" 0.00000000000000e+00 ERANGE
strtold  "2.225073858507201e-308" "" 2.22507385850720100000000000000000e-308'\
$' OK
strntod  22 "2.225073858507201e-308" "" 0.00000000000000e+00 ERANGE
strntod  21 "2.225073858507201e-30" "8" 2.22507385850720e-30 OK
strntold 22 "2.225073858507201e-308" "" 2.22507385850720100000000000000000e-'\
$'308 OK
strntold 21 "2.225073858507201e-30" "8" 2.22507385850720100000000000000000e-'\
$'30 OK

strtod   "2.225073858507202e-308" "" 2.22507385850720e-308 OK
strtold  "2.225073858507202e-308" "" 2.22507385850720200000000000000000e-308'\
$' OK
strntod  22 "2.225073858507202e-308" "" 2.22507385850720e-308 OK
strntod  21 "2.225073858507202e-30" "8" 2.22507385850720e-30 OK
strntold 22 "2.225073858507202e-308" "" 2.22507385850720200000000000000000e-'\
$'308 OK
strntold 21 "2.225073858507202e-30" "8" 2.22507385850720200000000000000000e-'\
$'30 OK

strtod   "2.225073858507203e-308" "" 2.22507385850720e-308 OK
strtold  "2.225073858507203e-308" "" 2.22507385850720300000000000000000e-308'\
$' OK
strntod  22 "2.225073858507203e-308" "" 2.22507385850720e-308 OK
strntod  21 "2.225073858507203e-30" "8" 2.22507385850720e-30 OK
strntold 22 "2.225073858507203e-308" "" 2.22507385850720300000000000000000e-'\
$'308 OK
strntold 21 "2.225073858507203e-30" "8" 2.22507385850720300000000000000000e-'\
'30 OK'

	EXEC	1.797693134862314e+308 1.797693134862315e+308 1.797693134862316e+308
		OUTPUT - $'strtod   "1.797693134862314e+308" "" 1.79769313486231e+308 OK
strtold  "1.797693134862314e+308" "" 1.79769313486231400000000000000000e+308'\
$' OK
strntod  22 "1.797693134862314e+308" "" 1.79769313486231e+308 OK
strntod  21 "1.797693134862314e+30" "8" 1.79769313486231e+30 OK
strntold 22 "1.797693134862314e+308" "" 1.79769313486231400000000000000000e+'\
$'308 OK
strntold 21 "1.797693134862314e+30" "8" 1.79769313486231400000000000000000e+'\
$'30 OK

strtod   "1.797693134862315e+308" "" 1.79769313486232e+308 OK
strtold  "1.797693134862315e+308" "" 1.79769313486231500000000000000000e+308'\
$' OK
strntod  22 "1.797693134862315e+308" "" 1.79769313486232e+308 OK
strntod  21 "1.797693134862315e+30" "8" 1.79769313486231e+30 OK
strntold 22 "1.797693134862315e+308" "" 1.79769313486231500000000000000000e+'\
$'308 OK
strntold 21 "1.797693134862315e+30" "8" 1.79769313486231500000000000000000e+'\
$'30 OK

strtod   "1.797693134862316e+308" "" inf ERANGE
strtold  "1.797693134862316e+308" "" 1.79769313486231600000000000000000e+308'\
$' OK
strntod  22 "1.797693134862316e+308" "" inf ERANGE
strntod  21 "1.797693134862316e+30" "8" 1.79769313486232e+30 OK
strntold 22 "1.797693134862316e+308" "" 1.79769313486231600000000000000000e+'\
$'308 OK
strntold 21 "1.797693134862316e+30" "8" 1.79769313486231600000000000000000e+'\
'30 OK'

	EXEC	3.362103143112093506262677817321752E-4932 3.362103143112093506262677817321753E-4932 3.362103143112093506262677817321754E-4932
		OUTPUT - $'strtod   "3.362103143112093506262677817321752E-4932" "" 0.00000000000000e+0'\
$'0 ERANGE
strtold  "3.362103143112093506262677817321752E-4932" "" 0.000000000000000000'\
$'00000000000000e+00 ERANGE
strntod  41 "3.362103143112093506262677817321752E-4932" "" 0.00000000000000e'\
$'+00 ERANGE
strntod  40 "3.362103143112093506262677817321752E-493" "2" 0.00000000000000e'\
$'+00 ERANGE
strntold 41 "3.362103143112093506262677817321752E-4932" "" 0.000000000000000'\
$'00000000000000000e+00 ERANGE
strntold 40 "3.362103143112093506262677817321752E-493" "2" 3.362103143112093'\
$'50626267781732175e-493 OK

strtod   "3.362103143112093506262677817321753E-4932" "" 0.00000000000000e+00'\
$' ERANGE
strtold  "3.362103143112093506262677817321753E-4932" "" 3.362103143112093506'\
$'26267781732175e-4932 OK
strntod  41 "3.362103143112093506262677817321753E-4932" "" 0.00000000000000e'\
$'+00 ERANGE
strntod  40 "3.362103143112093506262677817321753E-493" "2" 0.00000000000000e'\
$'+00 ERANGE
strntold 41 "3.362103143112093506262677817321753E-4932" "" 3.362103143112093'\
$'50626267781732175e-4932 OK
strntold 40 "3.362103143112093506262677817321753E-493" "2" 3.362103143112093'\
$'50626267781732175e-493 OK

strtod   "3.362103143112093506262677817321754E-4932" "" 0.00000000000000e+00'\
$' ERANGE
strtold  "3.362103143112093506262677817321754E-4932" "" 3.362103143112093506'\
$'26267781732175e-4932 OK
strntod  41 "3.362103143112093506262677817321754E-4932" "" 0.00000000000000e'\
$'+00 ERANGE
strntod  40 "3.362103143112093506262677817321754E-493" "2" 0.00000000000000e'\
$'+00 ERANGE
strntold 41 "3.362103143112093506262677817321754E-4932" "" 3.362103143112093'\
$'50626267781732175e-4932 OK
strntold 40 "3.362103143112093506262677817321754E-493" "2" 3.362103143112093'\
'50626267781732175e-493 OK'

	EXEC	1.189731495357231765085759326628006E+4932 1.189731495357231765085759326628007E+4932 1.189731495357231765085759326628008E+4932
		OUTPUT - $'strtod   "1.189731495357231765085759326628006E+4932" "" inf ERANGE
strtold  "1.189731495357231765085759326628006E+4932" "" 1.189731495357231765'\
$'08575932662802e+4932 OK
strntod  41 "1.189731495357231765085759326628006E+4932" "" inf ERANGE
strntod  40 "1.189731495357231765085759326628006E+493" "2" inf ERANGE
strntold 41 "1.189731495357231765085759326628006E+4932" "" 1.189731495357231'\
$'76508575932662802e+4932 OK
strntold 40 "1.189731495357231765085759326628006E+493" "2" 1.189731495357231'\
$'76508575932662801e+493 OK

strtod   "1.189731495357231765085759326628007E+4932" "" inf ERANGE
strtold  "1.189731495357231765085759326628007E+4932" "" 1.189731495357231765'\
$'08575932662802e+4932 OK
strntod  41 "1.189731495357231765085759326628007E+4932" "" inf ERANGE
strntod  40 "1.189731495357231765085759326628007E+493" "2" inf ERANGE
strntold 41 "1.189731495357231765085759326628007E+4932" "" 1.189731495357231'\
$'76508575932662802e+4932 OK
strntold 40 "1.189731495357231765085759326628007E+493" "2" 1.189731495357231'\
$'76508575932662801e+493 OK

strtod   "1.189731495357231765085759326628008E+4932" "" inf ERANGE
strtold  "1.189731495357231765085759326628008E+4932" "" inf ERANGE
strntod  41 "1.189731495357231765085759326628008E+4932" "" inf ERANGE
strntod  40 "1.189731495357231765085759326628008E+493" "2" inf ERANGE
strntold 41 "1.189731495357231765085759326628008E+4932" "" inf ERANGE
strntold 40 "1.189731495357231765085759326628008E+493" "2" 1.189731495357231'\
'76508575932662801e+493 OK'

TEST 02 'optional suffix'

	EXEC	1.1754943E-38F 3.4028234e+38F
		OUTPUT - $'strtod   "1.1754943E-38F" "" 1.17549430000000e-38 OK
strtold  "1.1754943E-38F" "" 1.17549430000000000000000000000000e-38 OK
strntod  14 "1.1754943E-38F" "" 1.17549430000000e-38 OK
strntod  13 "1.1754943E-38" "F" 1.17549430000000e-38 OK
strntold 14 "1.1754943E-38F" "" 1.17549430000000000000000000000000e-38 OK
strntold 13 "1.1754943E-38" "F" 1.17549430000000000000000000000000e-38 OK

strtod   "3.4028234e+38F" "" 3.40282340000000e+38 OK
strtold  "3.4028234e+38F" "" 3.40282340000000000000000000000000e+38 OK
strntod  14 "3.4028234e+38F" "" 3.40282340000000e+38 OK
strntod  13 "3.4028234e+38" "F" 3.40282340000000e+38 OK
strntold 14 "3.4028234e+38F" "" 3.40282340000000000000000000000000e+38 OK
strntold 13 "3.4028234e+38" "F" 3.40282340000000000000000000000000e+38 OK'

	EXEC	3.362103143112093506262677817321752603E-4932L 1.189731495357231765085759326628007016E+4932L
		OUTPUT - $'strtod   "3.362103143112093506262677817321752603E-4932L" "" 0.0000000000000'\
$'0e+00 ERANGE
strtold  "3.362103143112093506262677817321752603E-4932L" "" 3.36210314311209'\
$'350626267781732175e-4932 OK
strntod  45 "3.362103143112093506262677817321752603E-4932L" "" 0.00000000000'\
$'000e+00 ERANGE
strntod  44 "3.362103143112093506262677817321752603E-4932" "L" 0.00000000000'\
$'000e+00 ERANGE
strntold 45 "3.362103143112093506262677817321752603E-4932L" "" 3.36210314311'\
$'209350626267781732175e-4932 OK
strntold 44 "3.362103143112093506262677817321752603E-4932" "L" 3.36210314311'\
$'209350626267781732175e-4932 OK

strtod   "1.189731495357231765085759326628007016E+4932L" "" inf ERANGE
strtold  "1.189731495357231765085759326628007016E+4932L" "" 1.18973149535723'\
$'176508575932662802e+4932 OK
strntod  45 "1.189731495357231765085759326628007016E+4932L" "" inf ERANGE
strntod  44 "1.189731495357231765085759326628007016E+4932" "L" inf ERANGE
strntold 45 "1.189731495357231765085759326628007016E+4932L" "" 1.18973149535'\
$'723176508575932662802e+4932 OK
strntold 44 "1.189731495357231765085759326628007016E+4932" "L" 1.18973149535'\
'723176508575932662802e+4932 OK'

TEST 03 'hexadecimal floating point'

	EXEC	0x1.0000000000000000p-16382 0x1.fffffffffffffffep+16383
		OUTPUT - $'strtod   "0x1.0000000000000000p-16382" "" 0.00000000000000e+00 ERANGE
strtold  "0x1.0000000000000000p-16382" "" 3.36210314311209350626267781732175'\
$'e-4932 OK
strntod  27 "0x1.0000000000000000p-16382" "" 0.00000000000000e+00 ERANGE
strntod  26 "0x1.0000000000000000p-1638" "2" 0.00000000000000e+00 ERANGE
strntold 27 "0x1.0000000000000000p-16382" "" 3.36210314311209350626267781732'\
$'175e-4932 OK
strntold 26 "0x1.0000000000000000p-1638" "2" 8.18214369649906710739921082052'\
$'584e-494 OK

strtod   "0x1.fffffffffffffffep+16383" "" inf ERANGE
strtold  "0x1.fffffffffffffffep+16383" "" 1.18973149535723176502126385303098'\
$'e+4932 OK
strntod  27 "0x1.fffffffffffffffep+16383" "" inf ERANGE
strntod  26 "0x1.fffffffffffffffep+1638" "3" inf ERANGE
strntold 27 "0x1.fffffffffffffffep+16383" "" 1.18973149535723176502126385303'\
$'098e+4932 OK
strntold 26 "0x1.fffffffffffffffep+1638" "3" 2.44434719577920582078700480428'\
'470e+493 OK'

	EXEC	0x1p+16383 -0x1p+16383 0x1p-16382 -0x1p-16382
		OUTPUT - $'strtod   "0x1p+16383" "" inf ERANGE
strtold  "0x1p+16383" "" 5.94865747678615882542879663314008e+4931 OK
strntod  10 "0x1p+16383" "" inf ERANGE
strntod   9 "0x1p+1638" "3" inf ERANGE
strntold 10 "0x1p+16383" "" 5.94865747678615882542879663314008e+4931 OK
strntold  9 "0x1p+1638" "3" 1.22217359788960291045975656564166e+493 OK

strtod   "-0x1p+16383" "" -inf ERANGE
strtold  "-0x1p+16383" "" -5.94865747678615882542879663314008e+4931 OK
strntod  11 "-0x1p+16383" "" -inf ERANGE
strntod  10 "-0x1p+1638" "3" -inf ERANGE
strntold 11 "-0x1p+16383" "" -5.94865747678615882542879663314008e+4931 OK
strntold 10 "-0x1p+1638" "3" -1.22217359788960291045975656564166e+493 OK

strtod   "0x1p-16382" "" 0.00000000000000e+00 ERANGE
strtold  "0x1p-16382" "" 3.36210314311209350626267781732175e-4932 OK
strntod  10 "0x1p-16382" "" 0.00000000000000e+00 ERANGE
strntod   9 "0x1p-1638" "2" 0.00000000000000e+00 ERANGE
strntold 10 "0x1p-16382" "" 3.36210314311209350626267781732175e-4932 OK
strntold  9 "0x1p-1638" "2" 8.18214369649906710739921082052584e-494 OK

strtod   "-0x1p-16382" "" 0.00000000000000e+00 ERANGE
strtold  "-0x1p-16382" "" -3.36210314311209350626267781732175e-4932 OK
strntod  11 "-0x1p-16382" "" 0.00000000000000e+00 ERANGE
strntod  10 "-0x1p-1638" "2" -0.00000000000000e+00 ERANGE
strntold 11 "-0x1p-16382" "" -3.36210314311209350626267781732175e-4932 OK
strntold 10 "-0x1p-1638" "2" -8.18214369649906710739921082052584e-494 OK'

	EXEC	0x1p+16383 -0x1p+16383 0x1p+16384 -0x1p+16384 0x1p-16382 -0x1p-16382 0x1p-16383 -0x1p-16383
		OUTPUT - $'strtod   "0x1p+16383" "" inf ERANGE
strtold  "0x1p+16383" "" 5.94865747678615882542879663314008e+4931 OK
strntod  10 "0x1p+16383" "" inf ERANGE
strntod   9 "0x1p+1638" "3" inf ERANGE
strntold 10 "0x1p+16383" "" 5.94865747678615882542879663314008e+4931 OK
strntold  9 "0x1p+1638" "3" 1.22217359788960291045975656564166e+493 OK

strtod   "-0x1p+16383" "" -inf ERANGE
strtold  "-0x1p+16383" "" -5.94865747678615882542879663314008e+4931 OK
strntod  11 "-0x1p+16383" "" -inf ERANGE
strntod  10 "-0x1p+1638" "3" -inf ERANGE
strntold 11 "-0x1p+16383" "" -5.94865747678615882542879663314008e+4931 OK
strntold 10 "-0x1p+1638" "3" -1.22217359788960291045975656564166e+493 OK

strtod   "0x1p+16384" "" inf ERANGE
strtold  "0x1p+16384" "" inf ERANGE
strntod  10 "0x1p+16384" "" inf ERANGE
strntod   9 "0x1p+1638" "4" inf ERANGE
strntold 10 "0x1p+16384" "" inf ERANGE
strntold  9 "0x1p+1638" "4" 1.22217359788960291045975656564166e+493 OK

strtod   "-0x1p+16384" "" -inf ERANGE
strtold  "-0x1p+16384" "" -inf ERANGE
strntod  11 "-0x1p+16384" "" -inf ERANGE
strntod  10 "-0x1p+1638" "4" -inf ERANGE
strntold 11 "-0x1p+16384" "" -inf ERANGE
strntold 10 "-0x1p+1638" "4" -1.22217359788960291045975656564166e+493 OK

strtod   "0x1p-16382" "" 0.00000000000000e+00 ERANGE
strtold  "0x1p-16382" "" 3.36210314311209350626267781732175e-4932 OK
strntod  10 "0x1p-16382" "" 0.00000000000000e+00 ERANGE
strntod   9 "0x1p-1638" "2" 0.00000000000000e+00 ERANGE
strntold 10 "0x1p-16382" "" 3.36210314311209350626267781732175e-4932 OK
strntold  9 "0x1p-1638" "2" 8.18214369649906710739921082052584e-494 OK

strtod   "-0x1p-16382" "" 0.00000000000000e+00 ERANGE
strtold  "-0x1p-16382" "" -3.36210314311209350626267781732175e-4932 OK
strntod  11 "-0x1p-16382" "" 0.00000000000000e+00 ERANGE
strntod  10 "-0x1p-1638" "2" -0.00000000000000e+00 ERANGE
strntold 11 "-0x1p-16382" "" -3.36210314311209350626267781732175e-4932 OK
strntold 10 "-0x1p-1638" "2" -8.18214369649906710739921082052584e-494 OK

strtod   "0x1p-16383" "" 0.00000000000000e+00 ERANGE
strtold  "0x1p-16383" "" 0.00000000000000000000000000000000e+00 ERANGE
strntod  10 "0x1p-16383" "" 0.00000000000000e+00 ERANGE
strntod   9 "0x1p-1638" "3" 0.00000000000000e+00 ERANGE
strntold 10 "0x1p-16383" "" 0.00000000000000000000000000000000e+00 ERANGE
strntold  9 "0x1p-1638" "3" 8.18214369649906710739921082052584e-494 OK

strtod   "-0x1p-16383" "" 0.00000000000000e+00 ERANGE
strtold  "-0x1p-16383" "" -0.00000000000000000000000000000000e+00 ERANGE
strntod  11 "-0x1p-16383" "" 0.00000000000000e+00 ERANGE
strntod  10 "-0x1p-1638" "3" -0.00000000000000e+00 ERANGE
strntold 11 "-0x1p-16383" "" -0.00000000000000000000000000000000e+00 ERANGE
strntold 10 "-0x1p-1638" "3" -8.18214369649906710739921082052584e-494 OK'

	EXEC	0x1p127 0x1.p127 0x1.0p127 0x.1p131 0x0.1p131 0x0.10p131
		OUTPUT - $'strtod   "0x1p127" "" 1.70141183460469e+38 OK
strtold  "0x1p127" "" 1.70141183460469231731687303715884e+38 OK
strntod   7 "0x1p127" "" 1.70141183460469e+38 OK
strntod   6 "0x1p12" "7" 4.09600000000000e+03 OK
strntold  7 "0x1p127" "" 1.70141183460469231731687303715884e+38 OK
strntold  6 "0x1p12" "7" 4.09600000000000000000000000000000e+03 OK

strtod   "0x1.p127" "" 1.70141183460469e+38 OK
strtold  "0x1.p127" "" 1.70141183460469231731687303715884e+38 OK
strntod   8 "0x1.p127" "" 1.70141183460469e+38 OK
strntod   7 "0x1.p12" "7" 4.09600000000000e+03 OK
strntold  8 "0x1.p127" "" 1.70141183460469231731687303715884e+38 OK
strntold  7 "0x1.p12" "7" 4.09600000000000000000000000000000e+03 OK

strtod   "0x1.0p127" "" 1.70141183460469e+38 OK
strtold  "0x1.0p127" "" 1.70141183460469231731687303715884e+38 OK
strntod   9 "0x1.0p127" "" 1.70141183460469e+38 OK
strntod   8 "0x1.0p12" "7" 4.09600000000000e+03 OK
strntold  9 "0x1.0p127" "" 1.70141183460469231731687303715884e+38 OK
strntold  8 "0x1.0p12" "7" 4.09600000000000000000000000000000e+03 OK

strtod   "0x.1p131" "" 1.70141183460469e+38 OK
strtold  "0x.1p131" "" 1.70141183460469231731687303715884e+38 OK
strntod   8 "0x.1p131" "" 1.70141183460469e+38 OK
strntod   7 "0x.1p13" "1" 5.12000000000000e+02 OK
strntold  8 "0x.1p131" "" 1.70141183460469231731687303715884e+38 OK
strntold  7 "0x.1p13" "1" 5.12000000000000000000000000000000e+02 OK

strtod   "0x0.1p131" "" 1.70141183460469e+38 OK
strtold  "0x0.1p131" "" 1.70141183460469231731687303715884e+38 OK
strntod   9 "0x0.1p131" "" 1.70141183460469e+38 OK
strntod   8 "0x0.1p13" "1" 5.12000000000000e+02 OK
strntold  9 "0x0.1p131" "" 1.70141183460469231731687303715884e+38 OK
strntold  8 "0x0.1p13" "1" 5.12000000000000000000000000000000e+02 OK

strtod   "0x0.10p131" "" 1.70141183460469e+38 OK
strtold  "0x0.10p131" "" 1.70141183460469231731687303715884e+38 OK
strntod  10 "0x0.10p131" "" 1.70141183460469e+38 OK
strntod   9 "0x0.10p13" "1" 5.12000000000000e+02 OK
strntold 10 "0x0.10p131" "" 1.70141183460469231731687303715884e+38 OK
strntold  9 "0x0.10p13" "1" 5.12000000000000000000000000000000e+02 OK'

	EXEC	0x12345p127 0x12345.6789ap127 1.26866461572665980e+43
		OUTPUT - $'strtod   "0x12345p127" "" 1.26865773447299e+43 OK
strtold  "0x12345p127" "" 1.26865773447298882640732638015749e+43 OK
strntod  11 "0x12345p127" "" 1.26865773447299e+43 OK
strntod  10 "0x12345p12" "7" 3.05418240000000e+08 OK
strntold 11 "0x12345p127" "" 1.26865773447298882640732638015749e+43 OK
strntold 10 "0x12345p12" "7" 3.05418240000000000000000000000000e+08 OK

strtod   "0x12345.6789ap127" "" 1.26866461572666e+43 OK
strtold  "0x12345.6789ap127" "" 1.26866461572665987651685445358933e+43 OK
strntod  17 "0x12345.6789ap127" "" 1.26866461572666e+43 OK
strntod  16 "0x12345.6789ap12" "7" 3.05419896601563e+08 OK
strntold 17 "0x12345.6789ap127" "" 1.26866461572665987651685445358933e+43 OK
strntold 16 "0x12345.6789ap12" "7" 3.05419896601562500000000000000000e+08 OK

strtod   "1.26866461572665980e+43" "" 1.26866461572666e+43 OK
strtold  "1.26866461572665980e+43" "" 1.26866461572665980000000000000000e+43'\
$' OK
strntod  23 "1.26866461572665980e+43" "" 1.26866461572666e+43 OK
strntod  22 "1.26866461572665980e+4" "3" 1.26866461572666e+04 OK
strntold 23 "1.26866461572665980e+43" "" 1.26866461572665980000000000000000e'\
$'+43 OK
strntold 22 "1.26866461572665980e+4" "3" 1.26866461572665980000000000000000e'\
'+04 OK'

TEST 04 'to infinity and beyond'

	EXEC	inf +inf -inf
		OUTPUT - $'strtod   "inf" "" inf OK
strtold  "inf" "" inf OK
strntod   3 "inf" "" inf OK
strntod   2 "in" "inf" 0.00000000000000e+00 OK
strntold  3 "inf" "" inf OK
strntold  2 "in" "inf" 0.00000000000000000000000000000000e+00 OK

strtod   "+inf" "" inf OK
strtold  "+inf" "" inf OK
strntod   4 "+inf" "" inf OK
strntod   3 "+in" "+inf" 0.00000000000000e+00 OK
strntold  4 "+inf" "" inf OK
strntold  3 "+in" "+inf" 0.00000000000000000000000000000000e+00 OK

strtod   "-inf" "" -inf OK
strtold  "-inf" "" -inf OK
strntod   4 "-inf" "" -inf OK
strntod   3 "-in" "-inf" 0.00000000000000e+00 OK
strntold  4 "-inf" "" -inf OK
strntold  3 "-in" "-inf" 0.00000000000000000000000000000000e+00 OK'

	EXEC	Inf +Inf -Inf
		OUTPUT - $'strtod   "Inf" "" inf OK
strtold  "Inf" "" inf OK
strntod   3 "Inf" "" inf OK
strntod   2 "In" "Inf" 0.00000000000000e+00 OK
strntold  3 "Inf" "" inf OK
strntold  2 "In" "Inf" 0.00000000000000000000000000000000e+00 OK

strtod   "+Inf" "" inf OK
strtold  "+Inf" "" inf OK
strntod   4 "+Inf" "" inf OK
strntod   3 "+In" "+Inf" 0.00000000000000e+00 OK
strntold  4 "+Inf" "" inf OK
strntold  3 "+In" "+Inf" 0.00000000000000000000000000000000e+00 OK

strtod   "-Inf" "" -inf OK
strtold  "-Inf" "" -inf OK
strntod   4 "-Inf" "" -inf OK
strntod   3 "-In" "-Inf" 0.00000000000000e+00 OK
strntold  4 "-Inf" "" -inf OK
strntold  3 "-In" "-Inf" 0.00000000000000000000000000000000e+00 OK'

	EXEC	InFiNiTy +InFiNiTy -InFiNiTy
		OUTPUT - $'strtod   "InFiNiTy" "" inf OK
strtold  "InFiNiTy" "" inf OK
strntod   8 "InFiNiTy" "" inf OK
strntod   7 "InFiNiT" "iNiTy" inf OK
strntold  8 "InFiNiTy" "" inf OK
strntold  7 "InFiNiT" "iNiTy" inf OK

strtod   "+InFiNiTy" "" inf OK
strtold  "+InFiNiTy" "" inf OK
strntod   9 "+InFiNiTy" "" inf OK
strntod   8 "+InFiNiT" "iNiTy" inf OK
strntold  9 "+InFiNiTy" "" inf OK
strntold  8 "+InFiNiT" "iNiTy" inf OK

strtod   "-InFiNiTy" "" -inf OK
strtold  "-InFiNiTy" "" -inf OK
strntod   9 "-InFiNiTy" "" -inf OK
strntod   8 "-InFiNiT" "iNiTy" -inf OK
strntold  9 "-InFiNiTy" "" -inf OK
strntold  8 "-InFiNiT" "iNiTy" -inf OK'

	EXEC	infi +infi -infi
		OUTPUT - $'strtod   "infi" "i" inf OK
strtold  "infi" "i" inf OK
strntod   4 "infi" "i" inf OK
strntod   3 "inf" "i" inf OK
strntold  4 "infi" "i" inf OK
strntold  3 "inf" "i" inf OK

strtod   "+infi" "i" inf OK
strtold  "+infi" "i" inf OK
strntod   5 "+infi" "i" inf OK
strntod   4 "+inf" "i" inf OK
strntold  5 "+infi" "i" inf OK
strntold  4 "+inf" "i" inf OK

strtod   "-infi" "i" -inf OK
strtold  "-infi" "i" -inf OK
strntod   5 "-infi" "i" -inf OK
strntod   4 "-inf" "i" -inf OK
strntold  5 "-infi" "i" -inf OK
strntold  4 "-inf" "i" -inf OK'

	EXEC	in +in -in
		OUTPUT - $'strtod   "in" "in" 0.00000000000000e+00 OK
strtold  "in" "in" 0.00000000000000000000000000000000e+00 OK
strntod   2 "in" "in" 0.00000000000000e+00 OK
strntod   1 "i" "in" 0.00000000000000e+00 OK
strntold  2 "in" "in" 0.00000000000000000000000000000000e+00 OK
strntold  1 "i" "in" 0.00000000000000000000000000000000e+00 OK

strtod   "+in" "+in" 0.00000000000000e+00 OK
strtold  "+in" "+in" 0.00000000000000000000000000000000e+00 OK
strntod   3 "+in" "+in" 0.00000000000000e+00 OK
strntod   2 "+i" "+in" 0.00000000000000e+00 OK
strntold  3 "+in" "+in" 0.00000000000000000000000000000000e+00 OK
strntold  2 "+i" "+in" 0.00000000000000000000000000000000e+00 OK

strtod   "-in" "-in" 0.00000000000000e+00 OK
strtold  "-in" "-in" 0.00000000000000000000000000000000e+00 OK
strntod   3 "-in" "-in" 0.00000000000000e+00 OK
strntod   2 "-i" "-in" 0.00000000000000e+00 OK
strntold  3 "-in" "-in" 0.00000000000000000000000000000000e+00 OK
strntold  2 "-i" "-in" 0.00000000000000000000000000000000e+00 OK'

	EXEC	NaN +NaN -NaN
		OUTPUT - $'strtod   "NaN" "" nan OK
strtold  "NaN" "" nan OK
strntod   3 "NaN" "" nan OK
strntod   2 "Na" "NaN" 0.00000000000000e+00 OK
strntold  3 "NaN" "" nan OK
strntold  2 "Na" "NaN" 0.00000000000000000000000000000000e+00 OK

strtod   "+NaN" "" nan OK
strtold  "+NaN" "" nan OK
strntod   4 "+NaN" "" nan OK
strntod   3 "+Na" "+NaN" 0.00000000000000e+00 OK
strntold  4 "+NaN" "" nan OK
strntold  3 "+Na" "+NaN" 0.00000000000000000000000000000000e+00 OK

strtod   "-NaN" "" -nan OK
strtold  "-NaN" "" -nan OK
strntod   4 "-NaN" "" -nan OK
strntod   3 "-Na" "-NaN" 0.00000000000000e+00 OK
strntold  4 "-NaN" "" -nan OK
strntold  3 "-Na" "-NaN" 0.00000000000000000000000000000000e+00 OK'

	EXEC	NaN12-34abc.def +NaN12-34abc.def -NaN12-34abc.def
		OUTPUT - $'strtod   "NaN12-34abc.def" "" nan OK
strtold  "NaN12-34abc.def" "" nan OK
strntod  15 "NaN12-34abc.def" "" nan OK
strntod  14 "NaN12-34abc.de" "f" nan OK
strntold 15 "NaN12-34abc.def" "" nan OK
strntold 14 "NaN12-34abc.de" "f" nan OK

strtod   "+NaN12-34abc.def" "" nan OK
strtold  "+NaN12-34abc.def" "" nan OK
strntod  16 "+NaN12-34abc.def" "" nan OK
strntod  15 "+NaN12-34abc.de" "f" nan OK
strntold 16 "+NaN12-34abc.def" "" nan OK
strntold 15 "+NaN12-34abc.de" "f" nan OK

strtod   "-NaN12-34abc.def" "" -nan OK
strtold  "-NaN12-34abc.def" "" -nan OK
strntod  16 "-NaN12-34abc.def" "" -nan OK
strntod  15 "-NaN12-34abc.de" "f" -nan OK
strntold 16 "-NaN12-34abc.def" "" -nan OK
strntold 15 "-NaN12-34abc.de" "f" -nan OK'

	EXEC	0 -0 0. -0. 0.0 -0.0
		OUTPUT - $'strtod   "0" "" 0.00000000000000e+00 OK
strtold  "0" "" 0.00000000000000000000000000000000e+00 OK
strntod   1 "0" "" 0.00000000000000e+00 OK
strntod   0 "" "0" 0.00000000000000e+00 OK
strntold  1 "0" "" 0.00000000000000000000000000000000e+00 OK
strntold  0 "" "0" 0.00000000000000000000000000000000e+00 OK

strtod   "-0" "" -0.00000000000000e+00 OK
strtold  "-0" "" -0.00000000000000000000000000000000e+00 OK
strntod   2 "-0" "" -0.00000000000000e+00 OK
strntod   1 "-" "-0" 0.00000000000000e+00 OK
strntold  2 "-0" "" -0.00000000000000000000000000000000e+00 OK
strntold  1 "-" "-0" 0.00000000000000000000000000000000e+00 OK

strtod   "0." "" 0.00000000000000e+00 OK
strtold  "0." "" 0.00000000000000000000000000000000e+00 OK
strntod   2 "0." "" 0.00000000000000e+00 OK
strntod   1 "0" "." 0.00000000000000e+00 OK
strntold  2 "0." "" 0.00000000000000000000000000000000e+00 OK
strntold  1 "0" "." 0.00000000000000000000000000000000e+00 OK

strtod   "-0." "" -0.00000000000000e+00 OK
strtold  "-0." "" -0.00000000000000000000000000000000e+00 OK
strntod   3 "-0." "" -0.00000000000000e+00 OK
strntod   2 "-0" "." -0.00000000000000e+00 OK
strntold  3 "-0." "" -0.00000000000000000000000000000000e+00 OK
strntold  2 "-0" "." -0.00000000000000000000000000000000e+00 OK

strtod   "0.0" "" 0.00000000000000e+00 OK
strtold  "0.0" "" 0.00000000000000000000000000000000e+00 OK
strntod   3 "0.0" "" 0.00000000000000e+00 OK
strntod   2 "0." "0" 0.00000000000000e+00 OK
strntold  3 "0.0" "" 0.00000000000000000000000000000000e+00 OK
strntold  2 "0." "0" 0.00000000000000000000000000000000e+00 OK

strtod   "-0.0" "" -0.00000000000000e+00 OK
strtold  "-0.0" "" -0.00000000000000000000000000000000e+00 OK
strntod   4 "-0.0" "" -0.00000000000000e+00 OK
strntod   3 "-0." "0" -0.00000000000000e+00 OK
strntold  4 "-0.0" "" -0.00000000000000000000000000000000e+00 OK
strntold  3 "-0." "0" -0.00000000000000000000000000000000e+00 OK'

TEST 05 'simple, right?'

	EXEC	1 12 1.2 1.2.3
		OUTPUT - $'strtod   "1" "" 1.00000000000000e+00 OK
strtold  "1" "" 1.00000000000000000000000000000000e+00 OK
strntod   1 "1" "" 1.00000000000000e+00 OK
strntod   0 "" "1" 0.00000000000000e+00 OK
strntold  1 "1" "" 1.00000000000000000000000000000000e+00 OK
strntold  0 "" "1" 0.00000000000000000000000000000000e+00 OK

strtod   "12" "" 1.20000000000000e+01 OK
strtold  "12" "" 1.20000000000000000000000000000000e+01 OK
strntod   2 "12" "" 1.20000000000000e+01 OK
strntod   1 "1" "2" 1.00000000000000e+00 OK
strntold  2 "12" "" 1.20000000000000000000000000000000e+01 OK
strntold  1 "1" "2" 1.00000000000000000000000000000000e+00 OK

strtod   "1.2" "" 1.20000000000000e+00 OK
strtold  "1.2" "" 1.20000000000000000000000000000000e+00 OK
strntod   3 "1.2" "" 1.20000000000000e+00 OK
strntod   2 "1." "2" 1.00000000000000e+00 OK
strntold  3 "1.2" "" 1.20000000000000000000000000000000e+00 OK
strntold  2 "1." "2" 1.00000000000000000000000000000000e+00 OK

strtod   "1.2.3" ".3" 1.20000000000000e+00 OK
strtold  "1.2.3" ".3" 1.20000000000000000000000000000000e+00 OK
strntod   5 "1.2.3" ".3" 1.20000000000000e+00 OK
strntod   4 "1.2." ".3" 1.20000000000000e+00 OK
strntold  5 "1.2.3" ".3" 1.20000000000000000000000000000000e+00 OK
strntold  4 "1.2." ".3" 1.20000000000000000000000000000000e+00 OK'
