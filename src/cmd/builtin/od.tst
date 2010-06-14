# regression tests for the od utilitiy

UNIT - --swap=0

KEEP "*.dat"

export LC_ALL=C

function DATA
{
	typeset f
	integer i
	typeset -i8 n
	for f
	do	test -f $f && continue
		case $f in
		ascii.dat)
			typeset -i8 o
			for ((o = 0; o < 256; o++))
			do	print -f "\\${o#8#}"
			done | dd silent=1 from=native to=ascii
			;;
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
		ebcdic.dat)
			typeset -i8 o
			for ((o = 0; o < 256; o++))
			do	print -f "\\${o#8#}"
			done | dd silent=1 from=native to=ebcdic
			;;
		euros.dat)
			for ((i = 1; i <= 65; i++))
			do	print -n $'\342\202\254'
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

TEST 01 'compatibility'

	DO	DATA chars.dat
	EXEC
		OUTPUT - $'0000000'
	EXEC	-b chars.dat
		OUTPUT - $'0000000 000 001 002 003 004 005 006 007 010 011 012 013 014 015 016 017
0000020 020 021 022 023 024 025 026 027 030 031 032 033 034 035 036 037
0000040 040 041 042 043 044 045 046 047 050 051 052 053 054 055 056 057
0000060 060 061 062 063 064 065 066 067 070 071 072 073 074 075 076 077
0000100 100 101 102 103 104 105 106 107 110 111 112 113 114 115 116 117
0000120 120 121 122 123 124 125 126 127 130 131 132 133 134 135 136 137
0000140 140 141 142 143 144 145 146 147 150 151 152 153 154 155 156 157
0000160 160 161 162 163 164 165 166 167 170 171 172 173 174 175 176 177
0000200 200 201 202 203 204 205 206 207 210 211 212 213 214 215 216 217
0000220 220 221 222 223 224 225 226 227 230 231 232 233 234 235 236 237
0000240 240 241 242 243 244 245 246 247 250 251 252 253 254 255 256 257
0000260 260 261 262 263 264 265 266 267 270 271 272 273 274 275 276 277
0000300 300 301 302 303 304 305 306 307 310 311 312 313 314 315 316 317
0000320 320 321 322 323 324 325 326 327 330 331 332 333 334 335 336 337
0000340 340 341 342 343 344 345 346 347 350 351 352 353 354 355 356 357
0000360 360 361 362 363 364 365 366 367 370 371 372 373 374 375 376 377
0000400'
	EXEC	-d chars.dat
		OUTPUT - $'0000000     1   515  1029  1543  2057  2571  3085  3599
0000020  4113  4627  5141  5655  6169  6683  7197  7711
0000040  8225  8739  9253  9767 10281 10795 11309 11823
0000060 12337 12851 13365 13879 14393 14907 15421 15935
0000100 16449 16963 17477 17991 18505 19019 19533 20047
0000120 20561 21075 21589 22103 22617 23131 23645 24159
0000140 24673 25187 25701 26215 26729 27243 27757 28271
0000160 28785 29299 29813 30327 30841 31355 31869 32383
0000200 32897 33411 33925 34439 34953 35467 35981 36495
0000220 37009 37523 38037 38551 39065 39579 40093 40607
0000240 41121 41635 42149 42663 43177 43691 44205 44719
0000260 45233 45747 46261 46775 47289 47803 48317 48831
0000300 49345 49859 50373 50887 51401 51915 52429 52943
0000320 53457 53971 54485 54999 55513 56027 56541 57055
0000340 57569 58083 58597 59111 59625 60139 60653 61167
0000360 61681 62195 62709 63223 63737 64251 64765 65279
0000400'

TEST 02 'ascii compatibility'

	DO	DATA ascii.dat chars.dat ebcdic.dat
	EXEC
		OUTPUT - $'0000000'
	EXEC	-tc chars.dat
		OUTPUT - $'0000000  \\0 001 002 003 004 005 006  \\a  \\b  \\t  \\n  \\v  \\f  \\r 016 017
