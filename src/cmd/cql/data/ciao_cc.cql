/* Ciao cql schema for type cc */
entity {
    register char* id;
    register char* name;
    map* kind;
    entity* file;
    char* dtype;
    map* sclass;
    int bline;
    int hline;
    int eline;
    map* def;
    char* chksum;
    char* selected;
}
relationship {
    map* kind1;
    entity* id1;
    map* kind2;
    entity* id2;
    char* usage;
    char* rkind;
}
map {
    char* key;
    char* value;
}
entity.kind.input = {
    ";_ERROR_", "f;file", "m;macro", "p;function", "t;type", "v;variable"
}
entity.sclass.input = {
    ";_ERROR_", "n;none", "t;typedef", "d;macdef", "u;macudef", "e;extern", "s;static", "l;libsym", "m;enum", "g;global"
}
entity.def.input = {
    ";_ERROR_", "df;def", "dc;dec", "ud;undef"
}

relationship.kind1.input = {
    ";_ERROR_", "f;file", "m;macro", "p;function", "t;type", "v;variable"
}
relationship.kind2.input = {
    ";_ERROR_", "f;file", "m;macro", "p;function", "t;type", "v;variable"
}
