#pragma prototyped
#ifndef __TKSH_H_
#define __TKSH_H_

#include <shell.h>
#include <stdio.h>
/* #include <malloc.h> */
#if _BLD_tcl && defined(__EXPORT__)
#define extern	__EXPORT__
#endif
#include <tcl.h>
#undef	extern
#include <ctype.h>
#if 0
#undef free
#define free(x)	(1)
#endif
#undef NTcl_FreeResult
#define NTcl_FreeResult(interp) do { sfprintf(sfstderr, "Free of %s at %s %d (%x)\n", (interp)->result, __FILE__, __LINE__, (interp)->freeProc); (interp)->freeProc = 0; } while(0)

#include "shcompat.h"

#define DEFAULT_PD_PREC 6
#define DEFAULT_PD_FORMAT "%g"

/*
 * The macro below is used to modify a "char" value (e.g. by casting
 * it to an unsigned character) so that it can be used safely with
 * macros such as isspace.
 */

#define UCHAR(c) ((unsigned char) (c))

/*
 * Given a size or address, the macro below "aligns" it to the machine's
 * memory unit size (e.g. an 8-byte boundary) so that anything can be
 * placed at the aligned address without fear of an alignment error.
 */

#define TCL_ALIGN(x) ((x + 7) & ~7)

/*
 * Patchlevel
 */

#if TCL_MINOR_VERSION == 3
#define TCL_PATCH_LEVEL 106
#else
#ifndef TCL_PATCH_LEVEL
#define TCL_PATCH_LEVEL "7.4"	/* The only ver that wouldn't already set it */
#endif
#endif

/*
 * The stucture below defines a deletion callback, which is
 * a procedure to invoke just before an interpreter is deleted.
 */

typedef struct DeleteCallback {
    Tcl_InterpDeleteProc *proc;	/* Procedure to call. */
    ClientData clientData;	/* Value to pass to procedure. */
    struct DeleteCallback *nextPtr;
				/* Next in list of callbacks for this
				 * interpreter (or NULL for end of list). */
} DeleteCallback;


