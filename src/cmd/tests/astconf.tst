UNIT $SHELL

export PATH=/opt/ast/bin:$PATH

TEST 01 synthesized getconf

	EXEC -c '
		_AST_FEATURES="" $SHELL -c "
			print :\$_AST_FEATURES:
			getconf PATH_RESOLVE - metaphysical
			print :\$_AST_FEATURES:
			getconf PATH_RESOLVE - 0
			print :\$_AST_FEATURES:"'
		OUTPUT - $'::\n:PATH_RESOLVE - metaphysical:\n:PATH_RESOLVE - 0:'

	EXEC -c '
		_AST_FEATURES="PATH_RESOLVE - logical" $SHELL -c "
			print :\$_AST_FEATURES:
			getconf PATH_RESOLVE - metaphysical
			print :\$_AST_FEATURES:
			getconf PATH_RESOLVE - 0
			print :\$_AST_FEATURES:"'
		OUTPUT - $':PATH_RESOLVE - logical:\n:PATH_RESOLVE - metaphysical:\n:PATH_RESOLVE - 0:'

	EXEC -c '
		_AST_FEATURES="PATH_RESOLVE - logical PATH_test - 1" $SHELL -c "
			print :\$_AST_FEATURES:
			getconf PATH_RESOLVE - metaphysical
			print :\$_AST_FEATURES:
			getconf PATH_RESOLVE - 0
			print :\$_AST_FEATURES:"'
		OUTPUT - $':PATH_RESOLVE - logical PATH_test - 1:\n:PATH_test - 1 PATH_RESOLVE - metaphysical:\n:PATH_test - 1 PATH_RESOLVE - 0:'

	EXEC -c '
		_AST_FEATURES="PATH_test - 1 PATH_RESOLVE - logical" $SHELL -c "
			print :\$_AST_FEATURES:
			getconf PATH_RESOLVE - metaphysical
			print :\$_AST_FEATURES:
			getconf PATH_RESOLVE - 0
			print :\$_AST_FEATURES:"'
		OUTPUT - $':PATH_test - 1 PATH_RESOLVE - logical:\n:PATH_test - 1 PATH_RESOLVE - metaphysical:\n:PATH_test - 1 PATH_RESOLVE - 0:'

	EXEC -c '
		_AST_FEATURES="PATH_test - 1 PATH_RESOLVE - logical PATH_aha - 2" $SHELL -c "
			print :\$_AST_FEATURES:
			getconf PATH_RESOLVE - metaphysical
			print :\$_AST_FEATURES:
			getconf PATH_RESOLVE - 0
			print :\$_AST_FEATURES:"'
		OUTPUT - $':PATH_test - 1 PATH_RESOLVE - logical PATH_aha - 2:\n:PATH_test - 1 PATH_aha - 2 PATH_RESOLVE - metaphysical:\n:PATH_test - 1 PATH_aha - 2 PATH_RESOLVE - 0:'

	EXEC -c '
		_AST_FEATURES="UNIVERSE = att" $SHELL -c "
			getconf UNIVERSE
			getconf UNIVERSE = ucb
			getconf UNIVERSE
			getconf UNIVERSE = att
			getconf UNIVERSE'
		OUTPUT - $'att\nucb\natt'
