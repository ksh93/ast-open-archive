dat
{
	register key	name;
	char*		value;
};

key
{
	char*		first;
	char*		last;
};

key.delimiter = ":";

dat.input = "space.db";
dat.delimiter = ":";
