tst {
	char*	name;
	char*	list[];
	char*	comment;
}

tst.delimiter = ':';
tst.list.delimiter = '|';

void begin() {
	int	i;
}

void action() {
	for (tst.list[i])
		printf("%s %s\n", tst.name, tst.list[i]);
}
