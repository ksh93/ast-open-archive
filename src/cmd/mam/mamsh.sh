####################################################################
#                                                                  #
#             This software is part of the ast package             #
#                Copyright (c) 1989-2001 AT&T Corp.                #
#        and it may only be used by you under license from         #
#                       AT&T Corp. ("AT&T")                        #
#         A copy of the Source Code Agreement is available         #
#                at the AT&T Internet web site URL                 #
#                                                                  #
#       http://www.research.att.com/sw/license/ast-open.html       #
#                                                                  #
#        If you have copied this software without agreeing         #
#        to the terms of the license you are infringing on         #
#           the license and copyright and are violating            #
#               AT&T's intellectual property rights.               #
#                                                                  #
#                 This software was created by the                 #
#                 Network Services Research Center                 #
#                        AT&T Labs Research                        #
#                         Florham Park NJ                          #
#                                                                  #
#               Glenn Fowler <gsf@research.att.com>                #
####################################################################
:
#
# Glenn Fowler
# AT&T Bell Laboratories
#
# mam -> sh-script converter
#
# @(#)mamsh (gsf@research.att.com) 01/01/94
#
echo ": : generated from makefile abstraction : :"
echo 'eval ${1+"$@"}'
sed -n -e 's/^setv \([^ ]*\) \(.*\)/\1=${\1-\2}/p' -e 's/^exec [^ ]* //p' ${1+"$@"}
