entity {
    register char* id;
    char* name;
    map* kind;
    char* n1;
    char* n2;
    char* attr;
    char* selected;
}
relationship {
    map* kind1;
    entity* id1;
    map* kind2;
    entity* id2;
}
map {
    char* key;
    char* value;
}

entity.input = "hwent.db";
relationship.input = "hwrel.db";

entity.kind.input = relationship.kind1.input = relationship.kind2.input ={
    ";_ERROR_", "io;ioboard", "ct;controller", "dk;disk", "cd;cdrom", "tp;tape", "jb;jukebox", "pt;partition", "lv;volume", "fs;filesystem", "sw;swap"
}

schema = entity;
edge = { relationship.id1, relationship.id2 };

int select() {
        return name=="*" && kind.value=="partition";
}

void begin() {
    int isselected[];
    char* ispulled[];
}

int closure() {
    if (relationship.kind2.value == "*" &&
            cql.iteration <= 10000 &&
            relationship.id1.name != "") {
        printf("%s;%s;%s;%s\n", relationship.kind1, relationship.id1, relationship.kind2, relationship.id2);
        return 1;
    } else
        return 0;
}

void action() {
    if (cql.iteration <= 10000 && !isselected[id]) {
        isselected[id] = 1;
        ispulled[id] = "";
        printf("%s;%s;%s;%s;%s;%s;%s\n", id, name, kind, n1, n2, attr, "y");
    }
}

void end() {
    char* s;
    for (ispulled[s]) 
        if (ispulled[s] != "")
            printf("%s", ispulled[s]);
}
