/*
 * yacc %include file support
 *
 * @(#)yacc (AT&T Research) 1996-12-25
 *
 * %include in *.yy produces yacc output file *.y
 * %include suffix .yacc
 */

.SOURCE.%.yacc : $$(*.SOURCE.%.LCL.INCLUDE)

.SCAN.yy : .SCAN
	Q|/*|*/||C|
	Q|//||\\|LC|
	Q|"|"|\\|LQ|
	Q|'|'|\\|LQ|
	Q|\\|||CS|
	I| \% include "%"|A.LCL.INCLUDE|

.ATTRIBUTE.%.yy : .SCAN.yy
.ATTRIBUTE.%.Y : .SCAN.yy

%.y : %.yy
	$(.YACC.INCLUDE.) $(>) > $(<)

%.y : %.Y
	$(.YACC.INCLUDE.) $(>) > $(<)

.YACC.INCLUDE. : .FUNCTION
	local T V
	for T $(!$(>>))
		eval
		V += -e '/^[ 	]*%[ 	]*include[ 	][ 	]*"$$(T:P=U:C%/%\\/%G)"/ {$$("\n")r '$(T)'$$("\n")d$$("\n")}'
		end
	end
	return $(V:@?$$(SED) $$(V)?cat?)