0000020 020 021 022 023 024 025 026 027 030 031 032 033 034 035 036 037
0000040       !   "   #   $   %   &   \'   (   )   *   +   ,   -   .   /
0000060   0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?
0000100   @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O
0000120   P   Q   R   S   T   U   V   W   X   Y   Z   [   \\   ]   ^   _
0000140   `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o
0000160   p   q   r   s   t   u   v   w   x   y   z   {   |   }   ~ 177
0000200 200 201 202 203 204 205 206 207 210 211 212 213 214 215 216 217
0000220 220 221 222 223 224 225 226 227 230 231 232 233 234 235 236 237
0000240 240 241 242 243 244 245 246 247 250 251 252 253 254 255 256 257
0000260 260 261 262 263 264 265 266 267 270 271 272 273 274 275 276 277
0000300 300 301 302 303 304 305 306 307 310 311 312 313 314 315 316 317
0000320 320 321 322 323 324 325 326 327 330 331 332 333 334 335 336 337
0000340 340 341 342 343 344 345 346 347 350 351 352 353 354 355 356 357
0000360 360 361 362 363 364 365 366 367 370 371 372 373 374 375 376 377
0000400'
	EXEC	-tC chars.dat
		OUTPUT - $'0000000  \\0 001 002 003 004 005 006  \\a  \\b  \\t  \\n  \\v  \\f  \\r 016 017
0000020 020 021 022 023 024 025 026 027 030 031 032  \\E 034 035 036 037
0000040       !   "   #   $   %   &   \'   (   )   *   +   ,   -   .   /
0000060   0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?
0000100   @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O
0000120   P   Q   R   S   T   U   V   W   X   Y   Z   [   \\   ]   ^   _
0000140   `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o
0000160   p   q   r   s   t   u   v   w   x   y   z   {   |   }   ~ 177
0000200 200 201 202 203 204 205 206 207 210 211 212 213 214 215 216 217
0000220 220 221 222 223 224 225 226 227 230 231 232 233 234 235 236 237
0000240 240 241 242 243 244 245 246 247 250 251 252 253 254 255 256 257
0000260 260 261 262 263 264 265 266 267 270 271 272 273 274 275 276 277
0000300 300 301 302 303 304 305 306 307 310 311 312 313 314 315 316 317
0000320 320 321 322 323 324 325 326 327 330 331 332 333 334 335 336 337
0000340 340 341 342 343 344 345 346 347 350 351 352 353 354 355 356 357
0000360 360 361 362 363 364 365 366 367 370 371 372 373 374 375 376 377
0000400'
	EXEC	-tO chars.dat
		OUTPUT - $'0000000  \\0 001 002 003 004 005 006 007  \\b  \\t  \\n 013  \\f  \\r 016 017
0000020 020 021 022 023 024 025 026 027 030 031 032 033 034 035 036 037
0000040       !   "   #   $   %   &   \'   (   )   *   +   ,   -   .   /
0000060   0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?
0000100   @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O
0000120   P   Q   R   S   T   U   V   W   X   Y   Z   [   \\   ]   ^   _
0000140   `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o
0000160   p   q   r   s   t   u   v   w   x   y   z   {   |   }   ~ 177
0000200 200 201 202 203 204 205 206 207 210 211 212 213 214 215 216 217
0000220 220 221 222 223 224 225 226 227 230 231 232 233 234 235 236 237
0000240 240 241 242 243 244 245 246 247 250 251 252 253 254 255 256 257
0000260 260 261 262 263 264 265 266 267 270 271 272 273 274 275 276 277
0000300 300 301 302 303 304 305 306 307 310 311 312 313 314 315 316 317
0000320 320 321 322 323 324 325 326 327 330 331 332 333 334 335 336 337
0000340 340 341 342 343 344 345 346 347 350 351 352 353 354 355 356 357
0000360 360 361 362 363 364 365 366 367 370 371 372 373 374 375 376 377
0000400'
	EXEC	-tx1z chars.dat
		OUTPUT - $'0000000 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f ................
0000020 10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f ................
0000040 20 21 22 23 24 25 26 27 28 29 2a 2b 2c 2d 2e 2f  !"#$%&\'()*+,-./
0000060 30 31 32 33 34 35 36 37 38 39 3a 3b 3c 3d 3e 3f 0123456789:;<=>?
0000100 40 41 42 43 44 45 46 47 48 49 4a 4b 4c 4d 4e 4f @ABCDEFGHIJKLMNO
0000120 50 51 52 53 54 55 56 57 58 59 5a 5b 5c 5d 5e 5f PQRSTUVWXYZ[\\]^_
0000140 60 61 62 63 64 65 66 67 68 69 6a 6b 6c 6d 6e 6f `abcdefghijklmno
0000160 70 71 72 73 74 75 76 77 78 79 7a 7b 7c 7d 7e 7f pqrstuvwxyz{|}~.
0000200 80 81 82 83 84 85 86 87 88 89 8a 8b 8c 8d 8e 8f ................
0000220 90 91 92 93 94 95 96 97 98 99 9a 9b 9c 9d 9e 9f ................
0000240 a0 a1 a2 a3 a4 a5 a6 a7 a8 a9 aa ab ac ad ae af ................
0000260 b0 b1 b2 b3 b4 b5 b6 b7 b8 b9 ba bb bc bd be bf ................
0000300 c0 c1 c2 c3 c4 c5 c6 c7 c8 c9 ca cb cc cd ce cf ................
0000320 d0 d1 d2 d3 d4 d5 d6 d7 d8 d9 da db dc dd de df ................
0000340 e0 e1 e2 e3 e4 e5 e6 e7 e8 e9 ea eb ec ed ee ef ................
0000360 f0 f1 f2 f3 f4 f5 f6 f7 f8 f9 fa fb fc fd fe ff ................
0000400'
	EXEC	-ma -tm1 ascii.dat
		OUTPUT - $'0000000 00 01 02 03 04 05 06 \\a \\b \\t \\n \\v \\f \\r 0e 0f
