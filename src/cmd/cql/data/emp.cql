Emp {
    char* name;
    float rate;
    int   hours;
}

Emp.delimiter = "\t";
Emp.comment = "awkbook p1";
Emp.input = "emp.db";

schema=Emp;

void begin() {
	float rt[]; int hr[];
}
void select() {
	Emp.rate >= 9.00 && Emp.hours > 40;
}
void action() {
    rt[Emp.name] = Emp.rate;
    hr[Emp.name] = Emp.hours;
}
void end()
{
    char* nm;
    printf("highly paid hard workers\n\tName\tRate\tHours\n");
    for (rt[nm])
        printf("\t%s\t%.2f\t%d\n", nm, rt[nm], hr[nm]);
}
