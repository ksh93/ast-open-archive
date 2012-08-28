########################################################################
#                                                                      #
#               This software is part of the ast package               #
#                  Copyright (c) 1989-2005 AT&T Corp.                  #
#                      and is licensed under the                       #
#                  Common Public License, Version 1.0                  #
#                            by AT&T Corp.                             #
#                                                                      #
#                A copy of the License is available at                 #
#            http://www.opensource.org/licenses/cpl1.0.txt             #
#         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         #
#                                                                      #
#              Information and Software Systems Research               #
#                            AT&T Research                             #
#                           Florham Park NJ                            #
#                                                                      #
#                 Glenn Fowler <gsf@research.att.com>                  #
#                                                                      #
########################################################################
: we do that too
ls 	-D header='%(dir.count:case:[01]::*:\n)s Directory of %(path)s\n' \
	-D meridian='%(mtime:time=%p)s' \
	-D trailer='%9(dir.files)lu file(s) %10(dir.bytes)lu bytes\n' \
	-Z '%-8.8(name:edit:\\([^.]*\\).*:\\1:u)s %3.3(name:edit:[^.]*\\.*\\(.*\\):\\1:u)s %5(mode:case:d*:<DIR>:l*:<LNK>:*x*:<EXE>)s%7(size)lu %(mtime:time=%m-%d-%y  %I:%M)s%(meridian:edit:\\(.\\).*:\\1:l)s' \
	"$@"
