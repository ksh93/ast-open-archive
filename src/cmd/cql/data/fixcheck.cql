fix {
	char*:format="2S"	a;
	char*:format="2S"	b;
	char*:format="2S"	c;
	char*:format="2S"	d;
	char*:format="1S"	z;
}

fix.terminator = "\n";

void action() {
	printf("%s:%s:%s:%s\n", a, b, c, d);
}