typedef struct Interp {

    /*
     * Note:  the first three fields must match exactly the fields in
     * a Tcl_Interp struct (see tcl.h).  If you change one, be sure to
     * change the other.
     */

    char *result;               /* Points to result returned by last
                                 * command. */
    Tcl_FreeProc *freeProc;     /* Zero means result is statically allocated.
                                 * If non-zero, gives address of procedure
                                 * to invoke to free the result.  Must be
                                 * freed by Tcl_Eval before executing next
                                 * command. */
    int errorLine;              /* When TCL_ERROR is returned, this gives
                                 * the line number within the command where
                                 * the error occurred (1 means first line). */
    Tcl_HashTable xxxcommandTable; /* Contains all of the commands currently
                                 * registered in this interpreter.  Indexed
                                 * by strings; values have type (Command *). */
    Tcl_HashTable mathFuncTable;/* Contains all of the math functions currently
                                 * defined for the interpreter.  Indexed by
                                 * strings (function names);  values have
                                 * type (MathFunc *). */

    /*
     * Information related to procedures and variables.  See tclProc.c
     * and tclvar.c for usage.
     */

    Tcl_HashTable xxxglobalTable;  /* Contains all global variables for
                                 * interpreter. */
#ifndef NO_TCL_INTERP
    int numLevels;              /* Keeps track of how many nested calls to
                                 * Tcl_Eval are in progress for this
                                 * interpreter.  It's used to delay deletion
                                 * of the table until all Tcl_Eval invocations
                                 * are completed. */
    int maxNestingDepth;        /* If numLevels exceeds this value then Tcl
                                 * assumes that infinite recursion has
                                 * occurred and it generates an error. */
#endif
    void * /* CallFrame */ xxxframePtr;        /* Points to top-most in stack of all nested
                                 * procedure invocations.  NULL means there
                                 * are no active procedures. */
    void * /* CallFrame */ xxxvarFramePtr;     /* Points to the call frame whose variables
                                 * are currently in use (same as framePtr
                                 * unless an "uplevel" command is being
                                 * executed).  NULL means no procedure is
                                 * active or "uplevel 0" is being exec'ed. */
    void * /* ActiveVarTrace */ xxxactiveTracePtr;
                                /* First in list of active traces for interp,
                                 * or NULL if no active traces. */

    int returnCode;             /* Completion code to return if current
                                 * procedure exits with a TCL_RETURN code. */
    char *errorInfo;            /* Value to store in errorInfo if returnCode
                                 * is TCL_ERROR.  Malloc'ed, may be NULL */
    char *errorCode;            /* Value to store in errorCode if returnCode
                                 * is TCL_ERROR.  Malloc'ed, may be NULL */

    /*
     * Information related to history:
     */

#ifdef TCL_CODE
    int xxxnumEvents;              /* Number of previously-executed commands
                                 * to retain. */
#else
    int interpType;
#endif
    void * /*HistoryEvent */ xxxevents;       /* Array containing numEvents entries
                                 * (dynamically allocated). */
    int xxxcurEvent;               /* Index into events of place where current
                                 * (or most recent) command is recorded. */
    int xxxcurEventNum;            /* Event number associated with the slot
                                 * given by curEvent. */
    void * /*HistoryRev */ xxxrevPtr;         /* First in list of pending revisions. */
    char *xxxhistoryFirst;         /* First char. of current command executed
                                 * from history module or NULL if none. */
    int xxxrevDisables;            /* 0 means history revision OK;  > 0 gives
                                 * a count of number of times revision has
                                 * been disabled. */
    char *xxxevalFirst;            /* If TCL_RECORD_BOUNDS flag set, Tcl_Eval
                                 * sets this field to point to the first
                                 * char. of text from which the current
                                 * command came.  Otherwise Tcl_Eval sets
                                 * this to NULL. */
    char *xxxevalLast;             /* Similar to evalFirst, except points to
                                 * last character of current command. */


    /*
     * Information used by Tcl_AppendResult to keep track of partial
     * results.  See Tcl_AppendResult code for details.
     */

    char *appendResult;         /* Storage space for results generated
                                 * by Tcl_AppendResult.  Malloc-ed.  NULL
                                 * means not yet allocated. */
    int appendAvl;              /* Total amount of space available at
                                 * partialResult. */
    int appendUsed;             /* Number of non-null bytes currently
                                 * stored at partialResult. */

    /*
     * A cache of compiled regular expressions.  See TclCompileRegexp
     * in tclUtil.c for details.
     */

#define NUM_REGEXPS 5
    char *xxxpatterns[NUM_REGEXPS];/* Strings corresponding to compiled
                                 * regular expression patterns.  NULL
                                 * means that this slot isn't used.
                                 * Malloc-ed. */
    int xxxpatLengths[NUM_REGEXPS];/* Number of non-null characters in
                                 * corresponding entry in patterns.
                                 * -1 means entry isn't used. */
    void * /* regexp */ xxxregexps[NUM_REGEXPS];
                                /* Compiled forms of above strings.  Also
                                 * malloc-ed, or NULL if not in use yet. */


    /*
     * Information used by Tcl_PrintDouble:
     */

    char pdFormat[10];          /* Format string used by Tcl_PrintDouble. */
    int pdPrec;                 /* Current precision (used to restore the
                                 * the tcl_precision variable after a bogus
                                 * value has been put into it). */

#ifndef NO_TCL_INTERP

    /*
     * Miscellaneous information:
     */

    int cmdCount;               /* Total number of times a command procedure
                                 * has been called for this interpreter. */
    int noEval;                 /* Non-zero means no commands should actually
                                 * be executed:  just parse only.  Used in
                                 * expressions when the result is already
                                 * determined. */
    int evalFlags;              /* Flags to control next call to Tcl_Eval.
                                 * Normally zero, but may be set before
                                 * calling Tcl_Eval to an OR'ed combination
                                 * of TCL_BRACKET_TERM and TCL_RECORD_BOUNDS. */
    char *termPtr;              /* Character just after the last one in
                                 * a command.  Set by Tcl_Eval before
                                 * returning. */
    char *scriptFile;           /* NULL means there is no nested source
                                 * command active;  otherwise this points to
                                 * the name of the file being sourced (it's
                                 * not malloc-ed:  it points to an argument
                                 * to Tcl_EvalFile. */
    int flags;                  /* Various flag bits.  See below. */
    void * /* Trace */ tracePtr;            /* List of traces for this interpreter. */
    DeleteCallback* deleteCallbackPtr;
                                /* First in list of callbacks to invoke when
                                 * interpreter is deleted. */

#endif
    /*
     * Information about packages.  Used only in tclPkg.c.
     */

    Tcl_HashTable packageTable; /* Describes all of the packages loaded
                                 * in or available to this interpreter.
                                 * Keys are package names, values are
                                 * (Package *) pointers. */
    char *packageUnknown;       /* Command to invoke during "package
                                 * require" commands for packages that
                                 * aren't described in packageTable.
                                 * Malloc'ed, may be NULL. */

    Tcl_HashTable *assocData;   /* Hash table for associating data with
                                 * this interpreter. Cleaned up when
                                 * this interpreter is deleted. */

    char resultSpace[TCL_RESULT_SIZE+1];
                                /* Static space for storing small results. */
} Interp;