0000020 10 11 12 13 14 15 16 17 18 19 1a \\E 1c 1d 1e 1f
0000040     !  "  #  $  %  &  \'  (  )  *  +  ,  -  .  /
0000060  0  1  2  3  4  5  6  7  8  9  :  ;  <  =  >  ?
0000100  @  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O
0000120  P  Q  R  S  T  U  V  W  X  Y  Z  [  \\  ]  ^  _
0000140  `  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o
0000160  p  q  r  s  t  u  v  w  x  y  z  {  |  }  ~ 7f
0000200 80 81 82 83 84 85 86 87 88 89 8a 8b 8c 8d 8e 8f
0000220 90 91 92 93 94 95 96 97 98 99 9a 9b 9c 9d 9e 9f
0000240 a0 a1 a2 a3 a4 a5 a6 a7 a8 a9 aa ab ac ad ae af
0000260 b0 b1 b2 b3 b4 b5 b6 b7 b8 b9 ba bb bc bd be bf
0000300 c0 c1 c2 c3 c4 c5 c6 c7 c8 c9 ca cb cc cd ce cf
0000320 d0 d1 d2 d3 d4 d5 d6 d7 d8 d9 da db dc dd de df
0000340 e0 e1 e2 e3 e4 e5 e6 e7 e8 e9 ea eb ec ed ee ef
0000360 f0 f1 f2 f3 f4 f5 f6 f7 f8 f9 fa fb fc fd fe ff
0000400'
	EXEC	-me -tm1 ebcdic.dat
		OUTPUT - $'0000000 00 01 02 03 04 05 06 \\a \\b \\t \\n \\v \\f \\r 0e 0f
0000020 10 11 12 13 14 15 16 17 18 19 1a \\E 1c 1d 1e 1f
0000040     !  "  #  $  %  &  \'  (  )  *  +  ,  -  .  /
0000060  0  1  2  3  4  5  6  7  8  9  :  ;  <  =  >  ?
0000100  @  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O
0000120  P  Q  R  S  T  U  V  W  X  Y  Z  [  \\  ]  ^  _
0000140  `  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o
0000160  p  q  r  s  t  u  v  w  x  y  z  {  |  }  ~ 7f
0000200 80 81 82 83 84 85 86 87 88 89 8a 8b 8c 8d 8e 8f
0000220 90 91 92 93 94 95 96 97 98 99 9a 9b 9c 9d 9e 9f
0000240 a0 a1 a2 a3 a4 a5 a6 a7 a8 a9 aa ab ac ad ae af
0000260 b0 b1 b2 b3 b4 b5 b6 b7 b8 b9 ba bb bc bd be bf
0000300 c0 c1 c2 c3 c4 c5 c6 c7 c8 c9 ca cb cc cd ce cf
0000320 d0 d1 d2 d3 d4 d5 d6 d7 d8 d9 da db dc dd de df
0000340 e0 e1 e2 e3 e4 e5 e6 e7 e8 e9 ea eb ec ed ee ef
0000360 f0 f1 f2 f3 f4 f5 f6 f7 f8 f9 fa fb fc fd fe ff
0000400'

TEST 03 'C.UTF-8'

	EXPORT LC_CTYPE=C.UTF-8

	DO DATA euros.dat

	EXEC	-tC -w15 euros.dat
		OUTPUT - $'0000000   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **
*
0000303'
	EXEC	-tC euros.dat
		OUTPUT - $'0000000   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254
0000020  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **
0000040  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **
0000060   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254
0000100  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **
0000120  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **
0000140   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254
0000160  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **
0000200  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **
0000220   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254
0000240  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **
0000260  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **
0000300   \342\202\254  **  **
0000303'
	EXEC	-tC -w17 euros.dat
		OUTPUT - $'0000000   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **
0000021  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254
0000042  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **
0000063   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **
0000104  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254
0000125  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **
0000146   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **
0000167  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254
0000210  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **
0000231   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **
0000252  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254  **  **   \342\202\254
0000273  **  **   \342\202\254  **  **   \342\202\254  **  **
0000303'
