####################################################################
#                                                                  #
#             This software is part of the ast package             #
#                Copyright (c) 1989-2002 AT&T Corp.                #
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
#            Information and Software Systems Research             #
#                        AT&T Labs Research                        #
#                         Florham Park NJ                          #
#                                                                  #
#               Glenn Fowler <gsf@research.att.com>                #
####################################################################
: color ls
ls -D name='%(mode:case:d*:\E[01;34m%(name)s\E[0m:l*:\E[01;36m%(name)s\E[0m:*x*:\E[01;32m%(name)s\E[0m:*:%(name)s)s' "$@"