/*
 * Flag bits for Interp structures:
 *
 * DELETED:             Non-zero means the interpreter has been deleted:
 *                      don't process any more commands for it, and destroy
 *                      the structure as soon as all nested invocations of
 *                      Tcl_Eval are done.
 * ERR_IN_PROGRESS:     Non-zero means an error unwind is already in progress.
 *                      Zero means a command proc has been invoked since last
 *                      error occured.
 * ERR_ALREADY_LOGGED:  Non-zero means information has already been logged
 *                      in $errorInfo for the current Tcl_Eval instance,
 *                      so Tcl_Eval needn't log it (used to implement the
 *                      "error message log" command).
 * ERROR_CODE_SET:      Non-zero means that Tcl_SetErrorCode has been
 *                      called to record information for the current
 *                      error.  Zero means Tcl_Eval must clear the
 *                      errorCode variable if an error is returned.
 */

#define DELETED                 1
#define ERR_IN_PROGRESS         2
#define ERR_ALREADY_LOGGED	4
#define ERROR_CODE_SET          8


#ifndef NO_TCL_INTERP		/* Parsing stuff */

typedef struct ParseValue {
    char *buffer;		/* Address of first character in
				 * output buffer. */
    char *next;			/* Place to store next character in
				 * output buffer. */
    char *end;			/* Address of the last usable character
				 * in the buffer. */
    void (*expandProc)(struct ParseValue *pvPtr, int needed);
				/* Procedure to call when space runs out;
				 * it will make more space. */
    ClientData clientData;	/* Arbitrary information for use of
				 * expandProc. */
} ParseValue;

/*
 * A table used to classify input characters to assist in parsing
 * Tcl commands.  The table should be indexed with a signed character
 * using the CHAR_TYPE macro.  The character may have a negative
 * value.
 */

extern char tclTypeTable[];
#define CHAR_TYPE(c) (tclTypeTable+128)[c]

#if _BLD_tcl && defined(__EXPORT__)
#define extern	__EXPORT__
#endif

/*
 * Possible values returned by CHAR_TYPE:
 *
 * TCL_NORMAL -		All characters that don't have special significance
 *			to the Tcl language.
 * TCL_SPACE -		Character is space, tab, or return.
 * TCL_COMMAND_END -	Character is newline or null or semicolon or
 *			close-bracket.
 * TCL_QUOTE -		Character is a double-quote.
 * TCL_OPEN_BRACKET -	Character is a "[".
 * TCL_OPEN_BRACE -	Character is a "{".
 * TCL_CLOSE_BRACE -	Character is a "}".
 * TCL_BACKSLASH -	Character is a "\".
 * TCL_DOLLAR -		Character is a "$".
 */

#define TCL_NORMAL		0
#define TCL_SPACE		1
#define TCL_COMMAND_END		2
#define TCL_QUOTE		3
#define TCL_OPEN_BRACKET	4
#define TCL_OPEN_BRACE		5
#define TCL_CLOSE_BRACE		6
#define TCL_BACKSLASH		7
#define TCL_DOLLAR		8

#ifndef TCL_BRACKET_TERM
#define TCL_BRACKET_TERM	1	/* in tcl7.3 header, not 7.4 */
#endif
#define TCL_ALLOW_EXCEPTIONS	4	/* in tcl7.4 only */


extern void		TclExpandParseValue(ParseValue *pvPtr, int needed);

extern int		TclNeedSpace(char *start, char *end);

extern void		TclSetupEnv(Tcl_Interp *interp);

extern void		TclWinFlushEvents(void);

extern int		Tcl_NumEventsFound(void);

extern int		Tksh_CreatePipeline(Tcl_Interp*, int, char**, Tcl_DString*);

#endif


#ifndef NO_TCL_INTERP		/* Tcl backward compat. stuff */

#define INTERP_KSH		0
#define INTERP_TCL		1
#define INTERP_MASK		3
#define INTERP_CURRENT		2

/*
 *----------------------------------------------------------------
 * Data structures related to procedures.   These are used primarily
 * in tclProc.c
 *----------------------------------------------------------------
 */

