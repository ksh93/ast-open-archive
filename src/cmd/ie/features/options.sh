########################################################################
#                                                                      #
#               This software is part of the ast package               #
#                  Copyright (c) 1984-2005 AT&T Corp.                  #
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
#                  David Korn <dgk@research.att.com>                   #
#                             Pat Sullivan                             #
#                                                                      #
########################################################################
: include OPTIONS
eval $1
shift
. $1
for i in ACCT BRACEPAT ECHOPRINT ESH ESHPLUS FLOAT FS_3D IODELAY JOBS \
	MULTIBYTE NEWTEST OLDTERMIO OLDTEST P_UID PDU POSIX RDS RAWONLY \
	SEVENBIT SUID_EXEC TIMEOUT VSH LDYNAMIC
do	: This could be done with eval, but eval broken in some shells
	j=0
	case $i in
	ACCT)		j=$ACCT;;
	BRACEPAT)	j=$BRACEPAT;;
	ECHOPRINT)	j=$ECHOPRINT;;
	ESH)		j=$ESH;;
	ESHPLUS)	j=$ESHPLUS;;
	FLOAT)		j=$FLOAT;;
	FS_3D)		j=$FS_3D;;
	IODELAY)	j=$IODELAY;;
	JOBS)		j=$JOBS;;
	LDYNAMIC)	j=$LDYNAMIC;;
	MULTIBYTE)	j=$MULTIBYTE;;
	NEWTEST)	j=$NEWTEST;;
	OLDTERMIO)	echo "#include <sys/termios.h>" > $tmp.c
			echo "#include <sys/termio.h>" >> $tmp.c
			if	$CC -E $tmp.c > /dev/null 2>&1
			then	j=$OLDTERMIO
			fi ;;
	OLDTEST)	j=$OLDTEST;;
	P_UID)		j=$P_UID;;
	PDU)		j=$PDU
			if	test 0 -ne "$j" && test -f /usr/pdu/bin/pwd
			then	echo "#define PDUBIN	1"
			fi;;
	POSIX)		j=$POSIX;;
	RAWONLY)	j=$RAWONLY;;
	RDS)		j=$RDS;;
	SEVENBIT)	j=$SEVENBIT;;
	SUID_EXEC)	j=$SUID_EXEC;;
	TIMEOUT)	j=$TIMEOUT;;
	VSH)		j=$VSH;;
	esac
	case $j in
	0|"")	;;
	*)	echo "#define $i	$j" ;;
	esac
done
