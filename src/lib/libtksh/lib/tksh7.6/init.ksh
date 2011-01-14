########################################################################
#                                                                      #
#               This software is part of the ast package               #
#          Copyright (c) 1986-2011 AT&T Intellectual Property          #
#                      and is licensed under the                       #
#                  Common Public License, Version 1.0                  #
#                    by AT&T Intellectual Property                     #
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
#                  David Korn <dgk@research.att.com>                   #
#                   Phong Vo <kpv@research.att.com>                    #
#                                                                      #
########################################################################
typeset -A env

function env.get 
{
	[[ ${.sh.subscript} == "" ]] && return
	nameref v=${.sh.subscript}
	env[${.sh.subscript}]="$v"
}

function env.set
{
	[[ ${.sh.subscript} == "" ]] && return
	nameref v=${.sh.subscript}
	v="${.sh.value}"
}

function env.unset
{
	[[ ${.sh.subscript} == "" ]] && return
	nameref v=${.sh.subscript}
	unset v
}

argv0="$0"
argv="$(tcl_list "$@")"

tksh_info mode tcl

source $tcl_library/init.tcl

# PS1="Tksh $PS1"
# export TK_LIBRARY=$tcl_library/tk4.2
# tk_library=$TK_LIBRARY

: "${env[PWD]}"		# not sure why this is needed

tksh_info sideeffects on
