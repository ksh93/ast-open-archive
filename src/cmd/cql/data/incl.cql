/*
 * incl definitions for cql
 */

incl_file
{ 
	extern char*	id;
	register char*	name; 
	map*		fkind; 
}

incl_ref
{ 
	incl_file*	id1; 
	incl_file*	id2;
	map*		rkind;
        int		level1;
}

map
{ 
	char*		key;
	char*		value; 
}

incl_file.fkind.input = { ';_ERROR_', 'u;unused', 'p;onpath', 'n;needed' };

incl_ref.rkind.input = { ';_ERROR_', 'a;all', 'r;ref', 'i;inc' };
