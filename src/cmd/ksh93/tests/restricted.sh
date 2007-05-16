########################################################################
#                                                                      #
#               This software is part of the ast package               #
#           Copyright (c) 1982-2007 AT&T Knowledge Ventures            #
#                      and is licensed under the                       #
#                  Common Public License, Version 1.0                  #
#                      by AT&T Knowledge Ventures                      #
#                                                                      #
#                A copy of the License is available at                 #
#            http://www.opensource.org/licenses/cpl1.0.txt             #
#         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         #
#                                                                      #
#              Information and Software Systems Research               #
#                            AT&T Research                             #
#                           Florham Park NJ                            #
#                                                                      #
#                  David Korn <dgk@research.att.com>                   #
#                                                                      #
########################################################################
function err_exit
{
	print -u2 -n "\t"
	print -u2 -r ${Command}[$1]: "${@:2}"
	let Errors+=1
}
alias err_exit='err_exit $LINENO'

# test restricted shell
Command=${0##*/}
integer Errors=0
mkdir  /tmp/ksh$$ || err_exit "mkdir /tmp/ksh$$ failed" 
trap "cd /; rm -rf /tmp/ksh$$" EXIT
pwd=$PWD
case $SHELL in
/*)	;;
*/*)	SHELL=$pwd/$SHELL;;
*)	SHELL=$(whence "$SHELL");;
esac
function check_restricted
{
	rm -f out
	rksh -c "$@" 2> out > /dev/null
	grep restricted out  > /dev/null 2>&1 
}

[[ $SHELL != /* ]] && SHELL=$pwd/$SHELL
cd /tmp/ksh$$ || err_exit "cd /tmp/ksh$$ failed"
ln -s $SHELL rksh
PATH=$PWD:$PATH
rksh -c  '[[ -o restricted ]]' || err_exit 'restricted option not set' 
[[ $(rksh -c 'print hello') == hello ]] || err_exit 'unable to run print'
check_restricted /bin/echo || err_exit '/bin/echo not resticted'
check_restricted ./echo || err_exit './echo not resticted'
check_restricted 'SHELL=ksh' || err_exit 'SHELL asignment not resticted'
check_restricted 'PATH=/bin' || err_exit 'PATH asignment not resticted'
check_restricted 'FPATH=/bin' || err_exit 'FPATH asignment not resticted'
check_restricted 'ENV=/bin' || err_exit 'ENV asignment not resticted'
check_restricted 'print > file' || err_exit '> file not restricted'
> empty
check_restricted 'print <> empty' || err_exit '<> file not restricted'
print 'echo hello' > script
chmod +x ./script
! check_restricted script ||  err_exit 'script without builtins should run in restricted mode'
check_restricted ./script ||  err_exit 'script with / in name should not run in restricted mode'
print '/bin/echo hello' > script
check_restricted script ||  err_exit 'script with pathnames should not run in restricted mode'
print 'echo hello> file' > script
check_restricted script ||  err_exit 'script with output redirection should not run in restricted mode'
print 'PATH=/bin' > script
check_restricted script ||  err_exit 'script with PATH assignment should not run in restricted mode'
cat > script <<!
#! $SHELL
print hello
!
cp script /tmp/dgk
check_restricted 'script;:' ||  err_exit 'script with #! pathname should not run in restricted mode'
check_restricted 'script' ||  err_exit 'script with #! pathname should not run in restricted mode even if last command in script'
exit $((Errors))
