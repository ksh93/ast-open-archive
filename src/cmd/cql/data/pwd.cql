passwd {
	shadow*	name;
	char*	passwd;
	int	uid;
	group*	gid;
	info	info;
	char*	home;
	char*	shell;
}
shadow {
	char*	name;
	char*	passwd;
	int	lastchanged;
	int	minimum;
	int	maximum;
	int	warn;
	int	inactive;
	date_t	expire;
}
group {
	char*		name;
	char*		passwd;
	extern char*	gid;
	passwd*		members[];
}
info {
	char*		name;
	char*		address;
	char*		office;
	char*		home;
}

passwd.delimiter =	"\n";
passwd.comment =	"password file";
passwd.input =		"pwd.db";
shadow.delimiter =	":";
shadow.comment =	"secret location of passwords";
shadow.input =		"shadow.db";
group.delimiter =	"\n";
group.comment =		"group file";
group.input =		"grp.db";
glist.delimiter =	",";
glist.comment =		"group member list";
info.delimiter =	",";
info.comment =		"not enforced";
