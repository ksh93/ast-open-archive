/*
 * cia db definitions for cql
 */

symbol
{ 
	extern char*	id;
	register char*	name; 
	map*		kind; 
	symbol*		file; 
	char*		dtype;
	map*		sclass;
	int		bline;
	int		hline;
	int		eline;
	map*		def; 
	char*		chksum; 
}

reference
{ 
	map*		kind1; 
	symbol*		id1; 
	map*		kind2;
	symbol*		id2;
	char*		usage; 
	char*		rkind; 
}

map
{ 
	char*		key;
	char*		value; 
}

symbol.kind.input =
reference.id1.kind.input =
reference.id2.kind.input =
reference.kind1.input =
reference.kind2.input = 
{
	";_ERROR_",	"f;file",	"p;function", 
	"v;variable",	"m;macro",	"t;type"
};

symbol.sclass.input =
reference.id1.sclass.input =
reference.id2.sclass.input =
{
	";_ERROR_",	
	"n;none",					/* file */
	"t;typedef",    "n;none", 			/* type */
	"d;macdef",	"u;macudef",    		/* macro */
	"e;extern",	"s;static",	"l;libsym",	/* func & var */
	"m;enum",					/* var */
	"g;global"					/* func & var */
};

symbol.def.input =
reference.id1.def.input =
reference.id2.def.input =
{
	";_ERROR_",	"df;def",	"dc;dec",
	"ud;undef"      				/* macros */
};
