####################################################################
#                                                                  #
#             This software is part of the ast package             #
#                Copyright (c) 1989-2004 AT&T Corp.                #
#        and it may only be used by you under license from         #
#                       AT&T Corp. ("AT&T")                        #
#         A copy of the Source Code Agreement is available         #
#                at the AT&T Internet web site URL                 #
#                                                                  #
#       http://www.research.att.com/sw/license/ast-open.html       #
#                                                                  #
#    If you have copied or used this software without agreeing     #
#        to the terms of the license you are infringing on         #
#           the license and copyright and are violating            #
#               AT&T's intellectual property rights.               #
#                                                                  #
#            Information and Software Systems Research             #
#                        AT&T Labs Research                        #
#                         Florham Park NJ                          #
#                                                                  #
#               Glenn Fowler <gsf@research.att.com>                #
#                                                                  #
####################################################################
: we do that too
ls 	-D header='%(dir.count:case:[01]::*:\n)s Directory of %(path)s\n' \
	-D meridian='%(mtime:time=%p)s' \
	-D trailer='%9(dir.files)lu file(s) %10(dir.bytes)lu bytes\n' \
	-f '%-8.8(name:edit:\\([^.]*\\).*:\\1:u)s %3.3(name:edit:[^.]*\\.*\\(.*\\):\\1:u)s %5(mode:case:d*:<DIR>:l*:<LNK>:*x*:<EXE>)s%7(size)lu %(mtime:time=%m-%d-%y  %I:%M)s%(meridian:edit:\\(.\\).*:\\1:l)s' \
	"$@"
