mem {
    char*	one;
    char*	two;
    map*	type;
}

map {
    char*	key;
    char*	value;
}

mem.input = "mem.db";

mem.type.input = {
    ";_ERROR_", "a;AAA", "b;BBB", "c;CCC"
}

void begin() {
	int	sum[];
	char*	s;
}

void action() {
	sum[mem.type.value]++;
}

void end() {
	for (sum[s])
		printf("%s %d\n", s, sum[s]);
}
