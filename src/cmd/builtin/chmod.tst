# : : generated from chmod.rt by mktest : : #

# regression tests for the chmod command

UNIT chmod

TEST 01 '"=" vs. umask'

	PROG	touch f

	EXEC	777 f
		UMASK 000

	EXEC	-c = f
		OUTPUT - 'f: mode changed to 0000 (---------)'

	EXEC	-c =rw f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	EXEC	-c =,u=rw f
		OUTPUT - 'f: mode changed to 0600 (rw-------)'

	EXEC	-c =,g=rw f
		OUTPUT - 'f: mode changed to 0060 (---rw----)'

	EXEC	-c =,o=rw f
		OUTPUT - 'f: mode changed to 0006 (------rw-)'

	EXEC	-c =,ug=rw f
		OUTPUT - 'f: mode changed to 0660 (rw-rw----)'

	EXEC	-c =,uo=rw f
		OUTPUT - 'f: mode changed to 0606 (rw----rw-)'

	EXEC	-c =,go=rw f
		OUTPUT - 'f: mode changed to 0066 (---rw-rw-)'

	EXEC	-c =,a=rw f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	EXEC	777 f
		OUTPUT -
		UMASK 007

	EXEC	-c = f
		OUTPUT - 'f: mode changed to 0000 (---------)'

	EXEC	-c =rw f
		OUTPUT - 'f: mode changed to 0660 (rw-rw----)'

	EXEC	-c =,u=rw f
		OUTPUT - 'f: mode changed to 0600 (rw-------)'

	EXEC	-c =,g=rw f
		OUTPUT - 'f: mode changed to 0060 (---rw----)'

	EXEC	-c =,o=rw f
		OUTPUT - 'f: mode changed to 0006 (------rw-)'

	EXEC	-c =,ug=rw f
		OUTPUT - 'f: mode changed to 0660 (rw-rw----)'

	EXEC	-c =,uo=rw f
		OUTPUT - 'f: mode changed to 0606 (rw----rw-)'

	EXEC	-c =,go=rw f
		OUTPUT - 'f: mode changed to 0066 (---rw-rw-)'

	EXEC	-c =,a=rw f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	EXEC	777 f
		OUTPUT -
		UMASK 070

	EXEC	-c = f
		OUTPUT - 'f: mode changed to 0000 (---------)'

	EXEC	-c =rw f
		OUTPUT - 'f: mode changed to 0606 (rw----rw-)'

	EXEC	-c =,u=rw f
		OUTPUT - 'f: mode changed to 0600 (rw-------)'

	EXEC	-c =,g=rw f
		OUTPUT - 'f: mode changed to 0060 (---rw----)'

	EXEC	-c =,o=rw f
		OUTPUT - 'f: mode changed to 0006 (------rw-)'

	EXEC	-c =,ug=rw f
		OUTPUT - 'f: mode changed to 0660 (rw-rw----)'

	EXEC	-c =,uo=rw f
		OUTPUT - 'f: mode changed to 0606 (rw----rw-)'

	EXEC	-c =,go=rw f
		OUTPUT - 'f: mode changed to 0066 (---rw-rw-)'

	EXEC	-c =,a=rw f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	EXEC	777 f
		OUTPUT -
		UMASK 077

	EXEC	-c = f
		OUTPUT - 'f: mode changed to 0000 (---------)'

	EXEC	-c =rw f
		OUTPUT - 'f: mode changed to 0600 (rw-------)'

	EXEC	-c =,u=rw f
		OUTPUT -

	EXEC	-c =,g=rw f
		OUTPUT - 'f: mode changed to 0060 (---rw----)'

	EXEC	-c =,o=rw f
		OUTPUT - 'f: mode changed to 0006 (------rw-)'

	EXEC	-c =,ug=rw f
		OUTPUT - 'f: mode changed to 0660 (rw-rw----)'

	EXEC	-c =,uo=rw f
		OUTPUT - 'f: mode changed to 0606 (rw----rw-)'

	EXEC	-c =,go=rw f
		OUTPUT - 'f: mode changed to 0066 (---rw-rw-)'

	EXEC	-c =,a=rw f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	EXEC	777 f
		OUTPUT -
		UMASK 700

	EXEC	-c = f
		OUTPUT - 'f: mode changed to 0000 (---------)'

	EXEC	-c =rw f
		OUTPUT - 'f: mode changed to 0066 (---rw-rw-)'

	EXEC	-c =,u=rw f
		OUTPUT - 'f: mode changed to 0600 (rw-------)'

	EXEC	-c =,g=rw f
		OUTPUT - 'f: mode changed to 0060 (---rw----)'

	EXEC	-c =,o=rw f
		OUTPUT - 'f: mode changed to 0006 (------rw-)'

	EXEC	-c =,ug=rw f
		OUTPUT - 'f: mode changed to 0660 (rw-rw----)'

	EXEC	-c =,uo=rw f
		OUTPUT - 'f: mode changed to 0606 (rw----rw-)'

	EXEC	-c =,go=rw f
		OUTPUT - 'f: mode changed to 0066 (---rw-rw-)'

	EXEC	-c =,a=rw f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	EXEC	777 f
		OUTPUT -
		UMASK 707

	EXEC	-c = f
		OUTPUT - 'f: mode changed to 0000 (---------)'

	EXEC	-c =rw f
		OUTPUT - 'f: mode changed to 0060 (---rw----)'

	EXEC	-c =,u=rw f
		OUTPUT - 'f: mode changed to 0600 (rw-------)'

	EXEC	-c =,g=rw f
		OUTPUT - 'f: mode changed to 0060 (---rw----)'

	EXEC	-c =,o=rw f
		OUTPUT - 'f: mode changed to 0006 (------rw-)'

	EXEC	-c =,ug=rw f
		OUTPUT - 'f: mode changed to 0660 (rw-rw----)'

	EXEC	-c =,uo=rw f
		OUTPUT - 'f: mode changed to 0606 (rw----rw-)'

	EXEC	-c =,go=rw f
		OUTPUT - 'f: mode changed to 0066 (---rw-rw-)'

	EXEC	-c =,a=rw f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	EXEC	777 f
		OUTPUT -
		UMASK 770

	EXEC	-c = f
		OUTPUT - 'f: mode changed to 0000 (---------)'

	EXEC	-c =rw f
		OUTPUT - 'f: mode changed to 0006 (------rw-)'

	EXEC	-c =,u=rw f
		OUTPUT - 'f: mode changed to 0600 (rw-------)'

	EXEC	-c =,g=rw f
		OUTPUT - 'f: mode changed to 0060 (---rw----)'

	EXEC	-c =,o=rw f
		OUTPUT - 'f: mode changed to 0006 (------rw-)'

	EXEC	-c =,ug=rw f
		OUTPUT - 'f: mode changed to 0660 (rw-rw----)'

	EXEC	-c =,uo=rw f
		OUTPUT - 'f: mode changed to 0606 (rw----rw-)'

	EXEC	-c =,go=rw f
		OUTPUT - 'f: mode changed to 0066 (---rw-rw-)'

	EXEC	-c =,a=rw f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	EXEC	777 f
		OUTPUT -
		UMASK 777

	EXEC	-c = f

	EXEC	-c =rw f

	EXEC	-c =,u=rw f
		OUTPUT - 'f: mode changed to 0600 (rw-------)'

	EXEC	-c =,g=rw f
		OUTPUT - 'f: mode changed to 0060 (---rw----)'

	EXEC	-c =,o=rw f
		OUTPUT - 'f: mode changed to 0006 (------rw-)'

	EXEC	-c =,ug=rw f
		OUTPUT - 'f: mode changed to 0660 (rw-rw----)'

	EXEC	-c =,uo=rw f
		OUTPUT - 'f: mode changed to 0606 (rw----rw-)'

	EXEC	-c =,go=rw f
		OUTPUT - 'f: mode changed to 0066 (---rw-rw-)'

	EXEC	-c =,a=rw f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'
