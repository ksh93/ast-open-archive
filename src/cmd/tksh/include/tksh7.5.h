#pragma prototyped

typedef struct TclEventSource {
    Tcl_EventSetupProc *setupProc;      /* This procedure is called by
                                         * Tcl_DoOneEvent to set up information
                                         * for the wait operation, such as
                                         * files to wait for or maximum
                                         * timeout. */
    Tcl_EventCheckProc *checkProc;      /* This procedure is called by
                                         * Tcl_DoOneEvent after its wait
                                         * operation to see what events
                                         * are ready and queue them. */
    ClientData clientData;              /* Arbitrary one-word argument to pass
                                         * to setupProc and checkProc. */
    struct TclEventSource *nextPtr;     /* Next in list of all event sources
                                         * defined for applicaton. */
} TclEventSource;

#define TclPlatformExit(status) exit(status)

#if _BLD_tcl && defined(__EXPORT__)
#define extern	__EXPORT__
#endif

EXTERN Tcl_Channel      TclGetDefaultStdChannel(int type);
EXTERN Tcl_Channel	TclFindFileChannel(Tcl_File inFil, Tcl_File outFile,
			    int *fileUsedPtr);
EXTERN int              TclGetLoadedPackages(Tcl_Interp *interp,
			    char *targetName);
EXTERN int              TclInterpInit(Tcl_Interp *interp);
EXTERN void             TclPlatformInit(Tcl_Interp *interp);
EXTERN void             TclFreePackageInfo(Interp *iPtr);
EXTERN int              TclChdir(Tcl_Interp *interp, char *dirName);
EXTERN char *           TclGetCwd(Tcl_Interp *interp);
EXTERN int              TclPreventAliasLoop(Tcl_Interp *interp,
                            Tcl_Interp *cmdInterp, char *cmdName,
                            Tcl_CmdProc *proc, ClientData clientData);
EXTERN int              TclFindElement(Tcl_Interp *interp,
                            char *list, char **elementPtr, char **nextPtr,
                            int *sizePtr, int *bracePtr);
EXTERN void             TclCopyAndCollapse(int count, char *src, char *dst);
EXTERN int              TclGetListIndex(Tcl_Interp *interp,
                            char *string, int *indexPtr);
EXTERN unsigned long    TclGetSeconds(void);
EXTERN void             TclGetTime(Tcl_Time *time);
EXTERN int              TclGetTimeZ(unsigned long time);
EXTERN int              TclGetDate(char *p, unsigned long now, long zone,
                            unsigned long *timePtr);
EXTERN int              TclGuessPackageName(char *fileName,
                            Tcl_DString *bufPtr);
EXTERN int              TclLoadFile(Tcl_Interp *interp,
                            char *fileName, char *sym1, char *sym2,
                            Tcl_PackageInitProc **proc1Ptr,
                            Tcl_PackageInitProc **proc2Ptr);
EXTERN char *		TclGetExtension(char *name);
EXTERN int              TclGetOpenMode(Tcl_Interp *interp,
                            char *string, int *seekFlagPtr);
EXTERN unsigned long    TclGetClicks(void);
EXTERN int              TclIdlePending(void);
EXTERN int              TclServiceIdle(void);
EXTERN int              TclWaitForFile(Tcl_File file, int mask, int timeout);
EXTERN int              TclParseBraces(Tcl_Interp *interp,
                            char *string, char **termPtr, ParseValue *pvPtr);
EXTERN int              TclParseNestedCmd(Tcl_Interp *interp,
                            char *string, int flags, char **termPtr,
                            ParseValue *pvPtr);
EXTERN int              TclParseQuotes(Tcl_Interp *interp,
                            char *string, int termChar, int flags,
                            char **termPtr, ParseValue *pvPtr);
EXTERN int              TclParseWords(Tcl_Interp *interp,
                            char *string, int flags, int maxWords,
                            char **termPtr, int *argcPtr, char **argv,
                            ParseValue *pvPtr);



extern char *		TclPrecTraceProc(ClientData clientData,
				Tcl_Interp *interp, char *name1, char *name2,
				int flags);

#undef	extern

/*
 *----------------------------------------------------------------
 * Variables shared among Tcl modules but not used by the outside
 * world:
 *----------------------------------------------------------------
 */

extern int		tclInInterpreterDeletion;
extern char *           tclExecutableName;


void TkshCreateInterp(Tcl_Interp *interp, void *data);

typedef struct Trace {
    int level;                  /* Only trace commands at nesting level
                                 * less than or equal to this. */
    Tcl_CmdTraceProc *proc;     /* Procedure to call to trace command. */
    ClientData clientData;      /* Arbitrary value to pass to proc. */
    struct Trace *nextPtr;      /* Next in list of traces for this interp. */
} Trace;



typedef struct AssocData {
    Tcl_InterpDeleteProc *proc; /* Proc to call when deleting. */
    ClientData clientData;      /* Value to pass to proc. */
} AssocData;

