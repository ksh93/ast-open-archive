# : : generated from ip6.rt by mktest : : #

# regression tests for the testip6 command

TEST 01 basics

	EXEC	:: 2002:3E02:5473:1234::/48 2002:62.2.84.115:1234::/48 12AB:0:0:CD30:0:0:0:0/60 1080:0:0:0:8:800:200C:417A FF01:0:0:0:0:0:0:101 0:0:0:0:0:0:0:1 0:0:0:0:0:0:0:0 0:1 0:1:: 1:2 1:2:: 0:1:2:3:4 0:1:2:3:4:: 0:1:2:3:4:5:6:7 ::1:2:3:4:5:6:7 1:2:3:4:5:6:7:8 ::FFFF:1.2.3.4
		OUTPUT - $'0                               ::                                  ::
0         2002:3E02:5473:1234::/48                                  2002:62.'\
$'2.84.115:1234::/48
0       2002:62.2.84.115:1234::/48                                  2002:62.'\
$'2.84.115:1234::/48
0         12AB:0:0:CD30:0:0:0:0/60                                  12AB:0:0'\
$':CD30::/60
0       1080:0:0:0:8:800:200C:417A                                  1080::8:'\
$'800:200C:417A
0             FF01:0:0:0:0:0:0:101                                  FF01::10'\
$'1
0                  0:0:0:0:0:0:0:1                                  ::1
0                  0:0:0:0:0:0:0:0                                  ::
0                              0:1                                  0:1::
0                            0:1::                                  0:1::
0                              1:2                                  1:2::
0                            1:2::                                  1:2::
0                        0:1:2:3:4                                  0:1:2:3:'\
$'4::
0                      0:1:2:3:4::                                  0:1:2:3:'\
$'4::
0                  0:1:2:3:4:5:6:7                                  ::1:2:3:'\
$'4:5:6:7
0                  ::1:2:3:4:5:6:7                                  ::1:2:3:'\
$'4:5:6:7
0                  1:2:3:4:5:6:7:8                                  1:2:3:4:'\
$'5:6:7:8
0                   ::FFFF:1.2.3.4                                  ::FFFF:1'\
.2.3.4