date {
    char* name;
    date_t date;
    elapsed_t elapsed;
}
date.delimiter = ";";
date.input = "date.db";
schema=date;
void select() {
	date.date > "1998-05-01" || date.elapsed > "1d";
}
