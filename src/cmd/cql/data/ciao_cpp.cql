/* Ciao cql schema for type CC */
entity {
    register char* id;
    char* name;
    map* kind;
    entity* file;
    char* dtype;
    map* tclass;
    int bline;
    int hline;
    int eline;
    map* def;
    char* chksum;
    entity* pparam;
    entity* tparam;
    map* scope;
    entity* ptype;
    char* spec;
    char* sig;
    char* selected;
}
relationship {
    map* kind1;
    entity* id1;
    map* kind2;
    entity* id2;
    char* usage;
    map* rkind;
    map* vflag;
    map* pkind;
}
map {
    char* key;
    char* value;
}
entity.kind.input = {
    ";_ERROR_", "f;file", "m;macro", "p;function", "t;type", "v;variable", "s;string"
}
entity.tclass.input = {
    ";_ERROR_", "en;enum", "td;typedef", "sc;struct", "cl;class", "un;union", "pt;funtpl", "tp;tplfun", "ct;classtpl", "tc;tplclass", "rg;regular"
}
entity.def.input = {
    ";_ERROR_", "df;def", "dc;dec", "ud;undef"
}
entity.scope.input = {
    ";_ERROR_", "pt;protected", "pb;public", "pv;private", "st;static", "ex;extern", "us;unspecified"
}

relationship.kind1.input = {
    ";_ERROR_", "f;file", "m;macro", "p;function", "t;type", "v;variable", "s;string"
}
relationship.kind2.input = {
    ";_ERROR_", "f;file", "m;macro", "p;function", "t;type", "v;variable", "s;string"
}
relationship.rkind.input = {
    ";_ERROR_", "ac;accadj", "cn;containment", "fd;friendship", "in;inheritance", "rf;reference", "td;typedef", "ti;instantiation"
}
relationship.vflag.input = {
    ";_ERROR_", "y;yes", "n;no"
}
relationship.pkind.input = {
    ";_ERROR_", "pt;protected", "pb;public", "pv;private", "st;static", "ex;extern", "us;unspecified"
}
