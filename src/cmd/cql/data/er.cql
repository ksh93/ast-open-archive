entity {
	register char*	id;
	char*	name;
	map*	kind;
	entity*	file;
	char*	dtype;
	map*	tclass;
	int	bline;
	int	hline;
	int	eline;
	map*	def;
	char*	chksum;
	entity*	pparam;
	entity*	tparam;
	map*	scope;
	entity*	ptype;
	char*	spec;
	char*	sig;
	char*	selected;
}

relationship {
	map*	kind1;
	entity*	id1;
	map*	kind2;
	entity*	id2;
	char*	usage;
	map*	rkind;
	map*	vflag;
	map*	pkind;
}

map {
	char*	key;
	char*	value;
}

entity.kind.input = {
	";_ERROR_",
	"f;file",
	"m;macro",
	"p;function",
	"t;type",
	"v;variable",
	"s;string"
}
entity.tclass.input = {
	";_ERROR_",
	"en;enum",
	"td;typedef",
	"sc;struct",
	"cl;class",
	"un;union",
	"pt;funtpl",
	"tp;tplfun",
	"ct;classtpl",
	"tc;tplclass",
	"rg;regular"
}
entity.def.input = {
	";_ERROR_",
	"df;def",
	"dc;dec",
	"ud;undef"
}
entity.scope.input = {
	";_ERROR_",
	"pt;protected",
	"pb;public",
	"pv;private",
	"st;static",
	"ex;extern",
	"us;unspecified"
}

relationship.kind1.input = {
	";_ERROR_",
	"f;file",
	"m;macro",
	"p;function",
	"t;type",
	"v;variable",
	"s;string"
}
relationship.kind2.input = {
	";_ERROR_",
	"f;file",
	"m;macro",
	"p;function",
	"t;type",
	"v;variable",
	"s;string"
}
relationship.rkind.input = {
	";_ERROR_",
	"ac;accadj",
	"cn;containment",
	"fd;friendship",
	"in;inheritance",
	"rf;reference",
	"td;typedef",
	"ti;instantiation"
}
relationship.vflag.input = {
	";_ERROR_",
	"y;yes",
	"n;no"
}
relationship.pkind.input = {
	";_ERROR_",
	"pt;protected",
	"pb;public",
	"pv;private",
	"st;static",
	"ex;extern",
	"us;unspecified"
}

schema = entity;

edge = { relationship.id1, relationship.id2 };

void begin() {
	int	isselected[];
	char*	ispulled[];
}

int select() {
	return name == "fstream" && kind.value == "type";
}

int closure() {
	return relationship.kind2.value == "type" &&
	       cql.iteration <= 10000 &&
	       relationship.id1.name != "";
}

void action() {
	if (cql.iteration <= 10000 && !isselected[id]) {
		isselected[id] = 1;
		ispulled[id] = "";
		printf("%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s\n",
			id, name, kind, file, dtype, tclass,
			bline, hline, eline, def, chksum,
			pparam, tparam, scope, ptype, spec,
			sig, "y");
		if (!isselected[file.id])
			ispulled[file.id] = sprintf("%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s\n",
				file.id,
				file.name,
				file.kind,
				file.file,
				file.dtype,
				file.tclass,
				file.bline,
				file.hline,
				file.eline,
				file.def,
				file.chksum,
				file.pparam,
				file.tparam,
				file.scope,
				file.ptype,
				file.spec,
				file.sig,
				"n");
		if (!isselected[pparam.id])
			ispulled[pparam.id] = sprintf("%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s\n",
				pparam.id,
				pparam.name,
				pparam.kind,
				pparam.file,
				pparam.dtype,
				pparam.tclass,
				pparam.bline,
				pparam.hline,
				pparam.eline,
				pparam.def,
				pparam.chksum,
				pparam.pparam,
				pparam.tparam,
				pparam.scope,
				pparam.ptype,
				pparam.spec,
				pparam.sig,
				"n");
		if (!isselected[tparam.id])
			ispulled[tparam.id] = sprintf("%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s\n",
				tparam.id,
				tparam.name,
				tparam.kind,
				tparam.file,
				tparam.dtype,
				tparam.tclass,
				tparam.bline,
				tparam.hline,
				tparam.eline,
				tparam.def,
				tparam.chksum,
				tparam.pparam,
				tparam.tparam,
				tparam.scope,
				tparam.ptype,
				tparam.spec,
				tparam.sig,
				"n");
		if (!isselected[ptype.id])
			ispulled[ptype.id] = sprintf("%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s\n",
				ptype.id,
				ptype.name,
				ptype.kind,
				ptype.file,
				ptype.dtype,
				ptype.tclass,
				ptype.bline,
				ptype.hline,
				ptype.eline,
				ptype.def,
				ptype.chksum,
				ptype.pparam,
				ptype.tparam,
				ptype.scope,
				ptype.ptype,
				ptype.spec,
				ptype.sig,
				"n");
	}
}

void end() {
	char*	s;

	for (ispulled[s]) 
		if (ispulled[s] != "")
			printf("%s", ispulled[s]);
}