/*
 * The structure below defines an argument to a procedure, which
 * consists of a name and an (optional) default value.
 */

typedef struct Arg {
    struct Arg *nextPtr;	/* Next argument for this procedure,
				 * or NULL if this is the last argument. */
    char *defValue;		/* Pointer to arg's default value, or NULL
				 * if no default value. */
    char name[4];		/* Name of argument starts here.  The name
				 * is followed by space for the default,
				 * if there is one.  The actual size of this
				 * field will be as large as necessary to
				 * hold both name and default value.  THIS
				 * MUST BE THE LAST FIELD IN THE STRUCTURE!! */
} Arg;

/*
 * The structure below defines a command procedure, which consists of
 * a collection of Tcl commands plus information about arguments and
 * variables.
 */

typedef struct Proc {
    struct Interp *iPtr;	/* Interpreter for which this command
				 * is defined. */
    int refCount;		/* Reference count:  1 if still present
				 * in command table plus 1 for each call
				 * to the procedure that is currently
				 * active.  This structure can be freed
				 * when refCount becomes zero. */
    char *command;		/* Command that constitutes the body of
				 * the procedure (dynamically allocated). */
    Arg *argPtr;		/* Pointer to first of procedure's formal
				 * arguments, or NULL if none. */
} Proc;

extern Proc *		TclIsProc(Tcl_CmdInfo *cmdPtr);
extern Proc *		TclFindProc(Interp *iPtr, char *procName);
extern int		TclUpdateReturnInfo(Interp *iPtr);

extern char *		TkshMapName(char *name);
extern char *		TkshMapKeyword(char *name);
extern char *		TkshLibDir(void);

extern int		TkshSetListMode(int mode);

#endif



/* Tksh Definitions */


#define TKSH_TRACE_RUNNING	0x400	/* Set when trace is running */
#define COMMAND_ACTIVE		4	/* Used in commandType field */
#define NV_FUNC			(NV_NOADD | NV_NOASSIGN)

#ifndef LIB_DIR_ENV
#define LIB_DIR_ENV	"TKSH_LIBRARY"
#endif

#ifndef LIB_DIR
#define LIB_DIR		"lib/tksh7.5"
#endif

#define SPLIT_CMD_NAME "_tksh_split_list"

typedef struct TkshArrayInfo
{
	ClientData clientData;
} TkshArrayInfo;

typedef struct TkshCommandData
{
	Tcl_Interp *interp;
	Tcl_CmdInfo info;
	char commandType;
} TkshCommandData;


#define Tksh_MapReturn(i)	((((i) >= 0) && ((i) <= TCL_CONTINUE)) ? (i) \
				: TCL_ERROR)
#define Tksh_ReturnVal()	(Tksh_MapReturn(sh.exitval))
#define Tksh_OkOrErr()		((sh.exitval == 0) ? TCL_OK : TCL_ERROR)
#define Tksh_InterpString(i)	(((i)==INTERP_TCL)? "tcl": (((i)==INTERP_KSH) \
					? "ksh" : "either"))

#define Tksh_BeginBlock(interp, kind)	do { int oldInterp = ((Interp *)interp)->interpType; ((Interp *)interp)->interpType = kind
#define Tksh_EndBlock(interp)		((Interp *)interp)->interpType = oldInterp; } while(0)

extern void		TkshDeleteSearches(TkshArrayInfo *);
extern TkshArrayInfo *	TkshArrayData(Namval_t *namval);
extern void		TkshSubShell();
extern void		TkshTracesOff(void);

extern int		b_tclinit(int argc, char *argv[], void *);
extern int		b_tkinit(int argc, char *argv[], void *);

extern int		tksh_waitevent(int, long, int);
extern int		nv_getlevel(void);
extern int		Tcl_TclEval(Tcl_Interp *interp, char *cmd);
extern int		Tcl_TclEvalFile(Tcl_Interp *interp, char *fileName);

extern int		Tksh_Eval(Tcl_Interp *interp, char *cmd, int flag);
extern int		Tksh_SetCommandType(Tcl_Interp *, char *, int tp);
extern int		Tksh_Init(Tcl_Interp *interp);
char *			Tksh_ConvertList(Tcl_Interp *interp, char *lst, int to);

#undef	extern

extern int		b_print(int argc, char *argv[], void *);

#include "debug.h"
#include <tksh7.5.h>

#endif /* __TKSH_H_ */
