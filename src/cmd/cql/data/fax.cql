Facility {
	char* PlanVersion;
	char* RunID;
	register char* ClliA;
	register char* ClliZ;
	register char* FaxType;
	char* FaxNum;
	extern register char* Clfi;
	char* Direction;
	char* FaxOrder;
	char* cutCode;
	int   FaxVersion;
	char* SvcDesc;
	int StartDate;
	int EndDate;
	char* Rin;
	char* Ucr;
	char* ApsIndex;
	char* ApsType;
	char* FacilityStatus;
	char* VendorCode;
}

Reference {
	Facility* f;
}

Facility.delimiter = "|";
Facility.input = "faxdat.db";

Reference.delimiter = "|";
Reference.input = "faxref.db";
